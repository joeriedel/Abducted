/*******************************

	Input Manager

*******************************/

#ifndef __ININPUT_H__
#define __ININPUT_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct RezHeader_t;

#define PadLeft 0x1UL
#define PadRight 0x2UL
#define PadUp 0x4UL
#define PadDown 0x8UL
#define PadHatLeft 0x10UL
#define PadHatRight 0x20UL
#define PadHatUp 0x40UL
#define PadHatDown 0x80UL
#define PadThrottleUp 0x100UL
#define PadThrottleDown 0x200UL
#define PadTwistLeft 0x400UL
#define PadTwistRight 0x800UL
#define PadButton1 0x1000UL
#define PadButton2 0x2000UL
#define PadButton3 0x4000UL
#define PadButton4 0x8000UL
#define PadButton5 0x10000UL
#define PadButton6 0x20000UL
#define PadButton7 0x40000UL
#define PadButton8 0x80000UL
#define PadButton9 0x100000UL
#define PadButton10 0x200000UL
#define PadButton11 0x400000UL
#define PadButton12 0x800000UL
#define PadButton13 0x1000000UL
#define PadButton14 0x2000000UL
#define PadButton15 0x4000000UL
#define PadButton16 0x8000000UL
#define PadButton17 0x10000000UL
#define PadButton18 0x20000000UL
#define PadButton19 0x40000000UL
#define PadButton20 0x80000000UL

/* Private data */

#define KEYBUFFSIZE 128

typedef struct PollProcs_t {
	struct PollProcs_t **Next;		/* Handle to the next PollProcs_t in chain */
	void (BURGERCALL *Proc)(void *Input);		/* Function to call */
	void *Data;						/* User supplied point to call the proc with */
} PollProcs_t;

typedef struct ScanEntry_t {
	char *NameStr;		/* Pointer to key name */
	Word ScanCode;		/* Actual scan code */
} ScanEntry_t;

#if defined(__MAC__)
typedef struct MacRatEntry_t {
	struct OpaqueISpElementListReference* MouseRef;	/* Mice references */
	struct OpaqueISpElementReference*MouseElements[11];	/* 3 axis, 8 buttons */
} MacRatEntry_t;

typedef struct MacKeyEntry_t {
	struct OpaqueISpElementListReference* KeyboardRef;	/* Keyboard references */
	Word ScanCodeCount;									/* Number of keys found */
	struct OpaqueISpElementReference*KeyElements[128];	/* 128 keys */
	Word ScanCodes[128];								/* Valid scan codes */
} MacKeyEntry_t;
#endif

#if defined(__MAC__) || defined(__MACOSX__)
typedef struct MacInput_t {
	Word8 InputSprocketInited;			/* TRUE if InputSprocket is initialized */
	Word8 InputSprocketActive;			/* TRUE if inputsprocket is enabled */
	Word8 CursorDevicePresent;			/* Use CursorDevice calls for mouse control */
	Word8 Flags;							/* Flags for services enabled */
#if defined(__MAC__)
	Word32 InputSprocketTick;			/* Tick Mark to ISpTickle() */
	Word32 InputSprocketMutex;		/* Lock for accessing InputSprocket */
	
	/* Joystick */
	Word JoystickLastRead;				/* Last joystick device read */
	struct JoyDesc_t *JoystickDescriptionsArray;		/* Array of actual data for devices */
	struct OpaqueISpDeviceReference** JoystickDeviceArray;	/* Array of device references */

	/* Keyboard */
	Word KeyboardCount;					/* Number of InputSprocket Keyboards */
	MacKeyEntry_t KeyLists[2];			/* I only support 2 keyboards */
#endif
	struct OpaqueEventHandlerRef *KeyEventRef;	/* Carbon key event */
	void *KeyCarbonProc;				/* Carbon event pointer */

	/* Mouse */
#if defined(__MAC__)
	Word MiceCount;						/* Number of InputSprocket Mice */
	MacRatEntry_t MiceLists[4];			/* Mice data */
#endif
	struct OpaqueEventHandlerRef *MouseEventRef;	/* Carbon mouse event */
	void *MouseCarbonProc;				/* Carbon event pointer */
	Word MouseMaxX,MouseMaxY;			/* Mouse maximum bounds */
	int LastMouseX,LastMouseY;			/* Last mouse position */
	int LastMouseDeltaX,LastMouseDeltaY,LastMouseDeltaZ;	/* Accumulated deltas */
	Word LastMouseButton;				/* Last pressed mouse button */
#if defined(__MAC__)
	struct TimerTask_t *MouseTimerProc;	/* Timer used for reading the mouse */
	struct TimerTask_t *KeyboardTimerProc;	/* Timer used for reading the keyboard */
#endif
	int MouseXBase,MouseYBase;			/* Offset from global to local coordinates */
	Word8 CenterMouseFlag;				/* If TRUE, the re-center the mouse for delta motion */
	Word8 Dormant;						/* TRUE if InputSprocket is dormant because the game is in the background */
	Word8 Padding[2];
} MacInput_t;

