/**********************************

	Read data from an async stream
	MacOS version

**********************************/

#include "RsReadStream.h"

#if defined(__MAC__)
#include "ClStdlib.h"
#include "MmMemory.h"
#include "McMac.h"
#include <Files.h>
#include <Devices.h>

typedef struct ReadFileStream_t {
	ParamBlockRec ParmBlock;	/* File manager I/O block record (Must be first!) */
#if !TARGET_RT_MAC_CFM
	long myA5;				/* For old style 68000 code */
#endif
	Word8 *StreamBuffer;		/* Ring buffer for reading data into */
	Word32 BufferSize;	/* Size of the ring buffer in bytes */
	Word ChunkSize;			/* Size of each chunk of the read buffer (Power of 2) */
	Word ChunkCount;		/* Number of chunks in the read buffer (Power of 2) */
	Word ChunkSizeMask;		/* ChunkSize-1 */
	Word ChunkCountMask;	/* ChunkCount-1 */
	Word ChunkShift;		/* Bit mask for chunk reading (1<<ChunkShift) == ChunkSize */
	volatile Word CurrentIndex;	/* Data block currently reading from */
	volatile Word EndIndex;	/* Data block currently reading into */
	Word32 CurrentOffset;	/* Current file mark */
	Word32 FileSize;		/* Size of the file (For stopping at EOF) */
	
	Word32 LastTaken;		/* Number of bytes already taken from the block */
	Word32 LastLength;	/* Size of the last chunk in progress */
	Word32 LastOffset;	/* Read file mark */
	
	int FileRef;			/* MacOS file reference */
	IOCompletionUPP FileCallBackUPP;	/* UPP pointer for file manager callbacks */
	Word8 Stop;				/* TRUE if I should stop reading */
	volatile Word8 ParmBlockInUse;	/* Is the file block in use? */
	short Padding;			/* Not used */
} ReadFileStream_t;

/**********************************

	This code is called by the file manager when a data read
	is complete
	This routine was hell to write, but it was worth it!!!
	
	Note : When I call the file manager, I must exit this routine and assume
	that it is possible that I call my callback IMMEDIATELY. This occurs
	on devices that do not support async reads and as a result they block
	and force me to fill my buffer. Thankfully, most drivers are async.

**********************************/

