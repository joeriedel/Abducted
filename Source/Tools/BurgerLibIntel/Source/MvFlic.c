#include "MvMovie.h"
#include <BREndian.hpp>
#include "FmFile.h"
#include "TkTick.h"
#include "MmMemory.h"
#include "ClStdLib.h"

/* Flic file header, all entries are LITTLE endian! */

typedef struct FlicHeader_t {
	Word32 FileSize;	/* Size of the total file in bytes */
	Word16 Magic;		/* 0xAF11 for FLI, 0xAF12 for FLC */
	Word16 Frames;		/* Number of frames in movie (4000 max) */
	Word16 Width;		/* Width of the movie in pixels */
	Word16 Height;		/* Height of the movie in pixels */
	Word16 Depth;		/* Must be 8 for 8 bit color */
	Word16 Flags;		/* Must be 3 for proper save */
	Word32 Speed;		/* Number of milliseconds between frames */
	Word16 Reserved;		/* Zero */
	Word32 Created;	/* MS-DOS creation date */
	Word32 Creator;	/* Serial number of the creator or 'FLIB' */
	Word32 Updated;	/* MS-DOS update date */
	Word32 Updater;	/* Serial number of the editor or 'FLIB' */
	Word16 AspectX;		/* Aspect ratio for x (1:1 or 6:5 for 320x200) */
	Word16 AspectY;		/* Aspect ratio for y */
	Word8 Padding[38];	/* Filler */
	Word32 Frame1Offset;	/* Offset into the file for the first frame */
	Word32 Frame2Offset;	/* Offset into the file for the second frame */
	Word8 Padding2[40];		/* More filler */
} FlicHeader_t;

typedef struct FlicChunkHeader_t {
	Word32 Size;
	Word16 Type;
} FlicChunkHeader_t;

typedef struct FlicChunkCount_t {
	Word16 Count;
	Word8 Reserved[8];
} FlicChunkCount_t;

/**********************************

	Create a new Flic movie record

**********************************/