#define MACINITINPUTJOYSTICK 1
#define MACINITINPUTMOUSE 2
#define MACINITINPUTKEY 4

extern MacInput_t MacInputLocals;		/* Local state data of MacInput */
extern Word BURGERCALL MacInputInit(MacInput_t *Input,Word Flags);
extern void BURGERCALL MacInputDestroy(MacInput_t *Input,Word Flags);
extern Word BURGERCALL MouseReadInputSprocket(void *Input);
extern Word BURGERCALL KeyboardReadInputSprocket(void *LocalPtr);
#endif

/* Public data */

typedef struct JoyAutoRepeat_t {	/* Used by JoyAutoRepeater */
	Word32 JoyBits;	/* Bit field to test for */
	Word InitialTick;	/* Delay for initial joydown */
	Word RepeatTick;	/* Delay for repeater */
	Word32 TimeMark;	/* Internal time mark */
	Word HeldDown;		/* Zero this to init the struct */
} JoyAutoRepeat_t;

typedef void (BURGERCALL *KeyboardCallBack)(void *);
typedef Word (BURGERCALL *KeyboardGetchCallBackPtr)(Word Key);
typedef struct ForceFeedback_t ForceFeedback_t;
typedef struct ForceFeedbackData_t ForceFeedbackData_t;
typedef struct ForceFeedbackEffect_t ForceFeedbackEffect_t;

extern Word BURGERCALL InputSetState(Word ActiveFlag);
extern Word BURGERCALL InputGetState(void);

extern volatile Word8 KeyArray[128];		/* Scan codes of keys pressed */
extern void BURGERCALL KeyboardInit(void);
extern void BURGERCALL KeyboardDestroy(void);
extern KeyboardGetchCallBackPtr KeyboardGetchCallBack;	/* Key stealers */
extern Word BURGERCALL KeyboardGetch(void);
extern Word BURGERCALL KeyboardKbhit(void);
extern void BURGERCALL KeyboardAddRoutine(KeyboardCallBack Proc,void *Data);
extern void BURGERCALL KeyboardRemoveRoutine(KeyboardCallBack Proc,void *Data);
extern void BURGERCALL KeyboardFlush(void);
extern Word BURGERCALL KeyboardGet(void);
extern Word BURGERCALL KeyboardGet2(void);
extern void BURGERCALL KeyboardCallPollingProcs(void);
extern Word BURGERCALL KeyboardGetKeyLC(void);
extern Word BURGERCALL KeyboardGetKeyUC(void);
extern Word BURGERCALL KeyboardAnyPressed(void);
extern Word BURGERCALL KeyboardIsPressed(Word ScanCode);
extern Word BURGERCALL KeyboardHasBeenPressed(Word ScanCode);
extern void BURGERCALL KeyboardClearKey(Word ScanCode);
extern Word BURGERCALL KeyboardHasBeenPressedClear(Word ScanCode);
extern Word BURGERCALL KeyboardStringToScanCode(const char *StringPtr);
extern void BURGERCALL KeyboardScanCodeToString(char *StringPtr,Word StringSize,Word ScanCode);
extern Word BURGERCALL KeyboardWait(void);

extern Word MousePresent;
extern Word MouseClicked;
extern Word BURGERCALL MouseInit(void);
extern void BURGERCALL MouseDestroy(void);
extern Word BURGERCALL MouseReadButtons(void);
extern void BURGERCALL MouseReadAbs(Word *x,Word *y);
extern void BURGERCALL MouseReadDelta(int *x,int *y);
extern int BURGERCALL MouseReadWheel(void);
extern void BURGERCALL MouseSetRange(Word x,Word y);
extern void BURGERCALL MouseSetPosition(Word x,Word y);

