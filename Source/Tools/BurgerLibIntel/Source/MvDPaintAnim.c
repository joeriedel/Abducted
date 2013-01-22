#include "MvMovie.h"
#include <BREndian.hpp>
#include "FmFile.h"
#include "TkTick.h"
#include "MmMemory.h"
#include "ClStdLib.h"

/* DPaintAnim file header, all entries are LITTLE endian! */

typedef struct PaletteCycle_t {
	Word16 Count;	/* Number of colors to adjust */
	Word16 Rate;		/* Cycle rate */
	Word16 Flags;	/* Special flags */
	Word8 Low,High;	/* Range to modify */
} PaletteCycle_t;

typedef struct DPaintAnimHeader_t {
	char ID[4];			/* "LPF " Id */
	Word16 MaxLargePages;	/* 256 */
	Word16 LargePages;	/* Number of 64K chunks in file */
	Word32 Records;	/* Number of records in the file */
	Word16 MaxRecords;	/* Maximum records per large page */
	Word16 TableOffset;	/* Seek offset to data header */
	char ContentID[4];	/* Content ID "ANIM" */
	Word16 Width;		/* Width of the movie in pixels */
	Word16 Height;		/* Height of the movie in pixels */
	Word8 Variant;		/* 0==ANIM */
	Word8 Version;		/* 0 = 18FPS, 1 = 70FPS */
	Word8 HasLastDelta;	/* 1 = Last record is last to first frame */
	Word8 LastDeltaOk;	/* 0 = Bad Last Delta frame. */
	Word8 PixelType;		/* 0 = 256 colors */
	Word8 Compression;	/* 1 = RunSkipDump */
	Word8 OtherRecsPerFrame;	/* Any non-graphic records? */
	Word8 BitmapType;	/* 1 = 320*200 */
	Word8 Foo1[32];		/* Padding */
	Word32 Frames;	/* Number of frames in movie (65536) */
	Word16 Speed;		/* FPS */
	Word16 Foo2[29];		/* Filler */
	PaletteCycle_t Cyclers[16];	/* Palette cycling data */
	Word8 Palette[1024];	/* Palette for the movie */
} DPaintAnimHeader_t;

typedef struct DPaintDictionary_t {
	Word16 BaseRecord;	/* Base frame number */
	Word16 Records;		/* Number of frames in group */
	Word16 Length;		/* Size of the data */
	Word16 Overrun;		/* Number of bytes in overrun */
	Word16 Sizes[1];		/* Size of each record */
} DPaintDictionary_t;

/**********************************

	Create a new DPaint Anim movie record

**********************************/

DPaintAnimMovie_t * BURGERCALL DPaintAnimMovieNew(const char *Filename,Word Flags,Word32 FileOffset)
{
	DPaintAnimMovie_t *MoviePtr;
	MoviePtr = (DPaintAnimMovie_t *)AllocAPointer(sizeof(DPaintAnimMovie_t));
	if (MoviePtr) {
		if (!DPaintAnimMovieInit(MoviePtr,Filename,Flags,FileOffset)) {	/* Init the movie? */
			return MoviePtr;		/* Return the initialized struct */
		}
		DeallocAPointer(MoviePtr);	/* Dispose of the movie */
	}
	return 0;		/* I have an error */
}

/**********************************

	Dispose of a movie record and delete
	its contents

**********************************/

void BURGERCALL DPaintAnimMovieDelete(DPaintAnimMovie_t *Input)
{
	if (Input) {		/* Valid pointer? */
		DPaintAnimMovieDestroy(Input);	/* Dispose of the contents */
		DeallocAPointer(Input);		/* Dispose of the memory */
	}
}

/**********************************

	Read data for a flic movie
	Return FALSE is ok, error code otherwise

**********************************/

Word BURGERCALL DPaintAnimMovieDataRead(DPaintAnimMovie_t *Input,void *Data,Word32 Length)
{
	if (Input->fp) {			/* Reading from a file? */
		if (fread(Data,1,Length,Input->fp)==Length) {	/* Read it */
			return FALSE;		/* No error */
		}
		return TRUE;
	}
	StreamHandleGetMem(&Input->MyInput,Data,Length);
	return Input->MyInput.ErrorFlag;	/* Return the error code */
}

/**********************************

	Seek data for a flic movie
	Return FALSE is ok, error code otherwise

**********************************/

static void BURGERCALL DPaintAnimMovieDataSeek(DPaintAnimMovie_t *Input,Word32 Offset)
{
	if (Input->fp) {			/* Seek to the beginning of the file */
		fseek(Input->fp,Offset+Input->FileOffset,SEEK_SET);
	} else {
		StreamHandleSetMark(&Input->MyInput,Offset);	/* Set the stream handle */
	}
}

