#ifndef __WADEBUG_H__
#define __WADEBUG_H__

#include "../pub/wasnapshots.h"
#include "../pub/waengine.h"

typedef struct _DEBUG_CONFIG
{
  void * hInstance;
  unsigned int DelayMillisecs;
  char *(*GetDebugShowString)(WORLD_SNAPSHOT Snapshot);

} DEBUG_CONFIG,
  *PDEBUG_CONFIG;

#ifdef TREE_DEBUG_ENABLE

void DebugInit(PDEBUG_CONFIG pDbgCfg);
void ShowJob(WORLD_ANALYSIS_JOB_HANDLE hJob);
void DebugEnd();

#else 

#define DebugInit(a)
#define ShowJob(a)
#define DebugEnd()

#endif


#endif