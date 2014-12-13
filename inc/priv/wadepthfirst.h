#ifndef __DEPTHFIRST_H__
#define __DEPTHFIRST_H__

#include "../inc/pub/watypes.h"
#include "../inc/pub/waerror.h"
#include "../inc/pub/waengine.h"
#include "../inc/priv/waenginepriv.h"

WAERROR RunWaDepthFirstAnalysisJob
(IN WORLD_ANALYSIS_JOB_HANDLE hJob);

NOTHING FreeWaDepthFirstBranch
(PWORLD_ANALYSIS_JOB pWaJob,PWORLD_STATUS pWaStatus);

#endif