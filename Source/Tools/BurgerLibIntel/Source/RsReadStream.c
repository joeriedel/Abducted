/**********************************

	Read data from an async stream

**********************************/

#include "RsReadStream.h"
#if !defined(__MAC__) && !defined(__WIN32__)
#include "ClStdlib.h"
#include "MmMemory.h"
#include "FmFile.h"

typedef struct ReadFileStream_t {
	Word8 *StreamBuffer;		/* Ring buffer for reading data into */
	Word32 StreamSize;	/* Size of the current buffer */
	Word32 CurrentOffset;	/* Current file mark */
	Word32 FileSize;		/* Size of the file (For stopping) */
	Word Stop;				/* TRUE if I should stop reading */
	FILE *fp;				/* Standard open file */
} ReadFileStream_t;


/**********************************

	Init the async file read manager
	I always return this in "Stopped" mode

**********************************/

ReadFileStream_t *BURGERCALL ReadFileStreamNew(const char *FileName,Word Count,Word ChunkSize)
{
	ReadFileStream_t *Output;
	FILE *fp;
	
	Count = PowerOf2(Count);			/* Number of block must be a power of 2 */
	ChunkSize = PowerOf2(ChunkSize);	/* Same goes for the chunk size */
	if (Count && ChunkSize) {
		Output = (ReadFileStream_t *)AllocAPointerClear(sizeof(ReadFileStream_t));
		if (Output) {
			Word8 *Buffer;
			Buffer = (Word8 *)AllocAPointer(ChunkSize*2);
			if (Buffer) {
				Output->StreamBuffer = Buffer;
				Output->StreamSize = ChunkSize*2;
				fp = OpenAFile(FileName,"rb");		/* Can I open the file? */
				if (fp) {
					Output->fp = fp;
					Output->FileSize = fgetfilesize(fp);
					return Output;
				}
				DeallocAPointer(Output->StreamBuffer);
			}
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
		DeallocAPointer(Input->StreamBuffer);
		if (Input->fp) {
			fclose(Input->fp);
			Input->fp = 0;
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

Word BURGERCALL ReadFileStreamPending(ReadFileStream_t * /* Input */)
{
	return FALSE;
}

/**********************************

	Stop pending reads

**********************************/

void BURGERCALL ReadFileStreamStop(ReadFileStream_t *Input)
{
	if (Input) {
		Input->Stop = TRUE;		/* Stop pending reads */
	}
}

/**********************************

	Start reading at a certain point to the end of the file

**********************************/

void BURGERCALL ReadFileStreamStart(ReadFileStream_t *Input,Word32 Offset)
{
	if (Input) {
		Input->CurrentOffset = Offset;
		fseek(Input->fp,Offset,SEEK_SET);
		Input->Stop = FALSE;
	}
}

/**********************************

	Return a the amount of data available

**********************************/

Word32 BURGERCALL ReadFileStreamAvailBytes(ReadFileStream_t *Input)
{
	if (Input) {
		return Input->FileSize-Input->CurrentOffset;
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
	Word32 MaxSize;
	
	if (Input) {
		MaxSize = Input->FileSize-Input->CurrentOffset;
		if (ReadSize>MaxSize) {
			ReadSize = MaxSize;
		}

		if (ReadSizeOut) {
			ReadSizeOut[0] = ReadSize;
		}
		if (ReadSize) {
			if (ReadSize>Input->StreamSize) {
				Input->StreamBuffer = (Word8 *)ResizeAPointer((void *)Input->StreamBuffer,ReadSize+1024);
				if (!Input->StreamBuffer) {
					Input->StreamSize = 0;
					goto Abort;
				}
				Input->StreamSize = ReadSize+1024;
			}
			ReadSize = fread(Input->StreamBuffer,1,ReadSize,Input->fp);
			if (ReadSize) {
				if (ReadSizeOut) {
					ReadSizeOut[0] = ReadSize;
				}
				Input->CurrentOffset += ReadSize;
				return Input->StreamBuffer;
			}
		}
	}
Abort:;
	if (ReadSizeOut) {
		ReadSizeOut[0] = 0;
	}
	return 0;
}

/**********************************

	If there was a previous data request, flush it

**********************************/

void BURGERCALL ReadFileStreamAcceptData(ReadFileStream_t * /* Input */)
{
}

#endif