/**********************************

	Make sure that the record in the buffer contains the
	current frame, if not, then load it in

**********************************/

static Word BURGERCALL DPaintAnimMovieDataSeekToCurrentFrame(DPaintAnimMovie_t *Input)
{
	DPaintDictionary_t *DictPtr;
	DictPtr = (DPaintDictionary_t*)Input->Buffer;
	if ((Input->CurrentFrameNumber-Burger::LoadLittle(DictPtr->BaseRecord))>=Burger::LoadLittle(DictPtr->Records)) {
		Word i;
		i = Input->DictionarySize;
		if (i) {
			Word16 *WorkPtr;
			WorkPtr = (Word16 *)Input->DictionaryBuffer;
			do {
				if ((Input->CurrentFrameNumber-Burger::LoadLittle(WorkPtr[0]))<Burger::LoadLittle(WorkPtr[1])) {
					DPaintAnimMovieDataSeek(Input,((Word32)(Input->DictionarySize-i)*65536)+0xb00);
					DPaintAnimMovieDataRead(Input,Input->Buffer,65536);
					return FALSE;
				}
				WorkPtr+=3;
			} while (--i);
		}
		return TRUE;
	}
	return FALSE;
}

/**********************************

	Initialize a DPaint Anim movie player

**********************************/

Word BURGERCALL DPaintAnimMovieInit(DPaintAnimMovie_t *Input,const char *Filename,Word Flags,Word32 FileOffset)
{
	Word Temp;
	DPaintAnimHeader_t FileHeader;

	FastMemSet(Input,0,sizeof(DPaintAnimMovie_t));		/* Init all entries */
	if (Flags&MOVIERAMBASED) {
		if (StreamHandleInitGetFile(&Input->MyInput,Filename)) {
			return TRUE;		/* Can't initialize it! */
		}
	} else {
		Input->fp = OpenAFile(Filename,"rb");
		if (!Input->fp) {		/* Looking good! */
			return TRUE;		/* Damn... */
		}
		fseek(Input->fp,FileOffset,SEEK_SET);
	}

	/* Now read the header from the movie */

	if (!DPaintAnimMovieDataRead(Input,&FileHeader,sizeof(FileHeader))) {
		if (!memcmp(&FileHeader.ID[0],"LPF ",4) &&
			!memcmp(&FileHeader.ContentID[0],"ANIM",4)) {
			if (FileHeader.BitmapType==1) {
				Word8 *WorkPtr;
				Input->FileOffset = FileOffset;
				Input->FrameImage.Width = Burger::LoadLittle(FileHeader.Width);
				Input->FrameImage.Height = Burger::LoadLittle(FileHeader.Height);
				Input->FrameImage.DataType = IMAGE8_PAL;
				Input->FrameImage.RowBytes = Input->FrameImage.Width;
				Input->MaxFrameNumber = Burger::LoadLittle(&FileHeader.Frames);
				Input->Paused = FALSE;
				Temp = Burger::LoadLittle(FileHeader.Speed);
				if (Temp) {				/* Prevent a divide by zero error */
					Temp = 1000/Temp;
				}
				Input->MovieSpeed = Temp;
				Input->DictionarySize = Burger::LoadLittle(FileHeader.LargePages);	/* Size of the dictionary in records */
				WorkPtr = (Word8 *)AllocAPointer(768);
				if (WorkPtr) {
					Word8 *SrcPtr;
					Input->FrameImage.PalettePtr = WorkPtr;	/* Get the palette */
					SrcPtr = &FileHeader.Palette[0];
					Temp = 256;
					do {
						WorkPtr[0] = SrcPtr[0];
						WorkPtr[1] = SrcPtr[1];
						WorkPtr[2] = SrcPtr[2];
						SrcPtr+=4;
						WorkPtr+=3;
					} while (--Temp);
					WorkPtr = (Word8 *)AllocAPointer(6*256);
					if (WorkPtr) {
						if (!DPaintAnimMovieDataRead(Input,WorkPtr,6*256)) {
							Input->DictionaryBuffer = WorkPtr;		/* Save the dictionary */
							return FALSE;		/* It's done! */
						}
						DeallocAPointer(WorkPtr);
					}
					DeallocAPointer(Input->FrameImage.PalettePtr);
					Input->FrameImage.PalettePtr = 0;
				}
			} else {
				NonFatal("DPaint Anim file's bitmap type is not 3\n");
			}
		} else {
			NonFatal("DPaint Anim file doesn't have the LPF header\n");
		}
	} else {
		NonFatal("Can't read the DPaint Anim file's header\n");
	}
	if (Input->fp) {
		fclose(Input->fp);
		Input->fp = 0;		/* Make sure it's destroyed */
	} else {
		StreamHandleDestroy(&Input->MyInput);
	}
	return TRUE;
}

