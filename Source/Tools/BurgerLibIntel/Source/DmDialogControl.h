/**********************************

	Dialog and control manager
	
**********************************/

#ifndef __DMDIALOGCONTROL_H__
#define __DMDIALOGCONTROL_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef __LRRECT_H__
#include "LrRect.h"
#endif

#ifndef __LKLINKLIST_H__
#include "LkLinkList.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct DialogControl_t;

typedef enum {BUTTON_OUTSIDE,BUTTON_INSIDE,BUTTON_DOWN,BUTTON_RELEASED,BUTTON_CLICKED} DialogControlAction_e;

typedef void (BURGERCALL *DialogControlProc)(struct DialogControl_t *Input);
typedef void (BURGERCALL *DialogControlDrawProc)(struct DialogControl_t *Input, int x, int y);
typedef void (BURGERCALL *DialogControlEventProc)(struct DialogControl_t *Input,DialogControlAction_e type);
typedef DialogControlAction_e (BURGERCALL *DialogControlCheckProc)(struct DialogControl_t *Input, int x,int y,Word buttons,Word Key);

typedef void (BURGERCALL *DialogProc)(struct Dialog_t *Input);
typedef void (BURGERCALL *DialogDrawProc)(struct Dialog_t *Input, int x, int y);
typedef Bool (BURGERCALL *DialogEventProc)(struct Dialog_t *Input,int x,int y,Word buttons,Word Key);

typedef void (BURGERCALL *DialogControlGenericListDrawProc)(struct DialogControlGenericList_t *Input, void *Data, int x, int y, int Width, int Height, Bool highlighted);

typedef struct DialogControl_t {
	LBRect Bounds;							/* Bounds rect for the control */
	Word8 Active;							/* TRUE if an active control */
	Word8 Invisible;							/* TRUE if invisible (Not drawn) */
	Word8 Focus;		 						/* Flag for if the button was down */
	Word8 Inside;							/* Mouse currently inside */
	DialogControlDrawProc Draw;					/* Draw the control */
	DialogControlProc Delete;				/* Delete the control */
	DialogControlEventProc Event;			/* An event occured (Callback) */
	DialogControlCheckProc Check;			/* Check if I hit a hot spot */
	void *RefCon;							/* User data */
	Word HotKey;							/* Ascii for the Hot Key */
	int Id;
} DialogControl_t;

typedef struct DialogControlList_t {
	Word NumButtons;						/* Number of VALID buttons */
	Word MemListSize;						/* Size of buttonlist for memory allocation */
	Word Dormant;							/* True if waiting for a mouse up but no control has focus */
	DialogControl_t *FocusControl;			/* Control that has focus */
	DialogControl_t **ControlList;			/* Pointers to the button list */
} DialogControlList_t;

typedef struct Dialog_t {
	DialogControlList_t MyList;
	LBRect Bounds;
	Word8 Invisible;
	Word FillColor;
	Word OutlineColor;
	DialogDrawProc Draw;						/* Draw the control */
	DialogProc Delete;						/* Delete the control */
	DialogEventProc Event;					/* An event occured (Callback) */
} Dialog_t;

typedef struct DialogList_t {
	Word NumDialogs;						/* Number of VALID buttons */
	Dialog_t *FrontDialog;					/* Control that has focus */
	Dialog_t **DialogList;					/* Pointers to the button list */
} DialogList_t;

typedef struct DialogControlButton_t {		/* Simple button */
	DialogControl_t Root;					/* Base class */
	int x,y;								/* Coords to draw the shape */
	struct ScreenShape_t *Art[3];				/* Shape number indexes */
} DialogControlButton_t;

typedef struct DialogControlTextButton_t {	/* Simple text string button */
	DialogControl_t Root;					/* Base class */
	//int x;								/* Coords to draw the string */
	const char *TextPtr;					/* Text string */
	struct FontRef_t *FontPtr;				/* Font to draw with */
	struct FontRef_t *FontPtr2;				/* Highlight Font to draw with */
	struct ScreenShape_t **Art;				/* Shapes to use (Array of 4) */
} DialogControlTextButton_t;

