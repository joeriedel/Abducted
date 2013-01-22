/*******************************

	Rect, Point and Region handlers

*******************************/

#ifndef __LRRECT_H__
#define __LRRECT_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct StreamHandle_t;

typedef struct LBPoint {	/* Point coord in 2D space */
	int	x;		/* X coord of point */
	int y;		/* Y coord of point */
} LBPoint;

typedef struct LBRect {	/* Rect coord in 2D space */
	int left;	/* Topleft x of rect */
	int top;	/* Topleft y of rect */
	int right;	/* Bottomright x of rect */
	int bottom;	/* Bottomright y of rect */
} LBRect;

typedef struct LBRectList {	/* Array of rects */
	Word NumRects;	/* Current number of rects in list */
	Word MaxRects;	/* Size of the array */
	LBRect **RectList;	/* Handle to array of rects */
} LBRectList;

#if defined(__MAC__)
typedef struct Point SYSPOINT;		/* Mac OS has a precompiler header! */
typedef struct Rect SYSRECT;
#elif defined(__BEOS__)
typedef struct {			/* BeOS! */
	float x;
	float y;
} SYSPOINT;
typedef struct {
	float left;
	float top;
	float right;
	float bottom;
} SYSRECT;
#else
typedef struct {			/* Dos and Windows! */
	long x;
	long y;
} SYSPOINT;
typedef struct {
	long left;
	long top;
	long right;
	long bottom;
} SYSRECT;
#endif

/* Rect handlers */

extern void BURGERCALL LBPointFromSYSPOINT(LBPoint *Output,const SYSPOINT *Input);
extern void BURGERCALL LBPointToSYSPOINT(SYSPOINT *Output,const LBPoint *Input);
extern Word BURGERCALL SYSRECTPtInRect(const SYSRECT *InputRect,int x,int y);
extern Word BURGERCALL LBPointRead(LBPoint *Output,FILE *fp);
extern Word BURGERCALL LBPointWrite(const LBPoint *Input,FILE *fp);

