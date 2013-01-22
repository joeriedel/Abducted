/**********************************

	MS-DOS version

**********************************/

#include "InInput.h"

/**********************************
	
	The joystick port is 0x0201

**********************************/

#if defined(__MSDOS__)

Word32 MyAsm(Word Which);
#pragma aux MyAsm = \
	"PUSH	EBP" \
	"PUSH	EDI" \
	"PUSH	EBX" \
	"PUSH	ECX" \
	"PUSH	EDX" \
	"MOV	EDI,EAX" \
	"TEST	EAX,EAX"	/* Joystick 0 or 1 */ \
	"SETNE	CL"	 		/* Set or clear ECX */ \
	"MOV	EAX,-1"		/* Fill the init value with a bogus number */ \
	"ADD	CL,CL"		/* 0 or 2 */ \
	"MOV	EDX,0201H"	/* Joy port */ \
	"ADD	CL,4"		/* If zero then 4, else 6 */ \
"OhOh:" \
	"MOV	EBP,6"		/* Must match for 6 tries */ \
	"MOV	EBX,EAX"	/* Set the previous value */ \
"Again:" \
	"IN		AL,DX"		/* Read the joystick buttons */ \
	"SHR	AL,CL"		/* Shift to the low 2 bits */ \
	"NOT	AL"			/* Reverse the bits */ \
	"AND	EAX,3"		/* Return the valid bits */ \
	"CMP	EAX,EBX"	/* Save as before (Always fails the first time) */ \
	"JNZ	OhOh"		/* No good! */ \
	"DEC	EBP"		/* Count down */ \
	"JNZ	Again"		/* More */ \
	"CMP	EDI,4" \
	"JAE	Exit" \
	"MOV	[JoystickLastButtons+EDI*4],EAX" \
"Exit:" \
	"POP	EDX" \
	"POP	ECX" \
	"POP	EBX" \
	"POP	EDI" \
	"POP	EBP" \
	parm [eax] value [eax]

Word32 BURGERCALL JoystickReadButtons(Word Which)
{
	return MyAsm(Which);
}

Word BURGERCALL JoystickReadAbs(Word /* Axis */,Word /* Which */)
{
	return 128;		/* Read the default */
}

void BURGERCALL JoystickReadNow(Word /* Which */)
{
}

Word BURGERCALL JoystickInit(void)
{
	JoystickPresent = 0;
	return FALSE;
}

void BURGERCALL JoystickDestroy(void)
{
	JoystickPresent = 0;
}


#endif