typedef struct DialogControlCheckBox_t {	/* Simple check box */
	DialogControl_t Root;					/* Base class */
	DialogControlEventProc Proc;			/* Pass through so I can handle the check events */
	struct ScreenShape_t **Art;				/* Shapes to use (Array of 4) */
//	int CheckY;								/* Coords to draw the checkbox */
//	int TextX,TextY;						/* Coords to draw the text */
	const char *TextPtr;					/* Text string */
	struct FontRef_t *FontPtr;				/* Font to draw with */
	Word Checked;							/* True if checked */
} DialogControlCheckBox_t;

typedef struct DialogControlSliderBar_t {	/* Slider bar */
	DialogControl_t Root;					/* Base class */
	struct ScreenShape_t **Art;				/* Shapes to use (Array of 5) */
	int BarY;								/* Coords to draw the slider bar */
	int ThumbX,ThumbY;						/* Current position of the thumb */
	Word ThumbAnchor;						/* Offset into the thumb for "Grabbing" */
	Word ThumbMinX;							/* Minimum offset from the left side */
	Word BarWidth;							/* Range of pixels the thumb can move */
	Word Range;								/* Range of the value (0-Range inclusive) */
	Word Value;								/* Current value */
} DialogControlSliderBar_t;

typedef struct DialogControlRepeatButton_t {	/* Simple repeater button */
	DialogControl_t Root;					/* Base class */
	int x,y;								/* Coords to draw the shape */
	struct ScreenShape_t *Art[3];				/* Shape number indexes */
	Word32 TimeMark;						/* Time mark when clicked */
	Word32 RepeatDelay;					/* Time before another click */
} DialogControlRepeatButton_t;

typedef struct DialogControlVScrollSlider_t {	/* Slider bar */
	DialogControl_t Root;					/* Base class */
	struct ScreenShape_t **Art;				/* Shapes to use (Array of 11) */
	int ThumbY;								/* Current position of the thumb */
	Word ThumbSize;							/* Size of the thumb in pixels */
	Word ThumbAnchor;						/* Offset into the thumb for "Grabbing" */
	Word BarHeight;							/* Range of pixels the thumb can move */
	Word Range;								/* Range of the value (0-Range inclusive) */
	Word Step;								/* Motion to step if clicked in a dead region */
	Word Value;								/* Current value */
	Word32 TimeMark;						/* Time mark when clicked */
	Word32 RepeatDelay;					/* Time before another click */
} DialogControlVScrollSlider_t;

typedef struct DialogControlVScroll_t {		/* Simple button */
	DialogControl_t Root;					/* Base class */
	DialogControlList_t MyList;				/* List of controls */
	DialogControlVScrollSlider_t *Slider;	/* Slider for up and down */
	Word ButtonStep;						/* Step for button press */
	Word Value;								/* Current value */
} DialogControlVScroll_t;

typedef struct DialogControlTextBox_t {		/* Simple box of text */
	DialogControl_t Root;					/* Base class */
	struct FontRef_t *FontPtr;				/* Font to use */
	const char *TextPtr;					/* Current text */
	struct FontWidthLists_t *TextDescription;	/* Format info for the text */
	struct ScreenShape_t **Art;				/* Art for the scroll bar */
	DialogControlVScroll_t *Slider;			/* Scroll bar if present */
	Word Value;								/* Top visible Y coordinate */
	Word OutlineColor;						/* Color to draw the outline in */
	Bool ScrollBarNormalArrowStyle;		/* The scroll bar style to use */
	Bool AllowSlider;
} DialogControlTextBox_t;

typedef struct DialogControlStaticText_t {		/* Simple box of text */
	DialogControl_t Root;					/* Base class */
	struct FontRef_t *FontPtr;				/* Font to use */
	const char *TextPtr;					/* Current text */
	struct FontWidthLists_t *TextDescription;	/* Format info for the text */
} DialogControlStaticText_t;

