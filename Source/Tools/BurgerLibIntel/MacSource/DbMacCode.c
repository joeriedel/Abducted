#include "ClStdLib.h"

/********************************

	MacOS version

********************************/

#if defined(__MAC__)

#include "MmMemory.h"
#include "InInput.h"
#include "McMac.h"
#include <string.h>
#include <Gestalt.h>
#include <QuickDraw.h>
#include <MacMemory.h>
#include <Dialogs.h>
#if !TARGET_API_MAC_CARBON
#include <MetroNubUserInterface.h>
#endif

/**********************************

	Invoke the debugger
	
**********************************/

typedef struct Halt_t {
	Word Tested;	/* Did I test for the debugger? */
	Word Present;	/* Is it present? */
} Halt_t;

static Halt_t Locals;		/* Local variables */

void Halt(void)
{
#if !TARGET_API_MAC_CARBON
	if (!Locals.Tested) {
		long Answer;
		Locals.Tested = TRUE;		/* Checked for the debugger */
		if (Gestalt(kMetroNubUserSignature,&Answer) == noErr) {
			MetroNubUserEntryBlock* block = (MetroNubUserEntryBlock *)Answer;
			// make sure the version of the API is compatible
			if (block->apiLowVersion <= kMetroNubUserAPIVersion &&
				kMetroNubUserAPIVersion <= block->apiHiVersion) {
				if (CallIsDebuggerRunningProc(block->isDebuggerRunning)) {
					Locals.Present = TRUE;
				}
			}
		}
	}
	if (Locals.Present) {		/* Is it present? */
		Word Foo;
		Foo = InputSetState(FALSE);			/* Kill inputsprocket */
		Debugger();		/* Invoke the debugger */
		InputSetState(Foo);					/* Restore input sprocket */
	}
#else
	Word Foo;
	Foo = InputSetState(FALSE);			/* Kill inputsprocket */
	Debugger();		/* Invoke the debugger */
	InputSetState(Foo);					/* Restore input sprocket */
#endif
}


/**********************************

	Mac OS version

**********************************/

static Word8 Template[] = {
	0x00,0x01,			/* 2 items in the list */
	0x00,0x00,0x00,0x00,	/* Nothing*/
	0x00,160,0x00,141,0x00,180,0x00,209,	/* Rect for the OK button Width 68*/
	0x04,
	0x02,'O','K',
	0x00,0x00,0x00,0x00,
	0x00,20,0x00,20,0x00,140,0x01,330-256,	/* Width 310 */
	0x88,0x00
};

void BURGERCALL OkAlertMessage(const char *Title,const char *Message)
{
	Word8 *TitleStr;		/* Pointer to the window title */
	DialogPtr MyDialog;	/* My dialog pointer */
	Handle ItemList;	/* Handle to the item list */
	Rect DialogRect;	/* Rect of the dialog window */
	Word TitleLen;		/* Length of the title */
	Word MessLen;		/* Length of the caption */
	short ItemHit;		/* Junk */
	Rect WorkRect;
	GrafPtr MyPort;	/* My grafport */
	Word Foo;
	
	Foo = InputSetState(FALSE);
		
	GetPort(&MyPort);	/* Save the current port */
	
	/* Center my dialog to the screen */
	GetPortBounds(MyPort,&WorkRect);
	DialogRect.top = (((WorkRect.bottom-WorkRect.top)-190)/2)+WorkRect.top;
	DialogRect.left = (((WorkRect.right-WorkRect.left)-350)/2)+WorkRect.left;
	DialogRect.bottom = DialogRect.top+190;
	DialogRect.right = DialogRect.left+350;

	TitleLen = 0;			/* Assume no length */
	if (Title) {
		TitleLen = strlen(Title);		/* Get the length of the title string */
	}
	TitleStr = (Word8 *)AllocAPointer(TitleLen+1);	/* Get memory of pascal string */
	if (TitleStr) {			/* Did I get the memory? */
		FastMemCpy(TitleStr+1,Title,TitleLen);
		TitleStr[0] = TitleLen;		/* Set the pascal length */
		
		MessLen = strlen(Message);	/* Size of the message */
		ItemList = NewHandle(sizeof(Template)+MessLen);
		if (ItemList) {				/* Ok? */
			Template[sizeof(Template)-1]=MessLen;	/* Save the message length */
			FastMemCpy(ItemList[0],Template,sizeof(Template));	/* Copy the template */
			FastMemCpy((ItemList[0])+sizeof(Template),Message,MessLen);	/* Copy the message */
			MyDialog = NewDialog(0,&DialogRect,(Word8 *)TitleStr,TRUE,5,(WindowPtr)-1,FALSE,0,ItemList);
			if (MyDialog) {
				SetDialogDefaultItem(MyDialog,1);	/* Default for OK button */
				ModalDialog(0,&ItemHit);			/* Handle the event */
				DisposeDialog(MyDialog);			/* Kill the dialog */
			} else {
				DisposeHandle(ItemList);			/* I must kill this myself! */
			}
		}
		DeallocAPointer(TitleStr);				/* Kill the title */
	}
	SetPort(MyPort);			/* Restore my grafport */
	InputSetState(Foo);
}

