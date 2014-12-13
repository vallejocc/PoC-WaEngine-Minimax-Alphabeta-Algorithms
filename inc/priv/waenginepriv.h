////////////////////////////////////////////////////////////////////////////////////////////////////
//waenginepriv.h
//private header

#ifndef __WORLD_ANALYSIS_ENGINE_PRIVATE_H__
#define __WORLD_ANALYSIS_ENGINE_PRIVATE_H__

#include "../pub/watypes.h"
#include "../pub/wasnapshots.h"
#include "../pub/wamem.h"
#include "../pub/waerror.h"
#include "../pub/waengine.h"

#pragma pack(push,1)

typedef struct _WORLD_STATUS
{   
    struct _WORLD_STATUS*      Parent;
    WALIST_ENTRY               Link;
    WORLD_SNAPSHOT             SnapShot;
    WORLD_SNAPSHOT_EVALUATION  Evaluation;
    WORLD_SNAPSHOT_EVALUATION  BestInBranch;
    UL32                       nEvolutions;
    WORLD_SNAPSHOTS_GROUP*     SnapshotEvolutions;
    struct _WORLD_STATUS*      WorldStatusEvolutions;
#ifdef TREE_DEBUG_ENABLE
    WALIST_ENTRY               DebugLink;
#endif

} WORLD_STATUS,*PWORLD_STATUS;

typedef struct _WORLD_ANALYSIS_ENGINE
{
  WORLD_SNAPSHOTS_MANAGER       * SnapshotsManager;
  WORLD_ANALYSIS_MEMORY_MANAGER * MemoryManager;
  WAERROR                         LastError;

} WORLD_ANALYSIS_ENGINE,*PWORLD_ANALYSIS_ENGINE;

typedef struct _WORLD_ANALYSIS_JOB
{
  PWORLD_ANALYSIS_ENGINE      pWaEngine;
  WORLD_ANALYSIS_JOB_TYPE     JobType;
  PWORLD_STATUS               Start;
  WORLD_ANALYSIS_JOB_RESULTS  JobResults;
  WALIST_ENTRY                ListHead;
  UC8                         SpecificInfoByType[0];

} WORLD_ANALYSIS_JOB, *PWORLD_ANALYSIS_JOB;


NOTHING WaSetLastError(IN WORLD_ANALYSIS_ENGINE_HANDLE hEngine, WAERROR LastError);

#pragma pack(pop)

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////