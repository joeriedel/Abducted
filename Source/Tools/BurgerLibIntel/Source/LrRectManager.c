#include "LrRect.h"
#include <BREndian.hpp>
#include "ClStdLib.h"
#include "FxFixed.h"
#include "ShStream.h"
#include "MmMemory.h"

#if defined(__MAC__)
#include <MacTypes.h>
#endif

#define RECTLISTGROW 8		/* Number of rects I should grow a buffer by */

/**********************************

	Convert an OS native point to a Burgerlib point

**********************************/

void BURGERCALL LBPointFromSYSPOINT(LBPoint *Output,const SYSPOINT *Input)
{
#if defined(__MAC__)
	Output->x = (int)Input->h;		/* Mac OS calls them h and v */
	Output->y = (int)Input->v;
#else
	Output->x = (int)Input->x;		/* All others call them x and y */
	Output->y = (int)Input->y;
#endif
}

/**********************************

	Convert a Burgerlib point to an OS native point

**********************************/

void BURGERCALL LBPointToSYSPOINT(SYSPOINT *Output,const LBPoint *Input)
{
#if defined(__MAC__)
	Output->h = (short)Input->x;	/* MacOS uses shorts for it's points */
	Output->v = (short)Input->y;
#elif defined(__BEOS__)
	Output->x = (float)Input->x;	/* BeOS uses floats */
	Output->y = (float)Input->y;
#else
	Output->x = (long)Input->x;		/* Intel uses longs */
	Output->y = (long)Input->y;
#endif
}

/**********************************

	Check if a point is within an OS native rect

**********************************/

#if !defined(__BEOS__)
#define LESS <
#else
#define LESS <=		/* BEOS rects are INCLUSIVE!! */
#endif

Word BURGERCALL SYSRECTPtInRect(const SYSRECT *Input,int x,int y)
{
	if (x >= (int)Input->left && x LESS (int)Input->right &&
		y >= (int)Input->top && y LESS (int)Input->bottom) {
		return TRUE;		/* No intersection! */
	}
	return FALSE;		/* I intersect! */
}

/**********************************

	Read a point from a file

**********************************/

Word BURGERCALL LBPointRead(LBPoint *Output,FILE *fp)
{
	SWord16 Input[2];
	
	if (fread(&Input[0],1,4,fp)==4) {
		Output->x = Burger::LoadLittle(&Input[0]);
		Output->y = Burger::LoadLittle(&Input[1]);
		return FALSE;
	}
	return TRUE;
}

/**********************************

	Write a point to a file

**********************************/

Word BURGERCALL LBPointWrite(const LBPoint *Input,FILE *fp)
{
	SWord16 Output[2];
	
	Output[0] = Burger::LoadLittle(static_cast<SWord16>(Input->x));
	Output[1] = Burger::LoadLittle(static_cast<SWord16>(Input->y));
	if (fwrite(&Output[0],1,4,fp)==4) {
		return FALSE;
	}
	return TRUE;
}



/**********************************

	Given 4 coords, fill in a LBRect structure
	I must sort the the coords into a upper left and a lower right point

**********************************/

void BURGERCALL LBRectSetRect(LBRect *Input,int x1,int y1,int x2,int y2)
{
	if (x2<x1) {		/* Make sure that x1 is less than x2 */
		int temp;
		temp = x1;		/* Swap the x's! */
		x1 = x2;
		x2 = temp;
	}
	Input->left	= x1;	/* Save the leftmost and rightmost points */
	Input->right = x2;
	
	if (y2<y1) {		/* Make sure that y1 is less than y2 */
		int temp;
		temp = y1;		/* Swap the y's! */
		y1 = y2;
		y2 = temp;
	}
	Input->top = y1;	/* Save the highest and lowest points */
	Input->bottom = y2;
}

/**********************************

	Init a LBRect structure to all zeros

**********************************/

void BURGERCALL LBRectSetRectEmpty(LBRect *Input)
{
	int Temp;

	Temp = 0;			/* Init a zero to a register */
	Input->left = Temp;	/* Blank out all records */
	Input->top = Temp;
	Input->right = Temp;
	Input->bottom = Temp;
}

/**********************************

	Force a rect's width

**********************************/

void BURGERCALL LBRectSetWidth(LBRect *Input,int Width)
{
	Input->right = Input->left+Width;
}

/**********************************

	Force a rect's height

**********************************/

void BURGERCALL LBRectSetHeight(LBRect *Input,int Height)
{
	Input->bottom = Input->top+Height;
}

