/**********************************

	This is the control manager for game
	dialogs.

**********************************/

#include "DMDialogControl.h"
#include <BREndian.hpp>
#include "MmMemory.h"
#include "SsScreenShape.h"
#include "FnFont.h"
#include "StString.h"
#include "TkTick.h"
#include "ClStdlib.h"
#include "PlPalette.h"
#include "InInput.h"
#include "KeyCodes.h"
#include <ctype.h>

Word32 DialogControlTextBoxOutlineColor = (255U<<24)+(0<<16)+(0<<8)+(0);
Word32 DialogControlShadowColor1 = (0<<16)+(0<<8)+(0);
Word32 DialogControlShadowColor2 = (34<<16)+(34<<8)+(34);
Word32 DialogControlMenuBackgroundColor = (221<<16)+(221<<8)+(221);
Word32 DialogControlMenuSelectColor = (51<<16)+(51<<8)+(153);
Word32 DialogControlTextBoxFillColor =  (255U<<24)+(0<<16)+(0<<8)+(0);
Word32 DialogControlTextBoxSelectedRowColor = (255U<<24)+(0<<16)+(0<<8)+(0);

DialogList_t DialogMasterList;

/**********************************

	Generic routines that pertain to all controls

**********************************/

/**********************************

	Delete a dialog control

**********************************/

void BURGERCALL DialogControlDelete(DialogControl_t *Input)
{
	if (Input) {
		if (Input->Delete) {				/* Is there a destroy proc? */
			Input->Delete(Input);			/* Delete it */
		}
	}
}

/**********************************

	Check if a point is within a button
	I return one of these values...
	BUTTON_DOWN = If the mouse is held down and I want to keep focus
	BUTTON_CLICKED = If the mouse was released inside me and I had focus
	BUTTON_RELEASED = If the mouse was released out of me and I had focus
	BUTTON_INSIDE = If the mouse is over the control
	BUTTON_OUTSIDE = If the mouse is outside of the control

**********************************/

DialogControlAction_e BURGERCALL DialogControlCheck(DialogControl_t *Input,int x,int y,Word buttons,Word Key)
{
	DialogControlAction_e Result;

	if (buttons) {
		buttons = TRUE; 		/* I only want TRUE or false */
	}
	
	Input->Inside = static_cast<Word8>(LBRectPtInRect(&Input->Bounds,x,y));		/* Inside of the rect? */
	
	/* Now I have the current motion state */
			
	/* Mouse down? */
	
	if (buttons) {
		if (Input->Focus) {
			Result = BUTTON_DOWN;			/* Grab focus */
			goto Exit;
		}
		if (Input->Inside) {
			Input->Focus = TRUE;			/* I'll keep focus */
			Result = BUTTON_DOWN;			/* Grab focus */
			goto Exit;
		}
	} else if (Input->Focus) {		/* I only care if I have focus */
		Input->Focus = FALSE;		/* Drop focus */

		/* Did I release OUTSIDE? */
		
		if (!Input->Inside) {
			Result = BUTTON_RELEASED;		/* Don't accept the click */
		} else {
			Result = BUTTON_CLICKED;		/* I accept this click */
		}
		goto Exit;
	}

	/* Did I press a hot key? */
	
	if (Key) {			/* Was a hot key pressed? */
		if (Key>='a' && Key<('z'+1)) {
			Key &= 0xDF;
		}
		if (Key==Input->HotKey) {
			Result = BUTTON_CLICKED;
			goto Exit;
		}
	}

	/* Ok, now return my state flag */
	
	Result = BUTTON_INSIDE;
	if (!Input->Inside) {
		Result = BUTTON_OUTSIDE;		/* I'm not active */
	}
Exit:;
	/* Do I have a callback proc? */
	if (Input->Event) {
		Input->Event(Input,Result);	/* Pass the current state */
	}
	return Result;
}

/**********************************

	Standard base destructor

**********************************/

void BURGERCALL DialogControlDeleteProc(DialogControl_t *Input)
{
	DeallocAPointer(Input);
}

/**********************************

	Move a button a specified distance

**********************************/

void BURGERCALL DialogControlMove(DialogControl_t *Input,int xoffset,int yoffset)
{
	Input->Bounds.left += xoffset;
	Input->Bounds.top += yoffset;
	Input->Bounds.right += xoffset;
	Input->Bounds.bottom += yoffset;
}

/**********************************

	Move a button to a specific x,y coordinate
	
**********************************/

void BURGERCALL DialogControlMoveTo(DialogControl_t *Input,int x,int y)
{
	Word Width,Height;
	Width = Input->Bounds.right-Input->Bounds.left;
	Height = Input->Bounds.bottom-Input->Bounds.top;
	Input->Bounds.left = x;		/* Adjust the bounds rect */
	Input->Bounds.top = y;
	Input->Bounds.right = x+Width;
	Input->Bounds.bottom = y+Height;
}






void BURGERCALL DialogInit(Dialog_t *Input)
{
	DialogControlListInit(&Input->MyList);
	Input->Bounds.left = 0;
	Input->Bounds.top = 0;
	Input->Bounds.right = 0;
	Input->Bounds.bottom = 0;
	Input->OutlineColor = 0;
	Input->FillColor = 0;
	Input->Draw = 0;						/* Draw the dialog */
	Input->Delete = 0;						/* Delete the dialog */
	Input->Event = 0;					/* An event occured (Callback) */
}


void BURGERCALL DialogDestroy(Dialog_t *Input)
{
	DialogControlListDestroy(&Input->MyList);
	if (Input->Delete) {				/* Is there a destroy proc? */
		Input->Delete(Input);			/* Delete it */
	}
}


DialogControl_t * BURGERCALL DialogCheck(Dialog_t *Input,int x,int y,Word Buttons,Word Key)
{
	if (!Input->Event || !Input->Event(Input, x, y, Buttons, Key))
		return DialogControlListCheck(&Input->MyList, x, y, Buttons, Key);
	return 0;
}


void BURGERCALL DialogDraw(Dialog_t *Input)
{
	//	Draw myself!
	if (Input->Draw)
		Input->Draw(Input, Input->Bounds.left, Input->Bounds.right);
	
	DialogControlListDraw(&Input->MyList, Input->Bounds.left, Input->Bounds.top);
}

DialogControl_t * BURGERCALL DialogModal(Dialog_t *Input)
{
	Word x, y;
	Word Buttons;
	Word Key;
	
	DialogDraw(Input);
	
	Key = KeyboardGet();		/* Read in the keyboard */
	Key = toupper(Key);
	Buttons = MouseReadButtons();		/* Read in the mouse input */
	MouseReadAbs(&x,&y);
	
	return DialogCheck(Input, x, y, Buttons, Key);
}

/**********************************

	Move a button a specified distance

**********************************/

void BURGERCALL DialogMove(Dialog_t *Input,int xoffset,int yoffset)
{
	Input->Bounds.left += xoffset;
	Input->Bounds.top += yoffset;
	Input->Bounds.right += xoffset;
	Input->Bounds.bottom += yoffset;
}

/**********************************

	Move a button to a specific x,y coordinate
	
**********************************/

void BURGERCALL DialogMoveTo(Dialog_t *Input,int x,int y)
{
	Word Width,Height;
	Width = Input->Bounds.right-Input->Bounds.left;
	Height = Input->Bounds.bottom-Input->Bounds.top;
	Input->Bounds.left = x;		/* Adjust the bounds rect */
	Input->Bounds.top = y;
	Input->Bounds.right = x+Width;
	Input->Bounds.bottom = y+Height;
}



Word32 DialogInitParseMacDLOG(Dialog_t *Input, const Word8 *DLOGData, char *name)
{
	Word32	IdDITL;
	Word8		len;
	DialogInit(Input);
	
	//	Parse the DLOG
	Input->Bounds.top = Burger::LoadBig((short *)DLOGData);	DLOGData += sizeof(short);
	Input->Bounds.left = Burger::LoadBig((short *)DLOGData);	DLOGData += sizeof(short);
	Input->Bounds.bottom = Burger::LoadBig((short *)DLOGData);	DLOGData += sizeof(short);
	Input->Bounds.right = Burger::LoadBig((short *)DLOGData);	DLOGData += sizeof(short);
	
	DLOGData += sizeof(short);	//	Type
	DLOGData += sizeof(Word8);	//	Invisible/Visible
	DLOGData += sizeof(Word8);	//	Filler
	DLOGData += sizeof(Word8);	//	Go Away Flag
	DLOGData += sizeof(Word8);	//	Filler
	
	DLOGData += sizeof(long);	//	Ref Con
	
	IdDITL = Burger::LoadBig((short *)DLOGData);
	DLOGData += sizeof(short);	//	DITL id... better match!
	
	//	PString
	len = *DLOGData;
	if (name)
	{
		strncpy(name, (const char *)DLOGData + 1, len);
		name[len] = 0;
	}
	DLOGData += len + 1;
	
//	if ((len & 0x1) == 0)	//	Len was 1 byte; was not odd length, so it's off by 1 byte
	if ((int)DLOGData & 0x1)	//	We can do this by the ptr... hehe :-)
		DLOGData += sizeof(Word8);
		
	DLOGData += sizeof(short);	//	Positioning info
	
	return IdDITL;
}


Word DialogInitParseMacDITL(Dialog_t *Input, Word8 *DITLData)
{
	Word32	NumControls;
	Word32	ControlId = 1;
	
	//	Parse the DITL
	NumControls = Burger::LoadBig((short *)DITLData);
	if (NumControls)
	{
		do
		{
			LBRect		ControlRect;
			Word8		Bits;
			Word32	ExtraDataSize;
			Word8 *		ExtraData;
			DialogControl_t *	Control = 0;
			char		Title[256];
			
			DITLData += sizeof(long);	//	Filler
			
			ControlRect.top = Burger::LoadBig((short *)DITLData);		DITLData += sizeof(short);
			ControlRect.left = Burger::LoadBig((short *)DITLData);		DITLData += sizeof(short);
			ControlRect.bottom = Burger::LoadBig((short *)DITLData);	DITLData += sizeof(short);
			ControlRect.right = Burger::LoadBig((short *)DITLData);	DITLData += sizeof(short);
			
			Bits = *DITLData;	DITLData += sizeof(Word8);
			
			ExtraDataSize = *DITLData;
			DITLData += sizeof(Word8);
			ExtraData = DITLData;
			DITLData += ExtraDataSize;
			
			/* Determine the type of item... */
			switch (Bits & 0x7F)
			{
				default:
				case 0://	User Item
					{
						Control = DialogControlNew(&ControlRect,0, NULL);
						
						DialogControlListAddControl(&Input->MyList, Control);
					}
					break;
				
				case 1:
					//	Help Item
					//	Do nothing
					break;
				
				case 4://	Button
					{
						DialogControlTextButton_t * 	Button;
						
						PStr2CStr(Title, (char *)(ExtraData - sizeof(Word8)));
						
						Button = DialogControlListAddNewTextButton(
								&Input->MyList,
								NULL,
								Title,NULL,NULL,
								&ControlRect,
								0,NULL);
						
						Control = &Button->Root;
					}
					break;
				
				case 5://	Checkbox
				case 6://	Radio Button
					{
						DialogControlCheckBox_t *		Checkbox;
						
						PStr2CStr(Title, (char *)(ExtraData - sizeof(Word8)));
						
						Checkbox = DialogControlListAddNewCheckBox(
								&Input->MyList,
								NULL,
								Title, NULL,
								&ControlRect,
								0, NULL, FALSE);
						
						Control = &Checkbox->Root;
					}
					break;
				
				case 7://	Control
					{
						Control = DialogControlNew(&ControlRect,0, NULL);
						
						DialogControlListAddControl(&Input->MyList, Control);
						
						Control->RefCon = (void *)Burger::LoadBig((short *)ExtraData);
						ExtraData += sizeof(short);	//	Refcon - ICON id
					}
					break;
				
				case 8://	Static Text
					{
						DialogControlStaticText_t *		StaticText;
						
						PStr2CStr(Title, (char *)(ExtraData - sizeof(Word8)));
						
						StaticText = DialogControlListAddStaticText(
								&Input->MyList,
								&ControlRect, NULL,
								Title, NULL);
						
						Control = &StaticText->Root;
					}
					break;
				
				case 16:
					//	Edit Text
					break;
				
				case 32:
				case 64://	Picture
					{
						DialogControlPicture_t *		StaticText;
						
						StaticText = DialogControlListAddPicture(
								&Input->MyList,
								NULL,
								&ControlRect,
								0, NULL);
						
						Control = &StaticText->Root;
						
						Control->RefCon = (void *)Burger::LoadBig((short *)ExtraData);
						ExtraData += sizeof(short);	//	Refcon - PICT id
					}
					break;
			}
			
			if (Control)
			{
				Control->Active = ((Bits & 0x80) != 0x80);
				Control->Id = ControlId;
			}
			++ControlId;
			
			if ((int)DITLData & 0x1)	//	We can do this by the ptr... hehe :-)
				DITLData += sizeof(Word8);
		} while (--NumControls);
	}
	
	return 0;
}




void BURGERCALL DialogListInit(DialogList_t *Input)
{
	if (Input == NULL)
		Input = &DialogMasterList;
	
	Input->NumDialogs = 0;		/* Just zero out all records */
	Input->FrontDialog = 0;	
	Input->DialogList = 0;
}

void BURGERCALL DialogListDestroy(DialogList_t *Input)
{
	Word i;

	if (Input == NULL)
		Input = &DialogMasterList;

	i = Input->NumDialogs;		/* Get the control count */
	if (i) {					/* Any here? */
		Dialog_t **ppDialog;
		ppDialog = Input->DialogList;	/* Get the pointer */
		do {
			DialogDestroy(ppDialog[0]);	/* Call the destructor */
			++ppDialog;
		} while (--i);
		Input->NumDialogs = 0;			/* No valid region records */
	}
	DeallocAPointer(Input->DialogList);	/* Release the pointer list */
	Input->DialogList = 0;
	Input->FrontDialog = 0;	
}

void BURGERCALL DialogListAddDialog(DialogList_t *Input,Dialog_t *DialogPtr)
{
	if (Input == NULL)
		Input = &DialogMasterList;
	
	Input->NumDialogs++;		/* Add 1 to the count */
	Input->DialogList = (Dialog_t **)ResizeAPointer(Input->DialogList,Input->NumDialogs*sizeof(Dialog_t *));
	if (!Input->DialogList) {
		Input->NumDialogs = 0;
		return;
	}
	Input->DialogList[Input->NumDialogs] = DialogPtr;		/* Save the region pointer */
	Input->FrontDialog = DialogPtr;
}

void BURGERCALL DialogListRemoveDialog(DialogList_t *Input,Dialog_t *DialogPtr)
{
	Word	i, write, read;

	if (Input == NULL)
		Input = &DialogMasterList;

	if (Input->NumDialogs == 0)
		return;
	
	i = Input->NumDialogs;
	write = 0;
	read = 0;
	do
	{
		if (Input->DialogList[read] != DialogPtr)
		{
			Input->DialogList[write] = Input->DialogList[read];
			++write;
		}
		++read;
	} while (--i);
	Input->NumDialogs--;		/* Add 1 to the count */
	Input->DialogList = (Dialog_t **)ResizeAPointer(Input->DialogList,Input->NumDialogs*sizeof(Dialog_t *));
	
	if (Input->FrontDialog == DialogPtr)
	{
		if (Input->NumDialogs != 0)
			Input->FrontDialog = Input->DialogList[Input->NumDialogs - 1];
		else
			Input->FrontDialog = NULL;
	}
}


void BURGERCALL DialogListDraw(DialogList_t *Input, Bool RefreshAll)
{
	if (Input == NULL)
		Input = &DialogMasterList;

	if ((Input->NumDialogs > 1) && RefreshAll)
	{
		Word i, CurrentDialog;
		i = Input->NumDialogs - 1;
		CurrentDialog = 0;
		do
		{
			DialogDraw(Input->DialogList[CurrentDialog]);
			++CurrentDialog;
		} while (--i);
	}

	if (Input->FrontDialog)
	{
		DialogDraw(Input->FrontDialog);
	}
}


DialogControl_t * BURGERCALL DialogListModal(DialogList_t *Input, Bool RefreshAll, Dialog_t **OutDialog)
{
	if (Input == NULL)
		Input = &DialogMasterList;

	if ((Input->NumDialogs > 1) && RefreshAll)
	{
		Word i, CurrentDialog;
		i = Input->NumDialogs - 1;
		CurrentDialog = 0;
		do
		{
			DialogDraw(Input->DialogList[CurrentDialog]);
			++CurrentDialog;
		} while (--i);
	}

	if (Input->FrontDialog)
	{
		if (OutDialog)
			*OutDialog = Input->FrontDialog;
		return DialogModal(Input->FrontDialog);
	}
	else if (OutDialog)
		*OutDialog = NULL;
	return NULL;
}




/**********************************

	A DialogControlList_t manages a list of controls
	and assigns focus to a specific one if the
	need arises. It also allows orderly disposal of
	the entire control list

**********************************/

/**********************************

	Init a DialogControlList_t

**********************************/

void BURGERCALL DialogControlListInit(DialogControlList_t *Input)
{
	Input->NumButtons = 0;		/* Just zero out all records */
	Input->MemListSize = 0;
	Input->Dormant = 0;
	Input->FocusControl = 0;	
	Input->ControlList = 0;
}

/**********************************

	Dispose of a DialogControlList_t

**********************************/

void BURGERCALL DialogControlListDestroy(DialogControlList_t *Input)
{
	Word i;

	i = Input->NumButtons;		/* Get the control count */
	if (i) {					/* Any here? */
		DialogControl_t **ppButton;
		ppButton = Input->ControlList;	/* Get the pointer */
		do {
			DialogControlDelete(ppButton[0]);	/* Call the destructor */
			++ppButton;
		} while (--i);
		Input->NumButtons = 0;			/* No valid region records */
	}
	DeallocAPointer(Input->ControlList);	/* Release the pointer list */
	Input->MemListSize = 0;
	Input->Dormant = 0;
	Input->ControlList = 0;
	Input->FocusControl = 0;	
}

/**********************************

	Handle the interaction with the control list.
	If a control has focus, return the pointer to the
	control.

**********************************/

DialogControl_t * BURGERCALL DialogControlListCheck(DialogControlList_t *Input,int x,int y,Word Buttons,Word Key)
{
	DialogControl_t *Result;
	DialogControlAction_e ClickState;
	DialogControl_t *pButton;
	Word i;

	/* First, see if a control has focus, if so, */
	/* then send all events to it until it's released */
	
	pButton = Input->FocusControl;
	if (pButton) {					/* Does a control have mouse focus? */
		pButton->Check(pButton,x,y,Buttons,Key);	/* Handle the control */
		if (!Buttons) {				/* Button held down? */
			Input->FocusControl = 0;		/* I will drop focus */
		}
		return pButton;
	}
	
	/* At this time, no control has focus */
	/* Cycle through all the controls until one decides that */
	/* it wants to grab focus */
	
	if (Input->Dormant) {			/* Am I sleeping? */
		if (Buttons) {
			Buttons = FALSE;		/* Force all controls to ignore the mouse */
		} else {
			Input->Dormant = FALSE;	/* Don't zap the mouse anymore */
		}
	}

	Result = 0;				/* No control is being touched */
	i = Input->NumButtons;
	if (i) {
		DialogControl_t **ppButton;
		ppButton = Input->ControlList;
		do {
			pButton = ppButton[0];
			ClickState = pButton->Check(pButton,x,y,Buttons,Key);		/* Test for focus */
			if (ClickState!=BUTTON_OUTSIDE) {		/* Is it inside? */
				Result = pButton;		/* Mark the region count */
				if (ClickState==BUTTON_DOWN) {
					Input->FocusControl = pButton;	/* Get focus */
					break;
				}
				if (ClickState==BUTTON_INSIDE) {
					x = -666;		/* Do not allow any other control to be selected */
					y = -666;		/* But keep looping to give the others the ability to reset */
				}
			}
			++ppButton;
		} while (--i);
		if (Buttons && !Result) {		/* Was it a mouse down but ignored? */
			Input->Dormant = TRUE;		/* Sleep until a mouse up */
		}
	}
	return Result;
}

/**********************************

	Add a control to the control list
	I will dynamically allocate more memory to the list if needed

**********************************/

void BURGERCALL DialogControlListAddControl(DialogControlList_t *Input,DialogControl_t *ButtonPtr)
{
	if (Input->NumButtons>=Input->MemListSize) {		/* I need more memory? */
		Input->MemListSize+=8;		/* It's 8 entries larger */
		Input->ControlList = (DialogControl_t **)ResizeAPointer(Input->ControlList,Input->MemListSize*sizeof(DialogControl_t *));
		if (!Input->ControlList) {
			Input->MemListSize = 0;
			Input->NumButtons = 0;
			return;
		}
	}
	Input->ControlList[Input->NumButtons] = ButtonPtr;		/* Save the region pointer */
	Input->NumButtons++;		/* Add 1 to the count */
}

/**********************************

	Draw all of the buttons
	Note : I draw back to front

**********************************/