extern void BURGERCALL LBRectSetRect(LBRect *Input,int x1,int y1,int x2,int y2);
#define LBRectWidth(Input) ((Input)->right-(Input)->left)
#define LBRectHeight(Input) ((Input)->bottom-(Input)->top)
extern void BURGERCALL LBRectSetRectEmpty(LBRect *Input);
extern void BURGERCALL LBRectSetWidth(LBRect *Input,int Width);
extern void BURGERCALL LBRectSetHeight(LBRect *Input,int Height);
extern Word BURGERCALL LBRectPtInRect(const LBRect *Input,int x,int y);
extern Word BURGERCALL LBRectPointInRect(const LBRect *Input1,const LBPoint *Input2);
extern void BURGERCALL LBRectOffsetRect(LBRect *Input,int h,int v);
extern void BURGERCALL LBRectInsetRect(LBRect *Input,int x,int y);
extern Word BURGERCALL LBRectIsEqual(const LBRect *Input1,const LBRect *Input2);
extern Word BURGERCALL LBRectIntersectRect(LBRect *Input,const LBRect *rect1,const LBRect *rect2);
extern void BURGERCALL LBRectUnionRect(LBRect *Output,const LBRect *Input1,const LBRect *Input2);
extern void BURGERCALL LBRectAddPointToRect(LBRect *Output,const LBPoint *Input);
extern void BURGERCALL LBRectAddXYToRect(LBRect *Input,int x,int y);
extern Word BURGERCALL LBRectIsRectEmpty(const LBRect *Input);
extern Word BURGERCALL LBRectIsInRect(const LBRect *Input1,const LBRect *Input2);
extern void BURGERCALL LBRectClipWithinRect(LBRect *Input,const LBRect *Bounds);
extern void BURGERCALL LBRectMove(LBRect *Input,int x,int y);
extern void BURGERCALL LBRectMoveX(LBRect *Input,int x);
extern void BURGERCALL LBRectMoveY(LBRect *Input,int y);
extern void BURGERCALL LBRectMoveToPoint(LBRect *Input,const LBPoint *Input2);
extern void BURGERCALL LBRectMoveWithinRect(LBRect *Input,const LBRect *Bounds);
extern void BURGERCALL LBRectFix(LBRect *Input);
extern void BURGERCALL LBRectGetCenter(int *x,int *y,const LBRect *Input);
extern int BURGERCALL LBRectGetCenterX(const LBRect *Input);
extern int BURGERCALL LBRectGetCenterY(const LBRect *Input);
extern void BURGERCALL LBRectGetCenterPoint(LBPoint *Output,const LBRect *Input);
extern void BURGERCALL LBRectCenterAroundPoint(LBRect *Output,const LBPoint *Input);
extern void BURGERCALL LBRectCenterAroundXY(LBRect *Output,int x,int y);
extern void BURGERCALL LBRectCenterAroundX(LBRect *Output,int x);
extern void BURGERCALL LBRectCenterAroundY(LBRect *Output,int y);
extern void BURGERCALL LBRectCenterAroundRectCenter(LBRect *Output,const LBRect *Input);
extern void BURGERCALL LBRectCenterAroundRectCenterX(LBRect *Output,const LBRect *Input);
extern void BURGERCALL LBRectCenterAroundRectCenterY(LBRect *Output,const LBRect *Input);
extern void BURGERCALL LBRectMapPoint(LBPoint *Output,const LBRect *SrcBoundsRect,const LBRect *DestBoundsRect,const LBPoint *Input);
extern void BURGERCALL LBRectMapRect(LBRect *Output,const LBRect *SrcBoundsRect,const LBRect *DestBoundsRect,const LBRect *Input);
extern void BURGERCALL LBRectFromSYSRECT(LBRect *Output,const SYSRECT *Input);
extern void BURGERCALL LBRectToSYSRECT(SYSRECT *Output,const LBRect *Input);
extern Word BURGERCALL LBRectRead(LBRect *Output,FILE *fp);
extern Word BURGERCALL LBRectWrite(const LBRect *Input,FILE *fp);
extern void BURGERCALL LBRectReadStream(LBRect *Output,struct StreamHandle_t *fp);
extern void BURGERCALL LBRectWriteStream(const LBRect *Input,struct StreamHandle_t *fp);

extern LBRectList * BURGERCALL LBRectListNew(void);
extern void BURGERCALL LBRectListDelete(LBRectList *Input);
extern void BURGERCALL LBRectListInit(LBRectList *Input);
extern void BURGERCALL LBRectListDestroy(LBRectList *Input);
extern Word BURGERCALL LBRectListRectClip(LBRectList *Input,const LBRect* b,const LBRect* t);
extern void BURGERCALL LBRectListClipOutRect(LBRectList *Input,const LBRect *bound);
extern void BURGERCALL LBRectListClipOutRectList(LBRectList *Input,const LBRectList *list);
extern void BURGERCALL LBRectListAppendRect(LBRectList *Input,const LBRect *rect);
extern void BURGERCALL LBRectListAppendRectList(LBRectList *Input,const LBRectList *list);
extern void BURGERCALL LBRectListCopy(LBRectList *Input,const LBRectList *list);
extern void BURGERCALL LBRectListRead(LBRectList *Output,FILE *fp);
extern void BURGERCALL LBRectListWrite(const LBRectList *Input,FILE *fp);

#ifdef __cplusplus
}

/* LWPoint class and calls */

class LWPoint:public LBPoint {
public:
	LWPoint *New(void) {return new LWPoint;}
	void Delete(void) {delete this;}
	void Init(void) {}
	void Destroy(void) {}
	void FromSYSPOINT(const SYSPOINT *Input) {LBPointFromSYSPOINT(this,Input);}
	void FromSYSPOINT(const SYSPOINT &Input) {LBPointFromSYSPOINT(this,&Input);}
	void ToSYSPOINT(SYSPOINT *Output) {LBPointToSYSPOINT(Output,this);}
	void Read(FILE *fp) {LBPointRead(this,fp);}
	void Write(FILE *fp) {LBPointWrite(this,fp);}
};