/**********************************

	Given an x,y point, determine if it is inside of a rect
	if so, then return TRUE

**********************************/

Word BURGERCALL LBRectPtInRect(const LBRect *Input,int x,int y)
{
	if (x >= Input->left && x < Input->right && y >= Input->top && y < Input->bottom) {
		return TRUE;		/* No intersection! */
	}
	return FALSE;		/* I intersect! */
}

/**********************************

	Given a LBPoint, determine if it is inside of a rect
	if so, then return TRUE

**********************************/

Word BURGERCALL LBRectPointInRect(const LBRect *Input1,const LBPoint *Input2)
{
	if (Input2->x >= Input1->left && Input2->x < Input1->right && 
		Input2->y >= Input1->top && Input2->y < Input1->bottom) {
		return TRUE;		/* No intersection! */
	}
	return FALSE;		/* I intersect! */
}

/**********************************

	Move a rect by an offset in the horizontal
	and vertical directions

**********************************/

void BURGERCALL LBRectOffsetRect(LBRect *Input,int h,int v)
{
	Input->left += h;		/* Add the horizontal first */
	Input->right += h;
	Input->top += v;		/* Then the vertical */
	Input->bottom += v;
	LBRectFix(Input);		/* Make sure it's ok */
}

/**********************************

	Shrink or expand a rect by a specific number of pixels

**********************************/

void BURGERCALL LBRectInsetRect(LBRect *Input,int x,int y)
{
	Input->left += x;		/* Move the left towards the center */
	Input->right -= x;		/* Move the right towards the center */
	Input->top += y;		/* Etc... Etc... */
	Input->bottom -= y;
	LBRectFix(Input);		/* Make sure it's ok */
}

/**********************************

	Are two rects the same?

**********************************/

Word BURGERCALL LBRectIsEqual(const LBRect *Input1,const LBRect *Input2)
{
	if (Input1->top == Input2->top &&
		Input1->bottom == Input2->bottom &&
		Input1->left == Input2->left &&
		Input1->right == Input2->right) {
		return TRUE;
	}
	return FALSE;
}

/**********************************

	Create the intersection of two rects and return TRUE
	if they truly intersect.
	If "Input" is not 0, then return the resulting intersection rect.

**********************************/

Word BURGERCALL LBRectIntersectRect(LBRect *Input,const LBRect *RectPtr1,const LBRect *RectPtr2)
{
	int Var1,Var2;		/* Temps */
	LBRect TempRect;

	if (!Input) {		/* Does the caller want the intersection rect? */
		Input = &TempRect;	/* Nope, but I need a rect buffer ANYWAYS! */
	}

	Var1 = RectPtr1->left;		/* Get the higher of the two lefts */
	Var2 = RectPtr2->left;
	if (Var1 < Var2) {
		Var1 = Var2;
	}
	Input->left = Var1;		/* Save the larger left value */

	Var1 = RectPtr1->right;	/* Get the lower of the two rights */
	Var2 = RectPtr2->right;
	if (Var1 >= Var2) {
		Var1 = Var2;
	}
	Input->right = Var1;

	Var1 = RectPtr1->top;
	Var2 = RectPtr2->top;
	if (Var1 < Var2) {		/* Get the lower value for top */
		Var1 = Var2;
	}
	Input->top = Var1;

	Var1 = RectPtr1->bottom;
	Var2 = RectPtr2->bottom;
	if (Var1 >= Var2) {	/* Get the greater value to bottom */
		Var1 = Var2;
	}
	Input->bottom = Var1;

	Var1 = TRUE;		/* Assume OK */
	if (Input->left >= Input->right || Input->top >= Input->bottom ) {	/* Non-Empty rect? */
		Var1 = 0;		/* This is also FALSE */
		Input->left = Var1;	/* Force an empty rect */
		Input->right = Var1;
		Input->top = Var1;
		Input->bottom = Var1;
	}
	return Var1;		/* Return TRUE or FALSE */
}

/**********************************

	Generate the union between two rects

**********************************/