static pascal void ReadFileStreamCallback(ParamBlockRec *TempInput
#if !TARGET_RT_MAC_CFM
:__a0			/* For classic 68000 */
#endif
)
{
	ReadFileStream_t *LocalPtr;		/* Structure pointer */
	Word32 DataRead;				/* Data read in the last pass */
	Word Index;						/* Block index to read */
#if !TARGET_RT_MAC_CFM
	long OldA5;
#endif

	/* Convert the pointer to a ReadFileStream_t pointer */

	LocalPtr = (ReadFileStream_t *)TempInput;
	
#if !TARGET_RT_MAC_CFM
	OldA5 = SetA5(LocalPtr->myA5);	/* Get the A5 register */
#endif

	/* Was any data read in the last pass? */
	/* If so, then accept the data and move pointers properly */
	
	if (!LocalPtr->ParmBlock.ioParam.ioResult) {						/* No error? */
	
		/* This is a sanity check, if I got a partial read, then I will */
		/* only accept reads in "ChunkSize" granularity */
		
		DataRead = LocalPtr->ParmBlock.ioParam.ioActCount;				/* Get data read */
		if (DataRead != LocalPtr->ParmBlock.ioParam.ioReqCount) {		/* Error in reading? */
			DataRead = DataRead&(~LocalPtr->ChunkSizeMask);				/* Only accept total blocks */
		}

		/* Accept any data from the previous read */
		
		if (DataRead) {							/* Did the previous read accept data? */
			LocalPtr->CurrentOffset += DataRead;				/* More the file mark */
			LocalPtr->EndIndex = (LocalPtr->EndIndex+((DataRead+LocalPtr->ChunkSizeMask)>>LocalPtr->ChunkShift))&LocalPtr->ChunkCountMask;
		}
		LocalPtr->ParmBlock.ioParam.ioActCount = 0;					/* Accept the data */
	}
	
	/* Now, that I have dealt with the previous read, shall I start a new one? */

	/* Ok, have I reached the end of file or a stop condition? */
	
	if (LocalPtr->CurrentOffset<LocalPtr->FileSize && !LocalPtr->Stop) {
	
		/* Ok, I want to start a read, but */
		/* is my buffer full? */
		
		Index = ((LocalPtr->EndIndex+1)&LocalPtr->ChunkCountMask);		/* Mark to read to */
		if (Index!=LocalPtr->CurrentIndex) {
		
			/* Ok, I have space, let's read in the data */
			
			DataRead = LocalPtr->ChunkSize;			/* Size of chunk to read */
			
			/* Did I overflow? */
			
			if (DataRead>(LocalPtr->FileSize-LocalPtr->CurrentOffset)) {
				DataRead = LocalPtr->FileSize-LocalPtr->CurrentOffset;
			}

			if (DataRead) {
			
				/* Ok, let's start an async read */
				
				FastMemSet(&LocalPtr->ParmBlock,0,sizeof(LocalPtr->ParmBlock));
				LocalPtr->ParmBlock.ioParam.ioRefNum = LocalPtr->FileRef;
				LocalPtr->ParmBlock.ioParam.ioPosMode = fsFromStart;
				LocalPtr->ParmBlock.ioParam.ioPosOffset = LocalPtr->CurrentOffset;
				LocalPtr->ParmBlock.ioParam.ioCompletion = LocalPtr->FileCallBackUPP;
				LocalPtr->ParmBlock.ioParam.ioBuffer = (char *)&LocalPtr->StreamBuffer[LocalPtr->EndIndex*LocalPtr->ChunkSize];
				LocalPtr->ParmBlock.ioParam.ioReqCount = DataRead;
//				LocalPtr->ParmBlock.ioParam.ioActCount = 0;
//				LocalPtr->ParmBlock.ioParam.ioResult = 0;
				LocalPtr->ParmBlockInUse = TRUE;				/* My param block is now used */
				if (!PBReadAsync(&LocalPtr->ParmBlock)) {
					goto OutOut;			/* Ok, the call went through */
				}
			}
		}
	}
	LocalPtr->ParmBlockInUse = FALSE;				/* I am not using the buffer anymore */
OutOut:;									/* Exit here */
	#if !TARGET_RT_MAC_CFM
	OldA5 = SetA5(OldA5);					/* 680x0 support */
	#endif
}

/**********************************

	Init the async file read manager
	I always return this in "Stopped" mode

**********************************/

