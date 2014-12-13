#ifndef __WAMACROS_H__
#define __WAMACROS_H__

#include "../pub/watypes.h"

#define WaPostInsertEntry(a,b) \
  {(b)->Flink=(a)->Flink; \
   (b)->Blink=(a); \
   (a)->Flink=(b); \
   (b)->Flink->Blink=(b);}
  
#define WaPreInsertEntry(a,b) \
  {(b)->Blink=(a)->Blink; \
   (b)->Flink=(a); \
   (a)->Blink=(b); \
   (b)->Blink->Flink=(b);}

#define WaExtractEntry(a) \
  {(a)->Blink->Flink=(a)->Flink; \
   (a)->Flink->Blink=(a)->Blink;}

#define WaInitList(a) \
  {(a)->Flink=(a); \
  (a)->Blink=(a);}

#define WaIsListEmpty(a) \
  ((a)->Flink==(a))

#define WAFIELD_OFFSET(type,field) \
  ((UL32)(MEMSZ)&(((type *)0)->field))

#define BASE_FROM_FIELD(type, field, fieldptr) \
( ( type * ) ( ( (UC8*) fieldptr )-WAFIELD_OFFSET( type, field ) ) )


#endif