void BURGERCALL LBRectUnionRect(LBRect *Output,const LBRect *Input1,const LBRect *Input2)
{
	int Temp,Temp2;

	Temp = Input1->top;		/* Preload top */
	Temp2 = Input2->top;
	if (Temp>Temp2) {		/* Use smaller */
		Temp = Temp2;
	}
	Output->top = Temp;

	Temp = Input1->bottom;	/* Preload bottom */
	Temp2 = Input2->bottom;
	if (Temp<Temp2) {		/* Use larger */
		Temp = Temp2;
	}
	Output->bottom = Temp;

	Temp = Input1->left;	/* Preload left */
	Temp2 = Input2->left;
	if (Temp>Temp2) {		/* Use smaller */
		Temp = Temp2;
	}
	Output->left = Temp;

	Temp = Input1->right;	/* Preload right */
	Temp2 = Input2->right;
	if (Temp<Temp2) {		/* Use larger */
		Temp = Temp2;
	}
	Output->right = Temp;
}

/**********************************

	If a passed point is outside of the bounds of the current rect,
	expand the rect to contain the point.

	If the rect is empty, init the rect to encompass only the point

**********************************/

void BURGERCALL LBRectAddPointToRect(LBRect *Input1,const LBPoint *Input2)
{
	int Temp;
	Temp = Input2->x;
	if (Input1->left || Input1->right || Input1->top || Input1->bottom) {
		if (Temp<Input1->left) {		/* Off the left side? */
			Input1->left = Temp;		/* Set the new left side */
		}
		if (Temp>=Input1->right) {		/* Off the right side? */
			Input1->right = Temp+1;		/* Set the new right side? */
		}
		Temp = Input2->y;
		if (Temp<Input1->top) {			/* Off the top? */
			Input1->top = Temp;			/* Set the new top */
		}
		if (Temp>=Input1->bottom) {		/* Off the bottom? */
			Input1->bottom = Temp+1;	/* Set the new bottom */
		}
		return;				/* Exit now! */
	}
	Input1->left = Temp;		/* Create a single pixel rect! */
	Input1->right = Temp+1;		/* 1 pixel wide */
	Temp=Input2->y;
	Input1->top = Temp;
	Input1->bottom = Temp+1;	/* 1 pixel tall */
}

/**********************************

	If a passed point is outside of the bounds of the current rect,
	expand the rect to contain the point.

	If the rect is empty, init the rect to encompass only the point

**********************************/

void BURGERCALL LBRectAddXYToRect(LBRect *Input1,int x,int y)
{
	if (Input1->left || Input1->right || Input1->top || Input1->bottom) {
		if (x<Input1->left) {		/* Off the left side? */
			Input1->left = x;		/* Set the new left side */
		}
		if (x>=Input1->right) {		/* Off the right side? */
			Input1->right = x+1;		/* Set the new right side? */
		}
		if (y<Input1->top) {			/* Off the top? */
			Input1->top = y;			/* Set the new top */
		}
		if (y>=Input1->bottom) {		/* Off the bottom? */
			Input1->bottom = y+1;	/* Set the new bottom */
		}
		return;				/* Exit now! */
	}
	Input1->left = x;		/* Create a single pixel rect! */
	Input1->right = x+1;		/* 1 pixel wide */
	Input1->top = y;
	Input1->bottom = y+1;	/* 1 pixel tall */
}

/**********************************

	Return TRUE if the rect is empty

**********************************/

Word BURGERCALL LBRectIsRectEmpty(const LBRect *Input)
{
	if ((Input->top==Input->bottom) || (Input->left==Input->right)) {
		return TRUE;		/* At least one point was too small... */
	}
	return FALSE;		/* Empty rect! */
}

/**********************************

	Return TRUE if the Input1 is contained inside of Input2

**********************************/

Word BURGERCALL LBRectIsInRect(const LBRect *Input1,const LBRect *Input2)
{
	if ((Input1->left >= Input2->left) && (Input2->right >= Input1->right) &&
		(Input1->top >= Input2->top) && (Input2->bottom >= Input1->bottom)) {
		return TRUE;		/* Input1 is inside Input2 */
	}
	return FALSE;		/* Input1 is outside */
}
/**********************************

	Clip a rect to a bounds rect

**********************************/

void BURGERCALL LBRectClipWithinRect(LBRect *Input,const LBRect *Bounds)
{
	if ((Input->bottom >= Bounds->top) &&		/* Outside of the clip rect? */
		(Input->top < Bounds->bottom) &&
		(Input->right >= Bounds->left) &&
		(Input->left < Bounds->right)) {
		if (Input->top < Bounds->top) {		/* Clip the top? */
			Input->top = Bounds->top;
		}
		if (Input->left < Bounds->left) {	/* Clip the left? */
			Input->left = Bounds->left;
		}
		if (Bounds->bottom < Input->bottom) {	/* Clip the bottom */
			Input->bottom = Bounds->bottom;
		}
		if (Bounds->right < Input->right) {		/* Clip the right */
			Input->right = Bounds->right;
		}
		return;
	}
	Input->left = 0;			/* This is an empty rect */
	Input->top = 0;
	Input->right = 0;
	Input->bottom = 0;
}