/* LWRect class and calls */

class LWRect:public LBRect {
public:
	LWRect *New(void) {return new LWRect;}
	void Delete(void) {delete this;}
	void Init(void) {}
	void Destroy(void) {}
	void Set(int x1,int y1,int x2,int y2) {LBRectSetRect(this,x1,y1,x2,y2);}
	int Width(void) {return right-left;}
	int Height(void) {return bottom-top;}
	void Empty() {LBRectSetRectEmpty(this);}
	Word PtInRect(int x,int y) {return LBRectPtInRect(this,x,y);}
	void Offset(int h,int v) {LBRectOffsetRect(this,h,v);}
	void Inset(int x,int y) {LBRectInsetRect(this,x,y);}
	Word IsEqual(const LBRect *Input2) {return LBRectIsEqual(this,Input2);}
	Word IsEqual(const LBRect &Input2) {return LBRectIsEqual(this,&Input2);}
	Word Intersect(const LBRect *rect2) {return LBRectIntersectRect(0,this,rect2);}
	Word Intersect(const LBRect &rect2) {return LBRectIntersectRect(0,this,&rect2);}
	Word Intersect(LBRect *Output,const LBRect *rect2) {return LBRectIntersectRect(Output,this,rect2);}
	Word Intersect(LBRect *Output,const LBRect &rect2) {return LBRectIntersectRect(Output,this,&rect2);}
	void Union(const LBRect *Input2) {LBRectUnionRect(this,this,Input2);}
	void Union(const LBRect &Input2) {LBRectUnionRect(this,this,&Input2);}
	void Union(LBRect *Output,const LBRect *Input2) {LBRectUnionRect(Output,this,Input2);}
	void Union(LBRect *Output,const LBRect &Input2) {LBRectUnionRect(Output,this,&Input2);}
	void AddPoint(int x,int y) {LBRectAddXYToRect(this,x,y);}
	Word IsEmpty() {return LBRectIsRectEmpty(this);}
	void FromSYSRECT(const SYSRECT *Input) {LBRectFromSYSRECT(this,Input);}
	void FromSYSRECT(const SYSRECT &Input) {LBRectFromSYSRECT(this,&Input);}
	void ToSYSRECT(SYSRECT *Output) {LBRectToSYSRECT(Output,this);}
	void Read(FILE *fp) {LBRectRead(this,fp);}
	void Read(struct StreamHandle_t *fp) {LBRectReadStream(this,fp);}
	void Write(FILE *fp) {LBRectWrite(this,fp);}
	void Write(struct StreamHandle_t *fp) {LBRectWriteStream(this,fp);}
};

/* LWRectList class and calls */

class LWRectList:public LBRectList {
public:
	LWRectList(void) {LBRectListInit(this);}
	~LWRectList(void) {LBRectListDestroy(this);}
	LWRectList *New(void) {return (LWRectList *)LBRectListNew();}
	void Delete(void) {LBRectListDelete(this);}
	void Init(void) {LBRectListInit(this);}
	void Destroy(void) {LBRectListDestroy(this);}
	Word RectClip(const LBRect* b,const LBRect* t) {return LBRectListRectClip(this,b,t);}
	void ClipOut(const LBRect *bound) {LBRectListClipOutRect(this,bound);}
	void ClipOut(const LBRectList *list) {LBRectListClipOutRectList(this,list);}
	void Append(const LBRect *rect) {LBRectListAppendRect(this,rect);}
	void Append(const LBRectList *list) {LBRectListAppendRectList(this,list);}
	void Copy(const LBRectList *list) {LBRectListCopy(this,list);}
	void Read(FILE *fp) {LBRectListRead(this,fp);}
	void Write(FILE *fp) {LBRectListWrite(this,fp);}
};

#endif

#endif