void BURGERCALL DialogControlListDraw(DialogControlList_t *Input, int DrawX, int DrawY)
{
	Word i;

	i = Input->NumButtons;		/* Get the count */
	if (i) {
		DialogControl_t **ppButton;
		ppButton = &Input->ControlList[i];	/* Init the pointer */
		do {
			DialogControl_t *Entry;
			--ppButton;						/* Draw from back to front */
			Entry = ppButton[0];
			if (!Entry->Invisible && Entry->Draw) {		/* Shall I draw it? */
				Entry->Draw(Entry, DrawX, DrawY);			/* Draw the entry */
			}
		} while (--i);
	}
}


/**********************************

	
	Find the control with the Id

**********************************/

extern DialogControl_t * BURGERCALL DialogControlListControlById(DialogControlList_t *Input, int Id)
{
	Word i;

	i = Input->NumButtons;		/* Get the count */
	if (i) {
		DialogControl_t **ppButton;
		ppButton = &Input->ControlList[i];	/* Init the pointer */
		do {
			DialogControl_t *Entry;
			--ppButton;
			Entry = ppButton[0];
			if (Entry->Id == Id) {		/* Is this it? */
				return Entry;			/* Give it back */
			}
		} while (--i);
	}
	
	return 0;
}


/**********************************

	Create a new button and add it to the DialogControlList

**********************************/

DialogControlButton_t * BURGERCALL DialogControlListAddNewButton(DialogControlList_t *Input,ScreenShape_t *Shape1,ScreenShape_t *Shape2,ScreenShape_t *Shape3,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc)
{
	DialogControlButton_t *Result;
	Result = DialogControlButtonNew(Shape1,Shape2,Shape3,Bounds,HotKey,EventProc);	/* Create it */
	if (Result) {
		DialogControlListAddControl(Input,&Result->Root);		/* Add the control */
	}
	return Result;
}

/**********************************

	Create a new button and add it to the DialogControlList

**********************************/

DialogControlTextButton_t * BURGERCALL DialogControlListAddNewTextButton(DialogControlList_t *Input,ScreenShape_t **ShapeArray,const char *TextPtr,FontRef_t *FontPtr,FontRef_t *FontPtr2,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc)
{
	DialogControlTextButton_t *Result;
	Result = DialogControlTextButtonNew(ShapeArray,TextPtr,FontPtr,FontPtr2,Bounds,HotKey,EventProc);	/* Create it */
	if (Result) {
		DialogControlListAddControl(Input,&Result->Root);		/* Add the control */
	}
	return Result;
}

/**********************************

	Add a check box control
	
**********************************/

DialogControlCheckBox_t * BURGERCALL DialogControlListAddNewCheckBox(DialogControlList_t *Input,ScreenShape_t **ShapeArray,const char *TextPtr,struct FontRef_t *FontPtr,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc,Word Checked)
{
	DialogControlCheckBox_t *Result;
	Result = DialogControlCheckBoxNew(ShapeArray,TextPtr,FontPtr,Bounds,HotKey,EventProc,Checked);	/* Create it */
	if (Result) {
		DialogControlListAddControl(Input,&Result->Root);		/* Add the control */
	}
	return Result;
}

/**********************************

	Create a new button and add it to the DialogControlList

**********************************/

DialogControlSliderBar_t *BURGERCALL DialogControlListAddSliderBar(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range)
{
	DialogControlSliderBar_t *Result;
	Result = DialogControlSliderBarNew(ArtArray,Bounds,EventProc,Value,Range);	/* Create it */
	if (Result) {
		DialogControlListAddControl(Input,&Result->Root);		/* Add the control */
	}
	return Result;
}

/**********************************

	Create a new repeating button and add it to the DialogControlList

**********************************/

DialogControlRepeatButton_t *BURGERCALL DialogControlListAddRepeatButton(DialogControlList_t *Input,ScreenShape_t *Shape1,ScreenShape_t *Shape2,ScreenShape_t *Shape3,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc)
{
	DialogControlRepeatButton_t *Result;
	Result = DialogControlRepeatButtonNew(Shape1,Shape2,Shape3,Bounds,HotKey,EventProc);	/* Create it */
	if (Result) {
		DialogControlListAddControl(Input,&Result->Root);		/* Add the control */
	}
	return Result;
}

/**********************************

	Create a new button and add it to the DialogControlList

**********************************/

DialogControlVScrollSlider_t *BURGERCALL DialogControlListAddVScrollSlider(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range,Word Step)
{
	DialogControlVScrollSlider_t *Result;
	Result = DialogControlVScrollSliderNew(ArtArray,Bounds,EventProc,Value,Range,Step);	/* Create it */
	if (Result) {
		DialogControlListAddControl(Input,&Result->Root);		/* Add the control */
	}
	return Result;
}

/**********************************

	Create a new button and add it to the DialogControlList

**********************************/

DialogControlVScroll_t *DialogControlListAddVScroll(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range,Word Step,Word ButtonStep,Bool NormalArrowStyle)
{
	DialogControlVScroll_t *Result;
	Result = DialogControlVScrollNew(ArtArray,Bounds,EventProc,Value,Range,Step,ButtonStep,NormalArrowStyle);	/* Create it */
	if (Result) {
		DialogControlListAddControl(Input,&Result->Root);		/* Add the control */
	}
	return Result;
}

/**********************************

	Create a new DialogControlTextBox_t and add it to the control list

**********************************/

DialogControlTextBox_t *BURGERCALL DialogControlListAddTextBox(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,const char *TextPtr,Word Value,FontRef_t *FontPtr,Bool ScrollBarNormalArrowStyle,Bool AllowSlider)
{
	DialogControlTextBox_t *Result;
	Result = DialogControlTextBoxNew(ArtArray,Bounds,EventProc,TextPtr,Value,FontPtr,ScrollBarNormalArrowStyle,AllowSlider);	/* Create it */
	if (Result) {
		DialogControlListAddControl(Input,&Result->Root);		/* Add the control */
	}
	return Result;
}

/**********************************

	Create a new DialogControlStaticText_t and add it to the control list

**********************************/

DialogControlStaticText_t *BURGERCALL DialogControlListAddStaticText(DialogControlList_t *Input,const LBRect *Bounds,DialogControlEventProc EventProc,const char *TextPtr,FontRef_t *FontPtr)
{
	DialogControlStaticText_t *Result;
	Result = DialogControlStaticTextNew(Bounds,EventProc,TextPtr,FontPtr);	/* Create it */
	if (Result) {
		DialogControlListAddControl(Input,&Result->Root);		/* Add the control */
	}
	return Result;
}

/**********************************

	Create a new DialogControlTextList_t and add it to the control list

**********************************/

DialogControlTextList_t *BURGERCALL DialogControlListAddTextList(DialogControlList_t *Input,ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,DialogControlEventProc EventProc,Bool ScrollBarNormalArrowStyle)
{
	DialogControlTextList_t *Result;
	Result = DialogControlTextListNew(ArtArray,FontPtr,FontPtr2,Bounds,EventProc,ScrollBarNormalArrowStyle);	/* Create it */
	if (Result) {
		DialogControlListAddControl(Input,&Result->Root);		/* Add the control */
	}
	return Result;
}

/**********************************

	Create a new DialogControlLineEdit_t and add it to the control list

**********************************/

DialogControlLineEdit_t *BURGERCALL DialogControlListAddLineEdit(DialogControlList_t *Input, struct FontRef_t* FontPtr, const LBRect *Bounds, Word MaxLen, Word32 CursorColor, Word Flags, DialogControlEventProc EventProc)
{
	DialogControlLineEdit_t *Result;
	Result = DialogControlLineEditNew(FontPtr,Bounds,MaxLen,CursorColor,Flags,EventProc);	/* Create it */
	if (Result) {
		DialogControlListAddControl(Input,&Result->Root);		/* Add the control */
	}
	return Result;
}

/**********************************

	Create a new DialogControlTextMenu_t and add it to the control list

**********************************/

DialogControlTextMenu_t *BURGERCALL DialogControlListAddTextMenu(DialogControlList_t *Input,ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,int x,int y,LinkedList_t *ListPtr,Word Value,DialogControlEventProc EventProc)
{
	DialogControlTextMenu_t *Result;
	Result = DialogControlTextMenuNew(ArtArray,FontPtr,FontPtr2,x,y,ListPtr,Value,EventProc);	/* Create it */
	if (Result) {
		DialogControlListAddControl(Input,&Result->Root);		/* Add the control */
	}
	return Result;
}

/**********************************

	Create a new DialogControlTextList_t and add it to the control list

**********************************/

DialogControlPopupMenu_t *BURGERCALL DialogControlListAddPopupMenu(DialogControlList_t *Input,ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,DialogControlEventProc EventProc)
{
	DialogControlPopupMenu_t *Result;
	Result = DialogControlPopupMenuNew(ArtArray,FontPtr,FontPtr2,Bounds,EventProc);	/* Create it */
	if (Result) {
		DialogControlListAddControl(Input,&Result->Root);		/* Add the control */
	}
	return Result;
}


/**********************************

	Create a new DialogControlTextList_t and add it to the control list

**********************************/

DialogControlPicture_t *BURGERCALL DialogControlListAddPicture(DialogControlList_t *Input,ScreenShape_t *Art,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc)
{
	DialogControlPicture_t *Result;
	Result = DialogControlPictureNew(Art,Bounds,HotKey,EventProc);	/* Create it */
	if (Result) {
		DialogControlListAddControl(Input,&Result->Root);		/* Add the control */
	}
	return Result;
}


/**********************************

	This is a very simple edit field. It allows the user to input text, and numbers
	with constraints on the lenght of the input, and validation. The edit field also
	lets the user click inside to position the cursor, however, this edit control
	will not "scroll" text inside a small input box. Additions to this such as selecting
	text with the cursor etc. would be a cool, yet painful :)

**********************************/

/**********************************

	Draw the text in the line edit control

**********************************/

static void BURGERCALL DialogControlLineEditDraw(DialogControl_t* wow, int DrawX, int DrawY)
{
	DialogControlLineEdit_t* Input = (DialogControlLineEdit_t*)wow;
	LBRect ClipRect;
	
	GetTheClipRect(&ClipRect);
	
	SetTheClipBounds(Input->Root.Bounds.left+2+DrawX,Input->Root.Bounds.top+2+DrawY,Input->Root.Bounds.right-2+DrawX,Input->Root.Bounds.bottom-2+DrawY);
	FontDrawStringAtXY(Input->FontRef,Input->Root.Bounds.left+2+DrawX,Input->Root.Bounds.top+2+DrawY,Input->Value);
	if (Input->CursorFlag && ReadTick()&16) {
		Word x;
		Word Width;
		x = Input->FontRef->GetWidth(Input->FontRef,Input->Value,Input->CurPos);
		Width = Input->Value[Input->CurPos];
		if (!Width) {
			Width = ' ';
		}
		Width = FontWidthChar(Input->FontRef,Width);
		if (!Width) {
			Width = Input->FontRef->FontHeight/2;
		}
		ScreenRect(x+Input->Root.Bounds.left+2+DrawX,Input->Root.Bounds.top+2+Input->FontRef->FontHeight+DrawY,Width,1,Input->CursorColor);
	}
	SetTheClipRect(&ClipRect);
}

static DialogControlAction_e BURGERCALL DialogControlLineEditCheck(DialogControl_t* wow, int x, int y, Word Buttons, Word /* Key */)
{
	DialogControlAction_e r = DialogControlCheck( wow, x, y, Buttons, 0 );
	DialogControlLineEdit_t* Input = (DialogControlLineEdit_t*)wow;
	
	if( r == BUTTON_DOWN )
	{
		//
		// mouse is down.
		//
		Word Index;
		
		Index = 0;
		if (Input->Length) {
			x = x-(Input->Root.Bounds.left+2);			/* Pixel offset */
			do {
				if (Input->FontRef->GetWidth(Input->FontRef,Input->Value,Index+1)>(Word)x) {
					break;
				}
			} while (++Index<Input->Length);
		}
		Input->CurPos = Index;		/* Set the cursor position */
	}
	return r;
}

DialogControlLineEdit_t *BURGERCALL DialogControlLineEditNew(struct FontRef_t* FontPtr, const LBRect *Bounds, Word MaxLen, Word32 CursorColor, Word Flags, DialogControlEventProc EventProc)
{
	DialogControlLineEdit_t *MyPtr;

	MyPtr = (DialogControlLineEdit_t *)AllocAPointer(sizeof(DialogControlLineEdit_t));	/* Get the memory */
	if (MyPtr) {
		DialogControlLineEditInit(MyPtr,FontPtr,Bounds,MaxLen,CursorColor,Flags,EventProc);		/* Init it */
		MyPtr->Root.Delete = DialogControlDeleteProc;
	}
	return MyPtr;			/* Return the pointer */
}

void BURGERCALL DialogControlLineEditInit(DialogControlLineEdit_t *Input, struct FontRef_t* FontPtr, const LBRect *Bounds, Word MaxLen, Word32 CursorColor, Word Flags, DialogControlEventProc EventProc )
{
	/* Init the base class */
	
	Input->Root.Active = TRUE;
	Input->Root.Invisible = FALSE;
	Input->Root.Focus = FALSE;
	Input->Root.Inside = FALSE;
	Input->Root.Draw = DialogControlLineEditDraw;
	Input->Root.Delete = 0;
	Input->Root.Check = DialogControlLineEditCheck;
	Input->Root.Event = EventProc;
	Input->Root.RefCon = 0;
	Input->Root.HotKey = 0;
	
	Input->Root.Bounds = *Bounds;
	Input->FontRef = FontPtr;
	Input->Insert = TRUE;
		
	if( MaxLen >= DIALOGLINEEDIT_MAX_LEN )
		MaxLen = DIALOGLINEEDIT_MAX_LEN-1;
		
	Input->MaxLen = MaxLen;
	Input->Flags = Flags;
	Input->CursorColor = PaletteConvertPackedRGBToDepth(CursorColor, VideoColorDepth);
	Input->CursorFlag = 0;
	Input->Length = 0;
	Input->CurPos = 0;
}

void BURGERCALL DialogControlLineEditReset(DialogControlLineEdit_t *Input)
{
	Input->Length = 0;			/* No input */
	Input->CurPos = 0;			/* Cursor at the beginning */
	Input->Value[0] = 0;		/* No string */
}

void BURGERCALL DialogControlLineEditSetText(DialogControlLineEdit_t *Input,const char *text)
{
	strncpy(Input->Value, text,Input->MaxLen);	/* Copy the string */
	Input->Value[Input->MaxLen] = 0;		/* Make sure it's a "C" string */
	Input->Length = strlen(Input->Value);	/* Get the new length */
	Input->CurPos = Input->Length;			/* Set to the end */
}

void BURGERCALL DialogControlLineEditGetText(DialogControlLineEdit_t *Input, char* Buffer, Word BufferSize )
{
	Buffer[0] = 0;
	
	if( Input->Length < BufferSize )
	{
		BufferSize = Input->Length+1;	
	}
		
	if( BufferSize > 1 )
	{
		strncpy(Buffer, Input->Value, BufferSize-1);
		Buffer[BufferSize] = 0;
	}
}
void BURGERCALL DialogControlLineEditEnableCursor(DialogControlLineEdit_t* Input, Bool EnableCursor )
{
	Input->CursorFlag = EnableCursor;
}

void BURGERCALL DialogControlLineEditSetInsertMode(DialogControlLineEdit_t* Input, Bool InsertMode )
{
	Input->Insert = InsertMode;
}

Bool BURGERCALL DialogControlLineEditGetInsertMode(DialogControlLineEdit_t* Input )
{
	return static_cast<Bool>(Input->Insert);
}

Bool BURGERCALL DialogControlLineEditOnKeyPress(DialogControlLineEdit_t *Input,Word InKey)
{
	Word Flags;
	if (InKey) {		/* Any key in? */
		switch(InKey) {
		case ASCII_INSERT:		/* Toggle the insert mode? */
			if (Input->Insert) {
				Input->Insert = FALSE;
				break;
			}
			Input->Insert = TRUE;
			break;

		case ASCII_BACKSPACE:
			if (!Input->CurPos) {		/* Can I? */
				break;			/* Don't backspace */
			}
			Input->CurPos--;	/* Move back */
		case ASCII_DELETE:
			if (Input->CurPos < Input->Length) {		/* Can I delete? */
				memmove(Input->Value + Input->CurPos, Input->Value + Input->CurPos + 1,Input->Length-Input->CurPos);
				--Input->Length;		/* Remove one */
			}
			break;

		case ASCII_LEFTARROW:
			if (Input->CurPos) {
				Input->CurPos--;
			}
			break;

		case ASCII_RIGHTARROW:
			if (Input->CurPos < Input->Length) {
				Input->CurPos++;
			}
			break;

		case ASCII_HOME:
			Input->CurPos = 0;
			break;

		case ASCII_END:
			Input->CurPos = Input->Length;
			break;

		default:
			Flags = Input->Flags;
			if (Flags&DIALOGLINEEDIT_CAPS) {
				InKey = toupper(InKey);
			}
			if ((Flags&DIALOGLINEEDIT_SPACEOK) && InKey==' ') {
				goto Accept;
			}
			Flags &= (DIALOGLINEEDIT_ALPHAONLY|DIALOGLINEEDIT_NUMBERONLY);
			switch (Flags) {
			case DIALOGLINEEDIT_ALPHAONLY:
				if (((InKey<'A') || (InKey>'Z')) && ((InKey<'a') || (InKey>'z'))) {
					InKey = 0;
				}
				break;
			case DIALOGLINEEDIT_NUMBERONLY:
				if ((InKey<'0') || (InKey>'9')) {
					InKey = 0;
				}
				break;
			case DIALOGLINEEDIT_ALPHAONLY|DIALOGLINEEDIT_NUMBERONLY:
				if (((InKey<'A') || (InKey>'Z')) && ((InKey<'a') || (InKey>'z')) &&
					((InKey<'0') || (InKey>'9'))) {
					InKey = 0;
				}
				break;
			}
Accept:
			if (InKey >= ' ' && InKey <= 0x7F && (Input->CurPos < Input->MaxLen)) {
				if (Input->Insert) {
					memmove(Input->Value + Input->CurPos + 1,Input->Value + Input->CurPos,Input->MaxLen - Input->CurPos);
					Input->Value[Input->CurPos] = (char)InKey;
					if (Input->Length<Input->MaxLen) {
						Input->Length++;
					}
					Input->CurPos++;
				} else {
					Input->Value[Input->CurPos] = (char)InKey;
					if (Input->CurPos>=Input->Length) {
						Input->Length++;
					}
					Input->CurPos++;
				}
				Input->Value[Input->Length] = 0;	/* Zero terminate */
			}
			else {
				return TRUE;	// Bad InKey.
			}
		}
	}
	
	/* Input was bad? */
	if( InKey == 0 ) {
		return TRUE;
	}
	
	return FALSE;		/* No input accepted yet */
}


void BURGERCALL DialogControlInit(DialogControl_t *Input,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc)
{
	/* Init the base class */
	Input->Bounds = *Bounds;
	Input->Active = TRUE;
	Input->Invisible = FALSE;
	Input->Focus = FALSE;		/* Doesn't have focus */
	Input->Inside = FALSE;		/* Set the current default mode */
	Input->Draw = 0;
	Input->Delete = 0;
	Input->Check = DialogControlCheck;
	Input->Event = EventProc;		/* No button specific callback */
	Input->RefCon = 0;
	Input->HotKey = HotKey;

	/* Now init my local variables */
}


DialogControl_t *BURGERCALL DialogControlNew(const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc)
{
	DialogControl_t *MyPtr;

	MyPtr = (DialogControl_t *)AllocAPointer(sizeof(DialogControl_t));	/* Get the memory */
	if (MyPtr) {
		DialogControlInit(MyPtr,Bounds,HotKey,EventProc);		/* Init it */
		MyPtr->Delete = DialogControlDeleteProc;
	}
	return MyPtr;			/* Return the pointer */
}


/**********************************

	This is a standard button. You give me 3 pieces
	of art, the button in a dormant state, the button
	when the mouse is over it and the button when
	it's depressed. Since this button returns an action,
	you are required to give an event proc that handles
	a BUTTON_CLICKED event. Otherwise this button
	performs no useful action

**********************************/

/**********************************

	Draw a standard button

**********************************/

static void BURGERCALL DialogControlButtonDraw(DialogControl_t *wow, int DrawX, int DrawY)
{
	Word Index;
	ScreenShape_t *ShapePtr;
	DialogControlButton_t *Input;
	
	Input = (DialogControlButton_t *)wow;
	
	if (!Input->Art)
		return;
	
	Index = Input->Root.Inside;					/* Cursor is over the button (0-1) */
	if (Input->Root.Focus) {					/* Is the button being pressed */
		++Index;
	}
	ShapePtr = Input->Art[Index];		/* Get the shape to draw */
	if (ShapePtr) {								/* Can be a NULL pointer */
		ScreenShapeDraw(ShapePtr,Input->Root.Bounds.left-Input->x + DrawX,Input->Root.Bounds.top-Input->y + DrawY);		/* Draw the button */
	}
}

