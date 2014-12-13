#ifndef __ALPHABETA_H__
#define __ALPHABETA_H__

#include "../inc/pub/watypes.h"
#include "../inc/pub/waerror.h"
#include "../inc/pub/waengine.h"
#include "../inc/priv/waenginepriv.h"

WAERROR RunWaAlphaBetaAnalysisJob
(IN WORLD_ANALYSIS_JOB_HANDLE hJob);

NOTHING FreeWaAlphaBetaBranch
(PWORLD_ANALYSIS_JOB pWaJob,PWORLD_STATUS pWaStatus);

#endif