/**********************************

	Dispose of a flic movie file

**********************************/

void BURGERCALL DPaintAnimMovieDestroy(DPaintAnimMovie_t *Input)
{
	if (Input) {
		Input->Active = FALSE;	/* This is shut down! */
		if (Input->fp) {		/* Close the input file if open */
			fclose(Input->fp);
			Input->fp = 0;
		}
		StreamHandleDestroy(&Input->MyInput);
		DeallocAPointer(Input->Buffer);
		DeallocAPointer(Input->DictionaryBuffer);
		DeallocAPointer(Input->FrameImage.PalettePtr);
		DeallocAPointer(Input->FrameImage.ImagePtr);
		Input->Buffer = 0;
		Input->FrameImage.PalettePtr = 0;
		Input->FrameImage.ImagePtr = 0;
	}
}

/**********************************

	Resets a movie to the first frame

**********************************/

void BURGERCALL DPaintAnimMovieReset(DPaintAnimMovie_t *Input)
{
	DPaintAnimMovieStop(Input);
	Input->Active = FALSE;
	Input->CurrentFrameNumber = 0;
	Input->Paused = FALSE;
	Input->Completed = FALSE;
}

/**********************************

	Starts a movie

**********************************/

Word BURGERCALL DPaintAnimMovieStart(DPaintAnimMovie_t *Input)
{
	Input->TickStart = ReadTick();
	if (!Input->FrameImage.ImagePtr) {
		Input->FrameImage.ImagePtr = (Word8 *)AllocAPointerClear(Input->FrameImage.Width*Input->FrameImage.Height);
	}
	if (!Input->Buffer) {
		Input->Buffer = (Word8 *)AllocAPointerClear(65536);
	}
	if (Input->FrameImage.ImagePtr &&
		Input->Buffer) {
		Input->Paused = FALSE;
		Input->Active = TRUE;	/* Movie is active */
		return FALSE;
	}
	Input->Active = FALSE;		/* Failsafe! */
	return TRUE;
}

/**********************************

	Stop a DPaint Anim Movie

**********************************/

void BURGERCALL DPaintAnimMovieStop(DPaintAnimMovie_t *Input)
{
	Input->Active = FALSE;
	DeallocAPointer(Input->Buffer);
	DeallocAPointer(Input->FrameImage.ImagePtr);
	Input->Buffer = 0;
	Input->FrameImage.ImagePtr = 0;
}

/**********************************

	Get the image record

**********************************/

Image_t * BURGERCALL DPaintAnimMovieGetImage(DPaintAnimMovie_t *Input)
{
	if (Input->FrameImage.ImagePtr) {
		return &Input->FrameImage;
	}
	return 0;		/* The Image_t structure has been purged! */
}


/**********************************

	Process a frame of movie data
	Here is the data bytes...

	0x00,0x00 Break
	0x00,Count,Fill : Fill with Count bytes (Count can't be zero)
	0x01-0x7F,Data : Copy 1-127 bytes of raw data
	0x80,Word16 : If the short is < 0x8000 then I skip this many pixels
			If the short if >=0x8000 and < 0xC000 then I copy this many raw bytes
			If the short is >=0xC000 then I fill a single byte
	0x81-0xFF : Skip 1-127 bytes

**********************************/

