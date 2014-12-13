#include "../inc/pub/watypes.h"
#include "../inc/pub/waerror.h"
#include "../inc/pub/waengine.h"
#include "../inc/priv/wamacros.h"
#include "../inc/pub/waengine.h"
#include "../inc/priv/waenginepriv.h"
#include "../inc/pub/wadebug.h"

struct _DebugSnapshot;

/*
Esbozo del algoritmo de búsqueda en el árbol:

1. Lista L <- nodo raíz.
2. Si L vacía, stop.Sino, n <- obten-sin-extraer-primero( L ).
3. Si n tiene sucesores generados:
   3.1. n <- extraer-primero( L ).
   3.2. Eliminar todas las evoluciones excepto la que mejor evaluacion tenga.
   3.3. Si la evaluación de n es mejor que la evaluación de su mejor sucesor, eliminar dicho sucesor.
4. Si n no tiene sucesores generados:
  4.1.Evaluar n.
  4.2.Generar sucesores. 
    4.2.1.Si se generaron sucesores añadir al principio de L todos los sucesores de n. 
    4.2.2.Sino extraer-primero( L ).
5.Ir al paso 2.
*/

NOTHING FreeWaDepthFirstBranch(PWORLD_ANALYSIS_JOB pWaJob,PWORLD_STATUS pWaStatus)
{
  UL32 i;
  WALIST_ENTRY TempListHead;
  PWORLD_STATUS pTempStatus=pWaStatus;
  PWALIST_ENTRY TempEntry;
  
  if(!pWaStatus->nEvolutions)
    return;

  WaInitList(&TempListHead);
  WaPostInsertEntry(&TempListHead,&pWaStatus->Link);
  TempEntry=&pWaStatus->Link;
  
  do
  {
    pTempStatus=BASE_FROM_FIELD(WORLD_STATUS,Link,TempEntry);

    if(pTempStatus->nEvolutions)
    {
      for(i=0;i<pTempStatus->nEvolutions;i++)
      {
        WaPreInsertEntry(&TempListHead,&pTempStatus->WorldStatusEvolutions[i].Link);
      }
    }

    TempEntry=TempEntry->Flink;
  }
  while(TempEntry!=&TempListHead);

  TempEntry=TempListHead.Blink;

  while(TempEntry!=&TempListHead)
  {
    pTempStatus=BASE_FROM_FIELD(WORLD_STATUS,Link,TempEntry);
    if(pTempStatus->nEvolutions)
    {
      pTempStatus->nEvolutions=0;
      pWaJob->pWaEngine->MemoryManager->free((MEMBLOCK)pTempStatus->WorldStatusEvolutions);
      pWaJob->pWaEngine->SnapshotsManager->Functions.ReturnClientObject
        (pWaJob->pWaEngine->SnapshotsManager,
        pTempStatus->SnapshotEvolutions,
        pWaJob->pWaEngine->SnapshotsManager->ClientParam,
        WANULL);
    }
    TempEntry=TempEntry->Blink;
  }
}

