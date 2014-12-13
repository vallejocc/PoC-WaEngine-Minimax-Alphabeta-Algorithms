#include <conio.h>
#include <stdio.h>
#include <windows.h>
#include "../inc/pub/watypes.h"
#include "../inc/pub/waengine.h"
#include "../inc/pub/wasnapshots.h"
#include "../inc/pub/wadebug.h"
#include "../inc/pub/wahash.h"
#include "../inc/pub/warandom.h"
#include "../inc/priv/global.h"
#include "../inc/priv/md5.h"

MEMSZ totalmemusage=0;
UL32 lastmemcountid=1;
UL32 memcountids[2000000]={0};

//#define MALLOCS_DEBUG_WITHOUT_PRINTS
//#define MALLOCS_DEBUG_HARD
//#define MALLOCS_DEBUG_SOFT
//#define INITIALIZE_DEBUG

#define MAZE_MAX_WIDTH 30
#define MAZE_MAX_HEIGHT 30

#define MAZE_CORRIDOR   0
#define MAZE_WALL       1
#define MAZE_START      2
#define MAZE_END        3

#define PLY_OPPO        0
#define PLY_SELF        1

#define PLY_OPPO_MALE   0
#define PLY_OPPO_FEMALE 1

#define MAX_RECURSIVE_LEVEL_IN_MAZE_GENERATOR 1000

#define MAX_ANALYSIS_LEVEL_FOR_MOVE 40

typedef struct _MazePos
{
  unsigned int x;
  unsigned int y;
}MazePos,*pMazePos;

typedef struct _MazeSnapshot
{
  MazePos Man;
  MazePos MonsterMale;
  MazePos MonsterFemale;
  unsigned int Ply;
  unsigned int PlyOppo;
  unsigned int Level;
}MazeSnapshot,*pMazeSnapshot;

typedef struct _ClientPrivateData
{
  WAHASH_MANAGER_HANDLE hHashManSolutions;
  WAHASH_MANAGER_HANDLE hHashManForJob;
  WARAND_ENGINE_HANDLE hRandMan;
  MazePos End;
  MazePos Start;
  MazePos CurrentManPos;
  MazePos CurrentMonsterMalePos;
  MazePos CurrentMonsterFemalePos;
  unsigned char maze[MAZE_MAX_WIDTH][MAZE_MAX_HEIGHT];
}ClientPrivateData,*pClientPrivateData;

void CalculateMazeHash(WAHASH  hash, 
                       MazePos * CurrentManPos,
                       MazePos * CurrentMonsterMalePos,
                       MazePos * CurrentMonsterFemalePos)
{
  MD5_CTX md5_context;
  memset(hash,0,20);
  MD5Init(&md5_context);
  MD5Update(&md5_context, 
            CurrentManPos,
            sizeof(MazePos));
  MD5Update(&md5_context, 
            CurrentMonsterMalePos,
            sizeof(MazePos));
  MD5Update(&md5_context, 
            CurrentMonsterFemalePos,
            sizeof(MazePos));
  MD5Final(hash, &md5_context);
}

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
        else if((i==Cpd->CurrentMonsterFemalePos.x && j==Cpd->CurrentMonsterFemalePos.y)||
                (i==Cpd->CurrentMonsterMalePos.x && j==Cpd->CurrentMonsterMalePos.y)||
                (i==Cpd->CurrentManPos.x && j==Cpd->CurrentManPos.y))
          for(k=0;k<10;k++)fwrite(&red,3,1,f);
        else if(Cpd->maze[i][j]==MAZE_CORRIDOR)
          for(k=0;k<10;k++)fwrite(&green,3,1,f);
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

void InitMaze(ClientPrivateData * Cpd)
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

      Cpd->CurrentManPos.x = 0;
      Cpd->CurrentManPos.y = 0;
      Cpd->CurrentMonsterFemalePos.x = MAZE_MAX_WIDTH-1;
      Cpd->CurrentMonsterFemalePos.y = MAZE_MAX_HEIGHT-1;
      Cpd->CurrentMonsterMalePos.x = MAZE_MAX_WIDTH-1;
      Cpd->CurrentMonsterMalePos.y = MAZE_MAX_HEIGHT-1;

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