/**********************************

	Move a rect to a specific x and y coord

**********************************/

void BURGERCALL LBRectMove(LBRect *Input,int x,int y)
{
	int Offset;
	Offset = Input->left-x;		/* Get the motion in the x direction */
	Input->left = x;			/* Set the new X */
	Input->right -= Offset;		/* Adjust the right side */
	
	Offset = Input->top-y;		/* Get the motion in the y direction */
	Input->top = y;				/* Set the new Y */
	Input->bottom -= Offset;	/* Adjust the bottom */
}


/**********************************

	Move a rect to a specific x coord

**********************************/

void BURGERCALL LBRectMoveX(LBRect *Input,int x)
{
	int Offset;
	Offset = Input->left-x;		/* Get the motion in the x direction */
	Input->left = x;			/* Set the new X */
	Input->right -= Offset;		/* Adjust the right side */
}


/**********************************

	Move a rect to a specific y coord

**********************************/

void BURGERCALL LBRectMoveY(LBRect *Input,int y)
{
	int Offset;
	Offset = Input->top-y;		/* Get the motion in the y direction */
	Input->top = y;				/* Set the new Y */
	Input->bottom -= Offset;	/* Adjust the bottom */
}


/**********************************

	Move a rect to a specific x and y coord

**********************************/

void BURGERCALL LBRectMoveToPoint(LBRect *Input,const LBPoint *Input2)
{
	int Offset,Temp;
	Temp = Input2->x;
	Offset = Input->left-Temp;		/* Get the motion in the x direction */
	Input->left = Temp;			/* Set the new X */
	Input->right -= Offset;		/* Adjust the right side */
	
	Temp = Input2->y;
	Offset = Input->top-Temp;		/* Get the motion in the y direction */
	Input->top = Temp;				/* Set the new Y */
	Input->bottom -= Offset;	/* Adjust the bottom */
}


/**********************************

	Move a rect so that it is encompassed by a larger rect.
	I favor bounding to the top-leftmost edge in the case that the
	bounds rect cannot fit the rect in question.

**********************************/

void BURGERCALL LBRectMoveWithinRect(LBRect *Input,const LBRect *Bounds)
{
	if (Input->right > Bounds->right) {
		LBRectMoveX(Input,Bounds->right - LBRectWidth(Input));
	}
	if (Input->left < Bounds->left) {	/* Do this AFTER right to favor left */
		LBRectMoveX(Input,Bounds->left);
	}
	if (Input->bottom > Bounds->bottom) {
		LBRectMoveY(Input,Bounds->bottom - LBRectHeight(Input));
	}
	if (Input->top < Bounds->top) {		/* Do this AFTER bottom to favor top */
		LBRectMoveY(Input,Bounds->top);
	}
}


/**********************************

	A rect was arbitrarily modified. Sort the
	edges so that the left is less than right
	and top is less than bottom.

**********************************/

void BURGERCALL LBRectFix(LBRect *Input)
{
	int Temp,Temp2;
	
	Temp = Input->left;			/* Get the x edges */
	Temp2 = Input->right;
	if (Temp > Temp2) {			/* I swap the x's */
		Input->left = Temp2;
		Input->right = Temp;
	}
	Temp = Input->top;			/* Get the y edges */
	Temp2 = Input->bottom;
	if (Temp > Temp2) {
		Input->top = Temp2;		/* I swap the y's */
		Input->bottom = Temp;
	}
}


/**********************************

	Find and return the center position of a rect
	I use this form instead of (left+right)>>1 since it prevents
	overflow errors
	
**********************************/

void BURGERCALL LBRectGetCenter(int *x,int *y,const LBRect *Input)
{
	*x = (LBRectWidth(Input)>>1)+Input->left;		/* Get the center X */
	*y = (LBRectHeight(Input)>>1)+Input->top;		/* Get the center Y */
}


/**********************************

	Return the center x position of a rect

**********************************/

int BURGERCALL LBRectGetCenterX(const LBRect *Input)
{
	return (LBRectWidth(Input)>>1)+Input->left;		/* Get the center X */
}


