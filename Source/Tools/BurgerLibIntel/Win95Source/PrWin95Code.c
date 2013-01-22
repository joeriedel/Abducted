/**********************************

	Windows 95 specific code
	
**********************************/

#include "PrProfile.h"

#if defined(__WIN32__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/**********************************

	Does the hardware exist to support the
	Profile Manager?

**********************************/

Word BURGERCALL ProfileIsAvailable(void)
{
	LARGE_INTEGER Result;		/* Frequency value */
	if (!QueryPerformanceFrequency(&Result)) {	/* If no error, then I'm ok! */
		if (Result.QuadPart) {		/* Must be non-zero */
			return TRUE;
		}
	}
	return FALSE;
}

#endif

/**********************************

	This code is for 386 and above intel processors
	If executed on anything like a 286, it would be bad.

	This assembly routine will initialize the static variables
	below, it is based on code supplied by Intel to identify
	specific CPU's

**********************************/

#if defined(__INTEL__)

#include "ClStdLib.h"
#include "PfPrefs.h"
#include <stdio.h>

static char VendorIDText[12];	/* CPU Vendor ID string */
static Word32 CPUBitFlags;	/* Features flags on CPU */
static Word8 CPURevision;		/* Stepping (It's really a revision) */
static Word8 CPUType;		/* CPU Type (family) 3,4,5,6+ */
static Word8 CPUModel;		/* Model (Sub type) */
static Word8 CeleronFlag;	/* TRUE if it's a celeron */
static Word8 CPUFloatType;	/* Floating point processor */

#if defined(__MWERKS__)
#define pushfd pushf
#define popfd popf
#define THECPUID cpuid
#elif defined(__WATCOMC__)
#define THECPUID cpuid
#else
#define THECPUID __asm _emit 0x0f __asm _emit 0xa2
#endif