ReadFileStream_t *BURGERCALL ReadFileStreamNew(const char *FileName,Word Count,Word ChunkSize)
{
	ReadFileStream_t *Output;
	ParamBlockRec MyParms;
	
	Count = PowerOf2(Count);			/* Number of block must be a power of 2 */
	ChunkSize = PowerOf2(ChunkSize);	/* Same goes for the chunk size */
	if (Count && ChunkSize) {
		Output = (ReadFileStream_t *)AllocAPointerClear(sizeof(ReadFileStream_t));
		if (Output) {
			Word8 *Buffer;
			IOCompletionUPP FileCallBackUPP;	/* UPP pointer for file manager callbacks */

			/* Needed for 68k support */
			
#if !TARGET_RT_MAC_CFM
			{
				long MyA5;
				MyA5 = SetCurrentA5();		/* For the 680x0 version */
				Output->myA5 = MyA5;	/* Save the a5 register */
			}
#endif
	
			{
				int Shift;
				Word Temp1;
				Output->ChunkSize = ChunkSize;
				Output->ChunkCount = Count;
				Output->ChunkSizeMask = ChunkSize-1;
				Output->ChunkCountMask = Count-1;
				Temp1 = ChunkSize;
				Shift = -1;
				do {
					++Shift;
					Temp1>>=1;
				} while (Temp1);
				Output->ChunkShift = (Word)Shift;
			}

			FileCallBackUPP = NewIOCompletionUPP((IOCompletionProcPtr)ReadFileStreamCallback);
			if (FileCallBackUPP) {
				Word32 BufferSize;
				
				Output->FileCallBackUPP = FileCallBackUPP;
				BufferSize = (Count*ChunkSize);
				Output->BufferSize = BufferSize;
				Buffer = static_cast<Word8 *>(AllocAPointer(BufferSize));
				if (Buffer) {
					int Ref;
					HoldMemory(Buffer,BufferSize);		/* Tell VM not to swap out this memory */
					Output->StreamBuffer = Buffer;
					Ref = MacOpenFileForRead(FileName);		/* Can I open the file? */
					if (Ref!=-1) {
						Output->FileRef = Ref;
						FastMemSet(&MyParms,0,sizeof(MyParms));
						MyParms.ioParam.ioRefNum = Ref;
						if (!PBGetEOFSync(&MyParms)) {
							Output->FileSize = (Word32)MyParms.ioParam.ioMisc;		/* Return the length */
						}
						Output->Stop = TRUE;		/* Assume it is stopped */
						return Output;
					}
					DeallocAPointer(Buffer);		/* Release the ring buffer */
				}
				DeallocAPointer(Output);			/* Release the base structure */
			}
		}
	}
	return 0;
}

/**********************************

	Destroy the ASync file stream

**********************************/

void BURGERCALL ReadFileStreamDelete(ReadFileStream_t *Input)
{
	ParamBlockRec MyParms;
	if (Input) {
		ReadFileStreamStop(Input);		/* Stop the async reads */
		if (Input->FileRef!=-1) {
			FastMemSet(&MyParms,0,sizeof(MyParms));
			MyParms.ioParam.ioRefNum = Input->FileRef;
			PBCloseSync(&MyParms);
			Input->FileRef = -1;
		}
		if (Input->FileCallBackUPP) {
			DisposeIOCompletionUPP(Input->FileCallBackUPP);
			Input->FileCallBackUPP = 0;
		}
		if (Input->StreamBuffer) {
			UnholdMemory(Input->StreamBuffer,Input->BufferSize);		/* Release virtual memory */
			DeallocAPointer(Input->StreamBuffer);
			Input->StreamBuffer = 0;
		}
		DeallocAPointer(Input);
	}
}

/**********************************

	Am I actively reading data?
	Return TRUE if I am

**********************************/

Word BURGERCALL ReadFileStreamActive(ReadFileStream_t *Input)
{
	if (Input) {
		if (!Input->Stop && Input->CurrentOffset<Input->FileSize) {		/* Not stopped? */
			return TRUE;
		}
	}
	return FALSE;		/* No, I'm dormant */
}

/**********************************

	Return TRUE if I have a pending data read

**********************************/

Word BURGERCALL ReadFileStreamPending(ReadFileStream_t *Input)
{
	if (Input) {
		if (Input->ParmBlockInUse) {
			return TRUE;
		}
	}
	return FALSE;
}

/**********************************

	Stop pending reads

**********************************/

void BURGERCALL ReadFileStreamStop(ReadFileStream_t *Input)
{
	if (Input) {
		Input->Stop = TRUE;		/* Stop pending reads */
		while (Input->ParmBlockInUse) {}		/* Wait for release */
	}
}

/**********************************

	Start reading at a certain point to the end of the file

**********************************/

