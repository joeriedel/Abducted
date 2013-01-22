/**********************************

	Read data from an async stream
	MacOS version

**********************************/

#define WIN32_LEAN_AND_MEAN
#include "RsReadStream.h"

#if defined(__WIN32__)
#include "ClStdlib.h"
#include "MmMemory.h"
#include "FmFile.h"
#include <windows.h>

typedef struct ReadFileStream_t {
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
	
	Word32 FileOffset;	/* For seeking */
	HANDLE FileRef;			/* Windows file reference */
	HANDLE ThreadHandle;	/* Handle of the thread */
	DWORD ThreadID;			/* ID of the thread */
	CRITICAL_SECTION Block;
	Word8 Stop;				/* TRUE if I should stop reading */
	volatile Word8 PendingRead;	/* Is there a read in progress? */
	Word8 Exit;				/* TRUE when an abort is requested */
	volatile Word8 ExitAck;	/* TRUE when done */
} ReadFileStream_t;

/**********************************

	This thread will read data into the 

**********************************/

static DWORD WINAPI ReadFileStreamCallback(LPVOID TempInput)
{
	ReadFileStream_t *LocalPtr;		/* Structure pointer */
	Word32 DataRead;				/* Data read in the last pass */
	Word Index;						/* Block index to read */

	/* Convert the pointer to a ReadFileStream_t pointer */

	LocalPtr = (ReadFileStream_t *)TempInput;

	do {
		Word SleepyTime;

		/* Ok, have I reached the end of file or a stop condition? */
		
		EnterCriticalSection(&LocalPtr->Block);
		SleepyTime = TRUE;
		if (LocalPtr->CurrentOffset<LocalPtr->FileSize && !LocalPtr->Stop) {
			DWORD ReadIn;
			
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
					/* Ok, let's read (Will block) */
					if (LocalPtr->FileOffset!=LocalPtr->CurrentOffset) {
						if (SetFilePointer(LocalPtr->FileRef,LocalPtr->CurrentOffset,0,FILE_BEGIN)==LocalPtr->CurrentOffset) {
							LocalPtr->FileOffset = LocalPtr->CurrentOffset;
						}
					}
					LocalPtr->PendingRead = TRUE;
					if (ReadFile(LocalPtr->FileRef,&LocalPtr->StreamBuffer[LocalPtr->EndIndex*LocalPtr->ChunkSize],DataRead,&ReadIn,0)) {
						LocalPtr->FileOffset += ReadIn;
						if (DataRead != ReadIn) {		/* Error in reading? */
							DataRead = ReadIn&(~LocalPtr->ChunkSizeMask);				/* Only accept total blocks */
						}

						/* Accept any data from the previous read */
						if (DataRead) {							/* Did the previous read accept data? */
							LocalPtr->CurrentOffset += DataRead;				/* More the file mark */
							LocalPtr->EndIndex = (LocalPtr->EndIndex+((DataRead+LocalPtr->ChunkSizeMask)>>LocalPtr->ChunkShift))&LocalPtr->ChunkCountMask;
							SleepyTime = FALSE;
						}
					}
					LocalPtr->PendingRead = FALSE;
				}
			}
		}
		LeaveCriticalSection(&LocalPtr->Block);
		if (SleepyTime) {
			SleepEx(16,FALSE);		/* Wait a bit and try again */
		}
	} while (!LocalPtr->Exit);
	LocalPtr->ExitAck = TRUE;		/* I am all done! */
	return 0;		/* Bye bye */
}

/**********************************

	Init the async file read manager
	I always return this in "Stopped" mode

**********************************/

ReadFileStream_t *BURGERCALL ReadFileStreamNew(const char *FileName,Word Count,Word ChunkSize)
{
	ReadFileStream_t *Output;
	char PathName[FULLPATHSIZE];
	
	Count = PowerOf2(Count);			/* Number of block must be a power of 2 */
	ChunkSize = PowerOf2(ChunkSize);	/* Same goes for the chunk size */
	if (Count && ChunkSize) {
		Output = (ReadFileStream_t *)AllocAPointerClear(sizeof(ReadFileStream_t));
		if (Output) {
			Word8 *Buffer;
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

			/* Init the shared memory object */

			InitializeCriticalSection(&Output->Block);
			ExpandAPathToBufferNative(PathName,FileName);
//			Output->FileOffset = 0;
			Output->FileRef = CreateFile(PathName,GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
			if (Output->FileRef != INVALID_HANDLE_VALUE) {
				Word32 BufferSize;

				BufferSize = (Count*ChunkSize);
				Output->BufferSize = BufferSize;
				Buffer = static_cast<Word8 *>(AllocAPointer(BufferSize));
				if (Buffer) {
					DWORD HighWord;
					Output->StreamBuffer = Buffer;
					HighWord = 0;
					Output->FileSize = GetFileSize(Output->FileRef,&HighWord);
					if (HighWord) {		/* Too big? */
						Output->FileSize = (Word32)-1;
					}
					Output->Stop = TRUE;		/* Assume it is stopped */
					Output->ThreadHandle = CreateThread(0,1024,ReadFileStreamCallback,Output,0,&Output->ThreadID);
					if (Output->ThreadHandle != INVALID_HANDLE_VALUE) {
						SetThreadPriority(Output->ThreadHandle,THREAD_PRIORITY_HIGHEST);
						return Output;
					}
					DeallocAPointer(Buffer);		/* Release the ring buffer */
				}
				CloseHandle(Output->FileRef);
			}
			DeleteCriticalSection(&Output->Block);
			DeallocAPointer(Output);			/* Release the base structure */
		}
	}
	return 0;
}

/**********************************

	Destroy the ASync file stream

**********************************/

void BURGERCALL ReadFileStreamDelete(ReadFileStream_t *Input)
{
	if (Input) {
		if (Input->ThreadHandle!=INVALID_HANDLE_VALUE) {
			Input->Exit = TRUE;
			while (!Input->ExitAck) {}
			CloseHandle(Input->ThreadHandle);
			Input->ThreadHandle = INVALID_HANDLE_VALUE;
		}
		if (Input->FileRef!=INVALID_HANDLE_VALUE) {
			CloseHandle(Input->FileRef);
			Input->FileRef = INVALID_HANDLE_VALUE;
		}
		if (Input->StreamBuffer) {
			DeallocAPointer(Input->StreamBuffer);
			Input->StreamBuffer = 0;
		}
		DeleteCriticalSection(&Input->Block);
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
		if (Input->PendingRead) {
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
	if (Input && !Input->Stop) {
		EnterCriticalSection(&Input->Block);
		Input->Stop = TRUE;		/* Stop pending reads */
		LeaveCriticalSection(&Input->Block);
	}
}

/**********************************

	Start reading at a certain point to the end of the file

**********************************/

void BURGERCALL ReadFileStreamStart(ReadFileStream_t *Input,Word32 Offset)
{
	if (Input) {
		ReadFileStreamStop(Input);
		EnterCriticalSection(&Input->Block);
		Input->CurrentIndex = 0;
		Input->EndIndex = 0;
		Input->CurrentOffset = Offset;
		Input->LastTaken = 0;
		Input->LastLength = 0;
		Input->LastOffset = Offset;
		Input->Stop = FALSE;
		LeaveCriticalSection(&Input->Block);
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
	}
}

#endif