typedef struct DialogControlTextList_t {	/* Simple list of text */
	DialogControl_t Root;					/* Base class */
	struct FontRef_t *FontPtr;				/* Font to use */
	struct FontRef_t *FontPtr2;				/* Font to use (highlight) */
	DialogControlVScroll_t *Slider;			/* Scroll bar if present */
	struct ScreenShape_t **Art;				/* Scroll bar art */
	LinkedList_t List;						/* List of text */
	Word ScrollValue;						/* Which entry to display for scrolling */
	Word Value;								/* Which entry is valid */
	Word OutlineColor;						/* Color to draw the outline in */
	Word FillColor;							/* Color to draw background with */
	Word SelColor;							/* Color to draw highlighted row with */
	Bool ScrollBarNormalArrowStyle;		/* The scroll bar style to use */
} DialogControlTextList_t;

typedef struct DialogControlGenericList_t {	/* Simple list of text */
	DialogControl_t Root;					/* Base class */
	DialogControlVScroll_t *Slider;			/* Scroll bar if present */
	struct ScreenShape_t **Art;				/* Scroll bar art */
	LinkedList_t List;						/* List of text */
	Word ScrollValue;						/* Which entry to display for scrolling */
	Word Value;								/* Which entry is valid */
	Word OutlineColor;						/* Color to draw the outline in */
	Word FillColor;							/* Color to draw background with */
	Word SelColor;							/* Color to draw highlighted row with */
	Word CellHeight;
	Bool ScrollBarNormalArrowStyle;		/* The scroll bar style to use */
	DialogControlGenericListDrawProc CellDraw;
} DialogControlGenericList_t;

typedef struct DialogControlTextMenu_t {	/* Menu control (Used by pop-up menus) */
	DialogControl_t Root;					/* Base class */
	LinkedList_t *ListPtr;					/* List of text (I don't control the list) */
	struct FontRef_t *FontPtr;				/* Font to use */
	struct FontRef_t *FontPtr2;				/* Font to use (highlight) */
	Word Value;								/* Entry selected for highlight */
	Word CursorValue;						/* Entry the the cursor is over */
	int ScrollValue;						/* Scroll factor */
	Word CellHeight;						/* Number of cells high */
	Word NormColor;							/* Color to fill normally */
	Word CursorColor;						/* Color to fill for cursor highlight */
	Word BoxColor1;							/* Color for shadow box */
	Word BoxColor2;							/* Second color for shadow box */
	struct ScreenShape_t **Art;				/* Art for up and down arrows */
	Word32 TimeMark;						/* Time mark when clicked */
} DialogControlTextMenu_t;

typedef struct DialogControlPopupMenu_t {	/* Simple list of text */
	DialogControl_t Root;					/* Base class */
	struct FontRef_t *FontPtr;				/* Font to use */
	struct FontRef_t *FontPtr2;				/* Font to use (highlight) */
	DialogControlTextMenu_t *MenuPtr;		/* Pop up menu */
	LinkedList_t List;						/* List of text */
	Word Value;								/* Which entry is valid */
	Word NormColor;							/* Color to fill normally */
	Word CursorColor;						/* Color to fill for cursor highlight */
	Word BoxColor1;							/* Color for shadow box */
	Word BoxColor2;							/* Second color for shadow box */
	struct ScreenShape_t **Art;				/* Art list for controls */
} DialogControlPopupMenu_t;

#define DIALOGLINEEDIT_MAX_LEN 256
#define DIALOGLINEEDIT_ALPHAONLY 1
#define DIALOGLINEEDIT_NUMBERONLY 2
#define DIALOGLINEEDIT_CAPS 4
#define DIALOGLINEEDIT_SPACEOK 8

typedef struct DialogControlLineEdit_t {	/* Line Edit Control */
	DialogControl_t Root;					/* Base class */
	struct FontRef_t *FontRef;				/* Font to use */
	char Value[DIALOGLINEEDIT_MAX_LEN];	/* Current input */
	Word Length;							/* Length of valid input */
	Word CurPos;							/* Cursor index */
	Word Insert;							/* TRUE if insert mode */
	Word MaxLen;							/* Maximum length */
	Word CursorColor;						/* Color to draw the cursor line with */
	Word Flags;								/* Flags for line edit mode */
	Bool CursorFlag;						/* Whether to show the cursor */
} DialogControlLineEdit_t;

typedef struct DialogControlPicture_t {		/* Simple button */
	DialogControl_t Root;					/* Base class */
	struct ScreenShape_t *Art;				/* Shape number indexes */
} DialogControlPicture_t;