/**********************************

	Mac OS version

**********************************/

static Word8 TemplateX[] = {
	0x00,0x02,			/* 3 items in the list */
	0x00,0x00,0x00,0x00,	/* Nothing*/
	0x00, 160,0x01, 260-256,0x00, 180,0x01, 328-256,	/* Rect for the OK button Width 68*/
	0x04,
	0x02,'O','K',
	0x00,0x00,0x00,0x00,	/* Nothing*/
	0x00, 160,0x00, 180,0x00, 180,0x00, 248,	/* Rect for the OK button Width 68*/
	0x04,
	0x06,'C','a','n','c','e','l',
	0x00,0x00,0x00,0x00,
	0x00,  20,0x00,  20,0x00, 140,0x01, 330-256,	/* Width 310 */
	0x88,0x00
};

Word BURGERCALL OkCancelAlertMessage(const char *Title,const char *Message)
{
	Word Result;
	Word8 *TitleStr;		/* Pointer to the window title */
	DialogPtr MyDialog;	/* My dialog pointer */
	Handle ItemList;	/* Handle to the item list */
	Rect DialogRect;	/* Rect of the dialog window */
	Rect WorkRect;
	Word TitleLen;		/* Length of the title */
	Word MessLen;		/* Length of the caption */
	short ItemHit;		/* Junk */
	GrafPtr MyPort;	/* My grafport */
	Word Foo;
	
	Foo = InputSetState(FALSE);
	
	Result = FALSE;		/* Assume cancel */
	GetPort(&MyPort);	/* Save the current port */
	
	/* Center my dialog to the screen */
	GetPortBounds(MyPort,&WorkRect);
	DialogRect.top = (((WorkRect.bottom-WorkRect.top)-190)/2)+WorkRect.top;
	DialogRect.left = (((WorkRect.right-WorkRect.left)-350)/2)+WorkRect.left;
	DialogRect.bottom = DialogRect.top+190;
	DialogRect.right = DialogRect.left+350;

	TitleLen = 0;			/* Assume no length */
	if (Title) {
		TitleLen = strlen(Title);		/* Get the length of the title string */
	}
	TitleStr = (Word8 *)AllocAPointer(TitleLen+1);	/* Get memory of pascal string */
	if (TitleStr) {			/* Did I get the memory? */
		FastMemCpy(TitleStr+1,Title,TitleLen);
		TitleStr[0] = TitleLen;		/* Set the pascal length */
		
		MessLen = strlen(Message);	/* Size of the message */
		ItemList = NewHandle(sizeof(TemplateX)+MessLen);
		if (ItemList) {				/* Ok? */
			TemplateX[sizeof(TemplateX)-1]=MessLen;	/* Save the message length */
			FastMemCpy(ItemList[0],TemplateX,sizeof(TemplateX));	/* Copy the template */
			FastMemCpy((ItemList[0])+sizeof(TemplateX),Message,MessLen);	/* Copy the message */
			MyDialog = NewDialog(0,&DialogRect,(Word8 *)TitleStr,TRUE,5,(WindowPtr)-1,FALSE,0,ItemList);
			if (MyDialog) {
				SetDialogDefaultItem(MyDialog,1);	/* Default for OK button */
				SetDialogCancelItem(MyDialog,2);	/* Default for cancel button */
				ModalDialog(0,&ItemHit);			/* Handle the event */
				if (ItemHit==1) {		/* Pressed ok? */
					Result = TRUE;
				}
				DisposeDialog(MyDialog);			/* Kill the dialog */
			} else {
				DisposeHandle(ItemList);			/* I must kill this myself! */
			}
		}
		DeallocAPointer(TitleStr);				/* Kill the title */
	}
	SetPort(MyPort);			/* Restore my grafport */
	InputSetState(Foo);			/* Restore Inputsprocket */
	return Result;
}

#endif