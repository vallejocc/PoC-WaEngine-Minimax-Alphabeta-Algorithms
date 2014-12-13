#include "../inc/pub/watypes.h"
#include "../inc/pub/waerror.h"
#include "../inc/pub/wamem.h"
#include "../inc/pub/wasnapshots.h"
#include "../inc/pub/waengine.h"
#include "../inc/priv/waenginepriv.h"
#include "../inc/priv/wamacros.h"
#include "../inc/priv/wadepthfirst.h"
#include "../inc/priv/waalphabeta.h"

/////////////////////////////////////////////////////////////////////////////
//CreateWaEngine:   public
//
//  Description:
//
//    This function will create a world analysis engine and will return a 
//  handle for using it.
//
//  Parameters:
//  
//    Requirements: requirements for initializing the engine.
//
//  Returned Values:
//   
//    A handle to the engine or NULL if a error occured.
//  
/////////////////////////////////////////////////////////////////////////////

WAEXPORT WORLD_ANALYSIS_ENGINE_HANDLE 
CreateWaEngine
(IN PWORLD_ANALYSIS_INITIALIZATION_REQUIREMENTS Requirements)
{
  PWORLD_ANALYSIS_ENGINE pWaEngine = WANULL;

  do
  {
    if(!Requirements->MemoryManager->alloc || 
      !Requirements->MemoryManager->free ||
      !Requirements->SnapshotsManager->Functions.CompareSnapshots ||
      !Requirements->SnapshotsManager->Functions.DupSnapshot ||
      !Requirements->SnapshotsManager->Functions.EvaluateSnapshot ||
      !Requirements->SnapshotsManager->Functions.GenerateSnapshotEvolutions ||
      !Requirements->SnapshotsManager->Functions.GetSnapshotHash ||
      !Requirements->SnapshotsManager->Functions.ReturnClientObject)
    {
      break;
    }

    pWaEngine = 
      (PWORLD_ANALYSIS_ENGINE)Requirements->MemoryManager->alloc(sizeof(WORLD_ANALYSIS_ENGINE));

    if(!pWaEngine)
      break;

    pWaEngine->MemoryManager = 
      (PWORLD_ANALYSIS_MEMORY_MANAGER)Requirements->MemoryManager->alloc(sizeof(WORLD_ANALYSIS_MEMORY_MANAGER));

    if(!pWaEngine->MemoryManager)
      break;

    pWaEngine->SnapshotsManager = 
      (PWORLD_SNAPSHOTS_MANAGER)Requirements->MemoryManager->alloc(sizeof(WORLD_SNAPSHOTS_MANAGER));

    if(!pWaEngine->SnapshotsManager)
      break;

    pWaEngine->MemoryManager->alloc =                                        //alloc
      Requirements->MemoryManager->alloc;
    pWaEngine->MemoryManager->free  =                                        //free
      Requirements->MemoryManager->free;
    pWaEngine->SnapshotsManager->Functions.CompareSnapshots =                //CompareSnapshots
      Requirements->SnapshotsManager->Functions.CompareSnapshots;            
    pWaEngine->SnapshotsManager->Functions.DupSnapshot =                     //DupSnapshot
      Requirements->SnapshotsManager->Functions.DupSnapshot;
    pWaEngine->SnapshotsManager->Functions.EvaluateSnapshot =                //EvaluateSnapshot
      Requirements->SnapshotsManager->Functions.EvaluateSnapshot;            
    pWaEngine->SnapshotsManager->Functions.GenerateSnapshotEvolutions =      //GenerateSnapshotEvolutions
      Requirements->SnapshotsManager->Functions.GenerateSnapshotEvolutions;
    pWaEngine->SnapshotsManager->Functions.GetSnapshotHash =                 //GetSnapshotHash
      Requirements->SnapshotsManager->Functions.GetSnapshotHash;
    pWaEngine->SnapshotsManager->Functions.ReturnClientObject =              //ReturnClientObject
      Requirements->SnapshotsManager->Functions.ReturnClientObject;  

    pWaEngine->SnapshotsManager->ClientParam =
      Requirements->SnapshotsManager->ClientParam;

    pWaEngine->SnapshotsManager->PrivatePtr = 
      Requirements->SnapshotsManager->PrivatePtr;

    pWaEngine->LastError = WA_SUCCESS;

    return (WORLD_ANALYSIS_ENGINE_HANDLE)pWaEngine;
  }
  while(0);

  if(pWaEngine && 
     pWaEngine->MemoryManager && 
     pWaEngine->MemoryManager->free)
  {
    NOTHING (*tempfree)(MEMBLOCK) = pWaEngine->MemoryManager->free;
    if(pWaEngine->SnapshotsManager)
      tempfree((MEMBLOCK)pWaEngine->SnapshotsManager);
    tempfree((MEMBLOCK)pWaEngine->MemoryManager);
    tempfree((MEMBLOCK)pWaEngine);
  }

  return (WORLD_ANALYSIS_ENGINE_HANDLE)WANULL;  
}

