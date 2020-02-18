#pragma once
#include <list>
#define __ENIGMA_BEGIN__ namespace enigma {
#define __ENIGMA_END__ }

#define SAFE_DEL(P)				\
	do {						\
		delete P;		\
		P = nullptr;			\
	}while(0)					\

#define SAFE_DEL_ARRAY(A)		\
do {							\
		delete[] A;	\
		A = nullptr;			\
}while (0)						\


#define	ENIGMA_STATIC_ASSERT(EXP)			\
			do {							\
				enum{ __sa = 1 / (EXP) };	\
			}while(0)

#ifdef _WIN32 
#	include <vcruntime.h>
#	if _HAS_CXX17
#		define STATIC_ASSERT(EXP) ((void)0)
#	else
#		define STATIC_ASSERT(EXP) ENIGMA_STATIC_ASSERT(EXP)
#	endif
#else
#	if __cplusplus > 201701l
#		define STATIC_ASSERT(EXP) ((void)0)
#	else
#		define STATIC_ASSERT(EXP) ENIGMA_STATIC_ASSERT(EXP)
#	endif
#endif

