#include "../inc/pub/wahash.h"
#include "../inc/pub/watypes.h"
#include "../inc/pub/waerror.h"
#include "../inc/pub/waengine.h"
#include "../inc/priv/waenginepriv.h"

#define STARTING_HASHES_IN_HASH_TABLE_ENTRY 100

typedef UC8
  REDUCED_WAHASH[18];

typedef struct _HASH_TABLE_ENTRY
{
  UL32 CurHash;
  UL32 MaxHashes;
  REDUCED_WAHASH *Hashes;
}HASH_TABLE_ENTRY,
 *PHASH_TABLE_ENTRY;

typedef struct _WAHASH_MANAGER
{
  PWORLD_ANALYSIS_ENGINE pWaEngine;
  UC8 PreHashTable[256][256][256];
  HASH_TABLE_ENTRY HashTable[256][256];
}WAHASH_MANAGER, 
 *PWAHASH_MANAGER;

WAHASH_MANAGER_HANDLE CreateWaHashManager(WORLD_ANALYSIS_ENGINE_HANDLE hEngine)
{
  PWORLD_ANALYSIS_ENGINE pWaEngine = hEngine;
  PWAHASH_MANAGER pWaHashManager = WANULL;
  
  if(!hEngine)
    return WANULL;

  do
  {
    pWaHashManager = (PWAHASH_MANAGER)pWaEngine->MemoryManager->alloc(sizeof(WAHASH_MANAGER));

    {MEMSZ i;for(i=0;i<sizeof(WAHASH_MANAGER);i++)((UC8*)pWaHashManager)[i]=0;}

    if(!pWaHashManager)
      break;
    
    pWaHashManager->pWaEngine = pWaEngine;
    return (WAHASH_MANAGER_HANDLE)pWaHashManager;
  }
  while(0);

  if(pWaHashManager)
    pWaEngine->MemoryManager->free((MEMBLOCK)pWaHashManager);
  
  return WANULL;
}

NOTHING DestroyWaHashManager(IN WAHASH_MANAGER_HANDLE hWaHashMan)
{
  UL32 i,j;
  PWAHASH_MANAGER pWaHashManager = (PWAHASH_MANAGER)hWaHashMan;
  
  if(!hWaHashMan)
    return;

  for(i=0;i<256;i++)
  {
    for(j=0;j<256;j++)
    {
      if(pWaHashManager->HashTable[i][j].MaxHashes &&
         pWaHashManager->HashTable[i][j].Hashes)
        pWaHashManager->pWaEngine->MemoryManager->free
          ((MEMBLOCK)pWaHashManager->HashTable[i][j].Hashes);
    }
  }
  
  pWaHashManager->pWaEngine->MemoryManager->free
          ((MEMBLOCK)pWaHashManager);
}

WABOOL IsWaHashAdded(IN WAHASH_MANAGER_HANDLE hWaHashMan,IN WAHASH WaHash)
{
  PWAHASH_MANAGER pWaHashManager = (PWAHASH_MANAGER)hWaHashMan;
  
  if(!hWaHashMan)
    return WAFALSE;

  if(pWaHashManager->PreHashTable[WaHash[0]][WaHash[1]][WaHash[2]])
  {
    UL32 i;
    for(i=0;i<pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash;i++)
    {
      // TODO
      //optimize these lines...
      if(pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes[i][0]==WaHash[0+2] &&
         pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes[i][1]==WaHash[1+2] &&
         pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes[i][2]==WaHash[2+2] &&
         pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes[i][3]==WaHash[3+2] &&
         pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes[i][4]==WaHash[4+2] &&
         pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes[i][5]==WaHash[5+2] &&
         pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes[i][6]==WaHash[6+2] &&
         pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes[i][7]==WaHash[7+2] &&
         pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes[i][8]==WaHash[8+2] &&
         pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes[i][9]==WaHash[9+2] &&
         pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes[i][10]==WaHash[10+2] &&
         pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes[i][11]==WaHash[11+2] &&
         pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes[i][12]==WaHash[12+2] &&
         pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes[i][13]==WaHash[13+2] &&
         pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes[i][14]==WaHash[14+2] &&
         pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes[i][15]==WaHash[15+2] &&
         pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes[i][16]==WaHash[16+2] &&
         pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes[i][17]==WaHash[17+2])
      {
        return WATRUE;
      }
    }
  }  

  return WAFALSE;
}