extern Word32 DialogControlTextBoxOutlineColor;		/* Color for outlines */
extern Word32 DialogControlTextBoxFillColor;
extern Word32 DialogControlTextBoxSelectedRowColor;
extern Word32 DialogControlShadowColor1;
extern Word32 DialogControlShadowColor2;
extern Word32 DialogControlMenuBackgroundColor;
extern Word32 DialogControlMenuSelectColor;

extern DialogList_t DialogMasterList;	/* Convenience; you usually don't need more than one */

extern void BURGERCALL DialogControlDelete(DialogControl_t *Input);
extern DialogControlAction_e BURGERCALL DialogControlCheck(DialogControl_t *Input,int x,int y,Word buttons,Word Key);
extern void BURGERCALL DialogControlDeleteProc(DialogControl_t *Input);
extern void BURGERCALL DialogControlMove(DialogControl_t *Input,int xoffset,int yoffset);
extern void BURGERCALL DialogControlMoveTo(DialogControl_t *Input,int x,int y);

extern void BURGERCALL DialogInit(Dialog_t *Input);
extern void BURGERCALL DialogDestroy(Dialog_t *Input);
extern DialogControl_t * BURGERCALL DialogCheck(Dialog_t *Input,int x,int y,Word Buttons,Word Key);
extern void BURGERCALL DialogDraw(Dialog_t *Input);
extern DialogControl_t * BURGERCALL DialogModal(Dialog_t *Input);
extern void BURGERCALL DialogMove(Dialog_t *Input,int xoffset,int yoffset);
extern void BURGERCALL DialogMoveTo(Dialog_t *Input,int x,int y);
extern Word32 DialogInitParseMacDLOG(Dialog_t *Input, const Word8 *DLOGData, char *name);
extern Word DialogInitParseMacDITL(Dialog_t *Input, Word8 *DITLData);

extern void BURGERCALL DialogListInit(DialogList_t *Input);
extern void BURGERCALL DialogListDestroy(DialogList_t *Input);
extern void BURGERCALL DialogListAddDialog(DialogList_t *Input,Dialog_t *DialogPtr);
extern void BURGERCALL DialogListRemoveDialog(DialogList_t *Input,Dialog_t *DialogPtr);
extern void BURGERCALL DialogListDraw(DialogList_t *Input, Bool RefreshAll);
extern DialogControl_t * BURGERCALL DialogListModal(DialogList_t *Input, Bool RefreshAll, Dialog_t **OutDialog);