/////////////////////////////////////////////////////////////////////////////
//DestroyWaEngine:   public
//
//  Description:
//
//    This function will destroy a world analysis engine.
//
//  Parameters:
//  
//    hEngine: a handle to the engine to destroy.
//
//  Returned Values:
//   
//    None.
//  
/////////////////////////////////////////////////////////////////////////////

WAEXPORT NOTHING 
DestroyWaEngine
(IN WORLD_ANALYSIS_ENGINE_HANDLE hEngine)
{
  if(hEngine &&
     ((PWORLD_ANALYSIS_ENGINE)hEngine)->MemoryManager &&
     ((PWORLD_ANALYSIS_ENGINE)hEngine)->MemoryManager->free)
  {
    NOTHING (*tempfree)(MEMBLOCK) = 
      ((PWORLD_ANALYSIS_ENGINE)hEngine)->MemoryManager->free;
    
    tempfree((MEMBLOCK)((PWORLD_ANALYSIS_ENGINE)hEngine)->MemoryManager);
    tempfree((MEMBLOCK)((PWORLD_ANALYSIS_ENGINE)hEngine)->SnapshotsManager);
    tempfree((MEMBLOCK)hEngine);
  }
}

/////////////////////////////////////////////////////////////////////////////
//CreateWaAnalysisJob:   public
//
//  Description:
//
//    This function will create a world analysis job and it will return a 
//  handle to it. The created job starts stopped. It should be started with
//  RunWaAnalysisJob.
//
//  Parameters:
//  
//    hEngine: a handle to the wa engine. Over this engine the function will 
//  create the job.
//    StartupInfo: instructions for creating the job (type of job,etc...)
//
//  Returned Values:
//   
//    A handle to the created job, or NULL if a error occured. If a error
//  occured WaGetLastError will return a related error code.
//  
/////////////////////////////////////////////////////////////////////////////

WAEXPORT WORLD_ANALYSIS_JOB_HANDLE    
CreateWaAnalysisJob         
(IN WORLD_ANALYSIS_ENGINE_HANDLE hEngine,
 IN PWORLD_ANALYSIS_JOB_STARTUP_INFO StartupInfo)
{
  UL32 IsValidType = 1;
  UL32 SpecificInfoSize = 0;
  PWORLD_ANALYSIS_JOB pWaJob = WANULL;

  do
  {
    if(!hEngine ||
       !StartupInfo ||
       !((PWORLD_ANALYSIS_ENGINE)hEngine)->SnapshotsManager ||
       !((PWORLD_ANALYSIS_ENGINE)hEngine)->SnapshotsManager->Functions.CompareSnapshots ||
       !((PWORLD_ANALYSIS_ENGINE)hEngine)->SnapshotsManager->Functions.DupSnapshot ||
       !((PWORLD_ANALYSIS_ENGINE)hEngine)->SnapshotsManager->Functions.EvaluateSnapshot ||
       !((PWORLD_ANALYSIS_ENGINE)hEngine)->SnapshotsManager->Functions.GenerateSnapshotEvolutions ||
       !((PWORLD_ANALYSIS_ENGINE)hEngine)->SnapshotsManager->Functions.GetSnapshotHash ||
       !((PWORLD_ANALYSIS_ENGINE)hEngine)->SnapshotsManager->Functions.ReturnClientObject ||
       !((PWORLD_ANALYSIS_ENGINE)hEngine)->MemoryManager ||
       !((PWORLD_ANALYSIS_ENGINE)hEngine)->MemoryManager->alloc ||
       !((PWORLD_ANALYSIS_ENGINE)hEngine)->MemoryManager->free)
    {
      break;
    }

    switch(StartupInfo->Type)
    {
      case WORLD_ANALYSIS_JOB_ALPHA_BETA:
        SpecificInfoSize=sizeof(WORLD_ANALYSIS_JOB_ALPHA_BETA_SPECIFIC_STARTUP_INFO);
        break;

      case WORLD_ANALYSIS_JOB_DEPTH_FIRST:
        SpecificInfoSize=sizeof(WORLD_ANALYSIS_JOB_DEPTH_FIRST_SPECIFIC_STARTUP_INFO);
        break;

      case WORLD_ANALYSIS_JOB_BREADTH_FIRST:
        SpecificInfoSize=sizeof(WORLD_ANALYSIS_JOB_BREADTH_FIRST_SPECIFIC_STARTUP_INFO);
        break;

      default:
        IsValidType=0;
        break;
    }

    if(!IsValidType)
      break;

    pWaJob = (PWORLD_ANALYSIS_JOB)((PWORLD_ANALYSIS_ENGINE)hEngine)->MemoryManager->alloc(sizeof(WORLD_ANALYSIS_JOB)+SpecificInfoSize);

    if(!pWaJob)
      break;

    pWaJob->JobResults.nSnapshots=0;
    pWaJob->JobResults.Snapshots=WANULL;

    pWaJob->pWaEngine = (PWORLD_ANALYSIS_ENGINE)hEngine;
    pWaJob->JobType = StartupInfo->Type;
    pWaJob->Start = (PWORLD_STATUS)pWaJob->pWaEngine->MemoryManager->alloc(sizeof(WORLD_STATUS));
    
    if(!pWaJob->Start)
      break;
    
    pWaJob->Start->Parent = WANULL;
    WaInitList(&pWaJob->Start->Link);
    pWaJob->Start->nEvolutions = 0;
    pWaJob->Start->Evaluation = 0;
    pWaJob->Start->BestInBranch = 0;
    pWaJob->Start->SnapShot = StartupInfo->StartSnapshot;
    pWaJob->Start->SnapshotEvolutions = WANULL;
    pWaJob->Start->WorldStatusEvolutions = WANULL;
    WaInitList(&pWaJob->ListHead);

    return pWaJob;
  } 
  while(0);

  if(pWaJob && 
     hEngine &&
     ((PWORLD_ANALYSIS_ENGINE)hEngine)->MemoryManager &&
     ((PWORLD_ANALYSIS_ENGINE)hEngine)->MemoryManager->free)
  {
    if(pWaJob->Start)
    {
      ((PWORLD_ANALYSIS_ENGINE)hEngine)->MemoryManager->free((MEMBLOCK)pWaJob->Start);
    }
    ((PWORLD_ANALYSIS_ENGINE)hEngine)->MemoryManager->free((MEMBLOCK)pWaJob);
  }

  return (WORLD_ANALYSIS_JOB_HANDLE)WANULL;
}