#define AXISCOUNT 6
#define MAXJOYNUM 4
enum {AXISMIN,AXISMAX,AXISCENTER,AXISLESS,AXISMORE,AXISENTRIES};
extern Word JoystickPercent[MAXJOYNUM][AXISCOUNT];		/* Cache for percentages */
extern Word JoystickPresent;
extern Word32 JoystickLastButtons[MAXJOYNUM];
extern Word32 JoystickLastButtonsDown[MAXJOYNUM];
extern Word JoystickBoundaries[MAXJOYNUM][AXISENTRIES*AXISCOUNT];
extern Word BURGERCALL JoystickInit(void);
extern void BURGERCALL JoystickDestroy(void);
extern Word32 BURGERCALL JoystickReadButtons(Word Which);
extern void BURGERCALL JoystickReadNow(Word Which);
extern Word BURGERCALL JoystickReadAbs(Word Axis,Word Which);
extern int BURGERCALL JoystickReadDelta(Word Axis,Word Which);
extern Word BURGERCALL JoystickGetAxisCount(Word Which);
extern void BURGERCALL JoystickSetCenter(Word Axis,Word Which);
extern void BURGERCALL JoystickSetMin(Word Axis,Word Which);
extern void BURGERCALL JoystickSetMax(Word Axis,Word Which);
extern void BURGERCALL JoystickSetDigital(Word Axis,Word Percent,Word Which);
extern void BURGERCALL JoystickBoundariesChanged(void);
extern Word BURGERCALL JoyAutoRepeater(JoyAutoRepeat_t *Input,Word32 JoyBits);

extern ForceFeedback_t * BURGERCALL ForceFeedbackNew(void);
extern void BURGERCALL ForceFeedbackDelete(ForceFeedback_t *RefPtr);
extern void BURGERCALL ForceFeedbackReacquire(ForceFeedback_t *RefPtr);
extern ForceFeedbackData_t * BURGERCALL ForceFeedbackDataNew(ForceFeedback_t *RefPtr,const char *FilenamePtr);
extern ForceFeedbackData_t * BURGERCALL ForceFeedbackDataNewRez(ForceFeedback_t *RefPtr,struct RezHeader_t *RezRef,Word RezNum);
extern void BURGERCALL ForceFeedbackDataDelete(ForceFeedbackData_t *FilePtr);
extern ForceFeedbackEffect_t * BURGERCALL ForceFeedbackEffectNew(ForceFeedbackData_t *FilePtr,const char *EffectNamePtr);
extern void BURGERCALL ForceFeedbackEffectDelete(ForceFeedbackEffect_t *effect);
extern Word BURGERCALL ForceFeedbackEffectPlay(ForceFeedbackEffect_t *Input);
extern void BURGERCALL ForceFeedbackEffectStop(ForceFeedbackEffect_t *Input);
extern Word BURGERCALL ForceFeedbackEffectIsPlaying(ForceFeedbackEffect_t *Input);
extern void BURGERCALL ForceFeedbackEffectSetGain(ForceFeedbackEffect_t *Input,long NewGain);
extern void BURGERCALL ForceFeedbackEffectSetDuration(ForceFeedbackEffect_t *Input,Word32 NewDuration);

#if defined(__MAC__)
extern Word KeyModifiers;			/* If a key is read, pass back the keyboard modifiers */
extern Word ScanCode;				/* Scan code of key last read */
extern Word FixMacKey(struct EventRecord *Event);
extern Bool MacSystemTaskFlag;
extern Word (BURGERCALL *MacEventIntercept)(struct EventRecord *MyEventPtr);
extern Word DoMacEvent(Word Mask,struct EventRecord *Event);
extern Word BURGERCALL MacInputLockInputSprocket(void);
extern void BURGERCALL MacInputUnlockInputSprocket(void);
#endif

#if defined(__BEOS__)
extern int BURGERCALL BeOSSpawnMain(int (*MainCode)(int,char **),int argc, char **argv);
#endif

#ifdef __cplusplus
}
#endif

#endif