void BURGERCALL ReadFileStreamStart(ReadFileStream_t *Input,Word32 Offset)
{
	if (Input) {
		ReadFileStreamStop(Input);
		Input->CurrentIndex = 0;
		Input->EndIndex = 0;
		Input->CurrentOffset = Offset;
		Input->LastTaken = 0;
		Input->LastLength = 0;
		Input->LastOffset = Offset;
		Input->Stop = FALSE;
		FastMemSet(&Input->ParmBlock,0,sizeof(Input->ParmBlock));
		ReadFileStreamCallback(&Input->ParmBlock);
	}
}

/**********************************

	Return a the amount of data available

**********************************/

Word32 BURGERCALL ReadFileStreamAvailBytes(ReadFileStream_t *Input)
{
	if (Input) {
		Word32 Size;
		ReadFileStreamAcceptData(Input);
		Size = (((Input->EndIndex-Input->CurrentIndex)&Input->ChunkCountMask)<<Input->ChunkShift)-Input->LastTaken;
		if (Size>(Input->FileSize-Input->LastOffset)) {
			Size = Input->FileSize-Input->LastOffset;
		}
		return Size;
	}
	return 0;
}

/**********************************

	Accept some data and lock it down.
	If data was already locked, release it and 
	then accept from the last place I stopped at

**********************************/

Word8 * BURGERCALL ReadFileStreamGetData(ReadFileStream_t *Input,Word32 *ReadSizeOut,Word32 ReadSize)
{
	Word EndIndex;
	Word32 MaxSize;
	
	if (Input) {
		Input->LastOffset += Input->LastLength;
		MaxSize = Input->LastTaken+Input->LastLength;
		Input->CurrentIndex = (Input->CurrentIndex+(MaxSize>>Input->ChunkShift))&Input->ChunkCountMask;
		Input->LastTaken = MaxSize & Input->ChunkSizeMask;
		Input->LastLength = 0;
		
		if (!Input->ParmBlockInUse) {		/* Not running? */
			FastMemSet(&Input->ParmBlock,0,sizeof(Input->ParmBlock));
			ReadFileStreamCallback(&Input->ParmBlock);
		}

		EndIndex = Input->EndIndex;		/* Lock down the value */
		if (EndIndex<Input->CurrentIndex) {
			MaxSize = Input->ChunkCount-Input->CurrentIndex;
		} else {
			MaxSize = EndIndex-Input->CurrentIndex;
		}
		
		/* MaxSize has the number of blocks */
		/* Convert to bytes */
		
		MaxSize = (MaxSize<<Input->ChunkShift)-Input->LastTaken;
		if (MaxSize>(Input->FileSize-Input->LastOffset)) {
			MaxSize = Input->FileSize-Input->LastOffset;
		}

		if (ReadSize>MaxSize) {
			ReadSize = MaxSize;
		}
		Input->LastLength = ReadSize;
		if (ReadSizeOut) {
			ReadSizeOut[0] = ReadSize;
		}
		if (ReadSize) {
			return &Input->StreamBuffer[(Input->CurrentIndex<<Input->ChunkShift)+Input->LastTaken];
		}
	}
	if (ReadSizeOut) {
		ReadSizeOut[0] = 0;
	}
	return 0;
}

/**********************************

	If there was a previous data request, flush it

**********************************/

void BURGERCALL ReadFileStreamAcceptData(ReadFileStream_t *Input)
{
	Word32 MaxSize;
	
	if (Input) {
		Input->LastOffset += Input->LastLength;
		MaxSize = Input->LastTaken+Input->LastLength;
		Input->CurrentIndex = (Input->CurrentIndex+(MaxSize>>Input->ChunkShift))&Input->ChunkCountMask;
		Input->LastTaken = MaxSize & Input->ChunkSizeMask;
		Input->LastLength = 0;
				
		if (!Input->ParmBlockInUse) {		/* Not running? */
			FastMemSet(&Input->ParmBlock,0,sizeof(Input->ParmBlock));
			ReadFileStreamCallback(&Input->ParmBlock);
		}
	}
}

#endif