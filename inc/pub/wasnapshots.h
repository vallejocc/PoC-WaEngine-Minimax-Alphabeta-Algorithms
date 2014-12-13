////////////////////////////////////////////////////////////////////////////////////////////////////
//wasnapshots.h
//public header

#ifndef __WORLD_SNAPSHOTS_H__
#define __WORLD_SNAPSHOTS_H__

#include "watypes.h"

#pragma pack(push,1)

typedef UNKOBJ 
  WORLD_SNAPSHOT, 
  *PWORLD_SNAPSHOT;

typedef struct _WORLD_SNAPSHOTS_GROUP{
  UL32 nSnapshots;
  WORLD_SNAPSHOT Snapshots[1];
} WORLD_SNAPSHOTS_GROUP,  
  *PWORLD_SNAPSHOTS_GROUP;

typedef UL32   
  WORLD_SNAPSHOT_EVALUATION;

typedef SL32             
  WORLD_SNAPSHOT_COMPARISON_RESULT;

typedef UC8
  WORLD_SNAPSHOT_HASH[20];

typedef struct _WORLD_SNAPSHOTS_MANAGER
                WORLD_SNAPSHOTS_MANAGER,
                *PWORLD_SNAPSHOTS_MANAGER;

typedef void *
  WORLD_SNAPSHOT_CLIENT_PARAM;

typedef struct _WORLD_SNAPSHOT_MANAGING_FUNCTIONS
{
    WORLD_SNAPSHOT                   (*DupSnapshot)                (IN PWORLD_SNAPSHOTS_MANAGER,IN WORLD_SNAPSHOT,WORLD_SNAPSHOT_CLIENT_PARAM,RESERVED);
    WORLD_SNAPSHOT_COMPARISON_RESULT (*CompareSnapshots)           (IN PWORLD_SNAPSHOTS_MANAGER,IN WORLD_SNAPSHOT,IN WORLD_SNAPSHOT,WORLD_SNAPSHOT_CLIENT_PARAM,RESERVED);
    WORLD_SNAPSHOT_EVALUATION        (*EvaluateSnapshot)           (IN PWORLD_SNAPSHOTS_MANAGER,IN WORLD_SNAPSHOT,WORLD_SNAPSHOT_CLIENT_PARAM,RESERVED);
    UL32                             (*GenerateSnapshotEvolutions) (IN PWORLD_SNAPSHOTS_MANAGER,IN WORLD_SNAPSHOT,IN PWORLD_SNAPSHOTS_GROUP*,WORLD_SNAPSHOT_CLIENT_PARAM,RESERVED);
    NOTHING                          (*ReturnClientObject)         (IN PWORLD_SNAPSHOTS_MANAGER,IN UNKOBJ,WORLD_SNAPSHOT_CLIENT_PARAM,RESERVED);
    NOTHING                          (*GetSnapshotHash)            (IN PWORLD_SNAPSHOTS_MANAGER,IN WORLD_SNAPSHOT,OUT WORLD_SNAPSHOT_HASH,WORLD_SNAPSHOT_CLIENT_PARAM,RESERVED);
} WORLD_SNAPSHOT_MANAGING_FUNCTIONS,
  *PWORLD_SNAPSHOT_MANAGING_FUNCTIONS;

struct _WORLD_SNAPSHOTS_MANAGER
{
  WORLD_SNAPSHOT_MANAGING_FUNCTIONS   Functions;
  WORLD_SNAPSHOT_CLIENT_PARAM         ClientParam;
  UNKOBJ                              PrivatePtr;
};

#define INITIALIZE_SNAPSHOT_MANAGER(a,b,c,d,e,f,g,h,i) \
                                    a.PrivatePtr=b; \
                                    a.Functions.DupSnapshot=c; \
                                    a.Functions.CompareSnapshots=d; \
                                    a.Functions.EvaluateSnapshot=e; \
                                    a.Functions.GenerateSnapshotEvolutions=f; \
                                    a.Functions.ReturnClientObject=g; \
                                    a.Functions.GetSnapshotHash=h; \
                                    a.ClientParam=i;

#pragma pack(pop)

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