/**********************************

	Given a pointer to an uninitialized Button, Set the defaults

**********************************/

void BURGERCALL DialogControlButtonInit(DialogControlButton_t *Input,ScreenShape_t *Shape1,ScreenShape_t *Shape2,ScreenShape_t *Shape3,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc)
{
	/* Init the base class */
	Input->Root.Bounds = *Bounds;
	Input->Root.Active = TRUE;
	Input->Root.Invisible = FALSE;
	Input->Root.Focus = FALSE;		/* Doesn't have focus */
	Input->Root.Inside = FALSE;		/* Set the current default mode */
	Input->Root.Draw = DialogControlButtonDraw;
	Input->Root.Delete = 0;
	Input->Root.Check = DialogControlCheck;
	Input->Root.Event = EventProc;		/* No button specific callback */
	Input->Root.RefCon = 0;
	Input->Root.HotKey = HotKey;

	/* Now init my local variables */
	
#if 0
	Input->ButtonShapes[0] = Shape1;		/* Make a new frame */
	Input->ButtonShapes[1] = Shape2;
	Input->ButtonShapes[2] = Shape3;
	if (!Shape1) {				/* No root shape */
		Shape1 = Shape2;		/* Use the second shape */
		if (!Shape1) {			/* Still no shape? */
			Shape1 = Shape3;	/* Try this one */
		}
	}
	if( Shape1 ) {
		ScreenShapeGetBounds(Shape1,&Input->Root.Bounds);
		Input->x = Input->Root.Bounds.left;
		Input->y = Input->Root.Bounds.top;
		Input->Root.Bounds.left += x;
		Input->Root.Bounds.top += y;
		Input->Root.Bounds.right += x;
		Input->Root.Bounds.bottom += y;
	}
#endif
	DialogControlButtonSetShapes(Input,Shape1,Shape2,Shape3);
}


extern void BURGERCALL DialogControlButtonSetShapes(DialogControlButton_t *Input,struct ScreenShape_t *Shape1,struct ScreenShape_t *Shape2,struct ScreenShape_t *Shape3)
{
	Input->x = Input->y = 0;
	
	Input->Art[0] = Shape1;		/* Make a new frame */
	Input->Art[1] = Shape2;
	Input->Art[2] = Shape3;
	if (!Shape1) {				/* No root shape */
		Shape1 = Shape2;		/* Use the second shape */
		if (!Shape1) {			/* Still no shape? */
			Shape1 = Shape3;	/* Try this one */
		}
	}
	if( Shape1 ) {
		LBRect	ShapeBounds;
		ScreenShapeGetBounds(Shape1,&ShapeBounds);
		Input->x = ShapeBounds.left;
		Input->y = ShapeBounds.top;
	}
}

/**********************************

	Allocate a new DialogControlButton_t struct and init it

**********************************/

DialogControlButton_t * BURGERCALL DialogControlButtonNew(ScreenShape_t *Shape1,ScreenShape_t *Shape2,ScreenShape_t *Shape3,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc)
{
	DialogControlButton_t *MyPtr;

	MyPtr = (DialogControlButton_t *)AllocAPointer(sizeof(DialogControlButton_t));	/* Get the memory */
	if (MyPtr) {
		DialogControlButtonInit(MyPtr,Shape1,Shape2,Shape3,Bounds,HotKey,EventProc);		/* Init it */
		MyPtr->Root.Delete = DialogControlDeleteProc;
	}
	return MyPtr;			/* Return the pointer */
}


/**********************************

	This is a text button. A string is assigned to the
	button and this button is the string as it is drawn
	in the primary font. A secondary font is supplied to draw the
	string when it is depressed. Since this button returns an action,
	you are required to give an event proc that handles
	a BUTTON_CLICKED event. Otherwise this button
	performs no useful action

**********************************/

/**********************************

	Destructors

**********************************/

static void BURGERCALL DialogControlTextButtonDestroy(DialogControl_t *wow)
{
	DialogControlTextButton_t *Input;
	Input = (DialogControlTextButton_t *)wow;
	DeallocAPointer((void *)Input->TextPtr);
	Input->TextPtr = 0;
}

static void BURGERCALL DialogControlTextButtonDelete(DialogControl_t *wow)
{
	DialogControlTextButton_t *Input;
	Input = (DialogControlTextButton_t *)wow;
	DeallocAPointer((void *)Input->TextPtr);
	DeallocAPointer(Input);
}

/**********************************

	Draw a standard button

**********************************/

static void BURGERCALL DialogControlTextButtonDraw(DialogControl_t *wow, int DrawX, int DrawY)
{
	FontRef_t *Font;
	
	DialogControlTextButton_t *Input;
	Input = (DialogControlTextButton_t *)wow;
	
	/*
		Tricky...
		
		Here's how we do it... we have 3 shapes: left, fill, right
		and 2 levels of art... normal, pressed
		so...
	*/
	
	if (Input->Art)
	{
		Word Index = Input->Root.Focus ? 3 : 0;
		
		if (!Input->Art[Index])
			Index = 0;
		if (Input->Art[Index])
			ScreenShapeHPatternBar(Input->Art + Index,Input->Root.Bounds.top + DrawY,Input->Root.Bounds.left + DrawX,Input->Root.Bounds.right + DrawX);		/* Draw the button */
	}
	
	Font = Input->FontPtr;
	if (Input->Root.Focus || Input->Root.Inside) {
		Font = Input->FontPtr2;
	}
	
	if (Input->TextPtr) {			/* Failsafe */
//		FontDrawStringCenterX(Font,Input->Root.Bounds.left+Input->x+DrawX,Input->Root.Bounds.top+DrawY,Input->TextPtr);
		Word x = (Input->Root.Bounds.right - Input->Root.Bounds.left) >> 1;
		Word y = ((Input->Root.Bounds.bottom - Input->Root.Bounds.top) >> 1) - (Font->FontHeight >> 1);
		
		FontDrawStringCenterX(Font,Input->Root.Bounds.left+x+DrawX,Input->Root.Bounds.top+y+DrawY,Input->TextPtr);
	}
}

/**********************************

	Given a pointer to an uninitialized Button, Set the defaults

**********************************/

void BURGERCALL DialogControlTextButtonInit(DialogControlTextButton_t *Input,struct ScreenShape_t **ShapeArray,const char *TextPtr,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc)
{
//	Word Temp;
	/* Init the base class */
	
	Input->Root.Bounds = *Bounds;
	Input->Root.Active = TRUE;
	Input->Root.Invisible = FALSE;
	Input->Root.Focus = FALSE;		/* Doesn't have focus */
	Input->Root.Inside = FALSE;		/* Set the current default mode */
	Input->Root.Draw = DialogControlTextButtonDraw;
	Input->Root.Delete = DialogControlTextButtonDestroy;
	Input->Root.Check = DialogControlCheck;
	Input->Root.Event = EventProc;		/* No button specific callback */
	Input->Root.RefCon = 0;
	Input->Root.HotKey = HotKey;

	DialogControlTextButtonSetArt(Input,ShapeArray);
	DialogControlTextButtonSetText(Input,TextPtr,FontPtr,FontPtr2);
}


void BURGERCALL DialogControlTextButtonSetText(DialogControlTextButton_t *Input,const char *TextPtr,FontRef_t *FontPtr,FontRef_t *FontPtr2)
{
	/* Now init my local variables */
	
	Input->TextPtr = StrCopy(TextPtr);		/* Make a copy of the string */
	Input->FontPtr = FontPtr;
	Input->FontPtr2 = FontPtr2;
/*	Temp = FontWidthString(FontPtr,TextPtr);
	Input->x = (Temp>>1);
	x = x-Input->x;
	Input->Root.Bounds.left = x;
	Input->Root.Bounds.top = y;
	Input->Root.Bounds.right = x+Temp;
	Input->Root.Bounds.bottom = y+FontPtr->FontHeight;
*/
}


void BURGERCALL DialogControlTextButtonSetArt(DialogControlTextButton_t *Input,struct ScreenShape_t **ShapeArray)
{
	Input->Art = ShapeArray;
}


/**********************************

	Allocate a new DialogControlTextButton_t struct and init it

**********************************/

DialogControlTextButton_t * BURGERCALL DialogControlTextButtonNew(struct ScreenShape_t **ShapeArray,const char *TextPtr,FontRef_t *FontPtr,FontRef_t *FontPtr2,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc)
{
	DialogControlTextButton_t *MyPtr;

	MyPtr = (DialogControlTextButton_t *)AllocAPointer(sizeof(DialogControlTextButton_t));	/* Get the memory */
	if (MyPtr) {
		DialogControlTextButtonInit(MyPtr,ShapeArray,TextPtr,FontPtr,FontPtr2,Bounds,HotKey,EventProc);		/* Init it */
		MyPtr->Root.Delete = DialogControlTextButtonDelete;
	}
	return MyPtr;			/* Return the pointer */
}


/**********************************

	This is a check box. A string is assigned to the
	button and an initial value. This control is automatic and
	an event callback is not needed if dynamic user interaction
	is not required. Just look at the "Checked" field in the
	control to get the current value

	I need to also be given an array of 4 shapes, the shapes are :
	NotChecked,Checked,NotCheckHeld,CheckedHeld.

	Since it is common for many checkboxes to share art, I accept
	a pointer to an array of 4 ScreenShape_t pointers.

**********************************/

/**********************************

	Destructors
	
**********************************/

static void BURGERCALL DialogControlCheckBoxDestroy(DialogControl_t *wow)
{
	DialogControlCheckBox_t *Input;
	Input = (DialogControlCheckBox_t *)wow;
	DeallocAPointer((void *)Input->TextPtr);
	Input->TextPtr = 0;
}

static void BURGERCALL DialogControlCheckBoxDelete(DialogControl_t *wow)
{
	DialogControlCheckBox_t *Input;
	Input = (DialogControlCheckBox_t *)wow;
	DeallocAPointer((void *)Input->TextPtr);
	DeallocAPointer(Input);
}

/**********************************

	This is an event filter. When a "clicked" event happens,
	I will toggle the Checked flag
	
**********************************/

static void BURGERCALL DialogControlCheckBoxEventProc(DialogControl_t *wow,DialogControlAction_e type)
{
	DialogControlCheckBox_t *Input;
	Input = (DialogControlCheckBox_t *)wow;
	if (type==BUTTON_CLICKED) {				/* Clicked? */
		Input->Checked = !Input->Checked;
	}
	/* Now call the user supplied proc if any */
	if (Input->Proc) {
		Input->Proc(&Input->Root,type);
	}
}

/**********************************

	Draw the check box and the text that follows to the right side
	The shapes are in the sequence (Not checked, Checked, Held not checked, Held Checked)
	
**********************************/

static void BURGERCALL DialogControlCheckBoxDraw(DialogControl_t *wow, int DrawX, int DrawY)
{
	Word Index;
	DialogControlCheckBox_t *Input;
	ScreenShape_t *ArtPtr, *TempPtr;
	Word i;
	LBPoint ShapeSize;
	int	CheckY, TextX, TextY;
	
	Input = (DialogControlCheckBox_t *)wow;
	
	i = 0;
	TempPtr = 0;
	if (Input->Art)
	{
		do {
			TempPtr = Input->Art[i];
			if (TempPtr) {					/* First valid shape */
				break;
			}
		} while (++i<4);
	}
	
	/* The rest gets tricky, since the text is optional */
	
	if (!TempPtr) {					/* Should NEVER happen */
		ShapeSize.x = 0;
		ShapeSize.y = 0;
	} else {
		ScreenShapeGetSize(TempPtr,&ShapeSize);		/* Get the size of this shape */
	}
	
	/* Let's figure out the height */
	
	if (Input->TextPtr) {								/* Text is present? */
		i = ShapeSize.x+6;							/* Padding between shape and text */
		TextX = i;				/* This is the X coord to draw the text */
		i = Input->FontPtr->FontHeight;
		if (i<(Word)ShapeSize.y) {
			i = ShapeSize.y;
		}
		CheckY = (i-ShapeSize.y)>>1;
		TextY = (i-Input->FontPtr->FontHeight)>>1;
	} else {
		CheckY = 0;
		TextX = 0;
		TextY = 0;
	}
	
	if (Input->Art)
	{
		Index = 0;					/* Dormant state */
		if (Input->Root.Inside && Input->Root.Focus) {
			Index = 2;					/* Cursor is over the button */
		}
		if (Input->Checked) {
			++Index;					/* Checked/Unchecked */
		}
		
		ArtPtr = Input->Art[Index];
		if (ArtPtr) {					/* Is there a picture? */
			ScreenShapeDraw(ArtPtr,Input->Root.Bounds.left+DrawX,Input->Root.Bounds.top+CheckY+DrawY);
		}
	}
	if (Input->TextPtr) {				/* Text is optional */
		FontDrawStringAtXY(Input->FontPtr,Input->Root.Bounds.left+TextX+DrawX,Input->Root.Bounds.top+TextY+DrawY,Input->TextPtr);	/* Draw the string */
	}
}

/**********************************

	Initialize a check box control
	
**********************************/

void BURGERCALL DialogControlCheckBoxInit(DialogControlCheckBox_t *Input,ScreenShape_t **ShapeArray,const char *Text,struct FontRef_t *FontPtr,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc,Word Checked)
{
#if 0
	Word i;
	LBPoint ShapeSize;
	ScreenShape_t *TempPtr;
#endif
	
	/* Init the base class */
	
	Input->Root.Bounds = *Bounds;
	Input->Root.Active = TRUE;
	Input->Root.Invisible = FALSE;
	Input->Root.Focus = FALSE;		/* Doesn't have focus */
	Input->Root.Inside = FALSE;		/* Set the current default mode */
	Input->Root.Draw = DialogControlCheckBoxDraw;
	Input->Root.Delete = DialogControlCheckBoxDestroy;
	Input->Root.Check = DialogControlCheck;
	Input->Root.Event = DialogControlCheckBoxEventProc;		/* No button specific callback */
	Input->Root.RefCon = 0;
	Input->Root.HotKey = HotKey;

	/* Now init my local variables */
	
	Input->Proc = EventProc;
	Input->Checked = Checked;
	
#if 0
	if (!FontPtr || !Text) {			/* This is a failsafe in case you were stupid enough */
		Text = 0;						/* to not set up a font to draw your text with */
		FontPtr = 0;
	}

	Input->FontPtr = FontPtr;			/* Set the font and string */
	if (!Text) {
		Input->TextPtr = Text;			/* Store the zero */
	} else {
		Input->TextPtr = StrCopy(Text);	/* Make a copy of the string */
	}
	
	/* Copy the array of shapes */
	
	Input->Art = ShapeArray;
	i = 0;
	TempPtr = 0;
	do {
		TempPtr = ShapeArray[i];
		if (TempPtr) {					/* First valid shape */
			break;
		}
	} while (++i<4);

	/* The rest gets tricky, since the text is optional */
	
	if (!TempPtr) {					/* Should NEVER happen */
		ShapeSize.x = 0;
		ShapeSize.y = 0;
	} else {
		ScreenShapeGetSize(TempPtr,&ShapeSize);		/* Get the size of this shape */
	}
	
	/* Let's figure out the height */
	
	if (Text) {								/* Text is present? */
		i = ShapeSize.x+6;							/* Padding between shape and text */
		Input->TextX = i;				/* This is the X coord to draw the text */
		Input->Root.Bounds.right = FontWidthString(FontPtr,Text)+x+i;		/* Now here is the full with of the bounding box */
		i = FontPtr->FontHeight;
		if (i<(Word)ShapeSize.y) {
			i = ShapeSize.y;
		}
		Input->Root.Bounds.bottom = y+i;	/* Bounds rect bottom */
		Input->CheckY = (i-ShapeSize.y)>>1;
		Input->TextY = (i-FontPtr->FontHeight)>>1;
	} else {
		Input->Root.Bounds.right = x+ShapeSize.x;		/* Use the width of the art only */
		Input->CheckY = 0;
		Input->TextX = 0;
		Input->TextY = 0;
		Input->Root.Bounds.bottom = y+ShapeSize.y;		/* This is pretty simple */
	}
#else
	DialogControlCheckBoxSetText(Input,ShapeArray,Text,FontPtr);
#endif
}



void BURGERCALL DialogControlCheckBoxSetText(DialogControlCheckBox_t *Input,struct ScreenShape_t **ShapeArray,const char *Text,struct FontRef_t *FontPtr)
{
	if (!FontPtr || !Text) {			/* This is a failsafe in case you were stupid enough */
		Text = 0;						/* to not set up a font to draw your text with */
		FontPtr = 0;
	}

	Input->FontPtr = FontPtr;			/* Set the font and string */
	if (!Text) {
		Input->TextPtr = Text;			/* Store the zero */
	} else {
		Input->TextPtr = StrCopy(Text);	/* Make a copy of the string */
	}
	
	/* Copy the array of shapes */
	
	Input->Art = ShapeArray;
}

/**********************************

	Create a new check box control
	
**********************************/

DialogControlCheckBox_t *BURGERCALL DialogControlCheckBoxNew(ScreenShape_t **ShapeArray,const char *Text,struct FontRef_t *FontPtr,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc,Word Checked)
{
	DialogControlCheckBox_t *MyPtr;

	MyPtr = (DialogControlCheckBox_t *)AllocAPointer(sizeof(DialogControlCheckBox_t));	/* Get the memory */
	if (MyPtr) {
		DialogControlCheckBoxInit(MyPtr,ShapeArray,Text,FontPtr,Bounds,HotKey,EventProc,Checked);		/* Init it */
		MyPtr->Root.Delete = DialogControlCheckBoxDelete;
	}
	return MyPtr;			/* Return the pointer */
}


/**********************************

	This is a horizontal slider bar. The control is given an range
	and an initial value. This control is automatic and
	an event callback is not needed if dynamic user interaction
	is not required. Just look at the "Value" field in the
	control to get the current value

	I need to also be given an array of 5 shapes, the shapes are :
	Thumb,Thumb Pressed,Bar,LeftBar,RightBar.

	Since it is common for many slider bars to share art, I accept
	a pointer to an array of 5 ScreenShape_t pointers.

**********************************/


/**********************************

	Draw a slider bar
	
**********************************/

static void BURGERCALL DialogControlSliderBarDraw(DialogControl_t *wow, int DrawX, int DrawY)
{
	Word Index;
	DialogControlSliderBar_t *Input;
	Input = (DialogControlSliderBar_t *)wow;
	
	if (!Input->Art)
		return;
	
	/* First I need to draw the actual bar */
	
	ScreenShapeHPatternBar(&Input->Art[2],Input->Root.Bounds.top+Input->BarY+DrawY,Input->Root.Bounds.left+DrawX,Input->Root.Bounds.right+DrawX);
	
	/* Now, draw the thumb */
	
	Index = Input->Root.Focus;		/* Cursor is over the button */
	ScreenShapeDraw(Input->Art[Index],Input->Root.Bounds.left+Input->ThumbX+DrawX,Input->Root.Bounds.top+Input->ThumbY+DrawY);
}

/**********************************

	Check if a point is within a button
	I return one of these values...
	BUTTON_DOWN = If the mouse is held down and I want to keep focus
	BUTTON_CLICKED = If the mouse was released inside me and I had focus
	BUTTON_RELEASED = If the mouse was released out of me and I had focus
	BUTTON_INSIDE = If the mouse is over the control
	BUTTON_OUTSIDE = If the mouse is outside of the control

**********************************/

static DialogControlAction_e BURGERCALL DialogControlSliderCheck(DialogControl_t *Inputx,int x,int y,Word buttons,Word /* Key */)
{
	DialogControlAction_e Result;
	LBRect Bounds2;
	ScreenShape_t *ShapePtr;
	DialogControlSliderBar_t *Input;
	
	Input = (DialogControlSliderBar_t *)Inputx;
	if (buttons) {
		buttons = TRUE; 		/* I only want TRUE or false */
	}
	
	ShapePtr = Input->Art[0];
	Bounds2.top = Input->Root.Bounds.top+Input->ThumbY;
	Bounds2.bottom = Bounds2.top+ShapePtr->Height;
	Bounds2.left = Input->Root.Bounds.left+Input->ThumbX;
	Bounds2.right = Bounds2.left+ShapePtr->Width;
	
	Input->Root.Inside = static_cast<Word8>(LBRectPtInRect(&Bounds2,x,y));		/* Inside of the rect? */
				
	/* Mouse down? */
	
	if (buttons) {
		
		/* Dragging? */
		
		if (Input->Root.Focus) {
			int NewX;
			
			/* Determine the X coordinate on the bar */

			NewX = (((x-Input->ThumbAnchor)-Input->Root.Bounds.left)-Input->ThumbMinX);
			if (NewX<0) {			/* Bounds check */
				NewX = 0;
			}
			if ((Word)NewX>=Input->BarWidth) {
				NewX = Input->BarWidth;
			}
			
			/* Ok, Now where will the thumb map to? */
		
			Input->Value = ((NewX*Input->Range)+(Input->BarWidth>>1))/Input->BarWidth;
			if (Input->BarWidth>Input->Range) {						/* Is the range larger? */
				/* I need to keep the thumb on the tick marks */
				NewX = (Input->Value*Input->BarWidth)/Input->Range;
			}
			Input->ThumbX = NewX+Input->ThumbMinX;		/* Store the new X coordinate */
			
			/* Keep focus */
			
			Result = BUTTON_DOWN;
			goto Exit;
		}
		
		/* First time? */
		/* Mark the anchor point */
		
		if (Input->Root.Inside) {
			Input->ThumbAnchor = x-Bounds2.left;
			Input->Root.Focus = TRUE;			/* I'll keep focus */
			Result = BUTTON_DOWN;			/* Grab focus */
			goto Exit;
		}
	} else if (Input->Root.Focus) {		/* I only care if I have focus */
		Input->Root.Focus = FALSE;		/* Drop focus */

		/* Did I release OUTSIDE? */
		
		if (!Input->Root.Inside) {
			Result = BUTTON_RELEASED;		/* Don't accept the click */
		} else {
			Result = BUTTON_CLICKED;		/* I accept this click */
		}
		goto Exit;
	}

	/* Ok, now return my state flag */
	
	Result = BUTTON_INSIDE;
	if (!Input->Root.Inside) {
		Result = BUTTON_OUTSIDE;		/* I'm not active */
	}
Exit:;
	/* Do I have a callback proc? */
	if (Input->Root.Event) {
		Input->Root.Event(&Input->Root,Result);	/* Pass the current state */
	}
	return Result;
}


