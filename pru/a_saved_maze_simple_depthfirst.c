#include <conio.h>
#include <stdio.h>
#include <windows.h>
#include "../inc/pub/watypes.h"
#include "../inc/pub/waengine.h"
#include "../inc/pub/wasnapshots.h"
#include "../inc/pub/wadebug.h"
#include "../inc/pub/wahash.h"
#include "../inc/pub/warandom.h"

UL32 g_UserQueryStop=0;
UL32 g_PathFound=0;

MEMSZ totalmemusage=0;
UL32 lastmemcountid=1;
UL32 memcountids[2000000]={0};

//#define MALLOCS_DEBUG_WITHOUT_PRINTS
//#define MALLOCS_DEBUG_HARD
//#define MALLOCS_DEBUG_SOFT
//#define INITIALIZE_DEBUG
//#define INFINITE_TEST


#define MAZE_MAX_WIDTH 400
#define MAZE_MAX_HEIGHT 400


#define MAZE_CORRIDOR   0
#define MAZE_WALL       1
#define MAZE_START      2
#define MAZE_END        3
#define MAZE_GOOD_PATH  4

#define MAX_RECURSIVE_LEVEL_IN_MAZE_GENERATOR 1000

typedef struct _MazePos
{
  unsigned int x;
  unsigned int y;
}MazePos,*pMazePos;

typedef struct _MazeSnapshot
{
  MazePos Man;
}MazeSnapshot,*pMazeSnapshot;

typedef struct _ClientPrivateData
{
  WAHASH_MANAGER_HANDLE hHashMan;
  WARAND_ENGINE_HANDLE hRandMan;
  MazePos End;
  MazePos Start;
  unsigned char maze[MAZE_MAX_WIDTH][MAZE_MAX_HEIGHT];

}ClientPrivateData,*pClientPrivateData;

unsigned int GetMazeCellsDistance(pMazePos pos1, pMazePos pos2)
{
  int xdis=0;int ydis=0;
  if(pos2->x>pos1->x)xdis=pos2->x-pos1->x;
  else xdis=pos1->x-pos2->x;
  if(pos2->y>pos1->y)ydis=pos2->y-pos1->y;
  else ydis=pos1->y-pos2->y;
  return xdis+ydis;
}


#define IS_VALID_NEW_CORRIDOR(Maze,Pos,PrePos) \
  (Pos->x>=0 && Pos->x<MAZE_MAX_WIDTH && Pos->y>=0 && Pos->y<MAZE_MAX_HEIGHT && \
   (Pos->x==(MAZE_MAX_WIDTH-1) || PrePos->x==(Pos->x+1) || Maze[Pos->x+1][Pos->y]!=MAZE_CORRIDOR) && \
   (Pos->x==0 || PrePos->x==(Pos->x-1) || Maze[Pos->x-1][Pos->y]!=MAZE_CORRIDOR) && \
   (Pos->y==(MAZE_MAX_HEIGHT-1) || PrePos->y==(Pos->y+1) || Maze[Pos->x][Pos->y+1]!=MAZE_CORRIDOR) && \
   (Pos->y==0 || PrePos->y==(Pos->y-1) || Maze[Pos->x][Pos->y-1]!=MAZE_CORRIDOR) && \
   (Maze[Pos->x][Pos->y]!=MAZE_START && Maze[Pos->x][Pos->y]!=MAZE_END))


#pragma pack(push,1)

typedef struct _INFOHEADER
{
   unsigned int infosize;               /* Header size in bytes      */
   int width,height;                    /* Width and height of image */
   unsigned short int planes;           /* Number of colour planes   */
   unsigned short int bits;             /* Bits per pixel            */
   unsigned int compression;            /* Compression type          */
   unsigned int imagesize;              /* Image size in bytes       */
   int xresolution,yresolution;         /* Pixels per meter          */
   unsigned int ncolours;               /* Number of colours         */
   unsigned int importantcolours;       /* Important colours         */
}INFOHEADER;

typedef struct _BMPHEADER
{
   //HEADER
   unsigned short int type;                 /* Magic identifier            */
   unsigned int size;                       /* File size in bytes          */
   unsigned short int reserved1, reserved2;
   unsigned int offset;                     /* Offset to image data, bytes */
   //INFOHEADER
   unsigned int infosize;               /* Header size in bytes      */
   int width,height;                    /* Width and height of image */
   unsigned short int planes;           /* Number of colour planes   */
   unsigned short int bits;             /* Bits per pixel            */
   unsigned int compression;            /* Compression type          */
   unsigned int imagesize;              /* Image size in bytes       */
   int xresolution,yresolution;         /* Pixels per meter          */
   unsigned int ncolours;               /* Number of colours         */
   unsigned int importantcolours;       /* Important colours         */
}BMPHEADER,
 *PBMPHEADER;
