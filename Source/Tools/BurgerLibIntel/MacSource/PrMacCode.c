/**********************************

	Get the current CPU type and feature list.
	This is DIFFERENT for each processor family.

	Intel series will get the manufacturer, family and whether
	it supports MMX and or FPU operations.

	Power PC processors will return the family (601/603/604/750)
	and whether or not it supports ALTIVEC instructions.

**********************************/

#include "PrProfile.h"

#if defined(__MAC__)
#include <string.h>
#include "PfPrefs.h"
#include <Gestalt.h>

/**********************************

	Does the hardware exist to support the
	Profile Manager?

**********************************/

Word BURGERCALL ProfileIsAvailable(void)
{
	return TRUE;
}

/**********************************

	Macintosh code
	Note: I have code to determine the 680x0 and PowerPC code
	at the same time. Even though I know that 680x0 and PPC code CANNOT
	execute each other's code. However you can run through emulation.

**********************************/

void CPUFeaturesGet(CPUFeatures_t *CPUPtr)
{
	long Result;
	char *Name;
	char *TempPtr;
	
#if defined(__POWERPC__)				/* Here are my assumptions */
	CPUPtr->CPUFamily = CPU_601;		/* Minimum specs */
	CPUPtr->MMXFamily = MMX_NONE;
	CPUPtr->FPUFamily = FPU_601;
	CPUPtr->Vendor = VENDOR_IBM;
	strcpy(CPUPtr->VerboseName,"IBM Generic PowerPC");
#else
	CPUPtr->CPUFamily = CPU_68000;		/* Minimum specs */
	CPUPtr->MMXFamily = MMX_NONE;
	CPUPtr->FPUFamily = FPU_NONE;
	CPUPtr->Vendor = VENDOR_MOTOROLA;
	strcpy(CPUPtr->VerboseName,"Motorola Generic 680x0");
#endif

	/* I handle both since I could be in emulation */
	
	if (!Gestalt(gestaltNativeCPUtype,&Result)) {
		switch (Result) {
		case gestaltCPU68000:
		case gestaltCPU68010:
			CPUPtr->CPUFamily = CPU_68000;
			break;
		case gestaltCPU68020:
			CPUPtr->CPUFamily = CPU_68020;
			break;
		case gestaltCPU68030:
			CPUPtr->CPUFamily = CPU_68030;
			break;
		case gestaltCPU68040:
		case 5:			/* For the bug in system 7.5 */
			CPUPtr->CPUFamily = CPU_68040;
			CPUPtr->FPUFamily = FPU_68881;
			break;
		case gestaltCPU601:
			CPUPtr->CPUFamily = CPU_601;
			CPUPtr->FPUFamily = FPU_601;
			break;
		case gestaltCPU603:
		case gestaltCPU603e:
		case gestaltCPU603ev:
			CPUPtr->CPUFamily = CPU_603;
			CPUPtr->FPUFamily = FPU_603;
			break;
		case gestaltCPU604:
		case gestaltCPU604e:
		case gestaltCPU604ev:
			CPUPtr->CPUFamily = CPU_604;
			CPUPtr->FPUFamily = FPU_603;
			break;
		default:
//		case gestaltCPU750:
			CPUPtr->CPUFamily = CPU_750;
			CPUPtr->FPUFamily = FPU_603;
		}
		
		/* Now for the vendor, I'll just assume motorola and IBM */
		if (CPUPtr->CPUFamily>=CPU_68000 && CPUPtr->CPUFamily<=CPU_68040) {
			CPUPtr->Vendor = VENDOR_MOTOROLA;
			strcpy(CPUPtr->VerboseName,"Motorola");
			Name = CPUPtr->VerboseName+8;
		} else {
			CPUPtr->Vendor = VENDOR_IBM;
			strcpy(CPUPtr->VerboseName,"IBM");
			Name = CPUPtr->VerboseName+3;
		}
		
		switch (Result) {
		case gestaltCPU68000:
			TempPtr = " 68000";
			break;
		case gestaltCPU68010:
			TempPtr = " 68010";
			break;
		case gestaltCPU68020:
			TempPtr = " 68020";
			break;
		case gestaltCPU68030:
			TempPtr = " 68030";
			break;
		case gestaltCPU68040:
			TempPtr = " 68040";
			break;
		case gestaltCPU601:
			TempPtr = " PowerPC 601";
			break;
		case gestaltCPU603:
			TempPtr = " PowerPC 603";
			break;
		case gestaltCPU604:
			TempPtr = " PowerPC 604";
			break;
		case gestaltCPU603e:
			TempPtr = " PowerPC 603e";
			break;
		case gestaltCPU603ev:
			TempPtr = " PowerPC 603ev";
			break;
		case gestaltCPU750:
			TempPtr = " PowerPC G3";
			break;
		case gestaltCPU604e:
			TempPtr = " PowerPC 604e";
			break;
		case gestaltCPU604ev:
			TempPtr = " PowerPC 604ev";
			break;
		case gestaltCPUG4:
			TempPtr = " PowerPC G4";
			break;
		default:
			TempPtr = " Better than G3";
			break;
		}
		strcpy(Name,TempPtr);
		if (!Gestalt(gestaltProcClkSpeed,&Result)) {
			Name = Name+strlen(Name);
			Name[0] = ' ';
			Name = LongWordToAscii(Result/1000000,Name+1);
			strcpy(Name," Mhz");
		}
	}	
}

#endif