static void DPaintAnimMovieProcessFrame(DPaintAnimMovie_t *Input)
{
	DPaintDictionary_t *DictPtr;
	Word32 Offset;
	Word i,j;
	Word8 *WorkPtr;

	DictPtr = (DPaintDictionary_t*)Input->Buffer;
	i = Input->CurrentFrameNumber-Burger::LoadLittle(DictPtr->BaseRecord);
	Offset = Burger::LoadLittle(DictPtr->Records)*2+8;
	if (i) {
		j = 0;
		do {
			Offset += Burger::LoadLittle(DictPtr->Sizes[j]);
		} while (++j<i);
	}
	WorkPtr = Input->Buffer+Offset;
	if (WorkPtr[0]==66) {		/* Failsafe! */
		Word8 *DestPtr;
		WorkPtr += 4;
		DestPtr = Input->FrameImage.ImagePtr;
		Offset = 320*200;		/* Safety net */
		for (;;) {
			j = WorkPtr[0];		/* Get the token */
			++WorkPtr;			/* Accept it */
			if (!j) {			/* Word8 fill? */
				j = WorkPtr[0];	/* Repeat count */
				i = WorkPtr[1];	/* Fill byte */
				WorkPtr+=2;
				if (!j) {		/* Abort? */
					break;
				}
				if (j>Offset) {
					break;
				}
				Offset-=j;
				do {
					DestPtr[0] = static_cast<Word8>(i);	/* Fill 'er up! */
					++DestPtr;
				} while (--j);
				continue;
			}
			if (j<128) {		/* Raw data? */
				if (j>Offset) {
					break;
				}
				Offset-=j;
				do {
					DestPtr[0] = WorkPtr[0];
					++WorkPtr;
					++DestPtr;
				} while (--j);
				continue;
			}

			if (j!=0x80) {		/* Skip token 0x81-0xFF */
				j=j-128;
				if (j>Offset) {
					break;
				}
				Offset-=j;
				DestPtr = DestPtr+j;
				continue;
			}

			/* I am in pain */

			i = Burger::LoadLittle(((Word16 *)WorkPtr)[0]);	/* Get the short */
			WorkPtr += 2;
			j = i&0x3FFF;
			if (!j) {			/* Abort code? */
				break;
			}
			if (j>Offset) {
				break;
			}
			Offset-=j;

			if (i<0x8000) {		/* Big skip? */
				DestPtr+=j;
				continue;
			}
			if (i<0xC000) {		/* Big memcpy? */
				do {
					DestPtr[0] = WorkPtr[0];
					++WorkPtr;
					++DestPtr;
				} while (--j);
				continue;
			}
			i = WorkPtr[0];		/* Big mem fill? */
			++WorkPtr;
			do {
				DestPtr[0] = static_cast<Word8>(i);
				++DestPtr;
			} while (--j);
		}
	}
}

/**********************************

	If it's time, process a frame of the movie

**********************************/

Image_t *DPaintAnimMovieUpdate(DPaintAnimMovie_t *Input)
{
	Word32 Size;

	if (Input->Active) {	/* Is the movie active? */
		if (!Input->CurrentFrameNumber) {	/* First frame? */
			if (DPaintAnimMovieDataSeekToCurrentFrame(Input)) {	/* Seek to this frame */
				goto StopNow;
			}
			Input->TickStart = ReadTick();
		} else {
			Size = ReadTick()-Input->TickStart;
			if (Size < ((Input->MovieSpeed*Input->CurrentFrameNumber*(TICKSPERSEC/10))/(1000/10))) {
				return 0;
			}
		}

		/* Did the movie end? */

		if (Input->CurrentFrameNumber>=Input->MaxFrameNumber) {
			if (!Input->Looping) {
StopNow:
				Input->Active = FALSE;
				Input->Completed = TRUE;
				DeallocAPointer(Input->Buffer);
				Input->Buffer = 0;
				return 0;
			}
			Input->CurrentFrameNumber = 0;	/* Init to frame #1 */
			Input->TickStart = ReadTick();		/* Reset the timer */
		}

		if (DPaintAnimMovieDataSeekToCurrentFrame(Input)) {		/* Make sure it's loaded */
			goto StopNow;
		}
		DPaintAnimMovieProcessFrame(Input);		/* Process it */
		++Input->CurrentFrameNumber;		/* Next frame */
		return &Input->FrameImage;
	}
	return 0;
}

/**********************************

	Pause a DPaint Anim file

**********************************/

void BURGERCALL DPaintAnimMoviePause(DPaintAnimMovie_t *Input)
{
	if (!Input->Paused) {
		Input->Paused=TRUE;
		Input->Active=FALSE;
		Input->TickStop = ReadTick();
	}
}

/**********************************

	Resume a DPaint Anim file

**********************************/

void BURGERCALL DPaintAnimMovieResume(DPaintAnimMovie_t *Input)
{
	if (Input->Paused) {
		Input->Paused = FALSE;
		Input->Active = TRUE;
		Input->TickStart += ReadTick()-Input->TickStop;
	}
}

/**********************************

	Set the playback speed of a DPaint Anim file

**********************************/

void BURGERCALL DPaintAnimMovieSetSpeed(DPaintAnimMovie_t *Input,Word Speed)
{
	if (Speed) {				/* Prevent a divide by zero error */
		Speed = 1000/Speed;		/* Set the speed */
	}
	Input->MovieSpeed = Speed;
}

/**********************************

	Set the looping flag

**********************************/

void BURGERCALL DPaintAnimMovieSetLoopFlag(DPaintAnimMovie_t *Input,Word LoopFlag)
{
	Input->Looping = (LoopFlag) ? TRUE : FALSE;
}