#pragma pack(pop)

void PrintMaze(pClientPrivateData Cpd,char * bmpname)
{
  BMPHEADER bmp;

  char blue[]={0xff,0xff,0};
  char green[]={0xff,0,0xff};
  char red[]={0,0xff,0xff};

  unsigned int i,j,k,z;
  FILE * f = fopen(bmpname,"w+");
  
  bmp.type=0x4d42;
  bmp.compression=0;
  bmp.offset=sizeof(BMPHEADER);
  bmp.infosize=sizeof(INFOHEADER);
  bmp.bits=24;
  bmp.height=(MAZE_MAX_HEIGHT+2)*10;
  bmp.width=(MAZE_MAX_WIDTH+2)*10;
  bmp.planes=1;
  bmp.xresolution=0;
  bmp.yresolution=0;
  bmp.reserved2=0;
  bmp.reserved1=0;
  bmp.ncolours=0;
  bmp.size=sizeof(BMPHEADER)+(3*10*10*(MAZE_MAX_HEIGHT+2)*(MAZE_MAX_WIDTH+2));
  bmp.imagesize=(3*10*10*(MAZE_MAX_HEIGHT+2)*(MAZE_MAX_WIDTH+2));

  fwrite(&bmp,sizeof(bmp),1,f);

  for(i=0;i<((MAZE_MAX_WIDTH+2)*100);i++)
    fwrite(&blue,3,1,f);

  for(j=0;j<MAZE_MAX_HEIGHT;j++)
  {
    for(z=0;z<10;z++)
    {
      for(k=0;k<10;k++)fwrite(&blue,3,1,f);
      for(i=0;i<MAZE_MAX_WIDTH;i++)
      {
        if(Cpd->maze[i][j]==MAZE_WALL)
          for(k=0;k<10;k++)fwrite(&blue,3,1,f);
        else if(Cpd->maze[i][j]==MAZE_START)
          for(k=0;k<10;k++)fwrite(&red,3,1,f);
        else if(Cpd->maze[i][j]==MAZE_END)
          for(k=0;k<10;k++)fwrite(&red,3,1,f);
        else if(Cpd->maze[i][j]==MAZE_CORRIDOR)
          for(k=0;k<10;k++)fwrite(&green,3,1,f);
        else if(Cpd->maze[i][j]==MAZE_GOOD_PATH)
          for(k=0;k<10;k++)fwrite(&red,3,1,f);
      }
      for(k=0;k<10;k++)fwrite(&blue,3,1,f);
    }
  }

  for(i=0;i<((MAZE_MAX_WIDTH+2)*100);i++)
    fwrite(&blue,3,1,f);
  
  fclose(f);
}