WAERROR AddWaHash(IN WAHASH_MANAGER_HANDLE hWaHashMan,IN WAHASH WaHash)
{
  WAERROR RetVal = WA_SUCCESS;
  PWAHASH_MANAGER pWaHashManager = (PWAHASH_MANAGER)hWaHashMan;
  
  if(!hWaHashMan)
    return WA_ERROR_INVALID_PARAMETER;

  if(IsWaHashAdded(hWaHashMan,WaHash))
    return WA_SUCCESS;

  do
  {
    if(!pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes)
    {
      if(!(pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes=(REDUCED_WAHASH*)
            pWaHashManager->pWaEngine->MemoryManager->alloc
              (sizeof(REDUCED_WAHASH)*STARTING_HASHES_IN_HASH_TABLE_ENTRY)))
      {
        RetVal=WA_ERROR_MEMORY;
        break;
      }
      pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash=0;
      pWaHashManager->HashTable[WaHash[0]][WaHash[1]].MaxHashes=STARTING_HASHES_IN_HASH_TABLE_ENTRY;
    }
    else if(pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash==
            pWaHashManager->HashTable[WaHash[0]][WaHash[1]].MaxHashes)
    {
      UNKOBJ tempptr=pWaHashManager->pWaEngine->MemoryManager->alloc
                      (sizeof(REDUCED_WAHASH)*
                       (pWaHashManager->HashTable[WaHash[0]][WaHash[1]].MaxHashes+
                        STARTING_HASHES_IN_HASH_TABLE_ENTRY));
      if(!tempptr)
      {
        RetVal=WA_ERROR_MEMORY;
        break;
      }

      {
        MEMSZ i;
        for(i=0;
        i<(sizeof(REDUCED_WAHASH)*pWaHashManager->HashTable[WaHash[0]][WaHash[1]].MaxHashes);
        i++)
          ((UC8*)tempptr)[i]=(((UC8*)pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes)[i]);

        pWaHashManager->HashTable[WaHash[0]][WaHash[1]].MaxHashes+=
           STARTING_HASHES_IN_HASH_TABLE_ENTRY;
      }

      pWaHashManager->pWaEngine->MemoryManager->free
        ((MEMBLOCK)pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes);
      
      pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes=(REDUCED_WAHASH*)tempptr;
    }

    // TODO
    //optimize these lines...
    pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes
      [pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash][0]=WaHash[0+2];
    pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes
      [pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash][1]=WaHash[1+2];
    pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes
      [pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash][2]=WaHash[2+2];
    pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes
      [pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash][3]=WaHash[3+2];
    pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes
      [pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash][4]=WaHash[4+2];
    pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes
      [pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash][5]=WaHash[5+2];
    pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes
      [pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash][6]=WaHash[6+2];
    pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes
      [pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash][7]=WaHash[7+2];
    pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes
      [pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash][8]=WaHash[8+2];
    pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes
      [pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash][9]=WaHash[9+2];
    pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes
      [pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash][10]=WaHash[10+2];
    pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes
      [pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash][11]=WaHash[11+2];
    pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes
      [pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash][12]=WaHash[12+2];
    pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes
      [pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash][13]=WaHash[13+2];
    pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes
      [pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash][14]=WaHash[14+2];
    pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes
      [pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash][15]=WaHash[15+2];
    pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes
      [pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash][16]=WaHash[16+2];
    pWaHashManager->HashTable[WaHash[0]][WaHash[1]].Hashes
      [pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash][17]=WaHash[17+2];

    pWaHashManager->PreHashTable[WaHash[0]][WaHash[1]][WaHash[2]]=1;
    pWaHashManager->HashTable[WaHash[0]][WaHash[1]].CurHash++;
    
    return RetVal;
  }
  while(0);

  return RetVal;
}