WORLD_SNAPSHOT_EVALUATION EvaluateSnapshotMonsterFemale(PWORLD_SNAPSHOTS_MANAGER a,
                                              WORLD_SNAPSHOT b,
                                              WORLD_SNAPSHOT_CLIENT_PARAM cp,
                                              RESERVED c)
{
  pClientPrivateData Cpd=(pClientPrivateData)cp;
  unsigned int MonsterFemaleDis;
  unsigned int StartEndDis;
  pMazeSnapshot pCurSnapshot = (pMazeSnapshot)b;
  WAHASH hash;
  CalculateMazeHash(hash,&pCurSnapshot->Man,&pCurSnapshot->MonsterMale,&pCurSnapshot->MonsterFemale);
  
  StartEndDis = GetMazeCellsDistance(&Cpd->Start,&Cpd->End);
  MonsterFemaleDis = GetMazeCellsDistance(&pCurSnapshot->Man,&pCurSnapshot->MonsterFemale);
  
  if(!MonsterFemaleDis)
  {
    return 0xfffffffe;
  }
  
  if(1 || !IsWaHashAdded(Cpd->hHashManSolutions,hash))
    return StartEndDis-MonsterFemaleDis;
  else
    return 0;//(StartEndDis-MonsterFemaleDis)/2;
}

WORLD_SNAPSHOT_EVALUATION EvaluateSnapshotMonsterMale(PWORLD_SNAPSHOTS_MANAGER a,
                                              WORLD_SNAPSHOT b,
                                              WORLD_SNAPSHOT_CLIENT_PARAM cp,
                                              RESERVED c)
{
  pClientPrivateData Cpd=(pClientPrivateData)cp;
  unsigned int MonsterMaleDis;
  unsigned int StartEndDis;
  pMazeSnapshot pCurSnapshot = (pMazeSnapshot)b;
  WAHASH hash;
  CalculateMazeHash(hash,&pCurSnapshot->Man,&pCurSnapshot->MonsterMale,&pCurSnapshot->MonsterFemale);
  
  StartEndDis = GetMazeCellsDistance(&Cpd->Start,&Cpd->End);
  MonsterMaleDis = GetMazeCellsDistance(&pCurSnapshot->Man,&pCurSnapshot->MonsterMale);
  
  if(!MonsterMaleDis)
  {
    return 0xfffffffe;
  }
  
  if(1 || !IsWaHashAdded(Cpd->hHashManSolutions,hash))
    return StartEndDis-MonsterMaleDis;
  else
    return 0;//(StartEndDis-MonsterMaleDis)/2;
}

WORLD_SNAPSHOT_EVALUATION EvaluateSnapshotMan(PWORLD_SNAPSHOTS_MANAGER a,
                                              WORLD_SNAPSHOT b,
                                              WORLD_SNAPSHOT_CLIENT_PARAM cp,
                                              RESERVED c)
{
  pClientPrivateData Cpd=(pClientPrivateData)cp;
  unsigned int MonsterMaleDis;
  unsigned int MonsterFemaleDis;
  unsigned int StartEndDis;
  unsigned int ManEndDis;
  pMazeSnapshot pCurSnapshot = (pMazeSnapshot)b;
  WAHASH hash;

  CalculateMazeHash(hash,&pCurSnapshot->Man,&pCurSnapshot->MonsterMale,&pCurSnapshot->MonsterFemale);

  MonsterMaleDis = GetMazeCellsDistance(&pCurSnapshot->Man,&pCurSnapshot->MonsterMale);
  MonsterFemaleDis = GetMazeCellsDistance(&pCurSnapshot->Man,&pCurSnapshot->MonsterMale);
  ManEndDis = GetMazeCellsDistance(&pCurSnapshot->Man,&Cpd->End);
  StartEndDis = GetMazeCellsDistance(&Cpd->Start,&Cpd->End);
  
  if(!MonsterMaleDis || !MonsterFemaleDis)
  {
    return 0;
  }

  if(1 || !IsWaHashAdded(Cpd->hHashManSolutions,hash))
    return (StartEndDis-ManEndDis)+(MonsterMaleDis/2)+(MonsterFemaleDis/2);
  else
    return 0;//((StartEndDis-ManEndDis)+(MonsterMaleDis/2)+(MonsterFemaleDis/2))/2;
}