/**********************************

	Return the center y position of a rect

**********************************/

int BURGERCALL LBRectGetCenterY(const LBRect *Input)
{
	return (LBRectHeight(Input)>>1)+Input->top;		/* Get the center Y */
}

/**********************************

	Find and return the center position of a rect

**********************************/

void BURGERCALL LBRectGetCenterPoint(LBPoint *Output,const LBRect *Input)
{
	Output->x = (LBRectWidth(Input)>>1)+Input->left;		/* Get the center X */
	Output->y = (LBRectHeight(Input)>>1)+Input->top;		/* Get the center Y */
}

/**********************************

	Move a rect around a center point

**********************************/

void BURGERCALL LBRectCenterAroundPoint(LBRect *Output,const LBPoint *Input)
{
	LBRectMove(Output,
		Input->x-(LBRectWidth(Output)>>1),		/* New X coord */
		Input->y-(LBRectHeight(Output)>>1));	/* New Y coord */
}


/**********************************

	Move a rect around a center x,y

**********************************/

void BURGERCALL LBRectCenterAroundXY(LBRect *Output,int x,int y)
{
	LBRectMove(Output,
		x-(LBRectWidth(Output)>>1),		/* New X coord */
		y-(LBRectHeight(Output)>>1));	/* New Y coord */
}


/**********************************

	Move a rect around a center x

**********************************/

void BURGERCALL LBRectCenterAroundX(LBRect *Output,int x)
{
	LBRectMoveX(Output,
		x-(LBRectWidth(Output)>>1));	/* New X coord */
}


/**********************************

	Move a rect around a center y

**********************************/

void BURGERCALL LBRectCenterAroundY(LBRect *Output,int y)
{
	LBRectMoveY(Output,
		y-(LBRectHeight(Output)>>1));	/* New Y coord */
}


/**********************************

	Move a rect around the center of another rect

**********************************/

void BURGERCALL LBRectCenterAroundRectCenter(LBRect *Output,const LBRect *Input)
{
	LBRectCenterAroundXY(Output,
		(LBRectWidth(Input)>>1)+Input->left,		/* Center X */
		(LBRectHeight(Input)>>1)+Input->top);		/* Center Y */
}


/**********************************

	Move a rect around the center of another rect
	but only in the X direction

**********************************/

void BURGERCALL LBRectCenterAroundRectCenterX(LBRect *Output,const LBRect *Input)
{
	LBRectCenterAroundX(Output,
		(LBRectWidth(Input)>>1)+Input->left);		/* Center X */
}

/**********************************

	Move a rect around the center of another rect
	but only in the y direction

**********************************/

void BURGERCALL LBRectCenterAroundRectCenterY(LBRect *Output,const LBRect *Input)
{
	LBRectCenterAroundY(Output,
		(LBRectHeight(Input)>>1)+Input->top);		/* Center X */
}

/**********************************

	I take the input Point which is in direct relation to the SrcBoundsRect
	and map it to the DestBoundsRect as if the SrcBoundsRect was stretched to
	fit the DestBoundsRect and the Input Point stretches in the same proportions.
	
**********************************/

void BURGERCALL LBRectMapPoint(LBPoint *Output,const LBRect *SrcBoundsRect,
	const LBRect *DestBoundsRect,const LBPoint *Input)
{
	Fixed32 WidthRatio;
	Fixed32 HeightRatio;
	
	/* First thing i do is get the ratios between the width and heigths */
	/* of the two bounds rects */
	
	WidthRatio = (LBRectWidth(DestBoundsRect)<<16)/LBRectWidth(SrcBoundsRect);
	HeightRatio = (LBRectHeight(DestBoundsRect)<<16)/LBRectHeight(SrcBoundsRect);	

	/* Now I create the source point x and y */
	
	Output->x = Input->x-SrcBoundsRect->left;
	Output->y = Input->y-SrcBoundsRect->top;

	/* Let's scale the point! */
	
	Output->x = IMFixRound(Output->x*WidthRatio);
	Output->y = IMFixRound(Output->y*HeightRatio);

	/* Add the new origin */
	
	Output->x = Output->x+DestBoundsRect->left;
	Output->y = Output->y+DestBoundsRect->top;
		
	/* Voila!! */
}

/**********************************

	I take the input rect which is in direct relation to the SrcBoundsRect
	and map it to the DestBoundsRect as if the SrcBoundsRect was stretched to
	fit the DestBoundsRect and the Input rect stretches in the same proportions.
	
**********************************/

