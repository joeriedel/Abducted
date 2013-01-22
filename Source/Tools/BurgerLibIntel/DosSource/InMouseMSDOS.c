#include "InInput.h"

/**********************************

	DOS version

**********************************/

#if defined(__MSDOS__)			/* DOS version */

/**********************************

	Detect the mouse
	Return true if a mouse is present

**********************************/

extern short GetMouseInfo(void);
#pragma aux GetMouseInfo = \
	"XOR EAX,EAX"	/* Reset the mouse driver */ \
	"INT 033H"		\
	value [ax]		\
	modify [eax ebx ecx edx];

Word BURGERCALL MouseInit(void)
{
	Word Temp;
	Temp = (GetMouseInfo()==0) ? FALSE : TRUE;
	MousePresent = Temp;
	return Temp;
}

/**********************************

	Release the mouse

**********************************/

void BURGERCALL MouseDestroy(void)
{
}

/**********************************

	Read the buttons from the mouse

**********************************/

static Word LastMouseButton;

extern Word ReadMouse(void);
#pragma aux ReadMouse = \
	"MOV	EAX,3"	/* Call BIOS to read the keyboard */ \
	"INT	033H" \
	"AND	EBX,7" \
	value [ebx]		\
	modify [eax ebx ecx edx];

Word BURGERCALL MouseReadButtons(void)
{
	Word Temp,Temp2;
	Temp = 0;
	if (MousePresent) {
		Temp = ReadMouse();
		Temp2 = (LastMouseButton^Temp)&Temp;	/* Mouse down events */
		MouseClicked |= Temp2;
		LastMouseButton = Temp;
	}
	return Temp;
}

/**********************************

	Read the mouse x and y
	in LOCAL coordinates!

**********************************/

extern void GetMAbs(Word *x,Word *y);
#pragma aux GetMAbs = \
	"PUSH	ESI" \
	"PUSH	EDI" \
	"PUSH	ECX" \
	"PUSH	EBX" \
	"MOV	ESI,EAX" \
	"MOV	EDI,EDX" \
	"MOV	EAX,[MousePresent]" \
	"XOR	ECX,ECX" \
	"XOR	EDX,EDX" \
	"AND	EAX,EAX" \
	"JZ		NoRat" \
	"MOV	EAX,3"	/* Call BIOS to read the keyboard */ \
	"INT	033H"		\
	"AND	ECX,0FFFFH" \
	"AND	EDX,0FFFFH" \
"NoRat:" \
	"AND	ESI,ESI" \
	"JZ		NoX" \
	"MOV	[ESI],ECX" \
"NoX:" \
	"AND	EDI,EDI" \
	"JZ		NoY" \
	"MOV	[EDI],EDX" \
"NoY:" \
	"POP	EBX" \
	"POP	ECX" \
	"POP	EDI" \
	"POP	ESI" \
	parm [eax] [edx]

void BURGERCALL MouseReadAbs(Word *x,Word *y)
{
	GetMAbs(x,y);		/* Call assembly */
}

/**********************************

	Read the mouse delta movement from the last reading

**********************************/

extern void GetMDel(int *x,int *y);
#pragma aux GetMDel = \
	"PUSH	ESI" \
	"PUSH	EDI" \
	"PUSH	ECX" \
	"PUSH	EBX" \
	"MOV	ESI,EAX" \
	"MOV	EDI,EDX" \
	"MOV	EAX,[MousePresent]" \
	"XOR	ECX,ECX" \
	"XOR	EDX,EDX" \
	"AND	EAX,EAX" \
	"JZ		NoRat" \
	"MOV	EAX,11"	/* Call BIOS to read the keyboard */ \
	"INT	033H"		\
	"MOVSX	ECX,CX" \
	"MOVSX	EDX,DX" \
"NoRat:" \
	"AND	ESI,ESI" \
	"JZ		NoX" \
	"MOV	[ESI],ECX" \
"NoX:" \
	"AND	EDI,EDI" \
	"JZ		NoY" \
	"MOV	[EDI],EDX" \
"NoY:" \
	"POP	EBX" \
	"POP	ECX" \
	"POP	EDI" \
	"POP	ESI" \
	parm [eax] [edx]

void BURGERCALL MouseReadDelta(int *x,int *y)
{
	GetMDel(x,y);		/* Call assembly */
}

/**********************************

	Sets the mouse movement bounds

**********************************/

extern void SetR(Word x,Word y);
#pragma aux SetR = \
	"PUSH EAX" \
	"MOV EAX,8"	\
	"XOR ECX,ECX" \
	"DEC EDX" \
	"INT 033H" \
	"POP EDX" \
	"MOV EAX,7" \
	"XOR ECX,ECX" \
	"DEC EDX" \
	"INT 033H" \
	parm [eax] [edx] \
	modify [eax ebx ecx edx];

void BURGERCALL MouseSetRange(Word x,Word y)
{
	if (MousePresent) {		/* Is there a mouse? */
		SetR(x,y);			/* Call the mouse driver to set the bounds */
	}
}

/**********************************

	Set the new mouse position

**********************************/

extern void SetIt(Word x,Word y);
#pragma aux SetIt = \
	"MOV ECX,EAX" \
	"MOV EAX,4"	\
	"AND ECX,0FFFFH" \
	"AND EDX,0FFFFH" \
	"INT 033H" \
	parm [eax] [edx] \
	modify [eax ecx edx];

void BURGERCALL MouseSetPosition(Word x,Word y)
{
	if (MousePresent) {		/* Only call if a mouse driver is present */
		SetIt(x,y);			/* Perform the call */
	}
}


#endif