UL32 GenerateMovesForMonsterMale(pClientPrivateData Cpd,pMazeSnapshot pCurSnapshot,PWORLD_SNAPSHOTS_GROUP* c)
{
  WAHASH hash;
  MazePos TempPos;

  *c=(PWORLD_SNAPSHOTS_GROUP)myalloc(sizeof(WORLD_SNAPSHOTS_GROUP)+sizeof(WORLD_SNAPSHOT)*3+sizeof(UL32));

  if(!*c)
    return 0;

  *((UL32*)(*c))=1;
  ((UC8*)(*c))+=4;

  (*c)->nSnapshots=0;

  if(pCurSnapshot->MonsterMale.x>=0 && pCurSnapshot->MonsterMale.x<MAZE_MAX_WIDTH &&
     (pCurSnapshot->MonsterMale.y+1)>=0 && (pCurSnapshot->MonsterMale.y+1)<MAZE_MAX_HEIGHT &&
     Cpd->maze[pCurSnapshot->MonsterMale.x][pCurSnapshot->MonsterMale.y+1]!=MAZE_WALL)
  {
    TempPos.x=pCurSnapshot->MonsterMale.x;
    TempPos.y=pCurSnapshot->MonsterMale.y+1;
    CalculateMazeHash(hash,
                      &pCurSnapshot->Man,
                      &TempPos,
                      &pCurSnapshot->MonsterFemale);

    if(!IsWaHashAdded(Cpd->hHashManForJob,hash))
      if((*c)->Snapshots[(*c)->nSnapshots] = myalloc(sizeof(MazeSnapshot)))
      {
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.x=pCurSnapshot->MonsterMale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.y=pCurSnapshot->MonsterMale.y+1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.x=pCurSnapshot->MonsterFemale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.y=pCurSnapshot->MonsterFemale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.x=pCurSnapshot->Man.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.y=pCurSnapshot->Man.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Ply=pCurSnapshot->Ply^1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->PlyOppo=pCurSnapshot->PlyOppo;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Level=pCurSnapshot->Level+1;
        AddWaHash(Cpd->hHashManForJob,hash);
        (*c)->nSnapshots++;
      }
  }

  if((pCurSnapshot->MonsterMale.x+1)>=0 && (pCurSnapshot->MonsterMale.x+1)<MAZE_MAX_WIDTH &&
     pCurSnapshot->MonsterMale.y>=0 && pCurSnapshot->MonsterMale.y<MAZE_MAX_HEIGHT &&
     Cpd->maze[pCurSnapshot->MonsterMale.x+1][pCurSnapshot->MonsterMale.y]!=MAZE_WALL)
  {
    TempPos.x=pCurSnapshot->MonsterMale.x+1;
    TempPos.y=pCurSnapshot->MonsterMale.y;
    CalculateMazeHash(hash,
                      &pCurSnapshot->Man,
                      &TempPos,
                      &pCurSnapshot->MonsterFemale);

    if(!IsWaHashAdded(Cpd->hHashManForJob,hash))
      if((*c)->Snapshots[(*c)->nSnapshots] = myalloc(sizeof(MazeSnapshot)))
      {
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.x=pCurSnapshot->MonsterMale.x+1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.y=pCurSnapshot->MonsterMale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.x=pCurSnapshot->MonsterFemale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.y=pCurSnapshot->MonsterFemale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.x=pCurSnapshot->Man.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.y=pCurSnapshot->Man.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Ply=pCurSnapshot->Ply^1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->PlyOppo=pCurSnapshot->PlyOppo;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Level=pCurSnapshot->Level+1;
        AddWaHash(Cpd->hHashManForJob,hash);
        (*c)->nSnapshots++;
      }    
  }

  if(pCurSnapshot->MonsterMale.x>=0 && pCurSnapshot->MonsterMale.x<MAZE_MAX_WIDTH &&
     (pCurSnapshot->MonsterMale.y-1)>=0 && (pCurSnapshot->MonsterMale.y-1)<MAZE_MAX_HEIGHT &&
     Cpd->maze[pCurSnapshot->MonsterMale.x][pCurSnapshot->MonsterMale.y-1]!=MAZE_WALL)
  {
    TempPos.x=pCurSnapshot->MonsterMale.x;
    TempPos.y=pCurSnapshot->MonsterMale.y-1;
    CalculateMazeHash(hash,
                      &pCurSnapshot->Man,
                      &TempPos,
                      &pCurSnapshot->MonsterFemale);

    if(!IsWaHashAdded(Cpd->hHashManForJob,hash))
      if((*c)->Snapshots[(*c)->nSnapshots] = myalloc(sizeof(MazeSnapshot)))
      {
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.x=pCurSnapshot->MonsterMale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.y=pCurSnapshot->MonsterMale.y-1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.x=pCurSnapshot->MonsterFemale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.y=pCurSnapshot->MonsterFemale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.x=pCurSnapshot->Man.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.y=pCurSnapshot->Man.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Ply=pCurSnapshot->Ply^1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->PlyOppo=pCurSnapshot->PlyOppo;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Level=pCurSnapshot->Level+1;
        AddWaHash(Cpd->hHashManForJob,hash);
        (*c)->nSnapshots++;
      }    
  }

  if((pCurSnapshot->MonsterMale.x-1)>=0 && (pCurSnapshot->MonsterMale.x-1)<MAZE_MAX_WIDTH &&
    pCurSnapshot->MonsterMale.y>=0 && pCurSnapshot->MonsterMale.y<MAZE_MAX_HEIGHT &&
     Cpd->maze[pCurSnapshot->MonsterMale.x-1][pCurSnapshot->MonsterMale.y]!=MAZE_WALL)
  {

    TempPos.x=pCurSnapshot->MonsterMale.x-1;
    TempPos.y=pCurSnapshot->MonsterMale.y;
    CalculateMazeHash(hash,
                      &pCurSnapshot->Man,
                      &TempPos,
                      &pCurSnapshot->MonsterFemale);

    if(!IsWaHashAdded(Cpd->hHashManForJob,hash))
      if((*c)->Snapshots[(*c)->nSnapshots] = myalloc(sizeof(MazeSnapshot)))
      {
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.x=pCurSnapshot->MonsterMale.x-1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.y=pCurSnapshot->MonsterMale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.x=pCurSnapshot->MonsterFemale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.y=pCurSnapshot->MonsterFemale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.x=pCurSnapshot->Man.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.y=pCurSnapshot->Man.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Ply=pCurSnapshot->Ply^1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->PlyOppo=pCurSnapshot->PlyOppo;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Level=pCurSnapshot->Level+1;
        AddWaHash(Cpd->hHashManForJob,hash);
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

UL32 GenerateMovesForMonsterFemale(pClientPrivateData Cpd,pMazeSnapshot pCurSnapshot,PWORLD_SNAPSHOTS_GROUP* c)
{
  WAHASH hash;
  MazePos TempPos;

  *c=(PWORLD_SNAPSHOTS_GROUP)myalloc(sizeof(WORLD_SNAPSHOTS_GROUP)+sizeof(WORLD_SNAPSHOT)*3+sizeof(UL32));

  if(!*c)
    return 0;

  *((UL32*)(*c))=1;
  ((UC8*)(*c))+=4;

  (*c)->nSnapshots=0;

  if(pCurSnapshot->MonsterFemale.x>=0 && pCurSnapshot->MonsterFemale.x<MAZE_MAX_WIDTH &&
     (pCurSnapshot->MonsterFemale.y+1)>=0 && (pCurSnapshot->MonsterFemale.y+1)<MAZE_MAX_HEIGHT &&
     Cpd->maze[pCurSnapshot->MonsterFemale.x][pCurSnapshot->MonsterFemale.y+1]!=MAZE_WALL)
  {
    TempPos.x=pCurSnapshot->MonsterFemale.x;
    TempPos.y=pCurSnapshot->MonsterFemale.y+1;
    CalculateMazeHash(hash,
                      &pCurSnapshot->Man,
                      &pCurSnapshot->MonsterMale,
                      &TempPos);

    if(!IsWaHashAdded(Cpd->hHashManForJob,hash))
      if((*c)->Snapshots[(*c)->nSnapshots] = myalloc(sizeof(MazeSnapshot)))
      {
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.x=pCurSnapshot->MonsterFemale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.y=pCurSnapshot->MonsterFemale.y+1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.x=pCurSnapshot->Man.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.y=pCurSnapshot->Man.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.x=pCurSnapshot->MonsterMale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.y=pCurSnapshot->MonsterMale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Ply=pCurSnapshot->Ply^1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->PlyOppo=pCurSnapshot->PlyOppo;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Level=pCurSnapshot->Level+1;
        AddWaHash(Cpd->hHashManForJob,hash);
        (*c)->nSnapshots++;
      }
  }

  if((pCurSnapshot->MonsterFemale.x+1)>=0 && (pCurSnapshot->MonsterFemale.x+1)<MAZE_MAX_WIDTH &&
     pCurSnapshot->MonsterFemale.y>=0 && pCurSnapshot->MonsterFemale.y<MAZE_MAX_HEIGHT &&
     Cpd->maze[pCurSnapshot->MonsterFemale.x+1][pCurSnapshot->MonsterFemale.y]!=MAZE_WALL)
  {
    TempPos.x=pCurSnapshot->MonsterFemale.x+1;
    TempPos.y=pCurSnapshot->MonsterFemale.y;
    CalculateMazeHash(hash,
                      &pCurSnapshot->Man,
                      &pCurSnapshot->MonsterMale,
                      &TempPos);

    if(!IsWaHashAdded(Cpd->hHashManForJob,hash))
      if((*c)->Snapshots[(*c)->nSnapshots] = myalloc(sizeof(MazeSnapshot)))
      {
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.x=pCurSnapshot->MonsterFemale.x+1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.y=pCurSnapshot->MonsterFemale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.x=pCurSnapshot->Man.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.y=pCurSnapshot->Man.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.x=pCurSnapshot->MonsterMale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.y=pCurSnapshot->MonsterMale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Ply=pCurSnapshot->Ply^1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->PlyOppo=pCurSnapshot->PlyOppo;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Level=pCurSnapshot->Level+1;
        AddWaHash(Cpd->hHashManForJob,hash);
        (*c)->nSnapshots++;
      }    
  }

  if(pCurSnapshot->MonsterFemale.x>=0 && pCurSnapshot->MonsterFemale.x<MAZE_MAX_WIDTH &&
     (pCurSnapshot->MonsterFemale.y-1)>=0 && (pCurSnapshot->MonsterFemale.y-1)<MAZE_MAX_HEIGHT &&
     Cpd->maze[pCurSnapshot->MonsterFemale.x][pCurSnapshot->MonsterFemale.y-1]!=MAZE_WALL)
  {
    TempPos.x=pCurSnapshot->MonsterFemale.x;
    TempPos.y=pCurSnapshot->MonsterFemale.y-1;
    CalculateMazeHash(hash,
                      &pCurSnapshot->Man,
                      &pCurSnapshot->MonsterMale,
                      &TempPos);

    if(!IsWaHashAdded(Cpd->hHashManForJob,hash))
      if((*c)->Snapshots[(*c)->nSnapshots] = myalloc(sizeof(MazeSnapshot)))
      {
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.x=pCurSnapshot->MonsterFemale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.y=pCurSnapshot->MonsterFemale.y-1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.x=pCurSnapshot->Man.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.y=pCurSnapshot->Man.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.x=pCurSnapshot->MonsterMale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.y=pCurSnapshot->MonsterMale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Ply=pCurSnapshot->Ply^1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->PlyOppo=pCurSnapshot->PlyOppo;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Level=pCurSnapshot->Level+1;
        AddWaHash(Cpd->hHashManForJob,hash);
        (*c)->nSnapshots++;
      }    
  }

  if((pCurSnapshot->MonsterFemale.x-1)>=0 && (pCurSnapshot->MonsterFemale.x-1)<MAZE_MAX_WIDTH &&
    pCurSnapshot->MonsterFemale.y>=0 && pCurSnapshot->MonsterFemale.y<MAZE_MAX_HEIGHT &&
     Cpd->maze[pCurSnapshot->MonsterFemale.x-1][pCurSnapshot->MonsterFemale.y]!=MAZE_WALL)
  {

    TempPos.x=pCurSnapshot->MonsterFemale.x-1;
    TempPos.y=pCurSnapshot->MonsterFemale.y;
    CalculateMazeHash(hash,
                      &pCurSnapshot->Man,
                      &pCurSnapshot->MonsterMale,
                      &TempPos);

    if(!IsWaHashAdded(Cpd->hHashManForJob,hash))
      if((*c)->Snapshots[(*c)->nSnapshots] = myalloc(sizeof(MazeSnapshot)))
      {
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.x=pCurSnapshot->MonsterFemale.x-1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.y=pCurSnapshot->MonsterFemale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.x=pCurSnapshot->Man.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.y=pCurSnapshot->Man.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.x=pCurSnapshot->MonsterMale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.y=pCurSnapshot->MonsterMale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Ply=pCurSnapshot->Ply^1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->PlyOppo=pCurSnapshot->PlyOppo;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Level=pCurSnapshot->Level+1;
        AddWaHash(Cpd->hHashManForJob,hash);
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

UL32 GenerateMovesForMan(pClientPrivateData Cpd,pMazeSnapshot pCurSnapshot,PWORLD_SNAPSHOTS_GROUP* c)
{
  WAHASH hash;
  MazePos TempPos;
  
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
    TempPos.x=pCurSnapshot->Man.x;
    TempPos.y=pCurSnapshot->Man.y+1;
    CalculateMazeHash(hash,
                      &TempPos,
                      &pCurSnapshot->MonsterMale,
                      &pCurSnapshot->MonsterFemale);

    if(!IsWaHashAdded(Cpd->hHashManForJob,hash))
      if((*c)->Snapshots[(*c)->nSnapshots] = myalloc(sizeof(MazeSnapshot)))
      {
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.x=pCurSnapshot->Man.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.y=pCurSnapshot->Man.y+1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.x=pCurSnapshot->MonsterMale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.y=pCurSnapshot->MonsterMale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.x=pCurSnapshot->MonsterFemale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.y=pCurSnapshot->MonsterFemale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Ply=pCurSnapshot->Ply^1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->PlyOppo=pCurSnapshot->PlyOppo^1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Level=pCurSnapshot->Level+1;
        AddWaHash(Cpd->hHashManForJob,hash);
        (*c)->nSnapshots++;
      }
  }

  if((pCurSnapshot->Man.x+1)>=0 && (pCurSnapshot->Man.x+1)<MAZE_MAX_WIDTH &&
     pCurSnapshot->Man.y>=0 && pCurSnapshot->Man.y<MAZE_MAX_HEIGHT &&
     Cpd->maze[pCurSnapshot->Man.x+1][pCurSnapshot->Man.y]!=MAZE_WALL)
  {
    TempPos.x=pCurSnapshot->Man.x+1;
    TempPos.y=pCurSnapshot->Man.y;
    CalculateMazeHash(hash,
                      &TempPos,
                      &pCurSnapshot->MonsterMale,
                      &pCurSnapshot->MonsterFemale);

    if(!IsWaHashAdded(Cpd->hHashManForJob,hash))
      if((*c)->Snapshots[(*c)->nSnapshots] = myalloc(sizeof(MazeSnapshot)))
      {
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.x=pCurSnapshot->Man.x+1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.y=pCurSnapshot->Man.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.x=pCurSnapshot->MonsterMale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.y=pCurSnapshot->MonsterMale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.x=pCurSnapshot->MonsterFemale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.y=pCurSnapshot->MonsterFemale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Ply=pCurSnapshot->Ply^1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->PlyOppo=pCurSnapshot->PlyOppo^1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Level=pCurSnapshot->Level+1;
        AddWaHash(Cpd->hHashManForJob,hash);
        (*c)->nSnapshots++;
      }    
  }

  if(pCurSnapshot->Man.x>=0 && pCurSnapshot->Man.x<MAZE_MAX_WIDTH &&
     (pCurSnapshot->Man.y-1)>=0 && (pCurSnapshot->Man.y-1)<MAZE_MAX_HEIGHT &&
     Cpd->maze[pCurSnapshot->Man.x][pCurSnapshot->Man.y-1]!=MAZE_WALL)
  {
    TempPos.x=pCurSnapshot->Man.x;
    TempPos.y=pCurSnapshot->Man.y-1;
    CalculateMazeHash(hash,
                      &TempPos,
                      &pCurSnapshot->MonsterMale,
                      &pCurSnapshot->MonsterFemale);

    if(!IsWaHashAdded(Cpd->hHashManForJob,hash))
      if((*c)->Snapshots[(*c)->nSnapshots] = myalloc(sizeof(MazeSnapshot)))
      {
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.x=pCurSnapshot->Man.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.y=pCurSnapshot->Man.y-1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.x=pCurSnapshot->MonsterMale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.y=pCurSnapshot->MonsterMale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.x=pCurSnapshot->MonsterFemale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.y=pCurSnapshot->MonsterFemale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Ply=pCurSnapshot->Ply^1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->PlyOppo=pCurSnapshot->PlyOppo^1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Level=pCurSnapshot->Level+1;
        AddWaHash(Cpd->hHashManForJob,hash);
        (*c)->nSnapshots++;
      }    
  }

  if((pCurSnapshot->Man.x-1)>=0 && (pCurSnapshot->Man.x-1)<MAZE_MAX_WIDTH &&
    pCurSnapshot->Man.y>=0 && pCurSnapshot->Man.y<MAZE_MAX_HEIGHT &&
     Cpd->maze[pCurSnapshot->Man.x-1][pCurSnapshot->Man.y]!=MAZE_WALL)
  {

    TempPos.x=pCurSnapshot->Man.x-1;
    TempPos.y=pCurSnapshot->Man.y;
    CalculateMazeHash(hash,
                      &TempPos,
                      &pCurSnapshot->MonsterMale,
                      &pCurSnapshot->MonsterFemale);

    if(!IsWaHashAdded(Cpd->hHashManForJob,hash))
      if((*c)->Snapshots[(*c)->nSnapshots] = myalloc(sizeof(MazeSnapshot)))
      {
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.x=pCurSnapshot->Man.x-1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Man.y=pCurSnapshot->Man.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.x=pCurSnapshot->MonsterMale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterMale.y=pCurSnapshot->MonsterMale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.x=pCurSnapshot->MonsterFemale.x;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->MonsterFemale.y=pCurSnapshot->MonsterFemale.y;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Ply=pCurSnapshot->Ply^1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->PlyOppo=pCurSnapshot->PlyOppo^1;
        ((pMazeSnapshot)((*c)->Snapshots[(*c)->nSnapshots]))->Level=pCurSnapshot->Level+1;
        AddWaHash(Cpd->hHashManForJob,hash);
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


UL32 GenerateSnapshotEvolutionsMonsterFemale(PWORLD_SNAPSHOTS_MANAGER a,
                                   WORLD_SNAPSHOT b,
                                   PWORLD_SNAPSHOTS_GROUP* c,
                                   WORLD_SNAPSHOT_CLIENT_PARAM cp,
                                   RESERVED d)
{
  pMazeSnapshot pCurSnapshot = (pMazeSnapshot)b;
  
  if(pCurSnapshot->Level==MAX_ANALYSIS_LEVEL_FOR_MOVE)
    return 0;

  if(pCurSnapshot->Ply==PLY_OPPO)
  {
    GenerateMovesForMan(cp,pCurSnapshot,c);
  }
  else
  {
    if(pCurSnapshot->PlyOppo==PLY_OPPO_MALE)
    {
      GenerateMovesForMonsterMale(cp,pCurSnapshot,c);
    }
    else
    {
      GenerateMovesForMonsterFemale(cp,pCurSnapshot,c);
    }
  }
  
  if(!*c)
    return 0;

  return (*c)->nSnapshots;
}

UL32 GenerateSnapshotEvolutionsMonsterMale(PWORLD_SNAPSHOTS_MANAGER a,
                                   WORLD_SNAPSHOT b,
                                   PWORLD_SNAPSHOTS_GROUP* c,
                                   WORLD_SNAPSHOT_CLIENT_PARAM cp,
                                   RESERVED d)
{
  pMazeSnapshot pCurSnapshot = (pMazeSnapshot)b;

  if(pCurSnapshot->Level==MAX_ANALYSIS_LEVEL_FOR_MOVE)
    return 0;

  if(pCurSnapshot->Ply==PLY_OPPO)
  {
    GenerateMovesForMan(cp,pCurSnapshot,c);
  }
  else
  {
    if(pCurSnapshot->PlyOppo==PLY_OPPO_MALE)
    {
      GenerateMovesForMonsterMale(cp,pCurSnapshot,c);
    }
    else
    {
      GenerateMovesForMonsterFemale(cp,pCurSnapshot,c);
    }
  }
  
  if(!*c)
    return 0;
  
  return (*c)->nSnapshots;
}

UL32 GenerateSnapshotEvolutionsMan(PWORLD_SNAPSHOTS_MANAGER a,
                                   WORLD_SNAPSHOT b,
                                   PWORLD_SNAPSHOTS_GROUP* c,
                                   WORLD_SNAPSHOT_CLIENT_PARAM cp,
                                   RESERVED d)
{
  pMazeSnapshot pCurSnapshot = (pMazeSnapshot)b;
  
  if(pCurSnapshot->Level==MAX_ANALYSIS_LEVEL_FOR_MOVE)
    return 0;

  if(pCurSnapshot->Ply==PLY_SELF)
  {
    GenerateMovesForMan(cp,pCurSnapshot,c);
  }
  else
  {
    if(pCurSnapshot->PlyOppo==PLY_OPPO_MALE)
    {
      GenerateMovesForMonsterMale(cp,pCurSnapshot,c);
    }
    else
    {
      GenerateMovesForMonsterFemale(cp,pCurSnapshot,c);
    }
  }

  if(!*c)
    return 0;

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

//CALLBACKS FOR THE ENGINE ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
///////////////////////////////////////////////////////////////////////////////

void MoveCreature(WORLD_ANALYSIS_ENGINE_HANDLE hEngine,
                  pClientPrivateData pPrivData,
                  pMazePos NewPosMan,
                  pMazePos NewPosMonsterMale,
                  pMazePos NewPosMonsterFemale)
{
  PWORLD_ANALYSIS_JOB_RESULTS pJobResults;
  WORLD_ANALYSIS_JOB_STARTUP_INFO StartupInfo;
  MazeSnapshot StartSnapshot;
  WORLD_ANALYSIS_JOB_HANDLE hJob;
  
  StartSnapshot.Man.x=pPrivData->CurrentManPos.x;
  StartSnapshot.Man.y=pPrivData->CurrentManPos.y;
  StartSnapshot.MonsterMale.x=pPrivData->CurrentMonsterMalePos.x;
  StartSnapshot.MonsterMale.y=pPrivData->CurrentMonsterMalePos.y;
  StartSnapshot.MonsterFemale.x=pPrivData->CurrentMonsterFemalePos.x;
  StartSnapshot.MonsterFemale.y=pPrivData->CurrentMonsterFemalePos.y;
  StartSnapshot.Ply=PLY_SELF;
  StartSnapshot.PlyOppo=PLY_OPPO_MALE;
  StartSnapshot.Level=1;
  
  pPrivData->hHashManForJob = CreateWaHashManager(hEngine);

  StartupInfo.Type=WORLD_ANALYSIS_JOB_ALPHA_BETA;
  StartupInfo.StartSnapshot=&StartSnapshot;
  
  hJob=CreateWaAnalysisJob(hEngine,&StartupInfo);

  RunWaAnalysisJob(hJob);

  pJobResults=GetWaAnalysisJobResults(hJob);

  if(NewPosMan)NewPosMan->x = ((pMazeSnapshot)pJobResults->Snapshots[1])->Man.x;
  if(NewPosMan)NewPosMan->y = ((pMazeSnapshot)pJobResults->Snapshots[1])->Man.y;
  if(NewPosMonsterMale)NewPosMonsterMale->x = ((pMazeSnapshot)pJobResults->Snapshots[1])->MonsterMale.x;
  if(NewPosMonsterMale)NewPosMonsterMale->y = ((pMazeSnapshot)pJobResults->Snapshots[1])->MonsterMale.y;
  if(NewPosMonsterFemale)NewPosMonsterFemale->x = ((pMazeSnapshot)pJobResults->Snapshots[1])->MonsterFemale.x;
  if(NewPosMonsterFemale)NewPosMonsterFemale->y = ((pMazeSnapshot)pJobResults->Snapshots[1])->MonsterFemale.y;

  DestroyWaAnalysisJob(hJob);
  DestroyWaHashManager(pPrivData->hHashManForJob);
}

void submain()
{ 
  pClientPrivateData pPrivData;
  WORLD_ANALYSIS_ENGINE_HANDLE hEngineMan;
  WORLD_ANALYSIS_ENGINE_HANDLE hEngineMonsterMale;
  WORLD_ANALYSIS_ENGINE_HANDLE hEngineMonsterFemale;
  WORLD_ANALYSIS_INITIALIZATION_REQUIREMENTS * WAReq;
  DEBUG_CONFIG DbgCfg={NULL,0,GetDebugShowString};

  pPrivData=(pClientPrivateData)myalloc(sizeof(ClientPrivateData));

  if(!pPrivData)
    return;

#ifdef INITIALIZE_DEBUG
  DebugInit(&DbgCfg);
#endif

  CREATE_WORLD_ANALYSIS_INITIALIZATION_REQUERIMENTS(WAReq,
                                                    NULL,
                                                    DupSnapshot,
                                                    CompareSnapshots,
                                                    EvaluateSnapshotMan,
                                                    GenerateSnapshotEvolutionsMan,
                                                    ReturnClientObject,
                                                    GetSnapshotHash,
                                                    pPrivData,
                                                    myalloc,
                                                    myfree);

  hEngineMan=CreateWaEngine(WAReq);

  CREATE_WORLD_ANALYSIS_INITIALIZATION_REQUERIMENTS(WAReq,
                                                    NULL,
                                                    DupSnapshot,
                                                    CompareSnapshots,
                                                    EvaluateSnapshotMonsterMale,
                                                    GenerateSnapshotEvolutionsMonsterMale,
                                                    ReturnClientObject,
                                                    GetSnapshotHash,
                                                    pPrivData,
                                                    myalloc,
                                                    myfree);

  hEngineMonsterMale=CreateWaEngine(WAReq);

  CREATE_WORLD_ANALYSIS_INITIALIZATION_REQUERIMENTS(WAReq,
                                                    NULL,
                                                    DupSnapshot,
                                                    CompareSnapshots,
                                                    EvaluateSnapshotMonsterFemale,
                                                    GenerateSnapshotEvolutionsMonsterFemale,
                                                    ReturnClientObject,
                                                    GetSnapshotHash,
                                                    pPrivData,
                                                    myalloc,
                                                    myfree);

  hEngineMonsterFemale=CreateWaEngine(WAReq);

  pPrivData->hRandMan = CreateWaRandManager(hEngineMan,TickCountFuncPtr,WAMERSENNE_TWISTER);
  InitMaze(pPrivData);
  pPrivData->hHashManSolutions = CreateWaHashManager(hEngineMan);

  while(1)
  {
    WAHASH hash;

    MoveCreature(hEngineMan, pPrivData, &pPrivData->CurrentManPos,NULL,NULL);
    CalculateMazeHash(hash,&pPrivData->CurrentManPos,&pPrivData->CurrentMonsterMalePos,&pPrivData->CurrentMonsterFemalePos);
    if(!IsWaHashAdded(pPrivData->hHashManSolutions,hash))
      AddWaHash(pPrivData->hHashManSolutions,hash);

    MoveCreature(hEngineMonsterMale, pPrivData,NULL,&pPrivData->CurrentMonsterMalePos,NULL);
    CalculateMazeHash(hash,&pPrivData->CurrentManPos,&pPrivData->CurrentMonsterMalePos,&pPrivData->CurrentMonsterFemalePos);
    if(!IsWaHashAdded(pPrivData->hHashManSolutions,hash))
      AddWaHash(pPrivData->hHashManSolutions,hash);

    MoveCreature(hEngineMonsterFemale, pPrivData,NULL,&pPrivData->CurrentMonsterFemalePos,NULL);
    CalculateMazeHash(hash,&pPrivData->CurrentManPos,&pPrivData->CurrentMonsterMalePos,&pPrivData->CurrentMonsterFemalePos);
    if(!IsWaHashAdded(pPrivData->hHashManSolutions,hash))
      AddWaHash(pPrivData->hHashManSolutions,hash);

    PrintMaze(pPrivData,"maze.bmp");
  }

  DestroyWaHashManager(pPrivData->hHashManSolutions);
  DestroyWaRandManager(pPrivData->hRandMan);
  DestroyWaEngine(hEngineMan);
  DestroyWaEngine(hEngineMonsterMale);
  DestroyWaEngine(hEngineMonsterFemale);

  DESTROY_WORLD_ANALYSIS_INITIALIZATION_REQUERIMENTS(WAReq);

#ifdef INITIALIZE_DEBUG  
  DebugEnd();
#endif

  myfree((MEMBLOCK)pPrivData);


  CheckMemUsage();

  printf("\npress any key\n");

}

void main()
{
  submain();
}