/**********************************

	Assign a new value

**********************************/

void BURGERCALL DialogControlSliderBarSetValue(DialogControlSliderBar_t *Input,Word NewValue)
{
	if (NewValue>Input->Range) {
		NewValue = Input->Range;			/* Bounds check */
	}
	Input->Value = NewValue;
	Input->ThumbX = ((NewValue*Input->BarWidth)/Input->Range)+Input->ThumbMinX;		/* Store the new Y coordinate */	
}

/**********************************

	Given a data range, determine the size
	of the slider bar control
	
**********************************/

void BURGERCALL DialogControlSliderBarSetParms(DialogControlSliderBar_t *Input,Word Range)
{
	Word Value;
	LBPoint ShapeSize1;		/* Size of the main thumb */
	LBPoint ShapeSize2;		/* Size of the bar */
	Word SmallY,BigY;
	Word Width;

	Value = Input->Value;
	if (Value>Range) {			/* Is the value too big? */
		Value = Range;			/* Max out the value */
	}
	if (!Range) {				/* Prevent a divide by zero */
		Range = 1;
	}
	Input->Range = Range;
	Input->Value = Value;

	ScreenShapeGetSize(Input->Art[0],&ShapeSize1);		/* Get the size of the thumb */
	BigY = ShapeSize1.x;				/* Width of the thumb */
	ScreenShapeGetSize(Input->Art[3],&ShapeSize1);		/* Get the size of the left */
	ScreenShapeGetSize(Input->Art[4],&ShapeSize2);		/* Get the size of the right */
	SmallY = ShapeSize1.x;					/* Minimum X for the thumb */
	Input->ThumbMinX = SmallY;				/* Save it */
	SmallY += BigY;							/* Add the width of the thumb */
	SmallY += ShapeSize2.x;					/* Add the width of the right edge */
	Width = Input->Root.Bounds.right-Input->Root.Bounds.left;
	if (Width<=SmallY) {					/* Is this scroll bar too small? */
		Input->Value = 0;					/* Your value will always be zero LOSER!!! */
		Input->BarWidth = 1;				/* Must prevent divide by zero */
		Input->ThumbX = 0;					/* You are screwed */
	} else {
	
		/* Here we can actually set up the initial position of the thumb */
		Width = Width-SmallY;		/* Size of the bar in pixels */
		Input->BarWidth = Width;

		/* I need to keep the thumb on the tick marks */
		
		BigY = (Value*Width)/Range;
		Input->ThumbX = BigY+Input->ThumbMinX;		/* Store the new X coordinate */
	}
}

/**********************************

	Given a pointer to an uninitialized Button, Set the defaults

**********************************/

void BURGERCALL DialogControlSliderBarInit(DialogControlSliderBar_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range)
{
	/* Init the base class */
	Input->Root.Bounds = *Bounds;
	Input->Root.Active = TRUE;
	Input->Root.Invisible = FALSE;
	Input->Root.Focus = FALSE;		/* Doesn't have focus */
	Input->Root.Inside = FALSE;		/* Set the current default mode */
	Input->Root.Draw = DialogControlSliderBarDraw;
	Input->Root.Delete = 0;
	Input->Root.Check = DialogControlSliderCheck;
	Input->Root.Event = EventProc;		/* No button specific callback */
	Input->Root.RefCon = 0;
	Input->Root.HotKey = 0;

	/* Now init my local variables */
	
	Input->ThumbAnchor = 0;
	Input->Value = Value;		/* Set the range */
#if 0
	Input->Art = ArtArray;		/* Set the art */
	Input->ThumbAnchor = 0;
	Input->Value = Value;		/* Set the range */

	ScreenShapeGetSize(ArtArray[0],&ShapeSize1);		/* Get the size of the thumb */
	ScreenShapeGetSize(ArtArray[2],&ShapeSize2);		/* Get the size of the bar */
	Input->Root.Bounds.left = x;
	Input->Root.Bounds.top = y;
	Input->Root.Bounds.right = x+Width;
	SmallY = ShapeSize1.y;			/* Height of the thumb */
	BigY = ShapeSize2.y;			/* Height of the bar */
	if (SmallY<BigY) {		/* Is the thumb shorter than the bar? */
		Input->BarY = 0;
		Input->ThumbY = (BigY-SmallY)>>1;
		y += BigY;
	} else {
		Input->ThumbY = 0;
		Input->BarY = (SmallY-BigY)>>1;
		y += SmallY;
	}
	Input->Root.Bounds.bottom = y;		/* Set the new bottom Y */
#else
	DialogControlSliderBarSetArt(Input,ArtArray);
#endif
	
	/* Now that I have the height, let's determine the width and */
	/* movement range of the thumb based on the size of the thumb */
	/* and the edge pieces */
	
	DialogControlSliderBarSetParms(Input,Range);
}

void BURGERCALL DialogControlSliderBarSetArt(DialogControlSliderBar_t *Input,struct ScreenShape_t **ArtArray)
{
	LBPoint ShapeSize1;		/* Size of the main thumb */
	LBPoint ShapeSize2;		/* Size of the bar */
	Word SmallY,BigY;
	
	Input->Art = ArtArray;		/* Set the art */
	
	if (ArtArray)
	{
		ScreenShapeGetSize(ArtArray[0],&ShapeSize1);		/* Get the size of the thumb */
		ScreenShapeGetSize(ArtArray[2],&ShapeSize2);		/* Get the size of the bar */
		SmallY = ShapeSize1.y;			/* Height of the thumb */
		BigY = ShapeSize2.y;			/* Height of the bar */
		if (SmallY<BigY) {		/* Is the thumb shorter than the bar? */
			Input->BarY = 0;
			Input->ThumbY = (BigY-SmallY)>>1;
		} else {
			Input->ThumbY = 0;
			Input->BarY = (SmallY-BigY)>>1;
		}
	}
}

/**********************************

	Allocate a new DialogControlSliderBar_t struct and init it

**********************************/

DialogControlSliderBar_t *BURGERCALL DialogControlSliderBarNew(struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range)
{
	DialogControlSliderBar_t *MyPtr;

	MyPtr = (DialogControlSliderBar_t *)AllocAPointer(sizeof(DialogControlSliderBar_t));	/* Get the memory */
	if (MyPtr) {
		DialogControlSliderBarInit(MyPtr,ArtArray,Bounds,EventProc,Value,Range);		/* Init it */
		MyPtr->Root.Delete = DialogControlDeleteProc;
	}
	return MyPtr;			/* Return the pointer */
}




/**********************************

	This is a repeating button. You give me 3 pieces
	of art, the button in a dormant state, the button
	when the mouse is over it and the button when
	it's depressed. Since this button returns an action,
	you are required to give an event proc that handles
	a BUTTON_CLICKED event. Otherwise this button
	performs no useful action. This button will send "CLICKED"
	events to the action proc at regular intervals while
	the button is still pressed

**********************************/

/**********************************

	Check for a repeat event

**********************************/

static DialogControlAction_e BURGERCALL DialogControlRepeatButtonCheck(DialogControl_t *wow,int x,int y,Word buttons,Word Key)
{
	DialogControlAction_e Result;
	DialogControlRepeatButton_t *Input;
	
	if (buttons) {
		buttons = TRUE; 		/* I only want TRUE or false */
	}
	
	Input = (DialogControlRepeatButton_t *)wow;
	Input->Root.Inside = static_cast<Word8>(LBRectPtInRect(&Input->Root.Bounds,x,y));		/* Inside of the rect? */
	
	/* Now I have the current motion state */
			
	/* Mouse down? */
	
	if (buttons) {
		Word32 Mark;
		if (Input->Root.Focus) {
			Mark = ReadTick();								/* Get the time mark */
			if ((Mark-Input->TimeMark)>=Input->RepeatDelay) {	/* Did enough time elapse? */
				Input->TimeMark = Mark;						/* Reset the timer */
				Input->RepeatDelay = TICKSPERSEC/6;
				if (Input->Root.Inside) {					/* Do I have focus and I'm inside? */
					if (Input->Root.Event) {
						Input->Root.Event(&Input->Root,BUTTON_CLICKED);		/* Send the event */
					}
				}
			}
			Result = BUTTON_DOWN;			/* Grab focus */
			goto Exit;
		}

		if (Input->Root.Inside) {
			Input->TimeMark = ReadTick();						/* Initial hit, store the mark */
			Input->RepeatDelay = TICKSPERSEC/3;			/* Long initial delay */
			Input->Root.Focus = TRUE;			/* I'll keep focus */
			if (Input->Root.Event) {
				Input->Root.Event(&Input->Root,BUTTON_CLICKED);		/* Send the event */
			}
			Result = BUTTON_DOWN;			/* Grab focus */
			goto Exit;
		}

	} else if (Input->Root.Focus) {		/* I only care if I have focus */
		Input->Root.Focus = FALSE;		/* Drop focus */

		/* Did I release OUTSIDE? */

		Result = BUTTON_RELEASED;		/* Don't accept the click */
		goto Exit;
	}

	/* Did I press a hot key? */
	
	if (Key) {			/* Was a hot key pressed? */
		if (Key>='a' && Key<('z'+1)) {
			Key &= 0xDF;
		}
		if (Key==Input->Root.HotKey) {
			Result = BUTTON_CLICKED;
			goto Exit;
		}
	}

	/* Ok, now return my state flag */
	
	Result = BUTTON_INSIDE;
	if (!Input->Root.Inside) {
		Result = BUTTON_OUTSIDE;		/* I'm not active */
	}
Exit:;
	/* Do I have a callback proc? */
	if (Input->Root.Event) {
		Input->Root.Event(&Input->Root,Result);	/* Pass the current state */
	}
	return Result;
}

/**********************************

	Given a pointer to an uninitialized Button, Set the defaults

**********************************/

void BURGERCALL DialogControlRepeatButtonInit(DialogControlRepeatButton_t *Input,ScreenShape_t *Shape1,ScreenShape_t *Shape2,ScreenShape_t *Shape3,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc)
{
	/* Init the base class */
	Input->Root.Bounds = *Bounds;
	Input->Root.Active = TRUE;
	Input->Root.Invisible = FALSE;
	Input->Root.Focus = FALSE;		/* Doesn't have focus */
	Input->Root.Inside = FALSE;		/* Set the current default mode */
	Input->Root.Draw = DialogControlButtonDraw;
	Input->Root.Delete = 0;
	Input->Root.Check = DialogControlRepeatButtonCheck;
	Input->Root.Event = EventProc;		/* No button specific callback */
	Input->Root.RefCon = 0;
	Input->Root.HotKey = HotKey;

	/* Now init my local variables */

	Input->RepeatDelay = 0;
	Input->TimeMark = 0;	
#if 0
	Input->Art[0] = Shape1;		/* Make a new frame */
	Input->Art[1] = Shape2;
	Input->Art[2] = Shape3;
	if (!Shape1) {				/* No root shape */
		Shape1 = Shape2;		/* Use the second shape */
		if (!Shape1) {			/* Still no shape? */
			Shape1 = Shape3;	/* Try this one */
		}
	}
	ScreenShapeGetBounds(Shape1,&Input->Root.Bounds);
	Input->x = Input->Root.Bounds.left;
	Input->y = Input->Root.Bounds.top;
	Input->Root.Bounds.left += x;
	Input->Root.Bounds.top += y;
	Input->Root.Bounds.right += x;
	Input->Root.Bounds.bottom += y;
#else
	DialogControlRepeatButtonSetArt(Input,Shape1,Shape2,Shape3);
#endif
}


void BURGERCALL DialogControlRepeatButtonSetArt(DialogControlRepeatButton_t *Input,struct ScreenShape_t *Shape1,struct ScreenShape_t *Shape2,struct ScreenShape_t *Shape3)
{
	Input->Art[0] = Shape1;		/* Make a new frame */
	Input->Art[1] = Shape2;
	Input->Art[2] = Shape3;
	if (!Shape1) {				/* No root shape */
		Shape1 = Shape2;		/* Use the second shape */
		if (!Shape1) {			/* Still no shape? */
			Shape1 = Shape3;	/* Try this one */
		}
	}
	ScreenShapeGetBounds(Shape1,&Input->Root.Bounds);
	Input->x = Input->Root.Bounds.left;
	Input->y = Input->Root.Bounds.top;
}


/**********************************

	Allocate a new DialogControlButton_t struct and init it

**********************************/

DialogControlRepeatButton_t * BURGERCALL DialogControlRepeatButtonNew(ScreenShape_t *Shape1,ScreenShape_t *Shape2,ScreenShape_t *Shape3,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc)
{
	DialogControlRepeatButton_t *MyPtr;

	MyPtr = (DialogControlRepeatButton_t *)AllocAPointer(sizeof(DialogControlRepeatButton_t));	/* Get the memory */
	if (MyPtr) {
		DialogControlRepeatButtonInit(MyPtr,Shape1,Shape2,Shape3,Bounds,HotKey,EventProc);		/* Init it */
		MyPtr->Root.Delete = DialogControlDeleteProc;
	}
	return MyPtr;			/* Return the pointer */
}




/**********************************

	This is a vertical scroll slider bar. The control is given an range
	and an initial value and a thumb ratio. This control is automatic and
	an event callback is not needed if dynamic user interaction
	is not required. Just look at the "Value" field in the
	control to get the current value

	I need to also be given an array of 11 shapes, the shapes are :
	EmptyPattern,EmptyTop,EmptyBottom,
	ThumbPattern,ThumbTop,ThumbBottom,Thumb,
	ThumbPattern,ThumbTop,ThumbBottom,Thumb (Hilighted)

	Since it is common for many slider bars to share art, I accept
	a pointer to an array of 7 ScreenShape_t pointers.

**********************************/

/**********************************

	Draw a Vertical scroll slider bar
	I call DialogControlVScrollSliderDrawEmpty() to do the real work.
	
**********************************/

static void BURGERCALL DialogControlVScrollSliderDraw(DialogControl_t *wow, int DrawX, int DrawY)
{
	LBPoint PicSize;
	DialogControlVScrollSlider_t *Input;
	ScreenShape_t *ArtPtr;
	int YCoord;
	int CenterY;
	Word Index;
	
	Input = (DialogControlVScrollSlider_t *)wow;
	
	if (!Input->Art)
		return;
	
	/* First I need to draw the actual bar */
	
	if (Input->Root.Focus==1) {
		Index = 7;					/* Draw the highlighted thumb? */
	} else {
		Index = 3;					/* Draw the normal thumb */
	}
	
	YCoord = Input->ThumbY+Input->Root.Bounds.top;		/* Top Y Coord of the thumb */
	ScreenShapeVPatternBar(Input->Art,Input->Root.Bounds.left+DrawX,Input->Root.Bounds.top+DrawY,YCoord+DrawY);		/* Top region */
	CenterY = ScreenShapeVPatternBar(&Input->Art[Index],Input->Root.Bounds.left+DrawX,YCoord+DrawY,YCoord+Input->ThumbSize+DrawY) - DrawY;	/* Center region */
	ScreenShapeVPatternBar(Input->Art,Input->Root.Bounds.left+DrawX,YCoord+Input->ThumbSize+DrawY,Input->Root.Bounds.bottom+DrawY);	/* Bottom region */
	if (CenterY!=-1) {					/* Was a thumb drawn? */
		ArtPtr = Input->Art[Index+3];
		ScreenShapeGetSize(ArtPtr,&PicSize);		/* Make sure that Width & Height are valid */
		if (Input->ThumbSize>=(Word)PicSize.y) {	/* Is the thumb large enough? */
			ScreenShapeDraw(ArtPtr,Input->Root.Bounds.left+(((Input->Root.Bounds.right-Input->Root.Bounds.left)-PicSize.x)>>1)+DrawX,CenterY-(PicSize.y>>1)+DrawY);
		}
	}
}

/**********************************

	Check if a point is within a button
	I return one of these values...
	BUTTON_DOWN = If the mouse is held down and I want to keep focus
	BUTTON_CLICKED = If the mouse was released inside me and I had focus
	BUTTON_RELEASED = If the mouse was released out of me and I had focus
	BUTTON_INSIDE = If the mouse is over the control
	BUTTON_OUTSIDE = If the mouse is outside of the control

**********************************/

static DialogControlAction_e BURGERCALL DialogControlVScrollSliderCheck(DialogControl_t *Inputx,int x,int y,Word buttons,Word /* Key */)
{
	DialogControlAction_e Result;
	DialogControlVScrollSlider_t *Input;
		
	Input = (DialogControlVScrollSlider_t *)Inputx;
	if (buttons) {
		buttons = TRUE; 		/* I only want TRUE or false */
	}

	Input->Root.Inside = static_cast<Word8>(LBRectPtInRect(&Input->Root.Bounds,x,y));		/* Inside of the rect? */
				
	/* Mouse down? */
	
	if (buttons) {
		int NewY;
		
		/* Dragging? */
		
		if (Input->Root.Focus) {
			
			/* Determine the X coordinate on the bar */
			if (Input->Root.Focus==1) {
				NewY = ((y-Input->ThumbAnchor)-Input->Root.Bounds.top);
				if (NewY<0) {			/* Bounds check */
					NewY = 0;
				}
				if ((Word)NewY>=Input->BarHeight) {
					NewY = Input->BarHeight;
				}
				
				/* Ok, Now where will the thumb map to? */
			
				Input->Value = ((NewY*Input->Range)+(Input->BarHeight>>1))/Input->BarHeight;
				if (Input->BarHeight>Input->Range) {						/* Is the range larger? */
					/* I need to keep the thumb on the tick marks */
					NewY = (Input->Value*Input->BarHeight)/Input->Range;
				}
				Input->ThumbY = NewY+1;		/* Store the new X coordinate */	
			} else {
				Word32 Mark;
				Mark = ReadTick();								/* Get the time mark */
				if ((Mark-Input->TimeMark)>=Input->RepeatDelay) {	/* Did enough time elapse? */
					LBRect Bounds2;
					Input->TimeMark = Mark;						/* Reset the timer */
					Input->RepeatDelay = TICKSPERSEC/6;
					Bounds2.left = Input->Root.Bounds.left;
					Bounds2.right = Input->Root.Bounds.right;
					if (Input->Root.Focus==2) {
						Bounds2.top = Input->Root.Bounds.top;
						Bounds2.bottom = Bounds2.top+Input->ThumbY;
					} else {
						Bounds2.top = Input->Root.Bounds.top+Input->ThumbY+Input->ThumbSize;
						Bounds2.bottom = Input->Root.Bounds.bottom;
					}
					if (LBRectPtInRect(&Bounds2,x,y)) {					/* Do I have focus and I'm inside? */
						Word NewValue;
						NewValue = Input->Value;
						if (Input->Root.Focus==2) {						/* Up button */
							NewValue = NewValue-Input->Step;
							if ((int)NewValue<0) {
								NewValue = 0;
							}
						} else {
							NewValue = NewValue+Input->Step;			/* Down button */
							if (NewValue>Input->Range) {
								NewValue = Input->Range;
							}
						}
						Input->Value = NewValue;						/* Set the new value */
						Input->ThumbY = ((NewValue*Input->BarHeight)/Input->Range)+1;		/* Store the new Y coordinate */	
					}
				}
			}
			/* Keep focus */
			
			Result = BUTTON_DOWN;
			goto Exit;
		}
		
		/* First time? */
		/* Mark the anchor point */
		
		if (Input->Root.Inside) {				/* Am I in the control? */
		
			NewY = Input->Root.Bounds.top+Input->ThumbY;		/* Top of the thumb */
			
			/* Slider? */
			
			if ((y>=NewY) && y<(NewY+(int)Input->ThumbSize)) {		/* Am I in the thumb? */
				Input->ThumbAnchor = y-NewY;	/* Set the anchor */
				Input->Root.Focus = 1;			/* I'll keep focus */
			} else {
			
			/* It's the top or bottom */
			
				Word NewValue;
				NewValue = Input->Value;
				if (y<NewY) {
					NewValue = NewValue-Input->Step;
					if ((int)NewValue<0) {
						NewValue = 0;
					}
					Input->Root.Focus = 2;			/* I'll keep focus */
				} else {
					NewValue = NewValue+Input->Step;
					if (NewValue>Input->Range) {
						NewValue = Input->Range;
					}
					Input->Root.Focus = 3;			/* I'll keep focus */
				}
				Input->Value = NewValue;
				Input->ThumbY = ((NewValue*Input->BarHeight)/Input->Range)+1;		/* Store the new Y coordinate */	
				Input->RepeatDelay = TICKSPERSEC/3;
				Input->TimeMark = ReadTick();
			}
			Result = BUTTON_DOWN;			/* Grab focus */
			goto Exit;
		}
	} else if (Input->Root.Focus) {		/* I only care if I have focus */
		Input->Root.Focus = FALSE;		/* Drop focus */

		/* Did I release OUTSIDE? */
		
		if (!Input->Root.Inside) {
			Result = BUTTON_RELEASED;		/* Don't accept the click */
		} else {
			Result = BUTTON_CLICKED;		/* I accept this click */
		}
		goto Exit;
	}

	/* Ok, now return my state flag */
	
	Result = BUTTON_INSIDE;
	if (!Input->Root.Inside) {
		Result = BUTTON_OUTSIDE;		/* I'm not active */
	}
Exit:;
	/* Do I have a callback proc? */
	if (Input->Root.Event) {
		Input->Root.Event(&Input->Root,Result);	/* Pass the current state */
	}
	return Result;
}

