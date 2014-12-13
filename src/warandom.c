#include "..\inc\pub\warandom.h"
#include "..\inc\pub\watypes.h"
#include "..\inc\pub\waengine.h"
#include "..\inc\priv\waenginepriv.h"

typedef enum {LITTLE_ENDIAN1, BIG_ENDIAN1, NONIEEE} TArch;

typedef struct _MERSENNE_TWISTER_ENGINE_HANDLE
{
#define MERS_N   624
#define MERS_M   397
#define MERS_R   31
#define MERS_U   11
#define MERS_S   7
#define MERS_T   15
#define MERS_L   18
#define MERS_A   0x9908B0DF
#define MERS_B   0x9D2C5680
#define MERS_C   0xEFC60000

UL32 mt[MERS_N];                   // state vector
UL32 mti;                          // index into mt
TArch Architecture;                  // conversion to float depends on computer architecture
}MERSENNE_TWISTER_ENGINE_HANDLE,*PMERSENNE_TWISTER_ENGINE_HANDLE;


typedef struct _RAND_HANDLE
{
  WAENGINE_TYPES types;
  WATICKCOUNT (*TickCountFuncPtr)();
  PWORLD_ANALYSIS_ENGINE pWaEngine;
  MERSENNE_TWISTER_ENGINE_HANDLE Mersenne;
}RAND_HANDLE, 
 *PRAND_HANDLE;


void TRandomMersenne_RandomInit(PMERSENNE_TWISTER_ENGINE_HANDLE pMersenne,UL32 seed) 
{
  UL32 convert[2];

  // re-seed generator
  pMersenne->mt[0]= seed;
  for (pMersenne->mti=1; pMersenne->mti < MERS_N; pMersenne->mti++) 
  {
    pMersenne->mt[pMersenne->mti] = 
      (1812433253UL * 
       (pMersenne->mt[pMersenne->mti-1] ^ 
       (pMersenne->mt[pMersenne->mti-1] >> 30)) + 
       pMersenne->mti);
  }

  // detect computer architecture
  *((double*)convert) = 1.0;
  // Note: Old versions of the Gnu g++ compiler may make an error here,
  // compile with the option  -fenum-int-equiv  to fix the problem
  if (convert[1] == 0x3FF00000) pMersenne->Architecture = LITTLE_ENDIAN1;
  else if (convert[0] == 0x3FF00000) pMersenne->Architecture = BIG_ENDIAN1;
  else pMersenne->Architecture = NONIEEE;
}

void TRandomMersenne_RandomInitByArray(PMERSENNE_TWISTER_ENGINE_HANDLE pMersenne,
                                       UL32 seeds[], 
                                       int length) 
{
  // seed by more than 32 bits
  SL32  i, j, k;
  TRandomMersenne_RandomInit(pMersenne,19650218UL);
  if (length <= 0) 
    return;
  i = 1;  j = 0;
  k = (MERS_N > length ? MERS_N : length);
  for (; k; k--) 
  {
    pMersenne->mt[i] = 
    (pMersenne->mt[i] ^ 
    ((pMersenne->mt[i-1] ^ 
    (pMersenne->mt[i-1] >> 30)) * 1664525UL)) + seeds[j] + j;
    i++; j++;
    if (i >= MERS_N) {pMersenne->mt[0] = pMersenne->mt[MERS_N-1]; i=1;}
    if (j >= length) j=0;}
  for (k = MERS_N-1; k; k--) {
    pMersenne->mt[i] = 
      (pMersenne->mt[i] ^ ((pMersenne->mt[i-1] ^ (pMersenne->mt[i-1] >> 30)) * 1566083941UL)) - i;
    if (++i >= MERS_N) {pMersenne->mt[0] = pMersenne->mt[MERS_N-1]; i=1;}}
  pMersenne->mt[0] = 0x80000000UL;// MSB is 1; assuring non-zero initial array
} 

  
UL32 TRandomMersenne_BRandom(PMERSENNE_TWISTER_ENGINE_HANDLE pMersenne) 
{
  // generate 32 random bits
  UL32 y;

  if (pMersenne->mti >= MERS_N) 
  {
    // generate MERS_N words at one time
    const UL32 LOWER_MASK = (1LU << MERS_R) - 1;         // lower MERS_R bits
    const UL32 UPPER_MASK = 0xFFFFFFFF << MERS_R;        // upper (32 - MERS_R) bits
    static const UL32 mag01[2] = {0, MERS_A};
    
    SL32 kk;
    for (kk=0; kk < MERS_N-MERS_M; kk++) 
    {    
      y = (pMersenne->mt[kk] & UPPER_MASK) | (pMersenne->mt[kk+1] & LOWER_MASK);
      pMersenne->mt[kk] = pMersenne->mt[kk+MERS_M] ^ (y >> 1) ^ mag01[y & 1];
    }

    for (; kk < MERS_N-1; kk++) 
    {    
      y = (pMersenne->mt[kk] & UPPER_MASK) | (pMersenne->mt[kk+1] & LOWER_MASK);
      pMersenne->mt[kk] = pMersenne->mt[kk+(MERS_M-MERS_N)] ^ (y >> 1) ^ mag01[y & 1];
    }      

    y = (pMersenne->mt[MERS_N-1] & UPPER_MASK) | (pMersenne->mt[0] & LOWER_MASK);
    pMersenne->mt[MERS_N-1] = pMersenne->mt[MERS_M-1] ^ (y >> 1) ^ mag01[y & 1];
    pMersenne->mti = 0;
  }

  y = pMersenne->mt[pMersenne->mti++];

  // Tempering (May be omitted):
  y ^=  y >> MERS_U;
  y ^= (y << MERS_S) & MERS_B;
  y ^= (y << MERS_T) & MERS_C;
  y ^=  y >> MERS_L;
  return y;
}
  
