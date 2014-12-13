#include <conio.h>
#include <stdio.h>
#include <windows.h>
#include "../inc/pub/watypes.h"
#include "../inc/pub/waengine.h"
#include "../inc/pub/wasnapshots.h"
#include "../inc/pub/wadebug.h"
#include "../inc/pub/wahash.h"

//#define MALLOCS_DEBUG_HARD
//#define MALLOCS_DEBUG_SOFT
#define INITIALIZE_DEBUG
#define USE_HASHES

typedef struct _DebugSnapshot
{
  WORLD_SNAPSHOT_EVALUATION Eval;
  unsigned int level;  
  unsigned int freed;
  unsigned int id;
}DebugSnapshot,*pDebugSnapshot;

typedef struct _ClientPrivateData
{
  WAHASH_MANAGER_HANDLE hHashMan;

}ClientPrivateData,*pClientPrivateData;

unsigned int lastid=2;
MEMSZ totalmemusage=0;
unsigned int lastmemcountid=1;
unsigned int memcountids[2000000]={0};

MEMBLOCK myalloc(MEMSZ sz)
{ 
#ifdef MALLOCS_DEBUG_HARD

  unsigned char * ptr=malloc((size_t)(sz+8));

  if(ptr)
  {
    totalmemusage+=sz;
    printf("malloc:%d bytes, mem usage:%d bytes\n",sz,totalmemusage);
    *((MEMSZ *)ptr)=sz;
    ((char*)ptr)+=4;
    *((int*)ptr)=lastmemcountid;
    {
      unsigned int i=0;
      while(memcountids[i])
        i++;
      memcountids[i]=lastmemcountid;
    }
    lastmemcountid++;
    ((char*)ptr)+=4;
  }
  return ptr;

#else

#ifdef MALLOCS_DEBUG_SOFT

  unsigned char * ptr=malloc((size_t)(sz+8));
  if(ptr)
  {
    totalmemusage+=sz;
    printf("malloc:%d bytes, mem usage:%d bytes\n",sz,totalmemusage);
    *((MEMSZ *)ptr)=sz;
    ((char*)ptr)+=4;
  }
  return ptr;

#else

  return malloc(sz);

#endif

#endif
}

NOTHING  myfree(MEMBLOCK mb)
{
#ifdef MALLOCS_DEBUG_HARD

  unsigned int i=0;
  unsigned int tempid;

  ((char*)mb)-=4;
  
  tempid=*((int*)mb);

  ((char*)mb)-=4;

  for(i=0;i<2000000;i++)
  {
    if(memcountids[i]==tempid)
    {
      memcountids[i]=0;
      break;
    }
  }

  if(i==2000000)
    __asm int 3;

  totalmemusage-=*((unsigned int*)mb);
  printf("free:%d bytes, mem usage:%d bytes\n",*((unsigned int*)mb),totalmemusage);
  free(mb);

#else

#ifdef MALLOCS_DEBUG_SOFT

  ((char*)mb)-=4;
  totalmemusage-=*((unsigned int*)mb);
  printf("free:%d bytes, mem usage:%d bytes\n",*((unsigned int*)mb),totalmemusage);
  free(mb);  

#else

  free(mb);

#endif

#endif
}

void CheckMemUsage()
{
#ifdef MALLOCS_DEBUG_HARD

  unsigned int i=0;

  for(i=0;i<2000000;i++)
  {
    if(memcountids[i])
    {
      __asm nop;
    }
  }

#endif
}

WORLD_SNAPSHOT DupSnapshot(PWORLD_SNAPSHOTS_MANAGER a,
                           WORLD_SNAPSHOT b,
                           WORLD_SNAPSHOT_CLIENT_PARAM cp,
                           RESERVED c)
{
  return NULL;
}

WORLD_SNAPSHOT_COMPARISON_RESULT CompareSnapshots(PWORLD_SNAPSHOTS_MANAGER a,
                                                  WORLD_SNAPSHOT b,
                                                  WORLD_SNAPSHOT c,
                                                  WORLD_SNAPSHOT_CLIENT_PARAM cp,
                                                  RESERVED d)
{
  return 0;
}