static void GetCPUIDFlag(void)
{
	_asm {

	pushfd
	mov		[CPUType],3		;Init CPU type to 386
	pop		edx			;Get the flags
	mov		ebx,edx		;Save in temp
	xor		edx,0x40000	;Can I change bit #18?
	push	edx			;Set the flags
	popfd
	pushfd				;Get the flags now
	pop		edx
	cmp		edx,ebx		;Did the change "Take"?
	jz		JQuit7		;It's a 386
	mov		edx,ebx		;Get the flags again
	mov		[CPUType],4
	xor		edx,0x200000	;Toggle bit #21
	push	edx
	popfd				;Place in flags
	pushfd
	pop		edx
	cmp		edx,ebx		;Did it take?
	jz		TestCyrix	;I don't have CPU ID
	xor		eax, eax	;Set up input for CPU ID instruction
	THECPUID
	mov		DWORD PTR [VendorIDText],ebx		;Copy the 12 bytes if the vendor string
	mov		DWORD PTR [VendorIDText+4],edx
	mov		DWORD PTR [VendorIDText+8],ecx
	cmp		eax,1		;Can I ask more info?
	jl		JQuit7		;If not, jump to end
	mov		esi,eax

	mov		eax,1		;Ask for steppings, model, family, etc.
	THECPUID
	mov		[CPUBitFlags],edx	; Save feature flag data
	mov		edx,eax
	shr		eax,4
	and		edx,0xF
	mov		[CPURevision],dl	; Save revision
	mov		edx,eax
	shr		eax,4
	and		edx,0xF
	mov		[CPUModel],dl
	and		eax,0xF
	mov		[CPUType],al

	cmp		eax,6		;Pentium II?
	jnz		JQuit7
	cmp		edx,5		;Celeron type?
	jnz		JQuit7
	cmp		esi,2		;Can I test the cache?
	jae		TestCeleron
JQuit7:
	jmp		Quit7x

;
; Special code to test for the Cyrix 486
;

TestCyrix:
	xor		eax,eax
	sahf
	mov		eax,5
	mov		ebx,2
	div		bl
	lahf
	cmp		ah,2
	jne		JQuit7
	mov		DWORD PTR [VendorIDText],'iryC'		;Copy the 12 bytes if the vendor string
	mov		DWORD PTR [VendorIDText+4],'snIx'
	mov		DWORD PTR [VendorIDText+8],'daet'
Quit7x:
	jmp		Quit7

;
; If there is no cache, then I have a celeron
;
TestCeleron:
	mov		eax,2		;CPU ID for cache
	THECPUID
	cmp		ah,0x40
	jz		Celeron
	shr		eax,16
	cmp		al,0x40
	jz		Celeron
	cmp		ah,0x40
	jz		Celeron

	cmp		bl,0x40
	jz		Celeron
	cmp		bh,0x40
	jz		Celeron
	shr		ebx,16
	cmp		bl,0x40
	jz		Celeron
	cmp		bh,0x40
	jz		Celeron

	cmp		cl,0x40
	jz		Celeron
	cmp		ch,0x40
	jz		Celeron
	shr		ecx,16
	cmp		cl,0x40
	jz		Celeron
	cmp		ch,0x40
	jz		Celeron

	cmp		dl,0x40
	jz		Celeron
	cmp		dh,0x40
	jz		Celeron
	shr		edx,16
	cmp		dl,0x40
	jz		Celeron
	cmp		dh,0x40
	jnz		Quit7
Celeron:
	mov		[CeleronFlag],1		;I have a celeron processor
Quit7:

; Perform the floating point test

	push	dword ptr 0x5a5a5a5a
	fninit				; Reset FP status word
	fnstsw	[esp]		; Save FP status word
	mov		ax,[esp]	; Check FP status word
	test	al,al		; See if correct status with written
	mov		[CPUFloatType],0	; Assume no FPU present
	jnz		Quit6		; No FPU -> quit

; Status word check out right, now check control word

	fnstcw	[esp]		; Save FP control word
	mov		ax,[esp]	; Load FP control word
	and		ax,0x103f	; See if selected parts looks OK
	cmp		ax,0x3f		; Check that 1's & 0's correctly read
	jne		Quit6		; No FPU -> quit
	mov		[CPUFloatType],1	; We know we got one, but which type?

; 80287/80387 check for the Intel386 CPU

	cmp		[CPUType],3	; Got a 386?
	jne		Quit6
	fld1			; Must use default control from FNINIT
	fldz			; Form infinity
	fdiv			; 8087 and Intel287 NDP say +inf = -inf
	fld		st		; Form negative infinity
	fchs			; Intel387 NDP says +inf <> -inf
	fcompp			; See if they are the same and remove them
	fstsw	[esp]	; Look at status from FCOMPP
	mov		ax,[esp]	; Load status
	mov		[CPUFloatType],2	; Store Intel287 NDP for fpu type
	sahf			; See if infinities matched
	jz		Quit6	; Jump if 8087 or Intel287 is present
	mov		[CPUFloatType],3	; Store Intel387 NDP for fpu type

Quit6:
	pop		eax		;Fix the stack

	}
}

static char SXDX[2][3]={"SX","DX"};