extern void BURGERCALL DialogControlListInit(DialogControlList_t *Input);
extern void BURGERCALL DialogControlListDestroy(DialogControlList_t *Input);
extern DialogControl_t * BURGERCALL DialogControlListCheck(DialogControlList_t *Input,int x,int y,Word Buttons,Word Key);
extern void BURGERCALL DialogControlListAddControl(DialogControlList_t *Input,DialogControl_t *ControlPtr);
extern void BURGERCALL DialogControlListDraw(DialogControlList_t *Input, int x, int y);
extern DialogControl_t * BURGERCALL DialogControlListControlById(DialogControlList_t *Input, int Id);
extern DialogControlButton_t * BURGERCALL DialogControlListAddNewButton(DialogControlList_t *Input,struct ScreenShape_t *Shape1,struct ScreenShape_t *Shape2,struct ScreenShape_t *Shape3,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern DialogControlTextButton_t * BURGERCALL DialogControlListAddNewTextButton(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,const char *TextPtr,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern DialogControlCheckBox_t * BURGERCALL DialogControlListAddNewCheckBox(DialogControlList_t *Input,struct ScreenShape_t **ShapeArray,const char *TextPtr,struct FontRef_t *FontPtr,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc,Word Checked);
extern DialogControlSliderBar_t * BURGERCALL DialogControlListAddSliderBar(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range);
extern DialogControlRepeatButton_t * BURGERCALL DialogControlListAddRepeatButton(DialogControlList_t *Input,struct ScreenShape_t *Shape1,struct ScreenShape_t *Shape2,struct ScreenShape_t *Shape3,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern DialogControlVScrollSlider_t * BURGERCALL DialogControlListAddVScrollSlider(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range,Word Step);
extern DialogControlVScroll_t *DialogControlListAddVScroll(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range,Word Step,Word ButtonStep,Bool NormalArrowStyle);
extern DialogControlTextBox_t *BURGERCALL DialogControlListAddTextBox(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,const char *TextPtr,Word Value,struct FontRef_t *FontPtr,Bool NormalArrowStyle,Bool AllowSlider);
extern DialogControlStaticText_t *BURGERCALL DialogControlListAddStaticText(DialogControlList_t *Input,const LBRect *Bounds,DialogControlEventProc EventProc,const char *TextPtr,struct FontRef_t *FontPtr);
extern DialogControlTextList_t *BURGERCALL DialogControlListAddTextList(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,DialogControlEventProc EventProc,Bool ScrollBarNormalArrowStyle);
extern DialogControlTextMenu_t *BURGERCALL DialogControlListAddTextMenu(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,int x,int y,struct LinkedList_t *ListPtr,Word Value,DialogControlEventProc EventProc);
extern DialogControlPopupMenu_t *BURGERCALL DialogControlListAddPopupMenu(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,DialogControlEventProc EventProc);
extern DialogControlLineEdit_t *BURGERCALL DialogControlListAddLineEdit(DialogControlList_t *Input, struct FontRef_t* FontPtr, const LBRect *Bounds, Word MaxLen, Word32 CursorColor, Word Flags, DialogControlEventProc EventProc);
extern DialogControlPicture_t *BURGERCALL DialogControlListAddPicture(DialogControlList_t *Input, struct ScreenShape_t *Art, const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);

extern void BURGERCALL DialogControlInit(DialogControl_t *Input,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern DialogControl_t *BURGERCALL DialogControlNew(const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);

extern void BURGERCALL DialogControlButtonInit(DialogControlButton_t *Input,struct ScreenShape_t *Shape1,struct ScreenShape_t *Shape2,struct ScreenShape_t *Shape3,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern DialogControlButton_t *BURGERCALL DialogControlButtonNew(struct ScreenShape_t *Shape1,struct ScreenShape_t *Shape2,struct ScreenShape_t *Shape3,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern void BURGERCALL DialogControlButtonSetShapes(DialogControlButton_t *Input,struct ScreenShape_t *Shape1,struct ScreenShape_t *Shape2,struct ScreenShape_t *Shape3);

extern void BURGERCALL DialogControlTextButtonInit(DialogControlTextButton_t *Input,struct ScreenShape_t **ShapeArray,const char *Text,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern DialogControlTextButton_t *BURGERCALL DialogControlTextButtonNew(struct ScreenShape_t **ShapeArray,const char *Text,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern void BURGERCALL DialogControlTextButtonSetText(DialogControlTextButton_t *Input,const char *TextPtr,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2);
extern void BURGERCALL DialogControlTextButtonSetArt(DialogControlTextButton_t *Input,struct ScreenShape_t **ShapeArray);

extern void BURGERCALL DialogControlCheckBoxInit(DialogControlCheckBox_t *Input,struct ScreenShape_t **ShapeArray,const char *Text,struct FontRef_t *FontPtr,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc,Word Checked);
extern DialogControlCheckBox_t *BURGERCALL DialogControlCheckBoxNew(struct ScreenShape_t **ShapeArray,const char *Text,struct FontRef_t *FontPtr,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc,Word Checked);
extern void BURGERCALL DialogControlCheckBoxSetText(DialogControlCheckBox_t *Input,struct ScreenShape_t **ShapeArray,const char *Text,struct FontRef_t *FontPtr);

extern void BURGERCALL DialogControlSliderBarInit(DialogControlSliderBar_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range);
extern DialogControlSliderBar_t * BURGERCALL DialogControlSliderBarNew(struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range);
extern void BURGERCALL DialogControlSliderBarSetValue(DialogControlSliderBar_t *Input,Word NewValue);
extern void BURGERCALL DialogControlSliderBarSetParms(DialogControlSliderBar_t *Input,Word Range);
extern void BURGERCALL DialogControlSliderBarSetArt(DialogControlSliderBar_t *Input,struct ScreenShape_t **ArtArray);

extern void BURGERCALL DialogControlRepeatButtonInit(DialogControlRepeatButton_t *Input,struct ScreenShape_t *Shape1,struct ScreenShape_t *Shape2,struct ScreenShape_t *Shape3,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern DialogControlRepeatButton_t * BURGERCALL DialogControlRepeatButtonNew(struct ScreenShape_t *Shape1,struct ScreenShape_t *Shape2,struct ScreenShape_t *Shape3,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern void BURGERCALL DialogControlRepeatButtonSetArt(DialogControlRepeatButton_t *Input,struct ScreenShape_t *Shape1,struct ScreenShape_t *Shape2,struct ScreenShape_t *Shape3);

extern void BURGERCALL DialogControlVScrollSliderInit(DialogControlVScrollSlider_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range,Word Step);
extern DialogControlVScrollSlider_t * BURGERCALL DialogControlVScrollSliderNew(struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range,Word Step);
extern void BURGERCALL DialogControlVScrollSliderSetValue(DialogControlVScrollSlider_t *Input,Word NewValue);
extern void BURGERCALL DialogControlVScrollSliderSetParms(DialogControlVScrollSlider_t *Input,Word Range,Word Step);

extern void BURGERCALL DialogControlVScrollSetValue(DialogControlVScroll_t *Input,Word NewValue);
extern void BURGERCALL DialogControlVScrollSetParms(DialogControlVScroll_t *Input,Word Range,Word Step,Word ButtonStep);
extern void BURGERCALL DialogControlVScrollInit(DialogControlVScroll_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range,Word Step,Word ButtonStep, Bool NormalArrowStyle);
extern DialogControlVScroll_t * BURGERCALL DialogControlVScrollNew(struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range,Word Step,Word ButtonStep,Bool NormalArrowStyle);

extern void BURGERCALL DialogControlTextBoxSetValue(DialogControlTextBox_t *Input,Word NewValue);
extern void BURGERCALL DialogControlTextBoxSetText(DialogControlTextBox_t *Input,const char *TextPtr,struct FontRef_t *FontPtr);
extern void BURGERCALL DialogControlTextBoxInit(DialogControlTextBox_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,const char *TextPtr,Word Value,struct FontRef_t *FontPtr,Bool ScrollBarNormalArrowStyle,Bool AllowSlider);
extern DialogControlTextBox_t * BURGERCALL DialogControlTextBoxNew(struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,const char *TextPtr,Word Value,struct FontRef_t *FontPtr,Bool ScrollBarNormalArrowStyle,Bool AllowSlider);

extern void BURGERCALL DialogControlStaticTextSetText(DialogControlStaticText_t *Input,const char *TextPtr,struct FontRef_t *FontPtr);
extern void BURGERCALL DialogControlStaticTextInit(DialogControlStaticText_t *Input,const LBRect *Bounds,DialogControlEventProc EventProc,const char *TextPtr,struct FontRef_t *FontPtr);
extern DialogControlStaticText_t * BURGERCALL DialogControlStaticTextNew(const LBRect *Bounds,DialogControlEventProc EventProc,const char *TextPtr,struct FontRef_t *FontPtr);

extern void BURGERCALL DialogControlTextListSetValue(DialogControlTextList_t *Input,Word NewValue);
extern Word BURGERCALL DialogControlTextListAddText(DialogControlTextList_t *Input,const char *TextPtr,Word EntryNum);
extern void BURGERCALL DialogControlTextListRemoveText(DialogControlTextList_t *Input,Word EntryNum);
extern void BURGERCALL DialogControlTextListRemoveAllText(DialogControlTextList_t *Input);
extern Word BURGERCALL DialogControlTextListFindText(DialogControlTextList_t *Input,const char *TextPtr);
extern void BURGERCALL DialogControlTextListInit(DialogControlTextList_t *Input,struct ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,DialogControlEventProc EventProc,Bool ScrollBarNormalArrowStyle);
extern DialogControlTextList_t *BURGERCALL DialogControlTextListNew(struct ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,DialogControlEventProc EventProc,Bool ScrollBarNormalArrowStyle);

extern void BURGERCALL DialogControlGenericListSetValue(DialogControlGenericList_t *Input,Word NewValue);
extern Word BURGERCALL DialogControlGenericListAddRow(DialogControlGenericList_t *Input,void *DataPtr,Word EntryNum);
extern void BURGERCALL DialogControlGenericListRemoveRow(DialogControlGenericList_t *Input,Word EntryNum);
extern void BURGERCALL DialogControlGenericListRemoveAllRows(DialogControlGenericList_t *Input);
extern void BURGERCALL DialogControlGenericListInit(DialogControlGenericList_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,Word CellHeight,DialogControlEventProc EventProc,DialogControlGenericListDrawProc CellDrawProc,Bool ScrollBarNormalArrowStyle);
extern DialogControlGenericList_t *BURGERCALL DialogControlGenericListNew(struct ScreenShape_t **ArtArray,const LBRect *Bounds,Word CellHeight,DialogControlEventProc EventProc,DialogControlGenericListDrawProc CellDrawProc,Bool ScrollBarNormalArrowStyle);

extern void BURGERCALL DialogControlTextMenuInit(DialogControlTextMenu_t *Input,struct ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,int x,int y,LinkedList_t *ListPtr,Word Value,DialogControlEventProc EventProc);
extern DialogControlTextMenu_t *BURGERCALL DialogControlTextMenuNew(struct ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,int x,int y,LinkedList_t *ListPtr,Word Value,DialogControlEventProc EventProc);

extern void BURGERCALL DialogControlPopupMenuSetValue(DialogControlPopupMenu_t *Input,Word NewValue);
extern Word BURGERCALL DialogControlPopupMenuAddText(DialogControlPopupMenu_t *Input,const char *TextPtr,Word EntryNum);
extern void BURGERCALL DialogControlPopupMenuRemoveText(DialogControlPopupMenu_t *Input,Word EntryNum);
extern void BURGERCALL DialogControlPopupMenuRemoveAllText(DialogControlPopupMenu_t *Input);
extern Word BURGERCALL DialogControlPopupMenuFindText(DialogControlPopupMenu_t *Input,const char *TextPtr);
extern void BURGERCALL DialogControlPopupMenuInit(DialogControlPopupMenu_t *Input,struct ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,DialogControlEventProc EventProc);
extern DialogControlPopupMenu_t *BURGERCALL DialogControlPopupMenuNew(struct ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,DialogControlEventProc EventProc);

extern void BURGERCALL DialogControlLineEditReset(DialogControlLineEdit_t *Input);
extern void BURGERCALL DialogControlLineEditSetText(DialogControlLineEdit_t *Input,const char *text);
extern void BURGERCALL DialogControlLineEditGetText(DialogControlLineEdit_t *Input, char* Buffer, Word BufferSize );
extern void BURGERCALL DialogControlLineEditEnableCursor(DialogControlLineEdit_t* Input, Bool EnableCursor );
extern void BURGERCALL DialogControlLineEditSetInsertMode(DialogControlLineEdit_t* Input, Bool InsertMode );
extern Bool BURGERCALL DialogControlLineEditGetInsertMode(DialogControlLineEdit_t* Input );
extern Bool BURGERCALL DialogControlLineEditOnKeyPress(DialogControlLineEdit_t *Input,Word InKey);
extern void BURGERCALL DialogControlLineEditInit(DialogControlLineEdit_t *Input, struct FontRef_t* FontPtr, const LBRect *Bounds, Word MaxLen, Word32 CursorColor, Word Flags, DialogControlEventProc EventProc );
extern DialogControlLineEdit_t *BURGERCALL DialogControlLineEditNew(struct FontRef_t* FontPtr, const LBRect *Bounds, Word MaxLen, Word32 CursorColor, Word Flags, DialogControlEventProc EventProc);

extern void BURGERCALL DialogControlPictureInit(DialogControlPicture_t *Input,struct ScreenShape_t *Art,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern DialogControlPicture_t *BURGERCALL DialogControlPictureNew(struct ScreenShape_t *Art,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern void BURGERCALL DialogControlPictureSetArt(DialogControlPicture_t *Input,struct ScreenShape_t *Art);

#ifdef __cplusplus
}
#endif

#endif