int DoMazeCorridors(ClientPrivateData * Cpd, pMazePos CurPos,unsigned int level)
{
  char Moves[]={0,0,0,0};
  unsigned int nMoves=0;
  unsigned int i=0;
  unsigned int k=100;
  MazePos NewPos;
  unsigned int retVal=0;
  
  level++;

  if(GetMazeCellsDistance(CurPos,&Cpd->End)==1)
    return 1;

  if(level==MAX_RECURSIVE_LEVEL_IN_MAZE_GENERATOR)
    return 0;

  nMoves=WaGenerateRand32(Cpd->hRandMan,1,4);
  
  while(nMoves)
  {
    Moves[WaGenerateRand32(Cpd->hRandMan,0,3)]=1;
    nMoves--;
  }

  for(i;i<4;i++)
    if(Moves[i])nMoves++;

  i=-1;

  while(i==-1 || !i)
  {
    i++;

    if(Moves[0])
    {
      NewPos.y = CurPos->y+1;
      NewPos.x = CurPos->x;
      if(IS_VALID_NEW_CORRIDOR(Cpd->maze,(&NewPos),CurPos))
      {
        i++;
        Cpd->maze[NewPos.x][NewPos.y]=MAZE_CORRIDOR;
        retVal+=DoMazeCorridors(Cpd,&NewPos,level);
      }
      else if(!i)
      {
        Moves[1]=1;
      }
    }

    if(Moves[1])
    {
      NewPos.y = CurPos->y;
      NewPos.x = CurPos->x+1;
      if(IS_VALID_NEW_CORRIDOR(Cpd->maze,(&NewPos),CurPos))
      {
        i++;
        Cpd->maze[NewPos.x][NewPos.y]=MAZE_CORRIDOR;
        retVal+=DoMazeCorridors(Cpd,&NewPos,level);
      }
      else if(!i)
      {
        Moves[2]=1;
      }    
    }

    if(Moves[2])
    {
      NewPos.y = CurPos->y-1;
      NewPos.x = CurPos->x;
      if(IS_VALID_NEW_CORRIDOR(Cpd->maze,(&NewPos),CurPos))
      {
        i++;
        Cpd->maze[NewPos.x][NewPos.y]=MAZE_CORRIDOR;
        retVal+=DoMazeCorridors(Cpd,&NewPos,level);
      }
      else if(!i)
      {
        Moves[3]=1;
      }
    }

    if(Moves[3])
    {
      NewPos.y = CurPos->y;
      NewPos.x = CurPos->x-1;
      if(IS_VALID_NEW_CORRIDOR(Cpd->maze,(&NewPos),CurPos))
      {
        i++;
        Cpd->maze[NewPos.x][NewPos.y]=MAZE_CORRIDOR;
        retVal+=DoMazeCorridors(Cpd,&NewPos,level);
      }
      else if(!i)
      {
        Moves[0]=1;
      }    
    }
  }

  if(level==1 && !retVal && k)
  {
    UL32 * MazeRangeWidth = malloc(sizeof(UL32)*MAZE_MAX_WIDTH);
    UL32 * MazeRangeHeight = malloc(sizeof(UL32)*MAZE_MAX_HEIGHT);

    while(level==1 && !retVal && k)
    {
      unsigned int i=0,j=0;

      k--;

      for(i=0;i<MAZE_MAX_WIDTH;i++)
        MazeRangeWidth[MAZE_MAX_WIDTH-i-1]=i;
      
      for(j=0;j<MAZE_MAX_HEIGHT;j++)
        MazeRangeHeight[MAZE_MAX_HEIGHT-j-1]=j;

      for(i=WaGenerateRand32(Cpd->hRandMan,0,MAZE_MAX_WIDTH-1);i<MAZE_MAX_WIDTH;i++)
      {
        if(i && WaGenerateRand32(Cpd->hRandMan,0,1))
        {
          MazeRangeWidth[i]=MazeRangeWidth[i-1];
          i=WaGenerateRand32(Cpd->hRandMan,0,MAZE_MAX_WIDTH-1);
        }
      }

      for(j=WaGenerateRand32(Cpd->hRandMan,0,MAZE_MAX_HEIGHT-1);j<MAZE_MAX_HEIGHT;j++)
      {
        if(j && WaGenerateRand32(Cpd->hRandMan,0,1))
        {
          MazeRangeHeight[j]=MazeRangeHeight[j-1];
          j=WaGenerateRand32(Cpd->hRandMan,0,MAZE_MAX_HEIGHT-1);
        }
      }

      for(i=0;i<MAZE_MAX_WIDTH && !retVal;i++)
      {
        for(j=0;j<MAZE_MAX_HEIGHT && !retVal;j++)
        {
          if(Cpd->maze[MazeRangeWidth[i]][MazeRangeHeight[j]]==MAZE_CORRIDOR || 
            Cpd->maze[MazeRangeWidth[i]][MazeRangeHeight[j]]==MAZE_START)
          {
            MazePos Temp;
            Temp.x=MazeRangeWidth[i];
            Temp.y=MazeRangeHeight[j];
            retVal+=DoMazeCorridors(Cpd,&Temp,1);
          }
        }  
      }
    }

    if(!k)
      __asm nop;

    free((MEMBLOCK)MazeRangeWidth);
    free((MEMBLOCK)MazeRangeHeight);
  }

  return retVal;
}