/**********************************

	Assign a new value

**********************************/

void BURGERCALL DialogControlVScrollSliderSetValue(DialogControlVScrollSlider_t *Input,Word NewValue)
{
	if (NewValue>Input->Range) {
		NewValue = Input->Range;			/* Bounds check */
	}
	Input->Value = NewValue;
	Input->ThumbY = ((NewValue*Input->BarHeight)/Input->Range)+1;		/* Store the new Y coordinate */	
}

/**********************************

	Given a data range and a step value, determine the size
	of the scroll bar control
	
**********************************/

void BURGERCALL DialogControlVScrollSliderSetParms(DialogControlVScrollSlider_t *Input,Word Range,Word Step)
{
	Word Value;
	Word Height;
	Value = Input->Value;		/* Get the current value */
	
	if (Value>Range) {			/* Is the value too big? */
		Value = Range;			/* Max out the value */
	}
	if (!Range) {				/* Prevent a divide by zero */
		Range = 1;
	}
	if (!Step) {				/* Prevent a divide by zero */
		Step = 1;
	}
	if (Step>Range) {			
		Step = Range;			/* Out of bounds? */
	}
	Input->Value = Value;		/* Set the range */
	Input->Range = Range;
	Input->Step = Step;
	
	Height = Input->Root.Bounds.bottom-Input->Root.Bounds.top;		/* Height of the control */
	
	/* Now that I have the height, let's determine the width and */
	/* movement range of the thumb based on the size of the thumb */
	/* and the edge pieces */
	
	if (Height<=2) {					/* Is this scroll bar too small? */
		Input->Value = 0;			/* Your value will always be zero LOSER!!! */
		Input->ThumbSize = 0;		/* No thumb */
		Input->BarHeight = 1;		/* Must prevent divide by zero */
		Input->ThumbY = 0;			/* You are screwed */
	} else {
	
		/* Here we can actually set up the initial position of the thumb */
		
		Height = Height-2;			/* A little slop */
		Step = (Step*Height)/(Range+Step);	/* Figure out the size of the thumb */
		if (Step<15) {
			Step = 15;				/* Minimum size */
		}
		Input->ThumbSize = Step;	/* Save it */
		Height = Height-Step;
		Input->BarHeight = Height;			/* Motion range */
		
		/* I need to keep the thumb on the tick marks */
		
		Input->ThumbY = ((Value*Height)/Range)+1;		/* Store the new Y coordinate */
	}
}

/**********************************

	Given a pointer to an uninitialized Button, Set the defaults

**********************************/

void BURGERCALL DialogControlVScrollSliderInit(DialogControlVScrollSlider_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range,Word Step)
{
#if 0
	LBPoint ShapeSize1;		/* Size of the main thumb */
#endif
	
	/* Init the base class */
	Input->Root.Bounds = *Bounds;
	Input->Root.Active = TRUE;
	Input->Root.Invisible = FALSE;
	Input->Root.Focus = FALSE;		/* Doesn't have focus */
	Input->Root.Inside = FALSE;		/* Set the current default mode */
	Input->Root.Draw = DialogControlVScrollSliderDraw;
	Input->Root.Delete = 0;
	Input->Root.Check = DialogControlVScrollSliderCheck;
	Input->Root.Event = EventProc;		/* No button specific callback */
	Input->Root.RefCon = 0;
	Input->Root.HotKey = 0;

	/* Now init my local variables */
	
	Input->Art = ArtArray;		/* Set the art */
	Input->ThumbAnchor = 0;
	Input->RepeatDelay = 0;
	Input->TimeMark = 0;
	Input->Value = Value;
#if 0
	ScreenShapeGetSize(ArtArray[0],&ShapeSize1);		/* Get the width of the control */
	Input->Root.Bounds.left = x;
	Input->Root.Bounds.top = y;
	Input->Root.Bounds.bottom = y+Height;
	Input->Root.Bounds.right = x+ShapeSize1.x;		/* Set the new bottom Y */
#endif

	/* Now that I have the height, let's determine the width and */
	/* movement range of the thumb based on the size of the thumb */
	/* and the edge pieces */
	
	DialogControlVScrollSliderSetParms(Input,Range,Step);
}


/**********************************

	Allocate a new DialogControlVScrollSlider_t struct and init it

**********************************/

DialogControlVScrollSlider_t *BURGERCALL DialogControlVScrollSliderNew(struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range,Word Step)
{
	DialogControlVScrollSlider_t *MyPtr;

	MyPtr = (DialogControlVScrollSlider_t *)AllocAPointer(sizeof(DialogControlVScrollSlider_t));	/* Get the memory */
	if (MyPtr) {
		DialogControlVScrollSliderInit(MyPtr,ArtArray,Bounds,EventProc,Value,Range,Step);		/* Init it */
		MyPtr->Root.Delete = DialogControlDeleteProc;
	}
	return MyPtr;			/* Return the pointer */
}




/**********************************

	Clicked the up button on the vertical scroll bar

**********************************/

static void BURGERCALL DialogControlVScrollUpProc(DialogControl_t *wow,DialogControlAction_e type)
{
	if (type==BUTTON_CLICKED) {								/* Clicked? */
		DialogControlVScrollSlider_t *Slider;
		DialogControlVScroll_t *Input;
		int New;
		Input = (DialogControlVScroll_t *)wow->RefCon;		/* Get the root control */
		Slider = Input->Slider;
		New = (Slider->Value-Input->ButtonStep);					/* Move it up */
		if (New<0) {			/* Check here, since the function only accepts unsigned values */
			New = 0;
		}
		DialogControlVScrollSliderSetValue(Slider,New);		/* Set the scroll bar to the new value */
	}
}

/**********************************

	Clicked the down button on the vertical scroll bar

**********************************/

static void BURGERCALL DialogControlVScrollDownProc(DialogControl_t *wow,DialogControlAction_e type)
{
	if (type==BUTTON_CLICKED) {				/* Clicked? */
		DialogControlVScrollSlider_t *Slider;
		DialogControlVScroll_t *Input;

		Input = (DialogControlVScroll_t *)wow->RefCon;
		Slider = Input->Slider;
		DialogControlVScrollSliderSetValue(Slider,Slider->Value+Input->ButtonStep);		/* New value */
	}
}

/**********************************

	Draw the Vertical scroll bar (Draw all the parts)

**********************************/

static void BURGERCALL DialogControlVScrollDraw(DialogControl_t *wow, int DrawX, int DrawY)
{
	DialogControlVScroll_t *Input;
	
	Input = (DialogControlVScroll_t *)wow;
	DialogControlListDraw(&Input->MyList, DrawX + Input->Root.Bounds.left, DrawY + Input->Root.Bounds.top);		/* Draw my controls */
}

/**********************************

	Check if a point is within a button
	I return one of these values...
	BUTTON_DOWN = If the mouse is held down and I want to keep focus
	BUTTON_CLICKED = If the mouse was released inside me and I had focus
	BUTTON_RELEASED = If the mouse was released out of me and I had focus
	BUTTON_INSIDE = If the mouse is over the control
	BUTTON_OUTSIDE = If the mouse is outside of the control

**********************************/

static DialogControlAction_e BURGERCALL DialogControlVScrollCheck(DialogControl_t *Inputx,int x,int y,Word buttons,Word Key)
{
	DialogControlVScroll_t *Input;
	DialogControlAction_e Result;
	
	Input = (DialogControlVScroll_t *)Inputx;
	DialogControlListCheck(&Input->MyList,x,y,buttons,Key);			/* Pass the event to the control list */

	Result = DialogControlCheck(&Input->Root,x,y,buttons,Key);
	Input->Value = Input->Slider->Value;							/* Propagate the slider bar value to my root */
	return Result;													/* Return the value */
}

/**********************************

	Destructors

**********************************/

static void BURGERCALL DialogControlVScrollDestroy(DialogControl_t *Input)
{
	DialogControlListDestroy(&((DialogControlVScroll_t *)Input)->MyList);
}

static void BURGERCALL DialogControlVScrollDelete(DialogControl_t *Input)
{
	DialogControlListDestroy(&((DialogControlVScroll_t *)Input)->MyList);
	DeallocAPointer(Input);
}

/**********************************

	Set a new value for the scoll bar

**********************************/

void BURGERCALL DialogControlVScrollSetValue(DialogControlVScroll_t *Input,Word NewValue)
{
	DialogControlVScrollSlider_t *Slider;

	Slider = Input->Slider;
	DialogControlVScrollSliderSetValue(Slider,NewValue);		/* New value */
	Input->Value = Slider->Value;
}

/**********************************

	Given a data range and a step value, determine the size
	of the scroll bar control
	
**********************************/

void BURGERCALL DialogControlVScrollSetParms(DialogControlVScroll_t *Input,Word Range,Word Step,Word ButtonStep)
{
	DialogControlVScrollSlider_t *Slider;

	Input->ButtonStep = ButtonStep;
	Slider = Input->Slider;
	DialogControlVScrollSliderSetParms(Slider,Range,Step);		/* New value */
	Input->Value = Slider->Value;								/* If the value changed, reflect it */
}

/**********************************

	Create a vertical scroll bar

**********************************/

void BURGERCALL DialogControlVScrollInit(DialogControlVScroll_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range,Word Step,Word ButtonStep,Bool NormalArrowStyle)
{
	Word CtrlHeight;
	DialogControlRepeatButton_t *Ctrl;
	ScreenShape_t *Shape;
	LBRect ButtonRect;
	Word Height;
	
	Input->Root.Bounds = *Bounds;
	Input->Root.Active = TRUE;
	Input->Root.Invisible = FALSE;
	Input->Root.Focus = FALSE;		/* Doesn't have focus */
	Input->Root.Inside = FALSE;		/* Set the current default mode */
	Input->Root.Draw = DialogControlVScrollDraw;
	Input->Root.Delete = DialogControlVScrollDestroy;
	Input->Root.Check = DialogControlVScrollCheck;
	Input->Root.Event = EventProc;		/* No button specific callback */
	Input->Root.RefCon = 0;
	Input->Root.HotKey = 0;
	
#if 0
	Input->Root.Bounds.left = x;
	Input->Root.Bounds.top = y;
	Input->Root.Bounds.bottom = y+Height;
#endif

	Input->ButtonStep = ButtonStep;
	
	Height = Bounds->bottom - Bounds->top;
	
	DialogControlListInit(&Input->MyList);		/* Init the master list */
	
	Shape = ArtArray[2];
	if (!Shape) {				/* No root shape */
		Shape = ArtArray[3];		/* Use the second shape */
	}
	ScreenShapeGetBounds(Shape,&ButtonRect);
	LBRectOffsetRect(&ButtonRect, -ButtonRect.left, -ButtonRect.top);
		
	Ctrl = DialogControlListAddRepeatButton(&Input->MyList,ArtArray[2],ArtArray[2],ArtArray[3],&ButtonRect,0,DialogControlVScrollDownProc);
	Ctrl->Root.RefCon = Input;
	CtrlHeight = (Ctrl->Root.Bounds.bottom-Ctrl->Root.Bounds.top);
	
	if( Height >= CtrlHeight )
		Height -= CtrlHeight;
		
	DialogControlMove(&Ctrl->Root,0,Height);
	
	Shape = ArtArray[0];
	if (!Shape) {				/* No root shape */
		Shape = ArtArray[1];		/* Use the second shape */
	}
	ScreenShapeGetBounds(Shape,&ButtonRect);
	LBRectOffsetRect(&ButtonRect, -ButtonRect.left, -ButtonRect.top);
	
	Ctrl = DialogControlListAddRepeatButton(&Input->MyList,ArtArray[0],ArtArray[0],ArtArray[1],&ButtonRect,0,DialogControlVScrollUpProc);
	Ctrl->Root.RefCon = Input;
	CtrlHeight = (Ctrl->Root.Bounds.bottom-Ctrl->Root.Bounds.top);
	
	if( Height >= CtrlHeight )
		Height -= CtrlHeight;	
	
	if( NormalArrowStyle == FALSE )
	{
		DialogControlMove(&Ctrl->Root,0,Height);
		CtrlHeight = 0;
	}
	
	ButtonRect = *Bounds;
	ButtonRect.top += CtrlHeight;
	ButtonRect.bottom = Height;
	
	Input->Slider =  DialogControlListAddVScrollSlider(&Input->MyList,&ArtArray[4],/*x,y+CtrlHeight,Height*/ &ButtonRect,0,Value,Range,Step);
	Input->Slider->Root.RefCon = Input;				/* Set the value */
	Input->Value = Input->Slider->Value;			/* Already bounds checked */
}

/**********************************

	Allocate a new DialogControlVScrollSlider_t struct and init it

**********************************/

DialogControlVScroll_t * BURGERCALL DialogControlVScrollNew(struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range,Word Step,Word ButtonStep,Bool NormalArrowStyle)
{
	DialogControlVScroll_t *MyPtr;

	MyPtr = (DialogControlVScroll_t *)AllocAPointer(sizeof(DialogControlVScroll_t));	/* Get the memory */
	if (MyPtr) {
		DialogControlVScrollInit(MyPtr,ArtArray,Bounds,EventProc,Value,Range,Step,ButtonStep,NormalArrowStyle);		/* Init it */
		MyPtr->Root.Delete = DialogControlVScrollDelete;
	}
	return MyPtr;			/* Return the pointer */
}



/**********************************

	Destructors

**********************************/

static void BURGERCALL DialogControlTextBoxDestroy(DialogControl_t *Input)
{
	DialogControlVScroll_t *Slider;			/* Scroll bar */
	Slider = ((DialogControlTextBox_t *)Input)->Slider;		/* Is there a scroll bar? */
	if (Slider) {
		DialogControlDelete(&Slider->Root);
	}
	FontWidthListDelete(((DialogControlTextBox_t *)Input)->TextDescription);	/* Delete my text formatting */
	DeallocAPointer((void *)((DialogControlTextBox_t *)Input)->TextPtr);		/* Delete my text */
}

static void BURGERCALL DialogControlTextBoxDelete(DialogControl_t *Input)
{
	DialogControlVScroll_t *Slider;			/* Scroll bar */
	Slider = ((DialogControlTextBox_t *)Input)->Slider;		/* Is there a scroll bar? */
	if (Slider) {
		DialogControlDelete(&Slider->Root);		/* Destroy it */
	}
	FontWidthListDelete(((DialogControlTextBox_t *)Input)->TextDescription);	/* Delete my text formatting */
	DeallocAPointer((void *)((DialogControlTextBox_t *)Input)->TextPtr);		/* Delete my text */
	DeallocAPointer(Input);
}

/**********************************

	Draw the text box

**********************************/

static void BURGERCALL DialogControlTextBoxDraw(DialogControl_t *wow, int DrawX, int DrawY)
{
	DialogControlTextBox_t *Input;
	DialogControlVScroll_t *Slider;			/* Scroll bar */
	Word Width,Height;
	LBRect TextRect;
	int x,y;
	
	Input = (DialogControlTextBox_t *)wow;
	Slider = Input->Slider;

	x = Input->Root.Bounds.left;				/* Get the origin X and Y */
	y = Input->Root.Bounds.top;
	Width = Input->Root.Bounds.right-x;			/* Get the bounds rect */
	Height = Input->Root.Bounds.bottom-y;
	
	x += DrawX;
	y += DrawY;
	
	if (Input->TextPtr) {						/* Is there even text here? */
		if (Slider) {
			Width -= (Slider->Root.Bounds.right-Slider->Root.Bounds.left)-1;
		}
		ScreenBox(x,y,Width,Height,Input->OutlineColor);			/* Draw the bounding box */
		x+=2;
		y+=2;
		Width-=4;
		Height-=4;
		TextRect.left = x;						/* Get the text box */
		TextRect.top = y;
		TextRect.right = x+Width;
		TextRect.bottom = y+Height;
		
		/* Draw the formatted text */
		
		FontWidthListDraw(Input->TextDescription,&TextRect,Input->Value,Input->TextPtr);
		if (Slider) {
			Slider->Root.Draw(&Slider->Root, DrawX, DrawY);		/* Draw the scroll bar if present */
		}
	} else {
		ScreenBox(x+DrawX,y+DrawY,Width,Height,Input->OutlineColor);	/* Frame the box as is */
	}
}

/**********************************

	Check if a point is within a button
	I return one of these values...
	BUTTON_DOWN = If the mouse is held down and I want to keep focus
	BUTTON_CLICKED = If the mouse was released inside me and I had focus
	BUTTON_RELEASED = If the mouse was released out of me and I had focus
	BUTTON_INSIDE = If the mouse is over the control
	BUTTON_OUTSIDE = If the mouse is outside of the control

**********************************/

static DialogControlAction_e BURGERCALL DialogControlTextBoxCheck(DialogControl_t *Inputx,int x,int y,Word buttons,Word Key)
{
	DialogControlTextBox_t *Input;
	DialogControlVScroll_t *Slider;			/* Scroll bar */
	
	Input = (DialogControlTextBox_t *)Inputx;
	Slider = Input->Slider;
	if (Slider) {
		Slider->Root.Check(&Slider->Root,x,y,buttons,Key);		/* Pass the event to the control list */
		Input->Value = Input->Slider->Value;					/* Propagate the slider bar value to my root */
	} else {
		Input->Value = 0;
	}
	return DialogControlCheck(&Input->Root,x,y,buttons,Key);	/* Return my state */
}

/**********************************

	Set the scroll value for the text

**********************************/

void BURGERCALL DialogControlTextBoxSetValue(DialogControlTextBox_t *Input,Word NewValue)
{
	DialogControlVScroll_t *Slider;			/* Scroll bar */
	
	Slider = Input->Slider;
	if (Slider) {
		DialogControlVScrollSetValue(Slider,NewValue);
		Input->Value = Slider->Value;
	} else {
		Input->Value = 0;					/* No slider, no change */
	}
}

/**********************************

	Change the text (If I need to, I will add a scroll bar)

**********************************/

void BURGERCALL DialogControlTextBoxSetText(DialogControlTextBox_t *Input,const char *TextPtr,FontRef_t *FontPtr)
{
	DialogControlVScroll_t *Slider;			/* Scroll bar */
	LBRect					ScrollBounds;
	
	/* First, kill off the previous state */
	
	Slider = Input->Slider;
	if (Slider) {
		DialogControlDelete(&Slider->Root);
		Input->Slider = 0;
	}
	FontWidthListDelete(Input->TextDescription);
	DeallocAPointer((void *)Input->TextPtr);
	Input->TextDescription = 0;
	Input->TextPtr = 0;
	Input->FontPtr = FontPtr;
	Input->Value = 0;			/* Force to the top */
	
	/* Any text to add? */
	
	if (TextPtr && TextPtr[0]) {
		FontWidthLists_t *TempWidths;
		Word TextWidth;
		Word TextHeight;
		TextWidth = (Input->Root.Bounds.right-Input->Root.Bounds.left)-4;		/* Text rect */
		TextHeight = (Input->Root.Bounds.bottom-Input->Root.Bounds.top)-4;
		TextPtr = StrCopy(TextPtr);										/* Get a copy of the text */
		if (TextPtr) {
			Input->TextPtr = TextPtr;
			TempWidths = FontWidthListNew(FontPtr,TextPtr,TextWidth);
			if (TempWidths) {
				LBPoint TempSize;
				if ((TextHeight>=(FontPtr->FontHeight*TempWidths->Count)) || Input->AllowSlider == FALSE) {
					Input->TextDescription = TempWidths;
					return;
				}
				
				FontWidthListDelete(TempWidths);
				ScreenShapeGetSize(Input->Art[0],&TempSize);
				
				LBRectSetRect(&ScrollBounds,
						Input->Root.Bounds.right-TempSize.x,
						Input->Root.Bounds.top,
						Input->Root.Bounds.right,
						Input->Root.Bounds.top + TextHeight+4);
				
				Slider = DialogControlVScrollNew(Input->Art,&ScrollBounds,0,0,1,1,1,Input->ScrollBarNormalArrowStyle);
				if (Slider) {
					Input->Slider = Slider;
					TextWidth -= (Slider->Root.Bounds.right-Slider->Root.Bounds.left);
					TempWidths = FontWidthListNew(FontPtr,TextPtr,TextWidth);
					if (TempWidths) {
						Input->TextDescription = TempWidths;
						DialogControlVScrollSetParms(Slider,(FontPtr->FontHeight*TempWidths->Count)-TextHeight,TextHeight,FontPtr->FontHeight);
						return;
					} 
					DialogControlDelete(&Slider->Root);		/* Destroy it */
					Input->Slider = 0;						/* Make sure and update us! */
				}
			}
			DeallocAPointer((void *)TextPtr);		/* You are boned */
			Input->TextPtr = 0;
		}
	}
}