void CPUFeaturesGet(CPUFeatures_t *CPUPtr)
{
	char *Name;

	GetCPUIDFlag();		/* Perform the actual tests */

	CPUPtr->Vendor = VENDOR_UNKNOWN;
	CPUPtr->VerboseName[0] = 0;
	CPUPtr->VendorID[0] = 0;
	CPUPtr->Revision = CPURevision;		/* Get the defaults */
	CPUPtr->Model = CPUModel;
	CPUPtr->Type = CPUType;
	CPUPtr->Features = CPUBitFlags;

	/* Check the FPU family */

	if (CPUPtr->Features & 1) {		/* CPU ID says I have floating point? */
		if (CPUPtr->Type>=5) {
			CPUPtr->FPUFamily = FPU_PENTIUM;
		} else {
			CPUPtr->FPUFamily = FPU_387;
		}
	} else if (CPUFloatType==2) {
		CPUPtr->FPUFamily = FPU_287;
	} else if (CPUFloatType==3) {
		CPUPtr->FPUFamily = FPU_387;
	} else {
		CPUPtr->FPUFamily = FPU_NONE;
	}

	/* Check the MMX family */

	if (CPUPtr->Features & 0x800000) {
		CPUPtr->MMXFamily = MMX_PENTIUM;		/* It's MMX! */
	} else {
		CPUPtr->MMXFamily = MMX_NONE;
	}

	/* Get the manufacturer's name */

	Name = "Intel";
	if (VendorIDText[0]) {
		FastMemCpy(CPUPtr->VendorID,VendorIDText,12);
		CPUPtr->VendorID[12] = 0;
		if (!strcmp(CPUPtr->VendorID,"GenuineIntel")) {
			CPUPtr->Vendor = VENDOR_INTEL;
			Name = "Intel";
		} else if (!strcmp(CPUPtr->VendorID,"AuthenticAMD") ||
			!strcmp(CPUPtr->VendorID,"AMD ISBETTER")) {
			CPUPtr->Vendor = VENDOR_AMD;
			Name = "AMD";
		} else if (!strcmp(CPUPtr->VendorID,"CyrixInstead")) {
			CPUPtr->Vendor = VENDOR_CYRIX;
			Name = "Cyrix";
		} else if (!strcmp(CPUPtr->VendorID,"UMC UMC UMC ")) {
			CPUPtr->Vendor = VENDOR_UMC;
			Name = "United Microelectronics Corporation";
		} else if (!strcmp(CPUPtr->VendorID,"NexGenDriven")) {
			CPUPtr->Vendor = VENDOR_NEXGEN;
			Name = "NexGen";
		}
	}
	strcpy(CPUPtr->VerboseName,Name);

	/* Determine the name of the CPU from the info */

	Name = CPUPtr->VerboseName+strlen(CPUPtr->VerboseName);
	switch(CPUPtr->Type) {
	case 3:			/* 386 code */
		CPUPtr->CPUFamily = CPU_386;
		strcpy(Name," 386");
		if (CPUPtr->FPUFamily==FPU_287) {
			strcat(Name," with 287 unit");
		} else if (CPUPtr->FPUFamily==FPU_387)	{
			strcat(Name," with 387 unit");
		}
		break;

	case 4:			/* 486 code */
		CPUPtr->CPUFamily = CPU_486;
		if (CPUPtr->Vendor==VENDOR_CYRIX) {
			strcpy(Name," 5x86");
			CPUPtr->CPUFamily = CPU_586;	/* Great integer performance */
			CPUPtr->FPUFamily = FPU_387;	/* Floating point is not so good */
		} else {
			strcpy(Name," 486");
			strcat(Name,SXDX[CPUFloatType!=0]);
		}
		break;

	case 5:			/* Pentium */
		CPUPtr->CPUFamily = CPU_586;
		if (CPUPtr->Vendor==VENDOR_AMD) {	/* AMD flavors */
			char *TempStr;
			if (CPUPtr->Model<6) {
				TempStr = " K5 (Model ";
			} else if (CPUModel<8) {
				TempStr = " K6 (Model ";
			} else if (CPUModel<9) {
				TempStr = " K6 3D (Model ";
			} else {
				TempStr = " K6+ 3D (Model ";
			}
			strcpy(Name,TempStr);
			Name = Name+strlen(Name);
			Name = LongWordToAscii(CPUPtr->Model,Name);
			Name[0] = ')';
			Name[1] = 0;
		} else if (CPUPtr->Vendor==VENDOR_CYRIX) {
			strcpy(Name," 6x86");
		} else if (CPUPtr->Vendor==VENDOR_NEXGEN) {
			strcpy(Name," Nx586");
		} else {
			strcpy(Name," Pentium");
		}
		break;

	case 6:			/* Pentium II */
		CPUPtr->CPUFamily = CPU_686;
		if (CPUPtr->Vendor==VENDOR_INTEL && CeleronFlag) {
			strcpy(Name," Celeron");
		} else if ((CPUPtr->Vendor==VENDOR_INTEL) && CPUPtr->MMXFamily!=MMX_NONE) {
			strcpy(Name," Pentium II");
		} else if ((CPUPtr->Vendor==VENDOR_CYRIX) && CPUPtr->MMXFamily!=MMX_NONE) {
			strcpy(Name," M2");
		} else {
			strcpy(Name," Pentium Pro");
		}
		break;
	default:
		strcpy(Name," type ");
		LongWordToAscii(CPUPtr->Type,Name+6);
		CPUPtr->CPUFamily = CPU_UNKNOWN;
	}
	if (CPUPtr->MMXFamily!=MMX_NONE) {
		strcat(Name," (MMX)");
	}
	if (CPUPtr->FPUFamily==FPU_NONE) {
		strcat(Name," without floating point");
	}
}

#endif