WAERROR RunWaDepthFirstAnalysisJob
(IN WORLD_ANALYSIS_JOB_HANDLE hJob)
{
  PWALIST_ENTRY pTempEntry;
  PWORLD_STATUS pCurStatus;
  WAERROR RetVal=WA_ERROR_UNKNOWN;
  PWORLD_ANALYSIS_JOB pWaJob = hJob;

  WaInitList(&pWaJob->ListHead);
  WaPostInsertEntry(&pWaJob->ListHead,&pWaJob->Start->Link);

  while(!WaIsListEmpty(&pWaJob->ListHead))
  {
    pTempEntry=pWaJob->ListHead.Flink;
    pCurStatus=BASE_FROM_FIELD(WORLD_STATUS,Link,pTempEntry);

    if(pCurStatus->nEvolutions)
    {
      //we have evaluated this node previously. Now we must see what children
      //is the best.

      UL32 i;
      UL32 BestOne=0;

      WaExtractEntry(&pCurStatus->Link);

      if(pCurStatus->nEvolutions>1)
      {
        for(i=1;i<pCurStatus->nEvolutions;i++)
        {
          if(pCurStatus->WorldStatusEvolutions[BestOne].BestInBranch < 
            pCurStatus->WorldStatusEvolutions[i].BestInBranch)
          {
            unsigned int preevols=pCurStatus->WorldStatusEvolutions[BestOne].nEvolutions;
            FreeWaDepthFirstBranch(pWaJob,&pCurStatus->WorldStatusEvolutions[BestOne]);
            BestOne=i;
            if(preevols)
              ShowJob(hJob);
          }
          else
          {
            unsigned int preevols=pCurStatus->WorldStatusEvolutions[i].nEvolutions;
            FreeWaDepthFirstBranch(pWaJob,&pCurStatus->WorldStatusEvolutions[i]);
            if(preevols)
              ShowJob(hJob);
          }
        }
      }

      if(pCurStatus->BestInBranch >=
         pCurStatus->WorldStatusEvolutions[BestOne].BestInBranch)
      {
        FreeWaDepthFirstBranch(pWaJob,&pCurStatus->WorldStatusEvolutions[BestOne]);
      }
      else
      {
        WORLD_STATUS temp;
        temp=pCurStatus->WorldStatusEvolutions[0];
        pCurStatus->WorldStatusEvolutions[0] = pCurStatus->WorldStatusEvolutions[BestOne];
        pCurStatus->WorldStatusEvolutions[BestOne]=temp; //if we lose the WORLD_STATUS in the
                                                         //position zero we will have problems freing
        pCurStatus->BestInBranch =
          pCurStatus->WorldStatusEvolutions[0].BestInBranch;  
      }
      ShowJob(hJob);
    }
    else
    {
      pCurStatus->Evaluation = 
        pWaJob->pWaEngine->SnapshotsManager->Functions.EvaluateSnapshot
        (pWaJob->pWaEngine->SnapshotsManager,
         pCurStatus->SnapShot,
         pWaJob->pWaEngine->SnapshotsManager->ClientParam,
         WANULL);

      pCurStatus->BestInBranch = pCurStatus->Evaluation;

      pCurStatus->nEvolutions = 
        pWaJob->pWaEngine->SnapshotsManager->Functions.GenerateSnapshotEvolutions
        (pWaJob->pWaEngine->SnapshotsManager,
         pCurStatus->SnapShot,
         &pCurStatus->SnapshotEvolutions,
         pWaJob->pWaEngine->SnapshotsManager->ClientParam,
         WANULL);

      if(pCurStatus->nEvolutions)
      {
        //The current status has evolutions, we must go down in the tree

        pCurStatus->WorldStatusEvolutions = 
          (PWORLD_STATUS)pWaJob->pWaEngine->MemoryManager->alloc
          (sizeof(WORLD_STATUS)*pCurStatus->nEvolutions);

        if(pCurStatus->WorldStatusEvolutions)
        {
          UL32 i;
          for(i=0;i<pCurStatus->nEvolutions;i++)
          {
            pCurStatus->WorldStatusEvolutions[i].BestInBranch=0;
            pCurStatus->WorldStatusEvolutions[i].Evaluation=0;
            pCurStatus->WorldStatusEvolutions[i].nEvolutions=0;
            pCurStatus->WorldStatusEvolutions[i].Parent=pCurStatus;
            pCurStatus->WorldStatusEvolutions[i].SnapShot=
              pCurStatus->SnapshotEvolutions->Snapshots[i];
            pCurStatus->WorldStatusEvolutions[i].SnapshotEvolutions=WANULL;
            pCurStatus->WorldStatusEvolutions[i].WorldStatusEvolutions=WANULL;
          }

          while(i)
          {
            i--;
            WaPostInsertEntry(&pWaJob->ListHead,&pCurStatus->WorldStatusEvolutions[i].Link);
          }

          ShowJob(hJob);
        }
        else
        {
          pWaJob->pWaEngine->SnapshotsManager->Functions.ReturnClientObject
            (pWaJob->pWaEngine->SnapshotsManager,
			       pCurStatus->SnapshotEvolutions,
             pWaJob->pWaEngine->SnapshotsManager->ClientParam,
			       WANULL);
          pCurStatus->nEvolutions=0;

          //It was a resources problem and we will manage this node as a final node
          WaExtractEntry(&pCurStatus->Link);
        }        
      }
      else
      {
        //The current status is a final node of the tree
        WaExtractEntry(&pCurStatus->Link);
      }      
    }
  }

  return RetVal;
}