void InitMaze(MazeSnapshot * StartSnapshot, ClientPrivateData * Cpd)
{
  unsigned int i = 0;
  unsigned int j = 0;

  do
  {
    i = 0;j = 0;

    while(1)
    {
      Cpd->Start.x = 0; //StartSnapshot->Man.x = WaGenerateRand32(Cpd->hRandMan,0,MAZE_MAX_WIDTH/4);
      Cpd->Start.y = 0; //StartSnapshot->Man.y = WaGenerateRand32(Cpd->hRandMan,0,MAZE_MAX_HEIGHT-1);
      Cpd->End.x = MAZE_MAX_WIDTH-1;//WaGenerateRand32(Cpd->hRandMan,MAZE_MAX_WIDTH-(MAZE_MAX_WIDTH/4),MAZE_MAX_WIDTH-1);
      Cpd->End.y = MAZE_MAX_HEIGHT-1;//WaGenerateRand32(Cpd->hRandMan,0,MAZE_MAX_HEIGHT-1);

      StartSnapshot->Man.x=0;
      StartSnapshot->Man.y=0;

      if(GetMazeCellsDistance(&Cpd->Start,&Cpd->End)>1)
        break;
    }
    
    for(i=0;i<MAZE_MAX_WIDTH;i++)
    {
      for(j=0;j<MAZE_MAX_HEIGHT;j++)
      {
        Cpd->maze[i][j]=MAZE_WALL;
      }
    }
    
    Cpd->maze[Cpd->Start.x][Cpd->Start.y]=MAZE_START;
    Cpd->maze[Cpd->End.x][Cpd->End.y]=MAZE_END;
  }
  while(!DoMazeCorridors(Cpd,&Cpd->Start,0));
  
  PrintMaze(Cpd,"maze.bmp");
}


///////////////////////////////////////////////////////////////////////////////
//CALLBACKS FOR THE ENGINE vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

