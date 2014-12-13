////////////////////////////////////////////////////////////////////////////////////////////////////
//waerror.h
//public header

#ifndef __WA_ERROR_H__
#define __WA_ERROR_H__

#include "watypes.h"

typedef UL32 WAERROR;

#define WA_SUCCESS                   0x00000000

#define WA_COMMON_ERRORS_BASE        0x00011000
#define WA_ERROR_UNKNOWN             WA_COMMON_ERRORS_BASE+0x00000001
#define WA_ERROR_INVALID_PARAMETER   WA_COMMON_ERRORS_BASE+0x00000002
#define WA_ERROR_NOT_SUPPORTED       WA_COMMON_ERRORS_BASE+0x00000003
#define WA_ERROR_MEMORY              WA_COMMON_ERRORS_BASE+0x00000003

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////