/////////////////////////////////////////////////////////////////////////////
//RunWaAnalysisJob:   public
//
//  Description:
//
//    This function will start the execution of a created job.
//
//  Parameters:
//  
//    hJob: a handle to the job to be started.
//
//  Returned Values:
//   
//    A error code.
//  
/////////////////////////////////////////////////////////////////////////////

WAEXPORT WAERROR RunWaAnalysisJob
(IN WORLD_ANALYSIS_JOB_HANDLE hJob)
{
  switch(((PWORLD_ANALYSIS_JOB)hJob)->JobType)
  {
    case WORLD_ANALYSIS_JOB_ALPHA_BETA:
      return RunWaAlphaBetaAnalysisJob(hJob);
        
    case WORLD_ANALYSIS_JOB_DEPTH_FIRST:
      return RunWaDepthFirstAnalysisJob(hJob);      

    case WORLD_ANALYSIS_JOB_BREADTH_FIRST:
      return WA_ERROR_NOT_SUPPORTED;

    default:
      return WA_ERROR_INVALID_PARAMETER;
  }
}

/////////////////////////////////////////////////////////////////////////////
//StopWaAnalysisJob:   public
//
//  Description:
//
//    This function will stop the execution of a created and started job. If the job 
//  wasnt started the function will do nothing.
//
//  Parameters:
//  
//    hJob: a handle to the job to be stopped.
//
//  Returned Values:
//   
//    A error code.
//  
/////////////////////////////////////////////////////////////////////////////

WAEXPORT WAERROR 
StopWaAnalysisJob
(IN WORLD_ANALYSIS_JOB_HANDLE hJob)
{
  return WA_ERROR_NOT_SUPPORTED;
}

/////////////////////////////////////////////////////////////////////////////
//GetWaAnalysisJobResults:   public
//
//  Description:
//
//    This function will return the results of a finished job. 
//
//  Parameters:
//  
//    hJob: a handle to the job to be queried.
//
//  Returned Values:
//   
//    The finished job results. If a error occured or the job is not 
//  finished, the function returns NULL and WaGetLastError returns a
//  related error code.
//  
/////////////////////////////////////////////////////////////////////////////

