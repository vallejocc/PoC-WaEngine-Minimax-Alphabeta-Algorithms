////////////////////////////////////////////////////////////////////////////////////////////////////
//waengine.h
//public header

#ifndef __WORLD_ANALYSIS_ENGINE_H__
#define __WORLD_ANALYSIS_ENGINE_H__

#include "watypes.h"
#include "wasnapshots.h"
#include "wamem.h"
#include "waerror.h"

#ifdef WA_ENGINE_COMPILE_DLL
#define WAEXPORT __declspec(dllexport)
#else
#define WAEXPORT
#endif

#pragma pack(push,1)

typedef void * WORLD_ANALYSIS_ENGINE_HANDLE;

typedef struct _WORLD_ANALYSIS_INITIALIZATION_REQUIREMENTS
{
  WORLD_SNAPSHOTS_MANAGER       * SnapshotsManager;
  WORLD_ANALYSIS_MEMORY_MANAGER * MemoryManager;

} WORLD_ANALYSIS_INITIALIZATION_REQUIREMENTS,
  *PWORLD_ANALYSIS_INITIALIZATION_REQUIREMENTS;

typedef void * WORLD_ANALYSIS_JOB_HANDLE;

typedef enum _WORLD_ANALYSIS_JOB_TYPE
{
  WORLD_ANALYSIS_JOB_DEPTH_FIRST=1,
  WORLD_ANALYSIS_JOB_BREADTH_FIRST,
  WORLD_ANALYSIS_JOB_ALPHA_BETA

}WORLD_ANALYSIS_JOB_TYPE;

typedef struct _WORLD_ANALYSIS_JOB_DEPTH_FIRST_SPECIFIC_STARTUP_INFO
{
  int pad;

}WORLD_ANALYSIS_JOB_DEPTH_FIRST_SPECIFIC_STARTUP_INFO,
 *PWORLD_ANALYSIS_JOB_DEPTH_FIRST_SPECIFIC_STARTUP_INFO;

typedef struct _WORLD_ANALYSIS_JOB_BREADTH_FIRST_SPECIFIC_STARTUP_INFO
{
  int pad;

}WORLD_ANALYSIS_JOB_BREADTH_FIRST_SPECIFIC_STARTUP_INFO,
 *PWORLD_ANALYSIS_JOB_BREADTH_FIRST_SPECIFIC_STARTUP_INFO;

typedef struct _WORLD_ANALYSIS_JOB_ALPHA_BETA_SPECIFIC_STARTUP_INFO
{
  int pad;

}WORLD_ANALYSIS_JOB_ALPHA_BETA_SPECIFIC_STARTUP_INFO,
 *PWORLD_ANALYSIS_JOB_ALPHA_BETA_SPECIFIC_STARTUP_INFO;


typedef struct _WORLD_ANALYSIS_JOB_STARTUP_INFO
{
  WORLD_ANALYSIS_JOB_TYPE  Type;
  WORLD_SNAPSHOT           StartSnapshot;    
  UC8                      SpecificInfoByType[0];

}WORLD_ANALYSIS_JOB_STARTUP_INFO,
 *PWORLD_ANALYSIS_JOB_STARTUP_INFO;

typedef struct _WORLD_ANALYSIS_JOB_RESULTS
{
  UL32 nSnapshots;
  WORLD_SNAPSHOT *Snapshots;

}WORLD_ANALYSIS_JOB_RESULTS,*PWORLD_ANALYSIS_JOB_RESULTS;


WAEXPORT WORLD_ANALYSIS_ENGINE_HANDLE CreateWaEngine              (IN PWORLD_ANALYSIS_INITIALIZATION_REQUIREMENTS);
WAEXPORT NOTHING                      DestroyWaEngine             (IN WORLD_ANALYSIS_ENGINE_HANDLE);
WAEXPORT WORLD_ANALYSIS_JOB_HANDLE    CreateWaAnalysisJob         (IN WORLD_ANALYSIS_ENGINE_HANDLE,IN PWORLD_ANALYSIS_JOB_STARTUP_INFO);
WAEXPORT WAERROR                      RunWaAnalysisJob            (IN WORLD_ANALYSIS_JOB_HANDLE);
WAEXPORT WAERROR                      StopWaAnalysisJob           (IN WORLD_ANALYSIS_JOB_HANDLE);
WAEXPORT PWORLD_ANALYSIS_JOB_RESULTS  GetWaAnalysisJobResults     (IN WORLD_ANALYSIS_JOB_HANDLE);
WAEXPORT NOTHING                      DestroyWaAnalysisJob        (IN WORLD_ANALYSIS_JOB_HANDLE);
WAEXPORT WAERROR                      WaGetLastError              (IN WORLD_ANALYSIS_ENGINE_HANDLE);


//helper macros

#define CREATE_WORLD_ANALYSIS_INITIALIZATION_REQUERIMENTS(a,b,c,d,e,f,g,h,i,j,k) \
        { \
        WORLD_SNAPSHOTS_MANAGER * a##__SnapshotManager; \
        WORLD_ANALYSIS_MEMORY_MANAGER * a##__MemoryManager; \
        a = (WORLD_ANALYSIS_INITIALIZATION_REQUIREMENTS*)j(sizeof(WORLD_ANALYSIS_INITIALIZATION_REQUIREMENTS)); \
        a##__SnapshotManager = (WORLD_SNAPSHOTS_MANAGER*)j(sizeof(WORLD_SNAPSHOTS_MANAGER)); \
        a##__MemoryManager = (WORLD_ANALYSIS_MEMORY_MANAGER*)j(sizeof(WORLD_ANALYSIS_MEMORY_MANAGER)); \
        INITIALIZE_SNAPSHOT_MANAGER((*a##__SnapshotManager),b,c,d,e,f,g,h,i); \
        INITIALIZE_MEMORY_MANAGER((*a##__MemoryManager),j,k); \
        a->SnapshotsManager=a##__SnapshotManager; \
        a->MemoryManager=a##__MemoryManager; \
        }
        
#define DESTROY_WORLD_ANALYSIS_INITIALIZATION_REQUERIMENTS(a) \
        { \
        void (*a##__tempfree)(MEMBLOCK)=a->MemoryManager->free; \
        a##__tempfree((MEMBLOCK)a->SnapshotsManager); \
        a##__tempfree((MEMBLOCK)a->MemoryManager); \
        a##__tempfree((MEMBLOCK)a); \
        }

#pragma pack(pop)

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////