/**********************************

	Initialized a DialogControlTextBox_t

**********************************/

void BURGERCALL DialogControlTextBoxInit(DialogControlTextBox_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,const char *TextPtr,Word Value,FontRef_t *FontPtr,Bool ScrollBarNormalArrowStyle,Bool AllowSlider){
	Input->Root.Bounds = *Bounds;
	Input->Root.Active = TRUE;
	Input->Root.Invisible = FALSE;
	Input->Root.Focus = FALSE;		/* Doesn't have focus */
	Input->Root.Inside = FALSE;		/* Set the current default mode */
	Input->Root.Draw = DialogControlTextBoxDraw;
	Input->Root.Delete = DialogControlTextBoxDestroy;
	Input->Root.Check = DialogControlTextBoxCheck;
	Input->Root.Event = EventProc;		/* No button specific callback */
	Input->Root.RefCon = 0;
	Input->Root.HotKey = 0;
	
	Input->Art = ArtArray;
	
	Input->Slider = 0;
	Input->Value = 0;
	Input->TextPtr = 0;
	Input->TextDescription = 0;
	Input->ScrollBarNormalArrowStyle = ScrollBarNormalArrowStyle;
	Input->AllowSlider = AllowSlider;
	Input->OutlineColor = PaletteConvertPackedRGBToDepth(DialogControlTextBoxOutlineColor,VideoColorDepth);
	DialogControlTextBoxSetText(Input,TextPtr,FontPtr);
	DialogControlTextBoxSetValue(Input,Value);
}

/**********************************

	Create a new DialogControlTextBox_t

**********************************/

DialogControlTextBox_t * BURGERCALL DialogControlTextBoxNew(struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,const char *TextPtr,Word Value,FontRef_t *FontPtr,Bool ScrollBarNormalArrowStyle,Bool AllowSlider)
{
	DialogControlTextBox_t *MyPtr;

	MyPtr = (DialogControlTextBox_t *)AllocAPointer(sizeof(DialogControlTextBox_t));	/* Get the memory */
	if (MyPtr) {
		DialogControlTextBoxInit(MyPtr,ArtArray,Bounds,EventProc,TextPtr,Value,FontPtr,ScrollBarNormalArrowStyle,AllowSlider);		/* Init it */
		MyPtr->Root.Delete = DialogControlTextBoxDelete;
	}
	return MyPtr;			/* Return the pointer */
}




/**********************************

	Destructors

**********************************/

static void BURGERCALL DialogControlStaticTextDestroy(DialogControl_t *Input)
{
	FontWidthListDelete(((DialogControlStaticText_t *)Input)->TextDescription);	/* Delete my text formatting */
	DeallocAPointer((void *)((DialogControlStaticText_t *)Input)->TextPtr);		/* Delete my text */
}

static void BURGERCALL DialogControlStaticTextDelete(DialogControl_t *Input)
{
	FontWidthListDelete(((DialogControlStaticText_t *)Input)->TextDescription);	/* Delete my text formatting */
	DeallocAPointer((void *)((DialogControlStaticText_t *)Input)->TextPtr);		/* Delete my text */
	DeallocAPointer(Input);
}

/**********************************

	Draw the text box

**********************************/

static void BURGERCALL DialogControlStaticTextDraw(DialogControl_t *wow, int DrawX, int DrawY)
{
	DialogControlStaticText_t *Input;
	Word Width,Height;
	LBRect TextRect;
	int x,y;
	
	Input = (DialogControlStaticText_t *)wow;

	x = Input->Root.Bounds.left;				/* Get the origin X and Y */
	y = Input->Root.Bounds.top;
	Width = Input->Root.Bounds.right-x;			/* Get the bounds rect */
	Height = Input->Root.Bounds.bottom-y;
	
	x += DrawX;
	y += DrawY;
	
	if (Input->TextPtr) {
		TextRect.left = x;						/* Get the text box */
		TextRect.top = y;
		TextRect.right = x+Width;
		TextRect.bottom = y+Height;
		
		/* Draw the formatted text */
		
		FontWidthListDraw(Input->TextDescription,&TextRect,0,Input->TextPtr);
	}
}

/**********************************

	Check if a point is within a button
	I return one of these values...
	BUTTON_DOWN = If the mouse is held down and I want to keep focus
	BUTTON_CLICKED = If the mouse was released inside me and I had focus
	BUTTON_RELEASED = If the mouse was released out of me and I had focus
	BUTTON_INSIDE = If the mouse is over the control
	BUTTON_OUTSIDE = If the mouse is outside of the control

**********************************/

static DialogControlAction_e BURGERCALL DialogControlStaticTextCheck(DialogControl_t *Inputx,int x,int y,Word buttons,Word Key)
{
	DialogControlStaticText_t *Input;
	
	Input = (DialogControlStaticText_t *)Inputx;
	return DialogControlCheck(&Input->Root,x,y,buttons,Key);	/* Return my state */
}

/**********************************

	Change the text (If I need to, I will add a scroll bar)

**********************************/

void BURGERCALL DialogControlStaticTextSetText(DialogControlStaticText_t *Input,const char *TextPtr,FontRef_t *FontPtr)
{
	/* First, kill off the previous state */
	
	FontWidthListDelete(Input->TextDescription);
	DeallocAPointer((void *)Input->TextPtr);
	Input->TextDescription = 0;
	Input->TextPtr = 0;
	Input->FontPtr = FontPtr;
	
	/* Any text to add? */
	
	if (TextPtr && TextPtr[0]) {
		FontWidthLists_t *TempWidths;
		Word TextWidth;
		TextWidth = (Input->Root.Bounds.right-Input->Root.Bounds.left);		/* Text rect */
		TextPtr = StrCopy(TextPtr);										/* Get a copy of the text */
		if (TextPtr) {
			Input->TextPtr = TextPtr;
			TempWidths = FontWidthListNew(FontPtr,TextPtr,TextWidth);
			if (TempWidths) {
				Input->TextDescription = TempWidths;
				return;
			}
			DeallocAPointer((void *)TextPtr);		/* You are boned */
			Input->TextPtr = 0;
		}
	}
}

/**********************************

	Initialized a DialogControlStaticText_t

**********************************/

void BURGERCALL DialogControlStaticTextInit(DialogControlStaticText_t *Input,const LBRect *Bounds,DialogControlEventProc EventProc,const char *TextPtr,FontRef_t *FontPtr)
{
	Input->Root.Bounds = *Bounds;
	Input->Root.Active = TRUE;
	Input->Root.Invisible = FALSE;
	Input->Root.Focus = FALSE;		/* Doesn't have focus */
	Input->Root.Inside = FALSE;		/* Set the current default mode */
	Input->Root.Draw = DialogControlStaticTextDraw;
	Input->Root.Delete = DialogControlStaticTextDestroy;
	Input->Root.Check = DialogControlStaticTextCheck;
	Input->Root.Event = EventProc;		/* No button specific callback */
	Input->Root.RefCon = 0;
	Input->Root.HotKey = 0;
	
	Input->TextPtr = 0;
	Input->TextDescription = 0;
	DialogControlStaticTextSetText(Input,TextPtr,FontPtr);
}

/**********************************

	Create a new DialogControlStaticText_t

**********************************/

DialogControlStaticText_t * BURGERCALL DialogControlStaticTextNew(const LBRect *Bounds,DialogControlEventProc EventProc,const char *TextPtr,FontRef_t *FontPtr)
{
	DialogControlStaticText_t *MyPtr;

	MyPtr = (DialogControlStaticText_t *)AllocAPointer(sizeof(DialogControlStaticText_t));	/* Get the memory */
	if (MyPtr) {
		DialogControlStaticTextInit(MyPtr,Bounds,EventProc,TextPtr,FontPtr);		/* Init it */
		MyPtr->Root.Delete = DialogControlStaticTextDelete;
	}
	return MyPtr;			/* Return the pointer */
}



/**********************************

	Destructors

**********************************/

static void BURGERCALL DialogControlTextListDestroy(DialogControl_t *Input)
{
	DialogControlVScroll_t *Slider;			/* Scroll bar */
	Slider = ((DialogControlTextList_t *)Input)->Slider;		/* Is there a scroll bar? */
	if (Slider) {
		DialogControlDelete(&Slider->Root);
	}
	LinkedListDestroy(&((DialogControlTextList_t *)Input)->List);	/* Delete my text formatting */
}

static void BURGERCALL DialogControlTextListDelete(DialogControl_t *Input)
{
	DialogControlVScroll_t *Slider;			/* Scroll bar */
	Slider = ((DialogControlTextList_t *)Input)->Slider;		/* Is there a scroll bar? */
	if (Slider) {
		DialogControlDelete(&Slider->Root);		/* Destroy it */
	}
	LinkedListDestroy(&((DialogControlTextList_t *)Input)->List);	/* Delete my text formatting */
	DeallocAPointer(Input);
}

/**********************************

	Draw the text list

**********************************/

static void BURGERCALL DialogControlTextListDraw(DialogControl_t *wow, int DrawX, int DrawY)
{
	DialogControlTextList_t *Input;
	DialogControlVScroll_t *Slider;			/* Scroll bar */
	Word Width,Height;
	LBRect TextRect;
	int x,y;
	
	Input = (DialogControlTextList_t *)wow;
	Slider = Input->Slider;

	x = Input->Root.Bounds.left;				/* Get the origin X and Y */
	y = Input->Root.Bounds.top;
	Width = Input->Root.Bounds.right-x;			/* Get the bounds rect */
	Height = Input->Root.Bounds.bottom-y;
	
	x += DrawX;
	y += DrawY;
	
	if (Input->List.Count) {						/* Is there even text here? */
		if (Slider) {
			Width -= (Slider->Root.Bounds.right-Slider->Root.Bounds.left)-1;
		}
		ScreenRect(x,y,Width,Height,Input->FillColor);			/* Draw Fill Color */
		ScreenBox(x,y,Width,Height,Input->OutlineColor);			/* Draw the bounding box */
		x+=2;
		y+=2;
		Width-=4;
		Height-=4;
		TextRect.left = x;						/* Get the text box */
		TextRect.top = y;
		TextRect.right = x+Width;
		TextRect.bottom = y+Height;
		
		/* Draw the formatted text */
		
		{
			LinkedListEntry_t *EntryPtr;
			LBRect ClipRect;
			Word StartIndex;
			Word FontHeight;
			Word YTop;
			
			YTop = Input->ScrollValue;
			FontHeight = Input->FontPtr->FontHeight;
			
			StartIndex = YTop/FontHeight;		/* Which line shall I start with? */
			if (StartIndex<Input->List.Count) {		/* Is this line even on the text? */
				GetTheClipRect(&ClipRect);		/* Get the screen clip rect */
				SetTheClipRect(&TextRect);		/* Set the bounds rect to draw to */
				EntryPtr = LinkedListGetEntry(&Input->List,StartIndex);		/* Get the pointer to the text to draw */
				y = y-(YTop-(StartIndex*FontHeight));	/* Starting Y */
				do {
					FontRef_t *FPtr;
					FPtr = Input->FontPtr;
					if (StartIndex==Input->Value) {
						FPtr = Input->FontPtr2;
						ScreenRect(x,y,Width,FontHeight,Input->SelColor);
					}
					FontDrawStringAtXY(FPtr,x,y,(char *)EntryPtr->Data);		/* Draw the line of text */
					y +=FontHeight;									/* Next line down */
					++StartIndex;
					EntryPtr = EntryPtr->Next;
					if (!EntryPtr) {					/* No more text? */
						break;
					}
				} while (y<TextRect.bottom);				/* Off the bottom? */
				SetTheClipRect(&ClipRect);					/* Restore the clip rect */
			}
		}
		if (Slider) {
			Slider->Root.Draw(&Slider->Root, DrawX, DrawY);		/* Draw the scroll bar if present */
		}
	} else {
		ScreenBox(x,y,Width,Height,Input->OutlineColor);	/* Frame the box as is */
	}
}

/**********************************

	Check if a point is within a button
	I return one of these values...
	BUTTON_DOWN = If the mouse is held down and I want to keep focus
	BUTTON_CLICKED = If the mouse was released inside me and I had focus
	BUTTON_RELEASED = If the mouse was released out of me and I had focus
	BUTTON_INSIDE = If the mouse is over the control
	BUTTON_OUTSIDE = If the mouse is outside of the control

**********************************/

static DialogControlAction_e BURGERCALL DialogControlTextListCheck(DialogControl_t *Inputx,int x,int y,Word buttons,Word Key)
{
	DialogControlAction_e Result;
	DialogControlTextList_t *Input;
	DialogControlVScroll_t *Slider;			/* Scroll bar */
	
	Input = (DialogControlTextList_t *)Inputx;
	Slider = Input->Slider;
	if (Slider) {
		Slider->Root.Check(&Slider->Root,x,y,buttons,Key);		/* Pass the event to the control list */
		Input->ScrollValue = Input->Slider->Value;					/* Propagate the slider bar value to my root */
	} else {
		Input->ScrollValue = 0;
	}
	
	if (buttons) {
		buttons = TRUE; 		/* I only want TRUE or false */
	}
	
	Input->Root.Inside = static_cast<Word8>(LBRectPtInRect(&Input->Root.Bounds,x,y));		/* Inside of the rect? */
	
	/* Now I have the current motion state */
			
	/* Mouse down? */
	
	if (buttons) {
		
		if (Input->Root.Inside) {
			Word Value;
			Input->Root.Focus = TRUE;			/* I'll keep focus */
			if (!Slider || x<(Input->Root.Bounds.right-(Slider->Root.Bounds.right-Slider->Root.Bounds.left))) {
				Value = ((y-Input->Root.Bounds.top)+Input->ScrollValue)/Input->FontPtr->FontHeight;
				if (Value<Input->List.Count) {
					Input->Value = Value;
					if (Input->Root.Event) {
						Input->Root.Event(&Input->Root,BUTTON_CLICKED);		/* Send the event */
					}
				}
			}
			Result = BUTTON_DOWN;			/* Grab focus */
			goto Exit;
		}
		
		if (Input->Root.Focus) {
			Result = BUTTON_DOWN;			/* Keep focus */
			goto Exit;
		}

	} else if (Input->Root.Focus) {		/* I only care if I have focus */
		Input->Root.Focus = FALSE;		/* Drop focus */

		/* Did I release OUTSIDE? */

		Result = BUTTON_RELEASED;		/* Don't accept the click */
		goto Exit;
	}

	/* Ok, now return my state flag */
	
	Result = BUTTON_INSIDE;
	if (!Input->Root.Inside) {
		Result = BUTTON_OUTSIDE;		/* I'm not active */
	}
Exit:;
	/* Do I have a callback proc? */
	if (Input->Root.Event) {
		Input->Root.Event(&Input->Root,Result);	/* Pass the current state */
	}
	return Result;

}

/**********************************

	Add or remove the scroll bar

**********************************/

static void BURGERCALL DialogControlTextListFixScroll(DialogControlTextList_t *Input)
{
	DialogControlVScroll_t *Slider;			/* Scroll bar */
	Word Lines;
	Word Pixels;
	Word TextHeight;
	
	Slider = Input->Slider;
	Lines = Input->List.Count;
	Pixels = Lines*Input->FontPtr->FontHeight;
	TextHeight = (Input->Root.Bounds.bottom-Input->Root.Bounds.top)-4;
	if (Pixels<=TextHeight) {
		Input->ScrollValue = 0;
		if (Slider) {
			DialogControlDelete(&Slider->Root);
			Input->Slider = 0;
		}
		return;
	}
	if (!Slider) {
		LBPoint TempSize;
		LBRect ScrollBounds;
		ScreenShapeGetSize(Input->Art[0],&TempSize);
		LBRectSetRect(&ScrollBounds,
			Input->Root.Bounds.right-TempSize.x,
			Input->Root.Bounds.top,
			Input->Root.Bounds.right,
			Input->Root.Bounds.top + TextHeight+4);
		Slider = DialogControlVScrollNew(Input->Art,&ScrollBounds,0,0,1,1,1,Input->ScrollBarNormalArrowStyle);
		if (!Slider) {
			Input->ScrollValue = 0;
			return;
		}	
		Input->Slider = Slider;
	}
	DialogControlVScrollSetParms(Slider,Pixels-TextHeight,TextHeight,Input->FontPtr->FontHeight);
}

/**********************************

	Set the scroll value for the text

**********************************/

void BURGERCALL DialogControlTextListSetValue(DialogControlTextList_t *Input,Word NewValue)
{
	if (NewValue>=Input->List.Count) {
		NewValue = 0;
	}
	Input->Value = NewValue;					/* No slider, no change */
}

/**********************************

	Add an entry to the list
	
**********************************/

Word BURGERCALL DialogControlTextListAddText(DialogControlTextList_t *Input,const char *TextPtr,Word EntryNum)
{
	LinkedListEntry_t *EntryPtr;

	if (EntryNum!=(Word)-1 && (EntryPtr = LinkedListGetEntry(&Input->List,EntryNum)) != 0) {		/* Check the list for a string */
		LinkedListAddNewEntryStringAfter(&Input->List,EntryPtr,TextPtr);
	} else {
		EntryNum = Input->List.Count;
		LinkedListAddNewEntryStringEnd(&Input->List,TextPtr);
	}
	DialogControlTextListFixScroll(Input);					/* Adjust the scroll bar */
	return EntryNum;
}

/**********************************

	Delete an entry in the list
	
**********************************/

void BURGERCALL DialogControlTextListRemoveText(DialogControlTextList_t *Input,Word EntryNum)
{
	LinkedListEntry_t *EntryPtr;

	EntryPtr = LinkedListGetEntry(&Input->List,EntryNum);		/* Check the list for a string */
	if (EntryPtr) {				/* Any data in the list? */
		LinkedListDeleteEntry(&Input->List,EntryPtr);
		DialogControlTextListFixScroll(Input);					/* Adjust the scroll bar */
	}
}

/**********************************

	Dispose of all the strings in the text box
	
**********************************/

void BURGERCALL DialogControlTextListRemoveAllText(DialogControlTextList_t *Input)
{
	if (Input->List.Count) {
		do {
			DialogControlTextListRemoveText(Input,0);
		} while (Input->List.Count);
	}
	
	Input->Value = (Word)-1;
}

/**********************************

	Find a string in the list box
	
**********************************/

Word BURGERCALL DialogControlTextListFindText(DialogControlTextList_t *Input,const char *TextPtr)
{
	return LinkedListFindString(&Input->List,TextPtr);
}

/**********************************

	Initialized a DialogControlTextList_t

**********************************/

void BURGERCALL DialogControlTextListInit(DialogControlTextList_t *Input,ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,DialogControlEventProc EventProc,Bool ScrollBarNormalArrowStyle)
{
	Input->Root.Bounds = *Bounds;
	Input->Root.Active = TRUE;
	Input->Root.Invisible = FALSE;
	Input->Root.Focus = FALSE;		/* Doesn't have focus */
	Input->Root.Inside = FALSE;		/* Set the current default mode */
	Input->Root.Draw = DialogControlTextListDraw;
	Input->Root.Delete = DialogControlTextListDestroy;
	Input->Root.Check = DialogControlTextListCheck;
	Input->Root.Event = EventProc;		/* No button specific callback */
	Input->Root.RefCon = 0;
	Input->Root.HotKey = 0;
	
	Input->FontPtr = FontPtr;
	Input->FontPtr2 = FontPtr2;
	Input->Art = ArtArray;
	Input->Slider = 0;
	Input->ScrollValue = 0;
	Input->Value = (Word)-1;
	Input->ScrollBarNormalArrowStyle = ScrollBarNormalArrowStyle;
	Input->OutlineColor = PaletteConvertPackedRGBToDepth(DialogControlTextBoxOutlineColor,VideoColorDepth);
	Input->FillColor = PaletteConvertPackedRGBToDepth(DialogControlTextBoxFillColor,VideoColorDepth);
	Input->SelColor = PaletteConvertPackedRGBToDepth(DialogControlTextBoxSelectedRowColor,VideoColorDepth);
	LinkedListInit(&Input->List);
}