WORLD_SNAPSHOT_EVALUATION EvaluateSnapshot(PWORLD_SNAPSHOTS_MANAGER a,
                                           WORLD_SNAPSHOT b,
                                           WORLD_SNAPSHOT_CLIENT_PARAM cp,
                                           RESERVED c)
{
  return ((pDebugSnapshot)b)->Eval;
}

UL32 GenerateSnapshotEvolutions(PWORLD_SNAPSHOTS_MANAGER a,
                               WORLD_SNAPSHOT b,
                               PWORLD_SNAPSHOTS_GROUP* c,
                               WORLD_SNAPSHOT_CLIENT_PARAM cp,
                               RESERVED d)
{
  unsigned int i=0;
  unsigned int nEvols=0;
  static unsigned int SetBest=1;
#ifdef USE_HASHES
  WAHASH hash;
#endif
  pClientPrivateData pPrivData=(pClientPrivateData)cp;
  
  PWORLD_SNAPSHOTS_GROUP group;

  if(((pDebugSnapshot)b)->level>6)
    nEvols=0;
  else if(((pDebugSnapshot)b)->level>4)
    nEvols=2;
  else if(((pDebugSnapshot)b)->level>1)
    nEvols=(rand()&0x00000007);
  else
    nEvols=2;

  if(!nEvols)
  {
    *c=WANULL;
    return 0;
  }

  group=(PWORLD_SNAPSHOTS_GROUP)myalloc(4+sizeof(WORLD_SNAPSHOTS_GROUP)+(sizeof(WORLD_SNAPSHOT)*(nEvols-1)));
  *((int*)group)=1;
  ((char*)group)+=4;
  group->nSnapshots=nEvols;
  
  for(i=0;i<nEvols;i++)
  {
    group->Snapshots[i]=myalloc(sizeof(DebugSnapshot));

    if(((((pDebugSnapshot)b)->level==9)&&rand()%2)&&SetBest)
    {
      SetBest=0;
      ((pDebugSnapshot)group->Snapshots[i])->Eval=1000000000;
    }
    else
    {
      ((pDebugSnapshot)group->Snapshots[i])->Eval=rand();
    }
     
    if(!((pDebugSnapshot)group->Snapshots[i])->Eval)
      ((pDebugSnapshot)group->Snapshots[i])->Eval=1;

#ifdef USE_HASHES    

    *((WORLD_SNAPSHOT_EVALUATION*)hash)=((pDebugSnapshot)group->Snapshots[i])->Eval;
    
    if(IsWaHashAdded(pPrivData->hHashMan,hash))
    {
      myfree((MEMBLOCK)group->Snapshots[i]);
      nEvols--;
      group->nSnapshots--;
      i--;

      if(!nEvols)
      {
        ((char*)group)-=4;
        {myfree((MEMBLOCK)group);group=WANULL;}
        break;
      }

      continue;
    }

    AddWaHash(pPrivData->hHashMan,hash);
#endif

    ((pDebugSnapshot)group->Snapshots[i])->level=((pDebugSnapshot)b)->level+1;
    ((pDebugSnapshot)group->Snapshots[i])->freed=0;
    ((pDebugSnapshot)group->Snapshots[i])->id=lastid;
    lastid++;
  }

  *c=group;
  return nEvols;
}

NOTHING ReturnClientObject(PWORLD_SNAPSHOTS_MANAGER a,
                           UNKOBJ b,
                           WORLD_SNAPSHOT_CLIENT_PARAM cp,
                           RESERVED c)
{
  unsigned int i=0;

  if(*((int*)(((char*)b)-4))==1)
  {
    //group
    PWORLD_SNAPSHOTS_GROUP group=(PWORLD_SNAPSHOTS_GROUP)b;
  
    if(((pDebugSnapshot)group->Snapshots[0])->freed)
      __asm int 3;

    ((pDebugSnapshot)group->Snapshots[0])->freed=1;

    for(i=0;i<group->nSnapshots;i++)
      myfree((MEMBLOCK)(pDebugSnapshot)group->Snapshots[i]);
    
    myfree(((char*)b)-4);
  }
  else
  {
    __asm int 3;
  }
}