FlicMovie_t * BURGERCALL FlicMovieNew(const char *Filename,Word Flags,Word32 FileOffset)
{
	FlicMovie_t *MoviePtr;
	MoviePtr = (FlicMovie_t *)AllocAPointer(sizeof(FlicMovie_t));
	if (MoviePtr) {
		if (!FlicMovieInit(MoviePtr,Filename,Flags,FileOffset)) {	/* Init the movie? */
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

void BURGERCALL FlicMovieDelete(FlicMovie_t *Input)
{
	if (Input) {		/* Valid pointer? */
		FlicMovieDestroy(Input);	/* Dispose of the contents */
		DeallocAPointer(Input);		/* Dispose of the memory */
	}
}

/**********************************

	Read data for a flic movie
	Return FALSE is ok, error code otherwise

**********************************/

Word BURGERCALL FlicMovieDataRead(FlicMovie_t *Input,void *Data,Word32 Length)
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

	Seek within the file

**********************************/

static void BURGERCALL FlicMovieSeek(FlicMovie_t *Input,Word32 Offset)
{
	if (Input->fp) {			/* Seek to the beginning of the file */
		fseek(Input->fp,Offset+Input->FileOffset,SEEK_SET);
	} else {
		StreamHandleSetMark(&Input->MyInput,Offset);	/* Set the stream handle */
	}
}

/**********************************

	Initialize a Flic movie player

**********************************/

Word BURGERCALL FlicMovieInit(FlicMovie_t *Input,const char *Filename,Word Flags,Word32 FileOffset)
{
	Word Temp;
	FlicHeader_t FileHeader;

	FastMemSet(Input,0,sizeof(FlicMovie_t));		/* Init all entries */
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

	if (!FlicMovieDataRead(Input,&FileHeader,sizeof(FileHeader))) {
		Temp = Burger::LoadLittle(FileHeader.Magic);
		if (Temp==0xAF11 || Temp==0xAF12) {
			Temp = Burger::LoadLittle(FileHeader.Flags);
			if (Temp==3) {
				Input->FileOffset = FileOffset;
				Input->FrameImage.Width = Burger::LoadLittle(FileHeader.Width);
				Input->FrameImage.Height = Burger::LoadLittle(FileHeader.Height);
				Input->FrameImage.DataType = IMAGE8_PAL;
				Input->FrameImage.RowBytes = Input->FrameImage.Width;
				Input->MaxFrameNumber = Burger::LoadLittle(FileHeader.Frames);
				Input->Paused = FALSE;
				Input->MovieSpeed = Burger::LoadLittle(&FileHeader.Speed);
				Input->FirstFrameMark = Burger::LoadLittle(&FileHeader.Frame1Offset);
				Input->LoopFrameMark = Burger::LoadLittle(&FileHeader.Frame2Offset);
				return FALSE;		/* It's done! */
			} else {
				NonFatal("Flic file's flag is not 3\n");
			}
		} else {
			NonFatal("Flic file doesn't have 0xAF11 or 0xAF12 ID value\n");
		}
	} else {
		NonFatal("Can't read the Flic file's header\n");
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

void BURGERCALL FlicMovieDestroy(FlicMovie_t *Input)
{
	if (Input) {
		Input->Active = FALSE;	/* This is shut down! */
		if (Input->fp) {		/* Close the input file if open */
			fclose(Input->fp);
			Input->fp = 0;
		}
		StreamHandleDestroy(&Input->MyInput);
		DeallocAPointer(Input->Buffer);
		DeallocAPointer(Input->FrameImage.PalettePtr);
		DeallocAPointer(Input->FrameImage.ImagePtr);
		Input->Buffer = 0;
		Input->BufferSize = 0;
		Input->FrameImage.PalettePtr = 0;
		Input->FrameImage.ImagePtr = 0;
	}
}

/**********************************

	Resets a movie to the first frame

**********************************/

void BURGERCALL FlicMovieReset(FlicMovie_t *Input)
{
	FlicMovieStop(Input);
	Input->Active = FALSE;
	Input->CurrentFrameNumber = 0;
	Input->Paused = FALSE;
	Input->Completed = FALSE;
}

/**********************************

	Starts a movie

**********************************/

Word BURGERCALL FlicMovieStart(FlicMovie_t *Input)
{
	Input->TickStart = ReadTick();
	if (!Input->FrameImage.PalettePtr) {
		Input->FrameImage.PalettePtr = (Word8 *)AllocAPointer(768);
	}
	if (!Input->FrameImage.ImagePtr) {
		Input->FrameImage.ImagePtr = (Word8 *)AllocAPointerClear(Input->FrameImage.Width*Input->FrameImage.Height);
	}
	if (!Input->BufferSize) {
		Input->BufferSize  = Input->FrameImage.Width*Input->FrameImage.Height;
		Input->Buffer = (Word8 *)AllocAPointer(Input->BufferSize);
	}
	if (Input->FrameImage.PalettePtr &&
		Input->FrameImage.ImagePtr &&
		Input->Buffer) {
		Input->Paused = FALSE;
		Input->Active = TRUE;	/* Movie is active */
		return FALSE;
	}
	Input->Active = FALSE;		/* Failsafe! */
	return TRUE;
}

/**********************************

	Stop a Flic Movie

**********************************/

void BURGERCALL FlicMovieStop(FlicMovie_t *Input)
{
	Input->Active = FALSE;
	DeallocAPointer(Input->Buffer);
	DeallocAPointer(Input->FrameImage.PalettePtr);
	DeallocAPointer(Input->FrameImage.ImagePtr);
	Input->Buffer = 0;
	Input->BufferSize = 0;
	Input->FrameImage.PalettePtr = 0;
	Input->FrameImage.ImagePtr = 0;
}

/**********************************

	Get the image record

**********************************/

Image_t * BURGERCALL FlicMovieGetImage(FlicMovie_t *Input)
{
	if (Input->FrameImage.ImagePtr) {
		return &Input->FrameImage;
	}
	return 0;		/* The Image_t structure has been purged! */
}

/**********************************

	Process a palette chunk using 8 bit data

**********************************/

static void FlicMovieProcess4(Word8 *PalPtr,Word8 *Input)
{
	Word Count;
	Count = Burger::LoadLittle(((Word16 *)Input)[0]);	/* Get the chunk count */
	if (Count) {		/* Any chunks? */
		Input+=2;		/* Accept it */
		do {
			Word Change;
			PalPtr += Input[0]*3;	/* Get the skip count */
			Change = Input[1];		/* Get the triplett count */
			Input+=2;				/* Accept it */
			if (!Change) {
				Change = 256;		/* 256? */
			}
			Change = Change*3;		/* Convert to bytes */
			do {
				PalPtr[0] = Input[0];	/* Copy the data */
				++PalPtr;
				++Input;
			} while (--Change);		/* All tripletts done? */
		} while (--Count);			/* All packets done? */
	}
}

/**********************************

	Unpack using word aligned delta compression

**********************************/

static void FlicMovieProcess7(Image_t *Input,Word8 *WorkPtr)
{
	Word PacketCount;
	Word8 *DestPtr;

	PacketCount = Burger::LoadLittle(((Word16 *)WorkPtr)[0]);
	if (PacketCount) {
		WorkPtr+=2;
		DestPtr = Input->ImagePtr;		/* Get the pointer to the image */
		do {
			Word Type;
Again:
			Type = Burger::LoadLittle(((Word16 *)WorkPtr)[0]);
			WorkPtr+=2;		/* Skip data */
			if (Type>=0xC000) {		/* Line skip? */
				Type = 65536-Type;	/* Negate it */
				DestPtr = DestPtr+(Input->Width*Type);
				goto Again;
			}
			if (Type>=0x8000) {		/* Last byte? */
				DestPtr[Input->Width-1] = static_cast<Word8>(Type);
				Type = Burger::LoadLittle(((Word16 *)WorkPtr)[0]);
				WorkPtr+=2;
			}
			if (Type) {
				Word8 *DestPtr2;
				DestPtr2 = DestPtr;
				do {
					Word Count;
					DestPtr2 += WorkPtr[0];
					Count = WorkPtr[1];
					WorkPtr+=2;
					if (Count<128) {
						if (Count) {
							Count = Count<<1;
							do {
								DestPtr2[0] = WorkPtr[0];
								++DestPtr2;
								++WorkPtr;
							} while (--Count);
						}
					} else {
						Word Temp;
						Count = 256-Count;
						Temp = ((Word16 *)WorkPtr)[0];
						do {
							((Word16 *)DestPtr2)[0] = static_cast<Word8>(Temp);	/* Memory fill */
							DestPtr2+=2;
						} while (--Count);	/* All done? */
						WorkPtr+=2;
					}
				} while (--Type);
			}
			DestPtr = DestPtr+Input->Width;
		} while (--PacketCount);		/* Any more? */
	}
}

/**********************************

	Process a palette chunk using 6 bit data

**********************************/

static void FlicMovieProcess11(Word8 *PalPtr,Word8 *Input)
{
	Word Count;
	Count = Burger::LoadLittle(((Word16 *)Input)[0]);	/* Get the chunk count */
	if (Count) {		/* Any chunks? */
		Input+=2;		/* Accept it */
		do {
			Word Change;
			PalPtr += Input[0]*3;	/* Get the skip count */
			Change = Input[1];		/* Get the triplett count */
			Input+=2;				/* Accept it */
			if (!Change) {
				Change = 256;		/* 256? */
			}
			Change = Change*3;		/* Convert to bytes */
			do {
				PalPtr[0] = Input[0]<<2;	/* Copy the data but convert to 8 bit */
				++PalPtr;
				++Input;
			} while (--Change);		/* All tripletts done? */
		} while (--Count);			/* All packets done? */
	}
}

/**********************************

	Unpack using byte aligned delta compression

**********************************/

static void FlicMovieProcess12(Image_t *Input,Word8 *WorkPtr)
{
	Word PacketCount;
	Word8 *DestPtr;

	DestPtr = Input->ImagePtr+(Burger::LoadLittle(((Word16 *)WorkPtr)[0])*Input->Width);
	PacketCount = Burger::LoadLittle(((Word16 *)WorkPtr)[1]);
	if (PacketCount) {
		WorkPtr+=4;
		do {
			Word Type;
			Word8 *DestPtr2;

			DestPtr2 = DestPtr+WorkPtr[0];
			Type = WorkPtr[1];
			WorkPtr+=2;
			if (Type) {
				do {
					Word Count;
					Count = WorkPtr[0];
					WorkPtr+=1;
					if (Count<128) {
						if (Count) {
							do {
								DestPtr2[0] = WorkPtr[0];
								++DestPtr2;
								++WorkPtr;
							} while (--Count);
						}
					} else {
						Word8 Temp;
						Count = 256-Count;
						Temp = WorkPtr[0];
						do {
							DestPtr2[0] = Temp;	/* Memory fill */
							DestPtr2+=1;
						} while (--Count);	/* All done? */
						WorkPtr+=1;
					}
				} while (--Type);
			}
			DestPtr = DestPtr+Input->Width;
		} while (--PacketCount);		/* Any more? */
	}
}

/**********************************

	Fill the image with black

**********************************/

static void FlicMovieProcess13(Image_t *Input)
{
	FastMemSet(Input->ImagePtr,0,(Word32)Input->Width*(Word32)Input->Height);
}

/**********************************

	Unpack using byte run length compression

**********************************/

static void FlicMovieProcess15(Image_t *Input,Word8 *WorkPtr)
{
	Word Height;
	Word Width;
	Word8 *DestPtr;

	Height = Input->Height;
	DestPtr = Input->ImagePtr;
	do {
		++WorkPtr;		/* Skip the packet count (Not used) */
		Width = Input->Width;
		do {
			Word Count;
			Count = WorkPtr[0];
			++WorkPtr;
			if (Count&0x80) {
				Count = 256-Count;
				Width-=Count;
				do {
					DestPtr[0] = WorkPtr[0];
					++DestPtr;
					++WorkPtr;
				} while (--Count);
			} else {
				if (Count) {		/* Failsafe */
					Word8 Temp;
					Width-=Count;	/* Remove the count */
					Temp = WorkPtr[0];
					do {
						DestPtr[0] = Temp;	/* Memory fill */
						++DestPtr;
					} while (--Count);	/* All done? */
				}
				++WorkPtr;
			}
		} while (Width);

	} while (--Height);
}

/**********************************

	Raw image copy

**********************************/

static void FlicMovieProcess16(Image_t *Input,Word8 *WorkPtr)
{
	FastMemCpy(Input->ImagePtr,WorkPtr,(Word32)Input->Width*(Word32)Input->Height);
}

/**********************************

	Process a frame of movie data

**********************************/

static void FlicMovieProcessFrame(FlicMovie_t *Input,Word8 *WorkPtr)
{
	Word Count;

	Count = Burger::LoadLittle(((Word16 *)WorkPtr)[0]);	/* Get the chunk count */
	if (Count) {
		WorkPtr+=10;		/* Skip the short and 8 bytes of padding */
		do {
			Word Type;

			Type = Burger::LoadLittle(((Word16 *)WorkPtr)[2]);
			switch (Type) {	/* Get the type */
			case 4:			/* 8 bit palette */
				FlicMovieProcess4(Input->FrameImage.PalettePtr,WorkPtr+6);
				break;
			case 7:			/* Word oriented delta compression */
				FlicMovieProcess7(&Input->FrameImage,WorkPtr+6);
				break;
			case 11:		/* 6 bit palette */
				FlicMovieProcess11(Input->FrameImage.PalettePtr,WorkPtr+6);
				break;
			case 12:		/* Word8 oriented delta compression */
				FlicMovieProcess12(&Input->FrameImage,WorkPtr+6);
				break;
			case 13:		/* Fill with 0 */
				FlicMovieProcess13(&Input->FrameImage);
				break;
			case 15:		/* Word8 run length compression */
				FlicMovieProcess15(&Input->FrameImage,WorkPtr+6);
				break;
			case 16:		/* Raw image */
				FlicMovieProcess16(&Input->FrameImage,WorkPtr+6);
				break;
			}
			WorkPtr = WorkPtr + Burger::LoadLittle((Word32 *)WorkPtr);
		} while (--Count);
	}
}

/**********************************

	Process a single frame of the movie

**********************************/

static Word FlicMovieProcessASingleFrame(FlicMovie_t *Input)
{
	Word32 Size;
	Word Type;
	struct {
		Word32 Size;		/* Size of the master chunk */
		Word16 Type;			/* Type of chunk */
	} SizeStruct;
	
	if (FlicMovieDataRead(Input,&SizeStruct,6)) {		/* Read the header */
		return TRUE;
	}
	Size = Burger::LoadLittle(&SizeStruct.Size);		/* Convert endian */
	Type = Burger::LoadLittle(&SizeStruct.Type);
	if (Size>Input->BufferSize) {			/* Is the data buffer large enough */
		Input->Buffer = (Word8 *)ResizeAPointer(Input->Buffer,Size);
		Input->BufferSize = Size;
	}
	if (FlicMovieDataRead(Input,Input->Buffer,Size-6)) {
		return TRUE;
	}
	if (Type==0xF1FA) {			/* Valid chunk? */
		FlicMovieProcessFrame(Input,Input->Buffer);		/* Process it */
	}
	++Input->CurrentFrameNumber;		/* Next frame */
	return FALSE;
}

/**********************************

	If it's time, process a frame of the movie

**********************************/

Image_t *FlicMovieUpdate(FlicMovie_t *Input)
{
	if (Input->Active) {	/* Is the movie active? */
		if (!Input->CurrentFrameNumber) {	/* First frame? */
			FlicMovieSeek(Input,Input->FirstFrameMark);
			Input->TickStart = ReadTick();
		} else {
			Word32 Mark;
			Mark = ReadTick()-Input->TickStart;
			if (Mark < ((Input->MovieSpeed*Input->CurrentFrameNumber*(TICKSPERSEC/10))/(1000/10))) {
				return 0;
			}
		}

		/* Did the movie end? */

		if (Input->CurrentFrameNumber>Input->MaxFrameNumber) {
			if (!Input->Looping) {
StopNow:
				Input->Active = FALSE;
				Input->Completed = TRUE;
				DeallocAPointer(Input->Buffer);
				Input->Buffer = 0;
				Input->BufferSize = 0;
				return 0;
			}
			Input->CurrentFrameNumber = 1;	/* Init to frame #1 */
			FlicMovieSeek(Input,Input->LoopFrameMark);
			Input->TickStart = ReadTick();		/* Reset the timer */
		}
		if (FlicMovieProcessASingleFrame(Input)) {
			goto StopNow;
		}
		return &Input->FrameImage;
	}
	return 0;
}

/**********************************

	Pause a Flic file

**********************************/

void BURGERCALL FlicMoviePause(FlicMovie_t *Input)
{
	if (!Input->Paused) {
		Input->Paused=TRUE;
		Input->Active=FALSE;
		Input->TickStop = ReadTick();
	}
}

/**********************************

	Resume a Flic file

**********************************/

void BURGERCALL FlicMovieResume(FlicMovie_t *Input)
{
	if (Input->Paused) {
		Input->Paused = FALSE;
		Input->Active = TRUE;
		Input->TickStart += ReadTick()-Input->TickStop;
	}
}

/**********************************

	Set the playback speed of a Flic file

**********************************/

void BURGERCALL FlicMovieSetSpeed(FlicMovie_t *Input,Word Speed)
{
	if (Speed) {
		Speed = 1000/Speed;
	}
	Input->MovieSpeed = Speed;
}

/**********************************

	Move the movie to a specific frame

**********************************/

void BURGERCALL FlicMovieSetToFrame(FlicMovie_t *Input,Word FrameNum)
{
	if (FrameNum>Input->MaxFrameNumber) {		/* Make sure I'm not crazy */
		FrameNum = Input->MaxFrameNumber;
	}
	if (FrameNum!=Input->CurrentFrameNumber) {		/* Do I need to process a frame? */
		if (FrameNum>=Input->CurrentFrameNumber) {
			Input->CurrentFrameNumber = 0;	/* Init to frame #1 */
			FlicMovieSeek(Input,Input->FirstFrameMark);
		}
		do {
			if (FlicMovieProcessASingleFrame(Input)) {
				break;
			}
		} while (Input->CurrentFrameNumber<FrameNum);
	}
	
	/* Reset the time to the current frame number */
	Input->TickStop = ReadTick();		/* Reset the pause if any */
	Input->TickStart = ReadTick()-((Input->MovieSpeed*Input->CurrentFrameNumber*(TICKSPERSEC/10))/(1000/10));
}

/**********************************

	Set the looping flag

**********************************/

void BURGERCALL FlicMovieSetLoopFlag(FlicMovie_t *Input,Word LoopFlag)
{
	Input->Looping = (LoopFlag) ? TRUE : FALSE;
}