/**********************************

	Create a new DialogControlTextList_t

**********************************/

DialogControlTextList_t *BURGERCALL DialogControlTextListNew(ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,DialogControlEventProc EventProc,Bool ScrollBarNormalArrowStyle)
{
	DialogControlTextList_t *MyPtr;

	MyPtr = (DialogControlTextList_t *)AllocAPointer(sizeof(DialogControlTextList_t));	/* Get the memory */
	if (MyPtr) {
		DialogControlTextListInit(MyPtr,ArtArray,FontPtr,FontPtr2,Bounds,EventProc,ScrollBarNormalArrowStyle);		/* Init it */
		MyPtr->Root.Delete = DialogControlTextListDelete;
	}
	return MyPtr;			/* Return the pointer */
}





/**********************************

	Destructors

**********************************/

static void BURGERCALL DialogControlGenericListDestroy(DialogControl_t *Input)
{
	DialogControlVScroll_t *Slider;			/* Scroll bar */
	Slider = ((DialogControlGenericList_t *)Input)->Slider;		/* Is there a scroll bar? */
	if (Slider) {
		DialogControlDelete(&Slider->Root);
	}
	LinkedListDestroy(&((DialogControlGenericList_t *)Input)->List);	/* Delete my text formatting */
}

static void BURGERCALL DialogControlGenericListDelete(DialogControl_t *Input)
{
	DialogControlVScroll_t *Slider;			/* Scroll bar */
	Slider = ((DialogControlGenericList_t *)Input)->Slider;		/* Is there a scroll bar? */
	if (Slider) {
		DialogControlDelete(&Slider->Root);		/* Destroy it */
	}
	LinkedListDestroy(&((DialogControlGenericList_t *)Input)->List);	/* Delete my text formatting */
	DeallocAPointer(Input);
}

/**********************************

	Draw the text list

**********************************/

static void BURGERCALL DialogControlGenericListDraw(DialogControl_t *wow, int DrawX, int DrawY)
{
	DialogControlGenericList_t *Input;
	DialogControlVScroll_t *Slider;			/* Scroll bar */
	Word Width,Height;
	LBRect TextRect;
	int x,y;
	
	Input = (DialogControlGenericList_t *)wow;
	Slider = Input->Slider;

	x = Input->Root.Bounds.left;				/* Get the origin X and Y */
	y = Input->Root.Bounds.top;
	Width = Input->Root.Bounds.right-x;			/* Get the bounds rect */
	Height = Input->Root.Bounds.bottom-y;
	
	x += DrawX;
	y += DrawY;
	
	if (Input->List.Count) {						/* Is there even text here? */
		if (Slider) {
			Width -= (Slider->Root.Bounds.right-Slider->Root.Bounds.left)-1;
		}
		ScreenRect(x,y,Width,Height,Input->FillColor);			/* Draw Fill Color */
		ScreenBox(x,y,Width,Height,Input->OutlineColor);			/* Draw the bounding box */
		TextRect.left = x;						/* Get the text box */
		TextRect.top = y;
		TextRect.right = x+Width;
		TextRect.bottom = y+Height;
		
		/* Draw the formatted text */
		
		{
			LinkedListEntry_t *EntryPtr;
			LBRect ClipRect;
			Word StartIndex;
			Word YTop;
			
			YTop = Input->ScrollValue;
			
			StartIndex = YTop/Input->CellHeight;		/* Which line shall I start with? */
			if (StartIndex<Input->List.Count) {		/* Is this line even on the text? */
				GetTheClipRect(&ClipRect);		/* Get the screen clip rect */
				SetTheClipRect(&TextRect);		/* Set the bounds rect to draw to */
				EntryPtr = LinkedListGetEntry(&Input->List,StartIndex);		/* Get the pointer to the text to draw */
				y = y-(YTop-(StartIndex*Input->CellHeight));	/* Starting Y */
				do {
					if (StartIndex==Input->Value) {
						ScreenRect(x,y,Width,Input->CellHeight,Input->SelColor);
					}
					if (Input->CellDraw)
						Input->CellDraw(Input, EntryPtr->Data, x, y, Width, Input->CellHeight,(Bool) (StartIndex==Input->Value));
					y +=Input->CellHeight;									/* Next line down */
					++StartIndex;
					EntryPtr = EntryPtr->Next;
					if (!EntryPtr) {					/* No more text? */
						break;
					}
				} while (y<TextRect.bottom);				/* Off the bottom? */
				SetTheClipRect(&ClipRect);					/* Restore the clip rect */
			}
		}
		if (Slider) {
			Slider->Root.Draw(&Slider->Root, DrawX, DrawY);		/* Draw the scroll bar if present */
		}
	} else {
		ScreenBox(x,y,Width,Height,Input->OutlineColor);	/* Frame the box as is */
	}
}

/**********************************

	Check if a point is within a button
	I return one of these values...
	BUTTON_DOWN = If the mouse is held down and I want to keep focus
	BUTTON_CLICKED = If the mouse was released inside me and I had focus
	BUTTON_RELEASED = If the mouse was released out of me and I had focus
	BUTTON_INSIDE = If the mouse is over the control
	BUTTON_OUTSIDE = If the mouse is outside of the control

**********************************/

static DialogControlAction_e BURGERCALL DialogControlGenericListCheck(DialogControl_t *Inputx,int x,int y,Word buttons,Word Key)
{
	DialogControlAction_e Result;
	DialogControlGenericList_t *Input;
	DialogControlVScroll_t *Slider;			/* Scroll bar */
	
	Input = (DialogControlGenericList_t *)Inputx;
	Slider = Input->Slider;
	if (Slider) {
		Slider->Root.Check(&Slider->Root,x,y,buttons,Key);		/* Pass the event to the control list */
		Input->ScrollValue = Input->Slider->Value;					/* Propagate the slider bar value to my root */
	} else {
		Input->ScrollValue = 0;
	}
	
	if (buttons) {
		buttons = TRUE; 		/* I only want TRUE or false */
	}
	
	Input->Root.Inside = static_cast<Word8>(LBRectPtInRect(&Input->Root.Bounds,x,y));		/* Inside of the rect? */
	
	/* Now I have the current motion state */
			
	/* Mouse down? */
	
	if (buttons) {
		
		if (Input->Root.Inside) {
			Word Value;
			Input->Root.Focus = TRUE;			/* I'll keep focus */
			if (!Slider || x<(Input->Root.Bounds.right-(Slider->Root.Bounds.right-Slider->Root.Bounds.left))) {
				Value = ((y-Input->Root.Bounds.top)+Input->ScrollValue)/Input->CellHeight;
				if (Value<Input->List.Count) {
					Input->Value = Value;
					if (Input->Root.Event) {
						Input->Root.Event(&Input->Root,BUTTON_CLICKED);		/* Send the event */
					}
				}
			}
			Result = BUTTON_DOWN;			/* Grab focus */
			goto Exit;
		}
		
		if (Input->Root.Focus) {
			Result = BUTTON_DOWN;			/* Keep focus */
			goto Exit;
		}

	} else if (Input->Root.Focus) {		/* I only care if I have focus */
		Input->Root.Focus = FALSE;		/* Drop focus */

		/* Did I release OUTSIDE? */

		Result = BUTTON_RELEASED;		/* Don't accept the click */
		goto Exit;
	}

	/* Ok, now return my state flag */
	
	Result = BUTTON_INSIDE;
	if (!Input->Root.Inside) {
		Result = BUTTON_OUTSIDE;		/* I'm not active */
	}
Exit:;
	/* Do I have a callback proc? */
	if (Input->Root.Event) {
		Input->Root.Event(&Input->Root,Result);	/* Pass the current state */
	}
	return Result;

}

/**********************************

	Add or remove the scroll bar

**********************************/

static void BURGERCALL DialogControlGenericListFixScroll(DialogControlGenericList_t *Input)
{
	DialogControlVScroll_t *Slider;			/* Scroll bar */
	Word Lines;
	Word Pixels;
	Word TextHeight;
	
	Slider = Input->Slider;
	Lines = Input->List.Count;
	Pixels = Lines*Input->CellHeight;
	TextHeight = (Input->Root.Bounds.bottom-Input->Root.Bounds.top)-4;
	if (Pixels<=TextHeight) {
		Input->ScrollValue = 0;
		if (Slider) {
			DialogControlDelete(&Slider->Root);
			Input->Slider = 0;
		}
		return;
	}
	if (!Slider) {
		LBPoint TempSize;
		LBRect ScrollBounds;
		ScreenShapeGetSize(Input->Art[0],&TempSize);
		LBRectSetRect(&ScrollBounds,
			Input->Root.Bounds.right-TempSize.x,
			Input->Root.Bounds.top,
			Input->Root.Bounds.right,
			Input->Root.Bounds.top+TextHeight+4);
		Slider = DialogControlVScrollNew(Input->Art,&ScrollBounds,0,0,1,1,1,Input->ScrollBarNormalArrowStyle);
		if (!Slider) {
			Input->ScrollValue = 0;
			return;
		}	
		Input->Slider = Slider;
	}
	DialogControlVScrollSetParms(Slider,Pixels-TextHeight,TextHeight,Input->CellHeight);
}

/**********************************

	Set the scroll value for the text

**********************************/

void BURGERCALL DialogControlGenericListSetValue(DialogControlGenericList_t *Input,Word NewValue)
{
	if (NewValue>=Input->List.Count) {
		NewValue = 0;
	}
	Input->Value = NewValue;					/* No slider, no change */
}

/**********************************

	Add an entry to the list
	
**********************************/

Word BURGERCALL DialogControlGenericListAddRow(DialogControlGenericList_t *Input,void *DataPtr,Word EntryNum)
{
	LinkedListEntry_t *EntryPtr;

	if (EntryNum!=(Word)-1 && (EntryPtr = LinkedListGetEntry(&Input->List,EntryNum)) != 0) {		/* Check the list for a string */
		LinkedListAddNewEntryAfter(&Input->List,EntryPtr,DataPtr);
	} else {
		EntryNum = Input->List.Count;
		LinkedListAddNewEntryEnd(&Input->List,DataPtr);
	}
	DialogControlGenericListFixScroll(Input);					/* Adjust the scroll bar */
	return EntryNum;
}

/**********************************

	Delete an entry in the list
	
**********************************/

void BURGERCALL DialogControlGenericListRemoveRow(DialogControlGenericList_t *Input,Word EntryNum)
{
	LinkedListEntry_t *EntryPtr;

	EntryPtr = LinkedListGetEntry(&Input->List,EntryNum);		/* Check the list for a string */
	if (EntryPtr) {				/* Any data in the list? */
		LinkedListDeleteEntry(&Input->List,EntryPtr);
		DialogControlGenericListFixScroll(Input);					/* Adjust the scroll bar */
	}
}

/**********************************

	Dispose of all the strings in the text box
	
**********************************/

void BURGERCALL DialogControlGenericListRemoveAllRows(DialogControlGenericList_t *Input)
{
	if (Input->List.Count) {
		do {
			DialogControlGenericListRemoveRow(Input,0);
		} while (Input->List.Count);
	}
	
	Input->Value = (Word)-1;
}


/**********************************

	Initialized a DialogControlGenericList_t

**********************************/

void BURGERCALL DialogControlGenericListInit(DialogControlGenericList_t *Input,ScreenShape_t **ArtArray,const LBRect *Bounds, Word CellHeight, DialogControlEventProc EventProc,DialogControlGenericListDrawProc CellDrawProc,Bool ScrollBarNormalArrowStyle)
{
	Input->Root.Bounds = *Bounds;
	Input->Root.Active = TRUE;
	Input->Root.Invisible = FALSE;
	Input->Root.Focus = FALSE;		/* Doesn't have focus */
	Input->Root.Inside = FALSE;		/* Set the current default mode */
	Input->Root.Draw = DialogControlGenericListDraw;
	Input->Root.Delete = DialogControlGenericListDestroy;
	Input->Root.Check = DialogControlGenericListCheck;
	Input->Root.Event = EventProc;		/* No button specific callback */
	Input->Root.RefCon = 0;
	Input->Root.HotKey = 0;
	
	Input->Art = ArtArray;
	Input->Slider = 0;
	Input->ScrollValue = 0;
	Input->Value = (Word)-1;
	Input->ScrollBarNormalArrowStyle = ScrollBarNormalArrowStyle;
	Input->FillColor = PaletteConvertPackedRGBToDepth(DialogControlTextBoxFillColor,VideoColorDepth);
	Input->SelColor = PaletteConvertPackedRGBToDepth(DialogControlTextBoxSelectedRowColor,VideoColorDepth);
	Input->CellHeight = CellHeight;
	Input->CellDraw = CellDrawProc;
	LinkedListInit(&Input->List);
}

/**********************************

	Create a new DialogControlGenericList_t

**********************************/

DialogControlGenericList_t *BURGERCALL DialogControlGenericListNew(ScreenShape_t **ArtArray,const LBRect *Bounds,Word CellHeight, DialogControlEventProc EventProc,DialogControlGenericListDrawProc CellDrawProc, Bool ScrollBarNormalArrowStyle)
{
	DialogControlGenericList_t *MyPtr;

	MyPtr = (DialogControlGenericList_t *)AllocAPointer(sizeof(DialogControlGenericList_t));	/* Get the memory */
	if (MyPtr) {
		DialogControlGenericListInit(MyPtr,ArtArray,Bounds,CellHeight,EventProc,CellDrawProc,ScrollBarNormalArrowStyle);		/* Init it */
		MyPtr->Root.Delete = DialogControlGenericListDelete;
	}
	return MyPtr;			/* Return the pointer */
}






/**********************************

	Draw the text in the pop-up menu
	Note the stuff I have to go through to draw
	the up and down arrows.

**********************************/

static void BURGERCALL DialogControlTextMenuDraw(DialogControl_t *wow, int DrawX, int DrawY)
{
	DialogControlTextMenu_t *Input;			/* Real control pointer */
	Word Width,Height;						/* Width and height of the control */
	Word CellHeight;						/* Number of cells being drawn */
	LBRect TextRect;						/* Rect to clip to */
	LBRect ClipRect;						/* Saved clip rect */
	int x,y;								/* Drawing origin */
	int TextX;								/* X Coord to draw text at */
	LinkedListEntry_t *EntryPtr;			/* Linked list traverer */
	int StartIndex;							/* Text line to draw from */
	Word FontHeight;						/* Height of each cell in pixels */
	LBPoint TempSize;						/* Used for shape centering */
	
	Input = (DialogControlTextMenu_t *)wow;

	x = Input->Root.Bounds.left;				/* Get the origin X and Y */
	y = Input->Root.Bounds.top;
	Width = Input->Root.Bounds.right-x;			/* Get the bounds rect */
	Height = Input->Root.Bounds.bottom-y;
	CellHeight = Input->CellHeight;				/* Height of the box in cells */
	
	x += DrawX;
	y += DrawY;
	
	/* First, draw the drop shadow that encompasses this menu */
	
	ScreenBoxDropShadow(x,y,Width,Height,Input->BoxColor1,Input->BoxColor2);			/* Draw the bounding box */
	TextX = (FontWidthChar(Input->FontPtr,' ')>>1)+2+x;			/* X Coord to draw text at */
	
	++x;				/* Inset the box a little */
	++y;
	Width-=2;

	StartIndex = Input->ScrollValue;
	FontHeight = Input->NormColor;
	if (!StartIndex && !Input->CursorValue) {		/* Special case to extend the fill color */
		FontHeight = Input->CursorColor;
	}
	ScreenRect(x,y,Width,1,FontHeight);				/* Leading line */

	++y;
	Height-=3;			/* Remove from the clip rect */
	
	if (CellHeight) {
		TextRect.left = x;						/* Get the text box */
		TextRect.top = y;
		TextRect.right = x+Width;
		TextRect.bottom = y+Height;
		GetTheClipRect(&ClipRect);		/* Get the screen clip rect */
		SetTheClipRect(&TextRect);		/* Set the bounds rect to draw to */
		
		/* Draw the text in the menu */
		
		FontHeight = Input->FontPtr->FontHeight;		/* Step value for Y */
		
		/* Should I draw the up arrow? */
		
		if (StartIndex>0) {								/* Clipped off the top? */
			ScreenRect(x,y,Width,FontHeight,Input->NormColor);		/* Blank the background */
			ScreenShapeGetSize(Input->Art[0],&TempSize);			/* Draw the icon centered */
			ScreenShapeDraw(Input->Art[0],(x+(Width>>1))-(TempSize.x>>1),(y+(FontHeight>>1))-(TempSize.y>>1));
			y+=FontHeight;
			++StartIndex;			/* Skip an entry */
			--CellHeight;			/* Remove a cell */
		}
				
		if (CellHeight) {				/* Failsafe? */
		
			/* Shall I draw blank entries? */
	
			if (StartIndex<0) {			/* Draw a blank? */
				Height = 0-StartIndex;	/* Get the number of cells to skip */
				if (Height>=CellHeight) {		/* Off the bottom? */
					Height = CellHeight-1;		/* Leave a single cell for the down arrow */
				}
				CellHeight -= Height;			/* Remove the cells from the count */
				Height = Height*FontHeight;		/* Number of pixels */
				ScreenRect(x,y,Width,Height,Input->NormColor);	/* Draw the empty cell */
				y+=Height;
				StartIndex = 0;			/* Start here */
			}
		
			/* Any text to draw? */
			
			Height = LinkedListGetSize(Input->ListPtr);
			if ((Word)StartIndex>Height) {	/* Off the table? */
				StartIndex = Height;		/* Number of strings to print */
			}
			Height -= StartIndex;			/* This is the string count */
			if (Height>CellHeight) {
				Height = CellHeight-1;		/* Leave space for the down arrow */
			}
			
			if (Height) {					/* Any room to draw (And anything to draw) */
				CellHeight-=Height;			/* Remove the string count */
				EntryPtr = LinkedListGetEntry(Input->ListPtr,StartIndex);		/* Get the pointer to the text to draw */
				do {
					FontRef_t *FPtr;
					Word Color;
					FPtr = Input->FontPtr;				/* Normal font */
					if (StartIndex==(int)Input->Value) {
						FPtr = Input->FontPtr2;			/* Highlighted font */
					}
					Color = Input->NormColor;			/* Normal background */
					if (StartIndex==(int)Input->CursorValue) {
						Color = Input->CursorColor;		/* Highlighted background */
					}
					ScreenRect(x,y,Width,FontHeight,Color);			/* Solid bar */
					FontDrawStringAtXY(FPtr,TextX,y,(char *)EntryPtr->Data);		/* Draw the line of text */
					y +=FontHeight;									/* Next line down */
					++StartIndex;
					EntryPtr = EntryPtr->Next;		/* Follow the list */
				} while (--Height);				/* Off the bottom? */
			}
			
			/* Down arrow? */
			
			if (CellHeight==1 && (Word)StartIndex<LinkedListGetSize(Input->ListPtr)) {		/* Clipped off the bottom? */
				ScreenRect(x,y,Width,FontHeight,Input->NormColor);
				ScreenShapeGetSize(Input->Art[1],&TempSize);
				ScreenShapeDraw(Input->Art[1],(x+(Width>>1))-(TempSize.x>>1),(y+(FontHeight>>1))-(TempSize.y>>1));
				y+=FontHeight;

			/* Fill the rest with the fill color */
			
			} else if (CellHeight) {		/* Empty cells up top */
				Height = FontHeight*CellHeight;
				ScreenRect(x,y,Width,Height,Input->NormColor);	/* Draw the empty cell */
				y+=Height;
			}
		}
		SetTheClipRect(&ClipRect);					/* Restore the clip rect */
	}
	
	/* Draw the bottom line */
	
	FontHeight = Input->NormColor;
	if ((Input->CursorValue==(LinkedListGetSize(Input->ListPtr)-1)) &&		/* Last one accepted */
		((Input->CellHeight+Input->ScrollValue)==(Word)StartIndex)) {
		FontHeight = Input->CursorColor;
	}
	ScreenRect(x,y,Width,1,FontHeight);		/* Bottom line */
}

/**********************************

	Check if a point is within a button
	I return one of these values...
	BUTTON_DOWN = If the mouse is held down and I want to keep focus
	BUTTON_CLICKED = If the mouse was released inside me and I had focus
	BUTTON_RELEASED = If the mouse was released out of me and I had focus
	BUTTON_INSIDE = If the mouse is over the control
	BUTTON_OUTSIDE = If the mouse is outside of the control

**********************************/