WAEXPORT PWORLD_ANALYSIS_JOB_RESULTS  
GetWaAnalysisJobResults     
(IN WORLD_ANALYSIS_JOB_HANDLE hJob)
{
  PWORLD_ANALYSIS_JOB pWaJob = (PWORLD_ANALYSIS_JOB)hJob;
  UL32 nResultingSnapshots=1;
  PWORLD_STATUS pTempWStatus;

  if(!pWaJob)
    return WANULL;

  pTempWStatus = pWaJob->Start;

  while(pTempWStatus->nEvolutions)
  {
    nResultingSnapshots++;
    pTempWStatus=&pTempWStatus->WorldStatusEvolutions[0];      
  }
  
  pWaJob->JobResults.nSnapshots=nResultingSnapshots;
    pWaJob->JobResults.Snapshots=
      (PWORLD_SNAPSHOT)pWaJob->pWaEngine->MemoryManager->alloc
        (sizeof(WORLD_SNAPSHOT)*nResultingSnapshots);

  if(!pWaJob->JobResults.Snapshots)
    return WANULL;

  pTempWStatus = pWaJob->Start;

  nResultingSnapshots=0;

  pWaJob->JobResults.Snapshots[nResultingSnapshots]=
    pTempWStatus->SnapShot;

  while(pTempWStatus->nEvolutions)
  {
    pTempWStatus=&pTempWStatus->WorldStatusEvolutions[0];
    nResultingSnapshots++;
    pWaJob->JobResults.Snapshots[nResultingSnapshots]=
      pTempWStatus->SnapShot;    
  }

  return &pWaJob->JobResults;
}

/////////////////////////////////////////////////////////////////////////////
//DestroyWaAnalysisJob:   public
//
//  Description:
//
//    This function will destroy a job.
//
//  Parameters:
//  
//    hJob: a handle to the job to be destroyed.
//
//  Returned Values:
//   
//    None.
//  
/////////////////////////////////////////////////////////////////////////////

WAEXPORT NOTHING 
DestroyWaAnalysisJob       
(IN WORLD_ANALYSIS_JOB_HANDLE hJob)
{

  switch(((PWORLD_ANALYSIS_JOB)hJob)->JobType)
  {
    case WORLD_ANALYSIS_JOB_ALPHA_BETA:
      FreeWaAlphaBetaBranch(hJob,((PWORLD_ANALYSIS_JOB)hJob)->Start);
      break;

    case WORLD_ANALYSIS_JOB_DEPTH_FIRST:
      FreeWaDepthFirstBranch(hJob,((PWORLD_ANALYSIS_JOB)hJob)->Start);
      break;

    case WORLD_ANALYSIS_JOB_BREADTH_FIRST:
      break;
    
    default:
      break;;
  }
  
  if(((PWORLD_ANALYSIS_JOB)hJob)->JobResults.Snapshots)
    ((PWORLD_ANALYSIS_JOB)hJob)->pWaEngine->MemoryManager->free((MEMBLOCK)((PWORLD_ANALYSIS_JOB)hJob)->JobResults.Snapshots);
  ((PWORLD_ANALYSIS_JOB)hJob)->pWaEngine->MemoryManager->free((MEMBLOCK)((PWORLD_ANALYSIS_JOB)hJob)->Start);
  ((PWORLD_ANALYSIS_JOB)hJob)->pWaEngine->MemoryManager->free((MEMBLOCK)hJob);  
}

/////////////////////////////////////////////////////////////////////////////
//WaGetLastError:   public
//
//  Description:
//
//    Some engine's functions will update a internal variable containing error
//  code of a previous operation.
//
//  Parameters:
//  
//    hEngine: a handle to the engine.
//
//  Returned Values:
//   
//    The last error code set in the error variable of the given engine.
//  
/////////////////////////////////////////////////////////////////////////////

WAEXPORT WAERROR 
WaGetLastError
(IN WORLD_ANALYSIS_ENGINE_HANDLE hEngine)
{
  if(hEngine)
    return ((PWORLD_ANALYSIS_ENGINE)hEngine)->LastError;

  return WA_ERROR_INVALID_PARAMETER;
}

/////////////////////////////////////////////////////////////////////////////
//WaSetLastError:   private
//
//  Description:
//
//    Some engine's functions will update a internal variable containing error
//  code of a previous operation.
//
//  Parameters:
//  
//    hEngine: a handle to the engine.
//    LastError: the error to set.
//
//  Returned Values:
//   
//    None.
//  
/////////////////////////////////////////////////////////////////////////////

NOTHING 
WaSetLastError
(IN WORLD_ANALYSIS_ENGINE_HANDLE hEngine, WAERROR LastError)
{
  if(hEngine)
    ((PWORLD_ANALYSIS_ENGINE)hEngine)->LastError = LastError;
}