void BURGERCALL LBRectMapRect(LBRect *Output,const LBRect *SrcBoundsRect,
	const LBRect *DestBoundsRect,const LBRect *Input)
{
	Fixed32 WidthRatio;
	Fixed32 HeightRatio;
	
	/* First thing i do is get the ratios between the width and heigths */
	/* of the two bounds rects */
	
	WidthRatio = (LBRectWidth(DestBoundsRect)<<16)/LBRectWidth(SrcBoundsRect);
	HeightRatio = (LBRectHeight(DestBoundsRect)<<16)/LBRectHeight(SrcBoundsRect);	

	/* Now I create the source point x and y */
	
	Output->left = Input->left-SrcBoundsRect->left;
	Output->top = Input->top-SrcBoundsRect->top;
	Output->right = Input->right-SrcBoundsRect->left;
	Output->bottom = Input->bottom-SrcBoundsRect->top;

	/* Let's scale the point! */
	
	Output->left = IMFixRound(Output->left*WidthRatio);
	Output->top = IMFixRound(Output->top*HeightRatio);
	Output->right = IMFixRound(Output->right*WidthRatio);
	Output->bottom = IMFixRound(Output->bottom*HeightRatio);

	/* Add the new origin */
	
	Output->left = Output->left+DestBoundsRect->left;
	Output->top = Output->top+DestBoundsRect->top;
	Output->right = Output->right+DestBoundsRect->left;
	Output->bottom = Output->bottom+DestBoundsRect->top;
		
	/* Voila!! */
}

/**********************************

	Convert an OS Rect to a Burgerlib Rect

**********************************/

#if !defined(__BEOS__)
#define ADDER
#else
#define ADDER +1
#endif

void BURGERCALL LBRectFromSYSRECT(LBRect *Output,const SYSRECT *Input)
{
	Output->top = (int)Input->top;
	Output->left = (int)Input->left;
	Output->bottom = (int)Input->bottom ADDER;
	Output->right = (int)Input->right ADDER;
}
#undef ADDER

/**********************************

	Convert a Burgerlib Rect to an OS Rect

**********************************/

#if defined(__MAC__)
#define ADDER
#define RTYPE short		/* Macs use shorts */
#elif defined(__BEOS__)
#define ADDER -1
#define RTYPE float
#else
#define ADDER
#define RTYPE long		/* All others use longs */
#endif

void BURGERCALL LBRectToSYSRECT(SYSRECT *Output,const LBRect *Input)
{
	Output->top = (RTYPE)Input->top;
	Output->left = (RTYPE)Input->left;
	Output->bottom = (RTYPE)Input->bottom ADDER;
	Output->right = (RTYPE)Input->right ADDER;
}
#undef ADDER
#undef RTYPE

/**********************************

	Read a rect from a file

**********************************/

Word BURGERCALL LBRectRead(LBRect *Output,FILE *fp)
{
	SWord16 Input[4];
	
	if (fread(&Input[0],1,8,fp)==8) {
		Output->left = Burger::LoadLittle(&Input[0]);
		Output->top = Burger::LoadLittle(&Input[1]);
		Output->right = Burger::LoadLittle(&Input[2]);
		Output->bottom = Burger::LoadLittle(&Input[3]);
		return FALSE;
	}
	return TRUE;
}

/**********************************

	Write a rect to a file

**********************************/

Word BURGERCALL LBRectWrite(const LBRect *Input,FILE *fp)
{
	SWord16 Output[4];
	
	Output[0] = Burger::LoadLittle(static_cast<SWord16>(Input->left));
	Output[1] = Burger::LoadLittle(static_cast<SWord16>(Input->top));
	Output[2] = Burger::LoadLittle(static_cast<SWord16>(Input->right));
	Output[3] = Burger::LoadLittle(static_cast<SWord16>(Input->bottom));
	if (fwrite(&Output[0],1,8,fp)==8) {
		return FALSE;
	}
	return TRUE;
}


/**********************************

	Read a rect from a streamhandle

**********************************/

void BURGERCALL LBRectReadStream(LBRect *Output,StreamHandle_t *fp)
{
	Output->left = StreamHandleGetShort(fp);
	Output->top = StreamHandleGetShort(fp);
	Output->right = StreamHandleGetShort(fp);
	Output->bottom = StreamHandleGetShort(fp);
}

/**********************************

	Write a rect to a stream

**********************************/

