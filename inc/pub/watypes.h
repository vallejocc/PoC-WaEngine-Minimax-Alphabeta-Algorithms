////////////////////////////////////////////////////////////////////////////////////////////////////
//watypes.h
//public header

#ifndef __WORLD_ANALYSIS_TYPES_H__
#define __WORLD_ANALYSIS_TYPES_H__

#define IN
#define OUT

#define WANULL ((void*)0)
#define WATRUE  1
#define WAFALSE 0

#define WAINFINITE       0xffffffff
#define WANEGINFINITE    0x00000000

typedef void*              UNKOBJ;
typedef void               NOTHING;
typedef void*              RESERVED;

typedef unsigned char      UC8;
typedef unsigned short     US16;
typedef unsigned long      UL32;
typedef unsigned __int64   UQ64;

typedef signed char        SC8;
typedef signed short       SS16;
typedef signed long        SL32;
typedef signed __int64     SQ64;

typedef UC8                WABOOL;

typedef unsigned char *      MEMBLOCK;
#ifndef _BUILDING_64_
typedef unsigned long      MEMSZ;
#else
typedef unsigned __int64   MEMSZ;
#endif

typedef struct _WALIST_ENTRY
{
  struct _WALIST_ENTRY * Flink;
  struct _WALIST_ENTRY * Blink;

}WALIST_ENTRY,*PWALIST_ENTRY;

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////