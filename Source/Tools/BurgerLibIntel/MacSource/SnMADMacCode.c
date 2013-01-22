/**********************************

	Mac specific MOD file code

**********************************/

#include "SnSound.h"
#if defined(__MAC__)

#include "MmMemory.h"
#include "SnMadMusic.h"
#include "McMac.h"
#include "FmFile.h"
#include "ClStdLib.h"
#include "StString.h"
#include <Gestalt.h>

/**********************************

	Sound manager double back proc

**********************************/

static pascal void MyDoubleBackProc(SndChannelPtr chan,SndCommand *doubleBuffer)
{
	char *myPtr;			
	MADDriverRec *intDriver;
	SndCommand mySndCmd;
	
#if _DEBUG && defined(__POWERPC__) && !_SHAREDLIB
	SLEnterInterrupt();
#endif

	intDriver = (MADDriverRec*) chan->userInfo;

	/********************/
	/**   Read Notes   **/
	/********************/
	
	myPtr = intDriver->IntDataPtr;		/* Save the pointer */
	intDriver->IntDataPtr = (char*) intDriver->SndBuffer;
	
	NoteAnalyse(intDriver);				/* Decompress */
	intDriver->IntDataPtr = myPtr;		/* Restore the pointer */

	intDriver->SndHeader.samplePtr = (char *)intDriver->SndBuffer;
	intDriver->SndHeader.numChannels = 2;
	intDriver->SndHeader.sampleRate = intDriver->DriverSettings.outPutRate;
	intDriver->SndHeader.loopStart = 0;
	intDriver->SndHeader.loopEnd = 0;
	intDriver->SndHeader.encode = extSH;
	intDriver->SndHeader.baseFrequency = 0;
	intDriver->SndHeader.numFrames = intDriver->ASCBUFFER;
	intDriver->SndHeader.markerChunk = NULL;
	intDriver->SndHeader.instrumentChunks = NULL;
	intDriver->SndHeader.AESRecording = NULL;
	intDriver->SndHeader.sampleSize = intDriver->DriverSettings.outPutBits;

	mySndCmd.cmd = bufferCmd;
	mySndCmd.param1 = 0;
	mySndCmd.param2 = (int)&intDriver->SndHeader;
	SndDoCommand(chan,&mySndCmd,TRUE);
	
	// and another callback
	mySndCmd.cmd = callBackCmd;
	mySndCmd.param1 = 0;
	mySndCmd.param2 = 0;
	SndDoCommand(chan,&mySndCmd,TRUE);

#if _DEBUG && defined(__POWERPC__)  && !_SHAREDLIB
	SLLeaveInterrupt();
#endif
}

/**********************************

	Start the sound system
	
**********************************/

static int DBSndPlay ( MADDriverRec *inMADDriver, SndChannelPtr chan)
{
	Word8 *doubleBuffer;

	doubleBuffer = (Word8 *)AllocAPointerClear(inMADDriver->BufSize + 20L);
	if (doubleBuffer) {
		inMADDriver->SndBuffer = doubleBuffer;
		MyDoubleBackProc(chan,0);
		return 0;
	}
	return MADNeedMemory;
}

/**********************************

	Start the sound system
	
**********************************/

int BURGERCALL MADSndOpen( MADDriverRec *inMADDriver)
{
	int err;
	SndChannelPtr mySndChan;	// pointer to a sound channel
	SndCallBackUPP CallBack;
	
	CallBack = NewSndCallBackUPP(MyDoubleBackProc);
	err=-1;
	if (CallBack) {
		mySndChan = 0L;
		err = SndNewChannel(&mySndChan, sampledSynth, 0, CallBack);
		if (!err) {
			if (mySndChan) {			/* Failsafe */
				inMADDriver->CallBackUPP = CallBack;
				inMADDriver->MusicChannelPP = mySndChan;
				mySndChan->userInfo = (long)inMADDriver;
				return DBSndPlay(inMADDriver, inMADDriver->MusicChannelPP);
			}
			err = MADSoundManagerErr;
		}
		DisposeSndCallBackUPP(CallBack);	/* Kill the callback */
	}
	return err;
}

/**********************************

	Shut down audio hardware
	
**********************************/

void BURGERCALL MADSndClose(MADDriverRec *inMADDriver)
{
	if (inMADDriver->MusicChannelPP) {
		SndDisposeChannel(inMADDriver->MusicChannelPP,TRUE);
		inMADDriver->MusicChannelPP = 0;
	}
	if (inMADDriver->CallBackUPP) {
		DisposeSndCallBackUPP(inMADDriver->CallBackUPP);
		inMADDriver->CallBackUPP = 0;
	}
	DeallocAPointer(inMADDriver->SndBuffer);
	inMADDriver->SndBuffer = 0;
}

/**********************************

	Start audio
	
**********************************/

void BURGERCALL MADStartDriver( MADDriverRec *MDriver)
{
	MDriver->MADPlay = TRUE;
	MDriver->Reading = FALSE;
	MADCleanDriver(MDriver);
	MADCheckSpeed(MDriver->curMusic, MDriver);
	MyDoubleBackProc(MDriver->MusicChannelPP,0);
}

/**********************************

	Stop audio
	
**********************************/

void BURGERCALL MADStopDriver( MADDriverRec *MDriver)
{
	SndCommand cmd;

	MDriver->MADPlay = FALSE;
	cmd.cmd = quietCmd;
	cmd.param1 = 0;
	cmd.param2 = 0;
	SndDoImmediate(	MDriver->MusicChannelPP,&cmd);
	cmd.cmd = flushCmd;
	cmd.param1 = 0;
	cmd.param2 = 0;
	SndDoImmediate(	MDriver->MusicChannelPP,&cmd);
	MADCleanDriver( MDriver);
}


/**********************************

	Get the driver defaults based on the system you are in
	
**********************************/

void BURGERCALL MADGetBestDriver( MADDriverSettings *Init)
{
	Init->numChn = 4;
	Init->repeatMusic = TRUE;
	Init->surround = FALSE;
	Init->Reverb = FALSE;
	Init->TickRemover = FALSE;
	Init->MicroDelaySize = 25;
	Init->ReverbSize = 100;
	Init->ReverbStrength = 20;
	Init->outPutBits = 8;
	Init->outPutRate = rate22khz;
	Init->driverMode = SoundManagerDriver;

	{
		long gestaltAnswer;
		Word Version;

	/****************************/
	/** SOUND MANAGER >= 3.0 ? **/
	/****************************/

		Version = MacOSGetSoundManagerVersion();
		if (Version>=0x300) {
			/* Rate and size */
			
			#if !TARGET_RT_MAC_CFM
			if (Version>=0x310) {
				if (GetSoundOutputInfo(0L,siSampleRate, &Init->outPutRate)) {
					goto oldWay;
				}
				if (GetSoundOutputInfo(0L,siSampleSize, (void*) &Init->outPutBits)) {
					goto oldWay;
				}
				if (Init->outPutBits!=16 && Init->outPutBits!=8) {
					Init->outPutBits = 16;		/* I don't support 32 bit hardware */
				}
			} else
			#endif
			{
oldWay:;
				Gestalt(gestaltSoundAttr,&gestaltAnswer);
				if (gestaltAnswer & (1<<gestalt16BitSoundIO)) {
					Init->outPutBits = 16;
					Init->outPutRate = rate44khz;
				}
			}
		}
	}
}


#endif