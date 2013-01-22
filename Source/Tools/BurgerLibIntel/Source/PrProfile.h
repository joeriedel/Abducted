/**********************************

	Profile manager

**********************************/

#ifndef __PRPROFILE_H__
#define __PRPROFILE_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {FPU_NONE,FPU_287,FPU_387,FPU_PENTIUM,
	FPU_601,FPU_603,FPU_68881} FPU_e;
typedef enum {CPU_UNKNOWN,CPU_386,CPU_486,CPU_586,CPU_686,
	CPU_601,CPU_603,CPU_604,CPU_750,
	CPU_68000,CPU_68020,CPU_68030,CPU_68040} CPU_e;
typedef enum {MMX_NONE,MMX_PENTIUM,MMX_K6,MMX_ALTIVEC} MMX_e;
typedef enum {VENDOR_UNKNOWN,VENDOR_INTEL,VENDOR_AMD,VENDOR_UMC,VENDOR_CYRIX,VENDOR_NEXGEN,
	VENDOR_IBM,VENDOR_HITACHI,VENDOR_MOTOROLA,VENDOR_ARM,VENDOR_MIPS} CPUVendor_e;

typedef struct CPUFeatures_t {
	CPU_e CPUFamily;	/* Class of CPU */
	FPU_e FPUFamily;	/* Class of FPU */
	MMX_e MMXFamily;	/* Extended instructions */
	CPUVendor_e Vendor;	/* Who made the chip? */
	char VerboseName[64];	/* Cpu name and feature string */
#if defined(__INTEL__)
	Word32 Features;	/* CPU ID features list */
	Word Revision;		/* CPU stepping flag */
	Word Model;			/* CPU model */
	Word Type;			/* CPU type */
	char VendorID[13];	/* Cpu vendor string */
#endif
} CPUFeatures_t;

typedef struct Profile_t {
	const char *Name;	/* Name of fragment being profiled */
	struct Profile_t *Next;	/* Next in linked list */
	Word32 Mark;		/* Current time mark */
	Word32 TimeIn;	/* Time inside the proc */
	Word32 TimeOut;	/* Time outside the proc */
	Word HitCount;		/* Number of times entered */
	Word RecurseCount;	/* Recursion flag */
	Word Initialized;	/* TRUE if initialized */
} Profile_t;

extern Profile_t *ProfileRoot;	/* Root pointer for linked list of Profile_t's */

extern void BURGERCALL CPUFeaturesGet(CPUFeatures_t *Input);
extern Profile_t * BURGERCALL ProfileNew(const char *Name);
extern void BURGERCALL ProfileInit(Profile_t *Input,const char *Name);
extern void BURGERCALL ProfileDelete(Profile_t *Input);
extern void BURGERCALL ProfileDestroy(Profile_t *Input);
extern void BURGERCALL ProfileEntry(Profile_t *Input);
extern void BURGERCALL ProfileExit(Profile_t *Input);
extern double BURGERCALL ProfileGetSecondsIn(const Profile_t *Input);
extern double BURGERCALL ProfileGetSecondsOut(const Profile_t *Input);
extern double BURGERCALL ProfileGetMicrosecondsIn(const Profile_t *Input);
extern double BURGERCALL ProfileGetMicrosecondsOut(const Profile_t *Input);
extern void BURGERCALL ProfileReset(Profile_t *Input);
extern void BURGERCALL ProfileResetAll(void);
extern Word BURGERCALL ProfileIsAvailable(void);

#ifdef __cplusplus
}
#endif


#endif