static DialogControlAction_e BURGERCALL DialogControlTextMenuCheck(DialogControl_t *Inputx,int x,int y,Word buttons,Word /* Key */)
{
	DialogControlAction_e Result;
	DialogControlTextMenu_t *Input;
	
	Input = (DialogControlTextMenu_t *)Inputx;
	
	if (buttons) {
		buttons = TRUE; 		/* I only want TRUE or false */
	}
	
	Input->Root.Inside = static_cast<Word8>(LBRectPtInRect(&Input->Root.Bounds,x,y));		/* Inside of the rect? */
	
	/* Now I have the current motion state */
			
	/* Mouse down? */
	
	if (buttons) {
		if (Input->Root.Focus || Input->Root.Inside) {		/* Keep focus */
			Word Value;
			Word32 Mark;
			
			Input->Root.Focus = TRUE;			/* I'll keep focus */
			Input->CursorValue = (Word)-1;		/* Assume outside */
			Result = BUTTON_DOWN;			/* Grab focus */
			if (Input->Root.Inside) {			/* Inside the menu bar? */
				Value = ((y-Input->Root.Bounds.top)/Input->FontPtr->FontHeight);	/* Which cell? */
				if (!Value && Input->ScrollValue>0) {		/* Clipped off the top? */
					Mark = ReadTick();
					if ((Mark-Input->TimeMark)>=(TICKSPERSEC/30)) {	/* Did enough time elapse? */
						Input->TimeMark = Mark;						/* Reset the timer */
						Input->ScrollValue--;
					}
					goto Exit;
				}
				if ((Value==(Input->CellHeight-1)) &&
					((Input->ScrollValue+Input->CellHeight)<LinkedListGetSize(Input->ListPtr))) {
					Mark = ReadTick();
					if ((Mark-Input->TimeMark)>=(TICKSPERSEC/30)) {	/* Did enough time elapse? */
						Input->TimeMark = Mark;						/* Reset the timer */
						Input->ScrollValue++;
					}
					goto Exit;
				}
				Value += Input->ScrollValue;
				if (Value<LinkedListGetSize(Input->ListPtr)) {		/* Inside the menu and a cell? */
					Input->CursorValue = Value;
				}
			}
			goto Exit;
		}

	} else if (Input->Root.Focus) {		/* I only care if I have focus */
		Input->Root.Focus = FALSE;		/* Drop focus */

		/* Did I release OUTSIDE? */

		if (!Input->Root.Inside) {
			Result = BUTTON_RELEASED;		/* Don't accept the click */
		} else {
			if (Input->CursorValue!=(Word)-1) {
				Input->Value = Input->CursorValue;
			}
			Result = BUTTON_CLICKED;		/* I accept this click */
		}
		goto Exit;
	}

	/* Ok, now return my state flag */
	
	Result = BUTTON_INSIDE;
	if (!Input->Root.Inside) {
		Result = BUTTON_OUTSIDE;		/* I'm not active */
	}
Exit:;
	/* Do I have a callback proc? */
	if (Input->Root.Event) {
		Input->Root.Event(&Input->Root,Result);	/* Pass the current state */
	}
	return Result;

}

/**********************************

	Initialized a DialogControlTextMenu_t

**********************************/

void BURGERCALL DialogControlTextMenuInit(DialogControlTextMenu_t *Input,ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,int x,int y,LinkedList_t *ListPtr,Word Value,DialogControlEventProc EventProc)
{
	Word Width,Height;
	Word CellHeight;
	int y2;
	
	Input->Root.Active = TRUE;
	Input->Root.Invisible = FALSE;
	Input->Root.Focus = FALSE;		/* Doesn't have focus */
	Input->Root.Inside = FALSE;		/* Set the current default mode */
	Input->Root.Draw = DialogControlTextMenuDraw;
	Input->Root.Delete = 0;
	Input->Root.Check = DialogControlTextMenuCheck;
	Input->Root.Event = EventProc;		/* No button specific callback */
	Input->Root.RefCon = 0;
	Input->Root.HotKey = 0;
	
	Input->FontPtr = FontPtr;
	Input->FontPtr2 = FontPtr2;
	Input->Art = ArtArray;
	Input->ScrollValue = 0;
	Input->TimeMark = 0;
	Input->NormColor = PaletteConvertPackedRGBToDepth(DialogControlMenuBackgroundColor,VideoColorDepth);
	Input->CursorColor = PaletteConvertPackedRGBToDepth(DialogControlMenuSelectColor,VideoColorDepth);
	Input->BoxColor1 = PaletteConvertPackedRGBToDepth(DialogControlShadowColor1,VideoColorDepth);
	Input->BoxColor2 = PaletteConvertPackedRGBToDepth(DialogControlShadowColor2,VideoColorDepth);
	Input->Value = Value;
	Input->CursorValue = (Word)-1;		/* Nothing selected */
	Input->ListPtr = ListPtr;

	/* The pop-up has a dynamic size, get the size */
	/* and make sure that it is on the screen! */
	
	Width = FontWidthListWidest(FontPtr,ListPtr)+FontWidthChar(FontPtr,' ')+4;		/* Width of the menu */

	y2 = y;						/* Save the anchor point */
	if ((x+(int)Width)>=(int)ScreenWidth) {		/* Off the right side? */
		x = ScreenWidth-Width;		/* Push back onto the screen */
	}
	if (x<0) {						/* Off the left side? (Has precedence if both are clipped) */
		x = 0;						/* Anchor to the left side */
	}
	Input->Root.Bounds.left = x;
	Input->Root.Bounds.right = x+Width;			/* Here is the base bounds rect */
	
	/* Height of the menu */
	
	CellHeight = LinkedListGetSize(ListPtr);
	Height = (CellHeight*FontPtr->FontHeight)+4;		/* Number of pixels high */

	if (Value!=(Word)-1) {
		y-=(FontPtr->FontHeight*Value);
	}
	/* Now, the menu cannot exceed the screen's size */
	
	if (Height>ScreenHeight) {
		CellHeight = ScreenHeight/FontPtr->FontHeight;
		Height = (CellHeight*FontPtr->FontHeight)+4;	/* Keep in increments of the font */
		if (Height>ScreenHeight) {				/* Did the +4 push me over the edge? */
			Height -= FontPtr->FontHeight;		/* Remove a cell */
			--CellHeight;
		}
	}
	Input->CellHeight = CellHeight;

	if ((y+(int)Height)>=(int)ScreenHeight) {
		y = ScreenHeight-Height;	/* Push back on the screen */
	}
	if (y<0) {
		y = 0;						/* Anchor to the top */
	}
	Input->Root.Bounds.top = y;
	Input->Root.Bounds.bottom = y+Height;
	
	/* Now comes the fun part */
	/* A pop-up menu is anchored where the SELECTED item is!!! */
	
	Input->ScrollValue = ((y-y2)/(int)FontPtr->FontHeight)+Value;
	
}

/**********************************

	Create a new DialogControlTextMenu_t

**********************************/

DialogControlTextMenu_t *BURGERCALL DialogControlTextMenuNew(ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,int x,int y,LinkedList_t *ListPtr,Word Value,DialogControlEventProc EventProc)
{
	DialogControlTextMenu_t *MyPtr;

	MyPtr = (DialogControlTextMenu_t *)AllocAPointer(sizeof(DialogControlTextMenu_t));	/* Get the memory */
	if (MyPtr) {
		DialogControlTextMenuInit(MyPtr,ArtArray,FontPtr,FontPtr2,x,y,ListPtr,Value,EventProc);		/* Init it */
		MyPtr->Root.Delete = DialogControlDeleteProc;
	}
	return MyPtr;			/* Return the pointer */
}





/**********************************

	Destructors

**********************************/

static void BURGERCALL DialogControlPopupMenuDestroy(DialogControl_t *Input)
{
	DialogControlTextMenu_t *MenuPtr;			/* Pop up menu */
	MenuPtr = ((DialogControlPopupMenu_t *)Input)->MenuPtr;		/* Is there a scroll bar? */
	if (MenuPtr) {
		DialogControlDelete(&MenuPtr->Root);
	}
	LinkedListDestroy(&((DialogControlPopupMenu_t *)Input)->List);	/* Delete my text formatting */
}

static void BURGERCALL DialogControlPopupMenuDelete(DialogControl_t *Input)
{
	DialogControlTextMenu_t *MenuPtr;			/* Pop up menu */
	MenuPtr = ((DialogControlPopupMenu_t *)Input)->MenuPtr;		/* Is there a scroll bar? */
	if (MenuPtr) {
		DialogControlDelete(&MenuPtr->Root);		/* Destroy it */
	}
	LinkedListDestroy(&((DialogControlPopupMenu_t *)Input)->List);	/* Delete my text formatting */
	DeallocAPointer(Input);
}

/**********************************

	Draw the text list

**********************************/

static void BURGERCALL DialogControlPopupMenuDraw(DialogControl_t *wow, int DrawX, int DrawY)
{
	DialogControlPopupMenu_t *Input;
	DialogControlTextMenu_t *MenuPtr;			/* Scroll bar */
	Word Width,Height;
	int x,y;
	int RightX;
	LBPoint TempSize;
	Word Index;
	LinkedListEntry_t *EntryPtr;
	ScreenShape_t *ArtPtr;

	Input = (DialogControlPopupMenu_t *)wow;

	x = Input->Root.Bounds.left;				/* Get the origin X and Y */
	y = Input->Root.Bounds.top;
	Width = Input->Root.Bounds.right-x;			/* Get the bounds rect */
	Height = Input->Root.Bounds.bottom-y;
	
	x += DrawX;
	y += DrawY;
	
	ArtPtr = Input->Art[0];
	Index = 2;
	if (Input->Root.Focus) {							/* Highlight the bar? */
		Index = 5;
		ArtPtr = Input->Art[1];							/* Highlight button */
	}
	
	ScreenShapeGetSize(ArtPtr,&TempSize);		/* Get the size of the button on the right */
	RightX = (x+Width)-TempSize.x;						/* X coord of the end of the bar */
	ScreenShapeHPatternBar(&Input->Art[Index],y,x,RightX);		/* Draw the background button */
	ScreenShapeDraw(ArtPtr,RightX,y);					/* Draw the right button */
	
	/* Shall I draw text? */
	
	EntryPtr = LinkedListGetEntry(&Input->List,Input->Value);		/* Get the pointer to the text to draw */
	if (EntryPtr) {
		FontRef_t *FPtr;
		ScreenShapeGetSize(Input->Art[Index+1],&TempSize);			/* Edge of the background for text to draw */
		FPtr = Input->FontPtr;
		FontDrawStringAtXY(FPtr,x+TempSize.x,y+((int)(Height-FPtr->FontHeight)>>1),(char *)EntryPtr->Data);
	}
	MenuPtr = Input->MenuPtr;					/* Is there a pop-up menu bar? */
	if (MenuPtr) {
		MenuPtr->Root.Draw(&MenuPtr->Root, DrawX, DrawY);		/* Draw the pop-up menu if present */
	}
}

/**********************************

	Check if a point is within a button
	I return one of these values...
	BUTTON_DOWN = If the mouse is held down and I want to keep focus
	BUTTON_CLICKED = If the mouse was released inside me and I had focus
	BUTTON_RELEASED = If the mouse was released out of me and I had focus
	BUTTON_INSIDE = If the mouse is over the control
	BUTTON_OUTSIDE = If the mouse is outside of the control

**********************************/

static DialogControlAction_e BURGERCALL DialogControlPopupMenuCheck(DialogControl_t *Inputx,int x,int y,Word buttons,Word Key)
{
	DialogControlAction_e Result;
	DialogControlPopupMenu_t *Input;
	DialogControlTextMenu_t *MenuPtr;			/* Scroll bar */
	
	Input = (DialogControlPopupMenu_t *)Inputx;
	MenuPtr = Input->MenuPtr;
	if (MenuPtr) {
		MenuPtr->Root.Check(&MenuPtr->Root,x,y,buttons,Key);		/* Pass the event to the control list */
		Input->Value = Input->MenuPtr->Value;					/* Propagate the slider bar value to my root */
	}
	
	if (buttons) {
		buttons = TRUE; 		/* I only want TRUE or false */
	}
	
	Input->Root.Inside = static_cast<Word8>(LBRectPtInRect(&Input->Root.Bounds,x,y));		/* Inside of the rect? */
	
	/* Now I have the current motion state */
			
	/* Mouse down? */
	
	if (buttons) {
		if (Input->Root.Focus) {
			Result = BUTTON_DOWN;			/* Keep focus */
			goto Exit;
		}

		if (Input->Root.Inside) {
			Input->Root.Focus = TRUE;			/* I'll keep focus */
			if (!MenuPtr && LinkedListGetSize(&Input->List)) {
				MenuPtr = DialogControlTextMenuNew(&Input->Art[8],Input->FontPtr,Input->FontPtr2,Input->Root.Bounds.left,
					Input->Root.Bounds.top,&Input->List,Input->Value,0);
				if (MenuPtr) {
					Input->MenuPtr = MenuPtr;
					MenuPtr->Root.Focus = TRUE;
					MenuPtr->NormColor = Input->NormColor;		/* Color overrides */
					MenuPtr->CursorColor = Input->CursorColor;
					MenuPtr->BoxColor1 = Input->BoxColor1;
					MenuPtr->BoxColor2 = Input->BoxColor2;
				}
			}
			Result = BUTTON_DOWN;			/* Grab focus */
			goto Exit;
		}

	} else if (Input->Root.Focus) {		/* I only care if I have focus */
		Input->Root.Focus = FALSE;		/* Drop focus */

		/* Did I release OUTSIDE? */

		Result = BUTTON_RELEASED;		/* Don't accept the click */
		if (MenuPtr) {
			Word NewValue;
			NewValue = MenuPtr->Value;
			if (NewValue!=(Word)-1) {
				Input->Value = NewValue;
				Result = BUTTON_CLICKED;
			}
			DialogControlDelete(&MenuPtr->Root);
			Input->MenuPtr = 0;
		}

		goto Exit;
	}

	/* Ok, now return my state flag */
	
	Result = BUTTON_INSIDE;
	if (!Input->Root.Inside) {
		Result = BUTTON_OUTSIDE;		/* I'm not active */
	}
Exit:;
	/* Do I have a callback proc? */
	if (Input->Root.Event) {
		Input->Root.Event(&Input->Root,Result);	/* Pass the current state */
	}
	return Result;

}

/**********************************

	Set the scroll value for the text

**********************************/

void BURGERCALL DialogControlPopupMenuSetValue(DialogControlPopupMenu_t *Input,Word NewValue)
{
	if (NewValue>=LinkedListGetSize(&Input->List)) {
		NewValue = 0;
	}
	Input->Value = NewValue;					/* No slider, no change */
}

/**********************************

	Add an entry to the list
	
**********************************/

Word BURGERCALL DialogControlPopupMenuAddText(DialogControlPopupMenu_t *Input,const char *TextPtr,Word EntryNum)
{
	LinkedListEntry_t *EntryPtr;

	if (EntryNum!=(Word)-1 && (EntryPtr = LinkedListGetEntry(&Input->List,EntryNum)) != 0) {		/* Check the list for a string */
		LinkedListAddNewEntryStringAfter(&Input->List,EntryPtr,TextPtr);
	} else {
		EntryNum = LinkedListGetSize(&Input->List);
		LinkedListAddNewEntryStringEnd(&Input->List,TextPtr);
	}
	return EntryNum;
}

/**********************************

	Delete an entry in the list
	
**********************************/

void BURGERCALL DialogControlPopupMenuRemoveText(DialogControlPopupMenu_t *Input,Word EntryNum)
{
	LinkedListEntry_t *EntryPtr;

	EntryPtr = LinkedListGetEntry(&Input->List,EntryNum);		/* Check the list for a string */
	if (EntryPtr) {				/* Any data in the list? */
		LinkedListDeleteEntry(&Input->List,EntryPtr);
		if (Input->Value>=EntryNum) {
			Input->Value = 0;
		}
	}
}

/**********************************

	Dispose of all the strings in the text box
	
**********************************/

void BURGERCALL DialogControlPopupMenuRemoveAllText(DialogControlPopupMenu_t *Input)
{
	if (Input->List.Count) {
		do {
			DialogControlPopupMenuRemoveText(Input,0);
		} while (Input->List.Count);
	}
}

/**********************************

	Find a string in the list box
	
**********************************/

Word BURGERCALL DialogControlPopupMenuFindText(DialogControlPopupMenu_t *Input,const char *TextPtr)
{
	return LinkedListFindString(&Input->List,TextPtr);		/* Find it */
}

/**********************************

	Initialized a DialogControlTextList_t

**********************************/

void BURGERCALL DialogControlPopupMenuInit(DialogControlPopupMenu_t *Input,ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,DialogControlEventProc EventProc)
{
#if 0
	LBPoint ShapeSize;
#endif
	
	Input->Root.Bounds = *Bounds;
	Input->Root.Active = TRUE;
	Input->Root.Invisible = FALSE;
	Input->Root.Focus = FALSE;		/* Doesn't have focus */
	Input->Root.Inside = FALSE;		/* Set the current default mode */
	Input->Root.Draw = DialogControlPopupMenuDraw;
	Input->Root.Delete = DialogControlPopupMenuDestroy;
	Input->Root.Check = DialogControlPopupMenuCheck;
	Input->Root.Event = EventProc;		/* No button specific callback */
	Input->Root.RefCon = 0;
	Input->Root.HotKey = 0;
	
#if 0
	ScreenShapeGetSize(ArtArray[2],&ShapeSize);		/* Get the size of this shape */
	Input->Root.Bounds.left = x;
	Input->Root.Bounds.right = x+Width;
	Input->Root.Bounds.top = y;
	Input->Root.Bounds.bottom = y+ShapeSize.y;
#endif
	Input->FontPtr = FontPtr;
	Input->FontPtr2 = FontPtr2;
	Input->Art = ArtArray;
	Input->MenuPtr = 0;
	Input->Value = (Word)-1;
	Input->NormColor = PaletteConvertPackedRGBToDepth(DialogControlMenuBackgroundColor,VideoColorDepth);
	Input->CursorColor = PaletteConvertPackedRGBToDepth(DialogControlMenuSelectColor,VideoColorDepth);
	Input->BoxColor1 = PaletteConvertPackedRGBToDepth(DialogControlShadowColor1,VideoColorDepth);
	Input->BoxColor2 = PaletteConvertPackedRGBToDepth(DialogControlShadowColor2,VideoColorDepth);
	LinkedListInit(&Input->List);
}

/**********************************

	Create a new DialogControlTextList_t

**********************************/

DialogControlPopupMenu_t *BURGERCALL DialogControlPopupMenuNew(ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,DialogControlEventProc EventProc)
{
	DialogControlPopupMenu_t *MyPtr;

	MyPtr = (DialogControlPopupMenu_t *)AllocAPointer(sizeof(DialogControlPopupMenu_t));	/* Get the memory */
	if (MyPtr) {
		DialogControlPopupMenuInit(MyPtr,ArtArray,FontPtr,FontPtr2,Bounds,EventProc);		/* Init it */
		MyPtr->Root.Delete = DialogControlPopupMenuDelete;
	}
	return MyPtr;			/* Return the pointer */
}


/**********************************

	Draw a standard button

**********************************/

static void BURGERCALL DialogControlPictureDraw(DialogControl_t *wow, int DrawX, int DrawY)
{
	ScreenShape_t *ShapePtr;
	DialogControlPicture_t *Input;
	
	Input = (DialogControlPicture_t *)wow;
	
	ShapePtr = Input->Art;		/* Get the shape to draw */
	if (ShapePtr) {								/* Can be a NULL pointer */
		ScreenShapeDraw(ShapePtr,Input->Root.Bounds.left/*-Input->x*/ + DrawX,Input->Root.Bounds.top/*-Input->y*/ + DrawY);		/* Draw the button */
	}
}

/**********************************

	Given a pointer to an uninitialized Button, Set the defaults

**********************************/

void BURGERCALL DialogControlPictureInit(DialogControlPicture_t *Input,ScreenShape_t *Art,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc)
{
	/* Init the base class */
	Input->Root.Bounds = *Bounds;
	Input->Root.Active = TRUE;
	Input->Root.Invisible = FALSE;
	Input->Root.Focus = FALSE;		/* Doesn't have focus */
	Input->Root.Inside = FALSE;		/* Set the current default mode */
	Input->Root.Draw = DialogControlPictureDraw;
	Input->Root.Delete = 0;
	Input->Root.Check = DialogControlCheck;
	Input->Root.Event = EventProc;		/* No Picture specific callback */
	Input->Root.RefCon = 0;
	Input->Root.HotKey = HotKey;

	/* Now init my local variables */
	DialogControlPictureSetArt(Input,Art);
}


extern void BURGERCALL DialogControlPictureSetArt(DialogControlPicture_t *Input,struct ScreenShape_t *Art)
{
	Input->Art = Art;		/* Make a new frame */
/*	if( Art ) {
		LBRect	ShapeBounds;
		ScreenShapeGetBounds(Art,&ShapeBounds);
		Input->x = ShapeBounds.left;
		Input->y = ShapeBounds.top;
	}
*/
}

/**********************************

	Allocate a new DialogControlPicture_t struct and init it

**********************************/

DialogControlPicture_t * BURGERCALL DialogControlPictureNew(struct ScreenShape_t *Art,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc)
{
	DialogControlPicture_t *MyPtr;

	MyPtr = (DialogControlPicture_t *)AllocAPointer(sizeof(DialogControlPicture_t));	/* Get the memory */
	if (MyPtr) {
		DialogControlPictureInit(MyPtr,Art,Bounds,HotKey,EventProc);		/* Init it */
		MyPtr->Root.Delete = DialogControlDeleteProc;
	}
	return MyPtr;			/* Return the pointer */
}

