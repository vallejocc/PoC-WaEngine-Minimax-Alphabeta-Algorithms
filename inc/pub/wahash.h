#ifndef __WAHASH_H__
#define __WAHASH_H__

#include "../pub/watypes.h"
#include "../pub/waerror.h"
#include "../pub/waengine.h"

typedef UC8
  WAHASH[20];

typedef void *
  WAHASH_MANAGER_HANDLE;

WAEXPORT WAHASH_MANAGER_HANDLE CreateWaHashManager(WORLD_ANALYSIS_ENGINE_HANDLE);
WAEXPORT NOTHING        DestroyWaHashManager(IN WAHASH_MANAGER_HANDLE);
WAEXPORT WAERROR        AddWaHash(IN WAHASH_MANAGER_HANDLE,IN WAHASH);
WAEXPORT WABOOL         IsWaHashAdded(IN WAHASH_MANAGER_HANDLE,IN WAHASH);

#endif