double TRandomMersenne_Random(PMERSENNE_TWISTER_ENGINE_HANDLE pMersenne) 
{
  // output random float number in the interval 0 <= x < 1
  UL32 convert[2];
  UL32 r = TRandomMersenne_BRandom(pMersenne); // get 32 random bits
  // The fastest way to convert random bits to floating point is as follows:
  // Set the binary exponent of a floating point number to 1+bias and set
  // the mantissa to random bits. This will give a random number in the 
  // interval [1,2). Then subtract 1.0 to get a random number in the interval
  // [0,1). This procedure requires that we know how floating point numbers
  // are stored. The storing method is tested in function RandomInit and saved 
  // in the variable Architecture. The following switch statement can be
  // omitted if the architecture is known. (A PC running Windows or Linux uses
  // LITTLE_ENDIAN1 architecture):
  switch (pMersenne->Architecture) {
  case LITTLE_ENDIAN1:
    convert[0] =  r << 20;
    convert[1] = (r >> 12) | 0x3FF00000;
    return (*(double*)convert) - 1.0;
  case BIG_ENDIAN1:
    convert[1] =  r << 20;
    convert[0] = (r >> 12) | 0x3FF00000;
    return (*(double*)convert) - 1.0;
  case NONIEEE: default:
  ;} 
  // This somewhat slower method works for all architectures, including 
  // non-IEEE floating point representation:
  return (double)r * (1./((double)(UL32)(-1L)+1.));
}

  
SL32 TRandomMersenne_IRandom(PMERSENNE_TWISTER_ENGINE_HANDLE pMersenne, SL32 min, SL32 max) 
{
  // output random integer in the interval min <= x <= max
  SL32 r; 
  // multiply interval with random and truncate
  r = ((SL32)((max - min + 1) * TRandomMersenne_Random(pMersenne))) + min;
  if (r > max) r = max;
  if (max < min) return 0x80000000;
  return r;
}


WARAND_ENGINE_HANDLE CreateWaRandManager(WORLD_ANALYSIS_ENGINE_HANDLE hWaEngine,
                         WATICKCOUNT (*TickCountFuncPtr)(),
                         WAENGINE_TYPES types)
{
  PWORLD_ANALYSIS_ENGINE pWaEngine;
  PRAND_HANDLE hRand;
  
  if(!(types&WAALL_RANDOM_TYPES))
  {
    return WANULL;
  }
  
  pWaEngine = (PWORLD_ANALYSIS_ENGINE)hWaEngine;

  if(!pWaEngine || !TickCountFuncPtr)
  {
    return WANULL;
  }

  hRand = (PRAND_HANDLE)pWaEngine->MemoryManager->alloc(sizeof(RAND_HANDLE));

  if(!hRand)
  {
    return WANULL;
  }
  
  hRand->pWaEngine = pWaEngine;
  hRand->TickCountFuncPtr = TickCountFuncPtr;
  hRand->types = types;

  if(types&WAMERSENNE_TWISTER)
    TRandomMersenne_RandomInit(&hRand->Mersenne,hRand->TickCountFuncPtr());

  return hRand;
}


WARAND32 WaGenerateRand32(WARAND_ENGINE_HANDLE hRand,WARAND32 min, WARAND32 max)
{
  if(((PRAND_HANDLE)hRand)->types&WAMERSENNE_TWISTER)
    return ((UL32)TRandomMersenne_IRandom(&((PRAND_HANDLE)hRand)->Mersenne,min,max));

  return min;
}


WARAND16 WaGenerateRand16(WARAND_ENGINE_HANDLE hRand,WARAND16 min, WARAND16 max)
{
  if(((PRAND_HANDLE)hRand)->types&WAMERSENNE_TWISTER)
    return (WARAND16)
       (0xffff&((UL32)TRandomMersenne_IRandom(&((PRAND_HANDLE)hRand)->Mersenne,min,max)));

  return min;
}


WARAND8  WaGenerateRand8(WARAND_ENGINE_HANDLE hRand,WARAND8 min, WARAND8 max)
{
  if(((PRAND_HANDLE)hRand)->types&WAMERSENNE_TWISTER)
    return (WARAND8)
        (0xff&((UL32)TRandomMersenne_IRandom(&((PRAND_HANDLE)hRand)->Mersenne,min,max)));

  return min;
}


void  DestroyWaRandManager(WARAND_ENGINE_HANDLE hRand)
{
  ((PRAND_HANDLE)hRand)->pWaEngine->MemoryManager->free((MEMBLOCK)hRand);
}