MEMBLOCK myalloc(MEMSZ sz)
{ 
#ifdef MALLOCS_DEBUG_HARD

  unsigned char * ptr=malloc((size_t)(sz+8));

  if(ptr)
  {
    totalmemusage+=sz;
#ifndef MALLOCS_DEBUG_WITHOUT_PRINTS
    printf("malloc:%d bytes, mem usage:%d bytes\n",sz,totalmemusage);
#endif
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
#ifndef MALLOCS_DEBUG_WITHOUT_PRINTS
    printf("malloc:%d bytes, mem usage:%d bytes\n",sz,totalmemusage);
#endif
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
#ifndef MALLOCS_DEBUG_WITHOUT_PRINTS
  printf("free:%d bytes, mem usage:%d bytes\n",*((unsigned int*)mb),totalmemusage);
#endif
  free(mb);

#else

#ifdef MALLOCS_DEBUG_SOFT

  ((char*)mb)-=4;
  totalmemusage-=*((unsigned int*)mb);
#ifndef MALLOCS_DEBUG_WITHOUT_PRINTS
  printf("free:%d bytes, mem usage:%d bytes\n",*((unsigned int*)mb),totalmemusage);
#endif
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

  printf("CheckMemUsage:%d\n",totalmemusage);

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
  UL32 retVal = 
    (MAZE_MAX_HEIGHT+
     MAZE_MAX_WIDTH-
     GetMazeCellsDistance(&((pMazeSnapshot)b)->Man,&((pClientPrivateData)cp)->End));
  
  if(retVal>=(MAZE_MAX_HEIGHT+MAZE_MAX_WIDTH-1))
  {
    g_PathFound=1;  
  }

  return retVal;
}

UL32 GenerateSnapshotEvolutions(PWORLD_SNAPSHOTS_MANAGER a,
                               WORLD_SNAPSHOT b,
                               PWORLD_SNAPSHOTS_GROUP* c,
                               WORLD_SNAPSHOT_CLIENT_PARAM cp,
                               RESERVED d)
{

  WAHASH hash={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  pMazeSnapshot pCurSnapshot = (pMazeSnapshot)b;
  pClientPrivateData Cpd = (pClientPrivateData)cp;

  if(g_UserQueryStop || g_PathFound)
    return 0;
  
  *c=(PWORLD_SNAPSHOTS_GROUP)myalloc(sizeof(WORLD_SNAPSHOTS_GROUP)+sizeof(WORLD_SNAPSHOT)*3+sizeof(UL32));

  if(!*c)
    return 0;

  *((UL32*)(*c))=1;
  ((UC8*)(*c))+=4;

  (*c)->nSnapshots=0;

  if(pCurSnapshot->Man.x>=0 && pCurSnapshot->Man.x<MAZE_MAX_WIDTH &&
     (pCurSnapshot->Man.y+1)>=0 && (pCurSnapshot->Man.y+1)<MAZE_MAX_HEIGHT &&
     Cpd->maze[pCurSnapshot->Man.x][pCurSnapshot->Man.y+1]!=MAZE_WALL)
  {
    *((UL32*)hash)=pCurSnapshot->Man.x;
    *((UL32*)&hash[4])=pCurSnapshot->Man.y+1;

    if(!IsWaHashAdded(Cpd->hHashMan,hash))
      if((*c)->Snapshots[(*c)->nSnapshots] = myalloc(sizeof(MazeSnapshot)))
      {
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.x=pCurSnapshot->Man.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.y=pCurSnapshot->Man.y+1;
        AddWaHash(Cpd->hHashMan,hash);
        (*c)->nSnapshots++;
      }
  }

  if((pCurSnapshot->Man.x+1)>=0 && (pCurSnapshot->Man.x+1)<MAZE_MAX_WIDTH &&
     pCurSnapshot->Man.y>=0 && pCurSnapshot->Man.y<MAZE_MAX_HEIGHT &&
     Cpd->maze[pCurSnapshot->Man.x+1][pCurSnapshot->Man.y]!=MAZE_WALL)
  {

    *((UL32*)hash)=pCurSnapshot->Man.x+1;
    *((UL32*)&hash[4])=pCurSnapshot->Man.y;

    if(!IsWaHashAdded(Cpd->hHashMan,hash))
      if((*c)->Snapshots[(*c)->nSnapshots] = myalloc(sizeof(MazeSnapshot)))
      {
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.x=pCurSnapshot->Man.x+1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.y=pCurSnapshot->Man.y;
        AddWaHash(Cpd->hHashMan,hash);
        (*c)->nSnapshots++;
      }    
  }

  if(pCurSnapshot->Man.x>=0 && pCurSnapshot->Man.x<MAZE_MAX_WIDTH &&
     (pCurSnapshot->Man.y-1)>=0 && (pCurSnapshot->Man.y-1)<MAZE_MAX_HEIGHT &&
     Cpd->maze[pCurSnapshot->Man.x][pCurSnapshot->Man.y-1]!=MAZE_WALL)
  {
    *((UL32*)hash)=pCurSnapshot->Man.x;
    *((UL32*)&hash[4])=pCurSnapshot->Man.y-1;
    if(!IsWaHashAdded(Cpd->hHashMan,hash))
      if((*c)->Snapshots[(*c)->nSnapshots] = myalloc(sizeof(MazeSnapshot)))
      {
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.x=pCurSnapshot->Man.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.y=pCurSnapshot->Man.y-1;
        AddWaHash(Cpd->hHashMan,hash);
        (*c)->nSnapshots++;
      }    
  }

  if((pCurSnapshot->Man.x-1)>=0 && (pCurSnapshot->Man.x-1)<MAZE_MAX_WIDTH &&
    pCurSnapshot->Man.y>=0 && pCurSnapshot->Man.y<MAZE_MAX_HEIGHT &&
     Cpd->maze[pCurSnapshot->Man.x-1][pCurSnapshot->Man.y]!=MAZE_WALL)
  {
    *((UL32*)hash)=pCurSnapshot->Man.x-1;
    *((UL32*)&hash[4])=pCurSnapshot->Man.y;

    if(!IsWaHashAdded(Cpd->hHashMan,hash))
      if((*c)->Snapshots[(*c)->nSnapshots] = myalloc(sizeof(MazeSnapshot)))
      {
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.x=pCurSnapshot->Man.x-1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.y=pCurSnapshot->Man.y;
        AddWaHash(Cpd->hHashMan,hash);
        (*c)->nSnapshots++;
      }    
  }

  if(!(*c)->nSnapshots)
  {
    myfree((MEMBLOCK)(((UC8*)(*c))-4));
    *c=WANULL;
    return 0;
  }

  return (*c)->nSnapshots;
}

NOTHING ReturnClientObject(PWORLD_SNAPSHOTS_MANAGER a,
                           UNKOBJ b,
                           WORLD_SNAPSHOT_CLIENT_PARAM cp,
                           RESERVED c)
{
  if(*((UL32*)(((UC8*)b)-4))==1)
  {
    UL32 i;
    PWORLD_SNAPSHOTS_GROUP pGroup = (PWORLD_SNAPSHOTS_GROUP)b;
    
    for(i=0;i<pGroup->nSnapshots;i++)
    {
      myfree((MEMBLOCK)pGroup->Snapshots[i]);
    }    

    myfree((MEMBLOCK)(((UC8*)b)-4));
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

  return "";
}

WATICKCOUNT TickCountFuncPtr()
{
  return (WATICKCOUNT)GetTickCount();
}

DWORD WINAPI QueryStopThreadProc(LPVOID lpParameter)
{
  MessageBox(WANULL,"Stop?","Stop?",0);
  g_UserQueryStop=1;
  return 0;
}

//CALLBACKS FOR THE ENGINE ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
///////////////////////////////////////////////////////////////////////////////


void submain()
{ 
  UL32 i;
  UL32 TickStart,TickEnd;
  pClientPrivateData pPrivData;
  MazeSnapshot StartSnapshot;
  PWORLD_ANALYSIS_JOB_RESULTS pJobResults;
  WORLD_ANALYSIS_JOB_STARTUP_INFO StartupInfo;
  WORLD_ANALYSIS_ENGINE_HANDLE hEngine;
  WORLD_ANALYSIS_JOB_HANDLE hJob;
  WORLD_ANALYSIS_INITIALIZATION_REQUIREMENTS * WAReq;
  DEBUG_CONFIG DbgCfg={NULL,0,GetDebugShowString};
  HANDLE hThread;
  UL32 ThreadId;

  pPrivData=(pClientPrivateData)myalloc(sizeof(ClientPrivateData));

  if(!pPrivData)
    return;

#ifndef INFINITE_TEST
  hThread=CreateThread(NULL,0,QueryStopThreadProc,NULL,0,&ThreadId);
#endif

#ifdef INITIALIZE_DEBUG
  DebugInit(&DbgCfg);
#endif
  
  CREATE_WORLD_ANALYSIS_INITIALIZATION_REQUERIMENTS(WAReq,
                                                    NULL,
                                                    DupSnapshot,
                                                    CompareSnapshots,
                                                    EvaluateSnapshot,
                                                    GenerateSnapshotEvolutions,
                                                    ReturnClientObject,
                                                    GetSnapshotHash,
                                                    pPrivData,
                                                    myalloc,
                                                    myfree);

  hEngine=CreateWaEngine(WAReq);

  pPrivData->hHashMan = CreateWaHashManager(hEngine);
  pPrivData->hRandMan = CreateWaRandManager(hEngine,TickCountFuncPtr,WAMERSENNE_TWISTER);

  InitMaze(&StartSnapshot,pPrivData);

  StartupInfo.Type=WORLD_ANALYSIS_JOB_DEPTH_FIRST;
  StartupInfo.StartSnapshot=&StartSnapshot;
  
  hJob=CreateWaAnalysisJob(hEngine,&StartupInfo);

  TickStart=GetTickCount();

  RunWaAnalysisJob(hJob);

  TickEnd=GetTickCount();

  //test
  ShowJob(hJob);

  pJobResults=GetWaAnalysisJobResults(hJob);

  for(i=0;i<pJobResults->nSnapshots;i++)
  {
    pPrivData->maze
    [((pMazeSnapshot)pJobResults->Snapshots[i])->Man.x]
    [((pMazeSnapshot)pJobResults->Snapshots[i])->Man.y]=MAZE_GOOD_PATH;
  }

  PrintMaze(pPrivData,"mazesol.bmp");

  DestroyWaAnalysisJob(hJob);

  DestroyWaRandManager(pPrivData->hRandMan);

  DestroyWaHashManager(pPrivData->hHashMan);

  DestroyWaEngine(hEngine);

  DESTROY_WORLD_ANALYSIS_INITIALIZATION_REQUERIMENTS(WAReq);
 

#ifdef INITIALIZE_DEBUG  
  DebugEnd();
#endif

  printf("ticks=%d\n",TickEnd-TickStart);

  myfree((MEMBLOCK)pPrivData);

#ifndef INFINITE_TEST
  MessageBox(WANULL,"Path found or user query stop!!",
             "Path found or user query stop!!",0);
  WaitForSingleObject(hThread,INFINITE);
  CloseHandle(hThread);
#endif

  CheckMemUsage();

  printf("\npress any key\n");

#ifndef INFINITE_TEST
  while(!_kbhit());
#endif
}

void main()
{
#ifdef INFINITE_TEST
  while(1)
  {
    unsigned int i;
    g_UserQueryStop=0;
    g_PathFound=0;
    totalmemusage=0;
    lastmemcountid=1;
#ifdef MALLOCS_DEBUG_HARD
    for(i=0;i<2000000;i++)
      memcountids[i]=0;
#endif
#endif
    submain();
#ifdef INFINITE_TEST
  }
#endif
}