void BURGERCALL LBRectWriteStream(const LBRect *Input,StreamHandle_t *fp)
{
	StreamHandlePutShort(fp,Input->left);
	StreamHandlePutShort(fp,Input->top);
	StreamHandlePutShort(fp,Input->right);
	StreamHandlePutShort(fp,Input->bottom);
}


/**********************************

	Allocate and init a new Rect list
	(Side effect, since all I need to do is blank the records,
	I just allocate zero'd out memory!)

**********************************/

LBRectList *LBRectListNew(void)
{
	return (LBRectList *)AllocAPointerClear(sizeof(LBRectList));
}

/**********************************

	Dispose of a rect list that was allocated

**********************************/

void BURGERCALL LBRectListDelete(LBRectList *Input)
{
	if (Input) {			/* Valid input? */
		DeallocAHandle((void **)Input->RectList);	/* Release the CRectList */
		DeallocAPointer(Input);		/* Dispose of the memory */
	}
}


/**********************************

	Init a LBRectList record

**********************************/

void BURGERCALL LBRectListInit(LBRectList *Input)
{
	Input->MaxRects = 0;		/* I assume no entries are valid */
	Input->NumRects = 0;		/* No entries are valid */
	Input->RectList = 0;		/* No memory is valid */
}

/**********************************

	Delete the contents of a LBRectList

**********************************/

void BURGERCALL LBRectListDestroy(LBRectList *Input)
{
	if (Input) {
		DeallocAHandle((void **)Input->RectList);	/* Release the CRectList */
		Input->RectList = 0;	/* No entries are valid */
		Input->MaxRects = 0;	/* Empty */
		Input->NumRects = 0;		/* No rects are valid */
	}
}

/**********************************

	Return a rect list of the differance of rects b and t
	Return TRUE if there is an intersection and I modified the LBRectList.
	Note : This routine is recursive!!

**********************************/

Word BURGERCALL LBRectListRectClip(LBRectList *Input,const LBRect* b,const LBRect* t)
{
	LBRect temp;		/* First temp rect */

	if ((b->left >= t->left) && (b->right <= t->right) && (b->top >= t->top) && (b->bottom <= t->bottom)) {
		return FALSE;	/* b is completely under t */
	}

	if ((b->right <= t->left) || (b->left >= t->right) || (b->bottom <= t->top) || (b->top >= t->bottom)) {
		LBRectListAppendRect(Input,b);
		return TRUE;	/* b is not at all under t */
	}

	temp = *b;		/* Get the bottom rect... */
	if (temp.top < t->top) {		/* piece above can be seen */
		temp.bottom = t->top;
		LBRectListAppendRect(Input,&temp);
		temp.bottom = b->bottom;
		temp.top = t->top;
	} else if (temp.bottom > t->bottom) { /* piece below can be seen */
		temp.top = t->bottom;
		LBRectListAppendRect(Input,&temp);
		temp.top = b->top;
		temp.bottom = t->bottom;
	} else if (temp.left < t->left) {		/* piece to the left can be seen */
		temp.right = t->left;
		LBRectListAppendRect(Input,&temp);
		temp.right = b->right;
		temp.left = t->left;
	} else { 							/* piece to the right can be seen */
		temp.left = t->right;
		LBRectListAppendRect(Input,&temp);
		temp.left = b->left;
		temp.right = t->right;
	}
	LBRectListRectClip(Input,&temp,t);		/* Call myself to finish subdividing */
	return TRUE;		/* I modified the LBRectlist */
}

/**********************************

	Return a rect list of the differance of rects b and t
	Return TRUE if there is an intersection and I modified the LBRectList

**********************************/

static LBRectList Rtemp;	/* This is static so I can keep a buffer around for */
							/* pretty much enternity. This way I can avoid calling AllocAHandle */
							/* and DeallocAHandle unnessasarly */