NOTHING GetSnapshotHash(PWORLD_SNAPSHOTS_MANAGER a,
                        WORLD_SNAPSHOT b,
                        WORLD_SNAPSHOT_HASH c,
                        WORLD_SNAPSHOT_CLIENT_PARAM cp,
                        RESERVED d)
{
}

char g_DebugStrBuffer[1000];

char* GetDebugShowString(WORLD_SNAPSHOT Snapshot)
{
  return itoa(((pDebugSnapshot)Snapshot)->id,g_DebugStrBuffer,10);
}

void main()
{ 
  UL32 TickStart,TickEnd;
  ClientPrivateData PrivData;
  DebugSnapshot StartSnapshot;
  WORLD_ANALYSIS_JOB_STARTUP_INFO StartupInfo;
  WORLD_ANALYSIS_ENGINE_HANDLE hEngine;
  WORLD_ANALYSIS_JOB_HANDLE hJob;
  WORLD_ANALYSIS_INITIALIZATION_REQUIREMENTS * WAReq;
  DEBUG_CONFIG DbgCfg={NULL,0,GetDebugShowString};
  
#ifdef INITIALIZE_DEBUG
  DebugInit(&DbgCfg);
#endif
  
  TickStart=GetTickCount();

  CREATE_WORLD_ANALYSIS_INITIALIZATION_REQUERIMENTS(WAReq,
                                                    NULL,
                                                    DupSnapshot,
                                                    CompareSnapshots,
                                                    EvaluateSnapshot,
                                                    GenerateSnapshotEvolutions,
                                                    ReturnClientObject,
                                                    GetSnapshotHash,
                                                    &PrivData,
                                                    myalloc,
                                                    myfree);

  hEngine=CreateWaEngine(WAReq);

  PrivData.hHashMan = CreateWaHashManager(hEngine);

  StartupInfo.Type=WORLD_ANALYSIS_JOB_DEPTH_FIRST;
  StartupInfo.StartSnapshot=&StartSnapshot;
  StartSnapshot.Eval=0;
  StartSnapshot.freed=0;
  StartSnapshot.level=0;
  StartSnapshot.id=1;

  hJob=CreateWaAnalysisJob(hEngine,&StartupInfo);

  srand(GetTickCount());

  RunWaAnalysisJob(hJob);

  //test
  ShowJob(hJob);

  DestroyWaAnalysisJob(hJob);

  DestroyWaHashManager(PrivData.hHashMan);

  DestroyWaEngine(hEngine);

  DESTROY_WORLD_ANALYSIS_INITIALIZATION_REQUERIMENTS(WAReq);

  TickEnd=GetTickCount();

#ifdef INITIALIZE_DEBUG  
  DebugEnd();
#endif

  CheckMemUsage();

  printf("ticks=%d",TickEnd-TickStart);

  while(!_kbhit());
}

/*
  WAHASH hashes[10000];
  {
    WAHASH_MANAGER_HANDLE hHashMan;
        
    UL32 k;
    hHashMan = CreateWaHashManager(hEngine);

    for(k=0;k<100000;k++)
    {
      hashes[k][0]=rand();hashes[k][1]=rand();hashes[k][2]=rand();hashes[k][3]=rand();
      hashes[k][4]=rand();hashes[k][5]=rand();hashes[k][6]=rand();hashes[k][7]=rand();
      hashes[k][8]=rand();hashes[k][9]=rand();hashes[k][10]=rand();hashes[k][11]=rand();
      hashes[k][12]=rand();hashes[k][13]=rand();hashes[k][14]=rand();hashes[k][15]=rand();
      hashes[k][16]=rand();hashes[k][17]=rand();hashes[k][18]=rand();hashes[k][19]=rand();
      AddWaHash(hHashMan,hashes[k]);
    }

    for(k=0;k<100000;k++)
    { 
      if(!IsWaHashAdded(hHashMan,hashes[k]))
        __asm int 3;
    }

    DestroyWaHashManager(hHashMan);
  }
*/