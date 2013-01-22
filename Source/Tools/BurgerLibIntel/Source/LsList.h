/**********************************

	Class for referencing and searching lists of strings

**********************************/

#ifndef __LSLIST_H__
#define __LSLIST_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char* FirstItemTextPtr;	/* Pointer to the string array */
	Word ItemSize;			/* Size of each string element */
	Word NumItems;			/* Number of elements */
} ListFixed_t;

typedef struct {
	char** FirstItemTextArrayPtr;	/* Pointer to the "C" string pointer array */
	Word ItemSize;			/* Size of each element */
	Word NumItems;			/* Number of elements */
} ListStatic_t;

typedef struct {
	void **ArrayHandle;		/* Handle to the array of "C" string pointers */
	Word NumItems;			/* Number of elements */
} ListDynamic_t;

typedef struct {
	int MinVal;				/* Lowest value integer */
	int MaxVal;				/* Highest value integer */
	char WorkString[16];	/* ASCII version of an integer (Used bu GetString()) */
} ListIntRange_t;

typedef struct {
	Fixed32 MinVal;			/* Lowest value fixed */
	Fixed32 MaxVal;			/* Highest value fixed */
	Fixed32 StepVal;			/* Step range */
	char WorkString[32];	/* Ascii version of a fixed point number */
} ListFixedRange_t;

typedef struct {
	float MinVal;			/* Lowest value float */
	float MaxVal;			/* Highest value float */
	float StepVal;			/* Step range */
	char WorkString[32];	/* Ascii version of a floating point number */
} ListFloatRange_t;

extern void BURGERCALL ListFixedInit(ListFixed_t *Input,char *FirstItem,Word Size,Word Count);
#define ListFixedDestroy(x)
extern Word BURGERCALL ListFixedFind(const ListFixed_t *Input,const char *ItemText);
extern char * BURGERCALL ListFixedGetString(const ListFixed_t *Input,Word Index);

extern void BURGERCALL ListStaticInit(ListStatic_t *Input,char **FirstItem,Word Size,Word Count);
#define ListStaticDestroy(x)
extern Word BURGERCALL ListStaticFind(const ListStatic_t *Input,const char *ItemText);
extern char * BURGERCALL ListStaticGetString(const ListStatic_t *Input,Word Index);

extern void BURGERCALL ListDynamicInit(ListDynamic_t *Input);
extern Word BURGERCALL ListDynamicFind(const ListDynamic_t *Input,const char *ItemText);
extern char * BURGERCALL ListDynamicGetString(const ListDynamic_t *Input,Word Index);
extern void BURGERCALL ListDynamicDestroy(ListDynamic_t *Input);
extern void BURGERCALL ListDynamicAdd(ListDynamic_t *Input,char *ItemText);
extern void BURGERCALL ListDynamicRemoveString(ListDynamic_t *Input,const char *ItemText);
extern void BURGERCALL ListDynamicRemoveIndex(ListDynamic_t *Input,Word Index);

extern void BURGERCALL ListIntRangeInit(ListIntRange_t *Input,int MinVal,int MaxVal);
#define ListIntRangeDestroy(x)
extern Word BURGERCALL ListIntRangeFind(const ListIntRange_t *Input,const char *ItemText);
extern char * BURGERCALL ListIntRangeGetString(ListIntRange_t *Input,Word Index);

extern void BURGERCALL ListFixedRangeInit(ListFixedRange_t *Input,Fixed32 MinVal,Fixed32 MaxVal,Fixed32 Step);
#define ListFixedRangeDestroy(x)
extern Fixed32 BURGERCALL ListFixedRangeFind(const ListFixedRange_t *Input,const char *ItemText);
extern char * BURGERCALL ListFixedRangeGetString(ListFixedRange_t *Input,Fixed32 Index);

extern void BURGERCALL ListFloatRangeInit(ListFloatRange_t *Input,float MinVal,float MaxVal,float Step);
#define ListFloatRangeDestroy(x)
extern float BURGERCALL ListFloatRangeFind(const ListFloatRange_t *Input,const char *ItemText);
extern char * BURGERCALL ListFloatRangeGetString(ListFloatRange_t *Input,float Index);

#ifdef __cplusplus
}
#endif


#endif