void BURGERCALL LBRectListClipOutRect(LBRectList *Input,const LBRect *bound)
{
	Word i;				/* Temp */
	LBRect **RectHandle;	/* Handle to rect list */

	Rtemp.NumRects = 0;		/* Discard previous contents */
	RectHandle = Rtemp.RectList;	/* Get the handle */
	if (RectHandle) {		/* Does Handle exist? */
		if (!*RectHandle) {	/* Handle purged? */
			DeallocAHandle((void **)RectHandle);	/* Discard it */
			Rtemp.RectList = 0;	/* No Handles */
			Rtemp.MaxRects = 0;	/* No Handles */
		} else {
			SetHandlePurgeFlag((void **)RectHandle,FALSE);
		}
	}
	i = Input->NumRects;	/* Any source rects? */
	if (i) {
		LBRect *RectPtr;	/* Pointer to the rect list */
		RectPtr = (LBRect *)LockAHandle((void **)Input->RectList);
		do {
			LBRectListRectClip(&Rtemp,RectPtr,bound);	/* Clip the rect */
			++RectPtr;
		} while (--i);		/* All done? */
		UnlockAHandle((void **)Input->RectList);	/* Release the memory */
	}
	LBRectListCopy(Input,&Rtemp);		/* Copy the rect list into the resulting buffer */
	RectHandle = Rtemp.RectList;		/* Does a handle exit? */
	if (RectHandle) {		/* Mark as purgable */
		SetHandlePurgeFlag((void **)RectHandle,TRUE);	/* Mark as purgeable */
	}
}


/**********************************

	Return a rect list of the differance of rects b and t
	Return TRUE if there is an intersection and I modified the LBRectList

**********************************/

void BURGERCALL LBRectListClipOutRectList(LBRectList *Input,const LBRectList *list)
{
	Word i;

	i = list->NumRects;		/* Get the rect count */
	if (i) {				/* Any rects? */
		LBRect *RectPtr;	/* Pointer to the rect list */
		RectPtr = (LBRect *)LockAHandle((void **)list->RectList);	/* Get the rect pointer */
		do {
			LBRectListClipOutRect(Input,RectPtr);	/* Clip out the rect */
			++RectPtr;			/* Next rect */
		} while (--i);			/* All scanned? */
		UnlockAHandle((void **)list->RectList);
	}
}

/**********************************

	Add a rect to the end of a LBRectList

**********************************/

void BURGERCALL LBRectListAppendRect(LBRectList *Input,const LBRect *rect)
{
	if (Input->NumRects >= Input->MaxRects) {	/* Too tight? */
		void **NewMem;		/* Handle to memory */

		Input->MaxRects+=RECTLISTGROW;		/* Up the memory size */
		NewMem = ResizeAHandle((void **)Input->RectList,sizeof(LBRect)*Input->MaxRects);	/* Get a larger buffer */
		Input->RectList=(LBRect **)NewMem;	/* Save the new pointer */
		if (!NewMem) {		/* Error!?!?! */
			Input->MaxRects = 0;		/* Kill off the rect list */
			Input->NumRects = 0;
			return;			/* Foobar! */
		}
	}
	(*Input->RectList)[Input->NumRects] = *rect;		/* Append a rect */
	Input->NumRects++;		/* +1 to the number of valid rects */
}


/**********************************

	Add a rect list to the end of a LBRectList

**********************************/

void BURGERCALL LBRectListAppendRectList(LBRectList *Input,const LBRectList *list)
{
	Word i;

	i = list->NumRects;		/* Get the number of rects to append */
	if (i) {
		LBRect *RectPtr;
		RectPtr = (LBRect *)LockAHandle((void **)list->RectList);	/* Lock it down */
		do {
			LBRectListAppendRect(Input,RectPtr);	/* Pass all the individual rects */
			++RectPtr;
		} while (--i);
		UnlockAHandle((void **)list->RectList);		/* Release it */
	}
}

/**********************************

	This is for speed. I will copy the valid rect count from list
	into Input, if the buffer is big enough, I'll just memcpy the rects as
	well. If not, then I will expand the buffer and do the copy

**********************************/

void BURGERCALL LBRectListCopy(LBRectList *Input,const LBRectList *list)
{
	if (Input->MaxRects<list->NumRects) {	/* Buffer too small? */
		void **NewMem;		/* Handle to memory */
		NewMem = ResizeAHandle((void **)Input->RectList,sizeof(LBRect)*(list->NumRects+RECTLISTGROW));	/* Get a larger buffer */
		Input->RectList=(LBRect **)NewMem;	/* Save the new handle */
		if (!NewMem) {		/* Error?!?!? */
			Input->MaxRects = 0;		/* Reset the struct */
			Input->NumRects = 0;
			return;			/* Foobar! */
		}
		Input->MaxRects=list->NumRects+RECTLISTGROW;		/* Up the memory size */
	}
	FastMemCpy(*Input->RectList,*list->RectList,sizeof(LBRect)*list->NumRects);
	Input->NumRects = list->NumRects;		/* Set the valid rect count */
}


