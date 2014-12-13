////////////////////////////////////////////////////////////////////////////////////////////////////
//wamem.h
//public header

#ifndef __WORLD_ANALYSIS_MEMORY_MANAGER_H__
#define __WORLD_ANALYSIS_MEMORY_MANAGER_H__

#include "watypes.h"

#pragma pack(push,1)

typedef struct _WORLD_ANALYSIS_MEMORY_MANAGER
{
  MEMBLOCK (*alloc) (MEMSZ);
  NOTHING  (*free)  (MEMBLOCK);

} WORLD_ANALYSIS_MEMORY_MANAGER, 
  *PWORLD_ANALYSIS_MEMORY_MANAGER;

#define INITIALIZE_MEMORY_MANAGER(a,b,c) \
    a.alloc=b; \
    a.free=c;

#pragma pack(pop)

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////