/**********************************

	MS-DOS specific MOD file code

**********************************/

#include "SnSound.h"
#if defined(__DOS4G__)

#include "MmMemory.h"
#include "SnMadMusic.h"
#include "McMac.h"
#include "FmFile.h"
#include "ClStdLib.h"
#include "StString.h"
#include "sos.h"

#define AUDIOSPEED 22050

static Word8 *AudioBuffers;
static Word AudioBufferSize;
static Word Index;

/**********************************

	Sound manager double back proc

**********************************/

static void __cdecl SampleCallback(PSOSSAMPLE sSample)
{
	Word i;
	MADDriverRec *intDriver;
	char *myPtr;

	intDriver = (MADDriverRec*) sSample->wUser[0];

	i = Index;
	i ^= 1;
	Index = i;

	sSample->wLength = intDriver->ASCBUFFER*4;		/* 16 bit stereo samples */
	sSample->pSample = (char *)AudioBuffers+(AudioBufferSize*i);
	myPtr = intDriver->IntDataPtr;
	intDriver->IntDataPtr = sSample->pSample;

	NoteAnalyse(intDriver);
	intDriver->IntDataPtr = myPtr;
}

/**********************************

	Start the sound system

**********************************/

int BURGERCALL MADSndOpen( MADDriverRec *inMADDriver)
{
	_SOS_SAMPLE *pSample;

	if (hDIGIDriver!=-1) {		/* No driver present? */
		pSample = &BurgerSamples[MAXVOICECOUNT].SampleRecord;
		FastMemSet(pSample,0,sizeof(_SOS_SAMPLE));
		AudioBufferSize = (inMADDriver->BufSize+64);
		AudioBuffers = (Word8 *)AllocAPointer(AudioBufferSize*2);
		if (AudioBuffers) {
			pSample->wRate = AUDIOSPEED;
			pSample->wPanPosition = 0x8000;
			pSample->wChannels = 2;
			pSample->wBitsPerSample = 16;
//			pSample->wFormat = 0;

			pSample->pSample = (char *)AudioBuffers; /* First buffer */
			pSample->wLength = inMADDriver->ASCBUFFER*2;
			pSample->wVolume = MK_VOLUME((MusicVolume<<7),(MusicVolume<<7));
			pSample->wPriority = 0;
			pSample->pfnSampleProcessed = SampleCallback;
			pSample->wUser[0] = (long)inMADDriver;
			BurgerSamples[MAXVOICECOUNT].DataType = SOUNDTYPESTEREO|SOUNDTYPELSHORT;
			return 0;
		}
	}
	return -1;
}

/**********************************

	Shut down audio hardware

**********************************/

void BURGERCALL MADSndClose(MADDriverRec * /* inMADDriver */ )
{
	DeallocAPointer(AudioBuffers);
	AudioBuffers = 0;
}

/**********************************

	Start audio

**********************************/

void BURGERCALL MADStartDriver( MADDriverRec *MDriver)
{
	if (hDIGIDriver!=-1) {
		MDriver->MADPlay = TRUE;
		MDriver->Reading = FALSE;

		MADCleanDriver( MDriver);
		MADCheckSpeed( MDriver->curMusic, MDriver);

		Index = 0;
		SampleCallback(&BurgerSamples[MAXVOICECOUNT].SampleRecord);
		SampleCallback(&BurgerSamples[MAXVOICECOUNT].SampleRecord);
		BurgerSamples[MAXVOICECOUNT].SampleResource = (Word)-1;		/* Save the resource number */
		BurgerSamples[MAXVOICECOUNT].SampleHandle = sosDIGIStartSample(hDIGIDriver,&BurgerSamples[MAXVOICECOUNT].SampleRecord);	/* Play the sound */
		BurgerSamples[MAXVOICECOUNT].SoundPtr = AudioBuffers;
		BurgerSamples[MAXVOICECOUNT].SampleRate = AUDIOSPEED;
	}
}

/**********************************

	Stop audio

**********************************/

void BURGERCALL MADStopDriver( MADDriverRec *MDriver)
{
	if (BurgerSamples[MAXVOICECOUNT].SoundPtr) {
		StopASound(SOUND_COOKIE|MAXVOICECOUNT);
	}
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
	Init->outPutBits = 16;
	Init->outPutRate = AUDIOSPEED<<16;
}


#endif
