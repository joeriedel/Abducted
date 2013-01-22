#include "ShStream.h"
#include <BREndian.h>
#include "MmMemory.h"
#include "FmFile.h"
#include "ClStdLib.h"
#include "FpFloat.h"

/**********************************

	Create and initialize an output data stream handle

**********************************/

StreamHandle_t * BURGERCALL StreamHandleNewPut(void)
{
	StreamHandle_t *Result;
	Result = (StreamHandle_t *)AllocAPointer(sizeof(StreamHandle_t));
	if (Result) {		/* Got the memory? */
		StreamHandleInitPut(Result);	/* Init the data */
	}
	return Result;		/* Return the new pointer */
}


/**********************************

	Create and initialize an input data stream handle

**********************************/

StreamHandle_t * BURGERCALL StreamHandleNewGet(void **GetHandle)
{
	StreamHandle_t *Result;
	Result = (StreamHandle_t *)AllocAPointer(sizeof(StreamHandle_t));
	if (Result) {		/* Got the memory? */
		StreamHandleInitGet(Result,GetHandle);	/* Init the data */
	}
	return Result;		/* Return the new pointer */
}


/**********************************

	Create and initialize an input data stream pointer

**********************************/

StreamHandle_t * BURGERCALL StreamHandleNewGetPtr(void *GetPtr,Word32 Size)
{
	StreamHandle_t *Result;
	Result = (StreamHandle_t *)AllocAPointer(sizeof(StreamHandle_t));
	if (Result) {		/* Got the memory? */
		StreamHandleInitGetPtr(Result,GetPtr,Size);	/* Init the data */
	}
	return Result;		/* Return the new pointer */
}


/**********************************

	Create and initialize an input data stream handle
	based on data from a file.

**********************************/

StreamHandle_t * BURGERCALL StreamHandleNewGetFile(const char *FileName)
{
	StreamHandle_t *Result;
	void **InputHandle;
	Result = (StreamHandle_t *)AllocAPointer(sizeof(StreamHandle_t));
	if (Result) {		/* Got the memory? */
		InputHandle = LoadAFileHandle(FileName);	/* Load in the file */
		if (InputHandle) {			/* Success? */
			StreamHandleInitGet(Result,InputHandle);	/* Init the data */
			return Result;
		}
		DeallocAPointer(Result);		/* Dispose of the struct */
	}
	return 0;		/* Return the new pointer */
}

/**********************************

	Initialize an output data stream handle

**********************************/

void BURGERCALL StreamHandleInitPut(StreamHandle_t *Input)
{
	Input->DataHandle = 0;	/* No handle present */
	Input->Mark = 0; 		/* No data stored (Yet) */
	Input->BufferSize = 0;	/* Initial data size */
	Input->EOFMark = 0;		/* No end of file */
	Input->ErrorFlag = FALSE;	/* No errors yet */
}

/**********************************

	Initialize an input data stream handle

**********************************/

void BURGERCALL StreamHandleInitGet(StreamHandle_t *Input,void **GetHandle)
{
	Input->DataHandle = (Word8 **)GetHandle;	/* Save the handle */
	Input->Mark = 0;				/* Init the index */
	Input->EOFMark = Input->BufferSize = GetAHandleSize(GetHandle);	/* Get the buffer size */
	Input->ErrorFlag = FALSE;		/* No errors */
}

/**********************************

	Initialize an input data stream pointer

**********************************/

void BURGERCALL StreamHandleInitGetPtr(StreamHandle_t *Input,void *GetPtr,Word32 Size)
{
	void **TempHand;
	
	Input->Mark = 0;	/* Always reset the mark */
	TempHand = AllocAHandle(Size);
	if (TempHand) {
		Input->DataHandle = (Word8 **)TempHand;		/* Save the handle */
		Input->EOFMark = Input->BufferSize = Size;	/* Get the buffer size */
		Input->ErrorFlag = FALSE;					/* No errors */
		FastMemCpy(TempHand[0],GetPtr,Size);		/* Copy from the pointer */
		return;
	}
	Input->DataHandle = 0;			/* No data is present! */
	Input->EOFMark = 0;
	Input->BufferSize = 0;
	Input->ErrorFlag = TRUE;		/* I have an error! */
}

/**********************************

	Initialize an input data stream handle
	based on data from a file.

**********************************/

Word BURGERCALL StreamHandleInitGetFile(StreamHandle_t *Input,const char *FileName)
{
	void **InputHandle;
	InputHandle = LoadAFileHandle(FileName);	/* Load in the file */
	if (InputHandle) {			/* Success? */
		Input->DataHandle = (Word8 **)InputHandle;	/* Save the handle */
		Input->Mark = 0;				/* Init the index */
		Input->EOFMark = Input->BufferSize = GetAHandleSize(InputHandle);	/* Get the buffer size */
		Input->ErrorFlag = FALSE;		/* No errors */
		return FALSE;
	}
	Input->DataHandle = 0;			/* No data is present! */
	Input->EOFMark = 0;
	Input->BufferSize = 0;
	Input->ErrorFlag = TRUE;		/* I have an error! */
	return TRUE;					/* Return the new pointer */
}


/**********************************

	Delete the contents of a StreamHandle_t record
	but don't delete the actual record

**********************************/

void BURGERCALL StreamHandleDestroy(StreamHandle_t *Input)
{
	if (Input) {		/* Valid pointer? */
		DeallocAHandle((void **)Input->DataHandle);	/* Discard the handle */
		Input->DataHandle=0;
	}
}


/**********************************

	Delete the contents of a StreamHandle_t record
	and delete the record as well

**********************************/

void BURGERCALL StreamHandleDelete(StreamHandle_t *Input)
{
	if (Input) {					/* Valid pointer? */
		DeallocAHandle((void **)Input->DataHandle);	/* Discard the handle */
		DeallocAPointer(Input);		/* Dispose of the memory */
	}
}


/**********************************

	Return control of the data handle to the calling
	application. Usually not used but here in case the application
	gave TEMPORARY control of a handle for data access purposes.

	Can return a NULL
	
**********************************/

void ** BURGERCALL StreamHandleDetachHandle(StreamHandle_t *Input)
{
	void **TempHand;
	TempHand = (void **)Input->DataHandle;	/* Get the current handle */
	if (Input->EOFMark!=Input->BufferSize) {	/* Should I return a different size? */
		TempHand = ResizeAHandle(TempHand,Input->EOFMark);
	}
	Input->DataHandle = 0;			/* No handle is present anymore */
	Input->Mark = 0;				/* Zap all the other fields */
	Input->EOFMark = 0;
	Input->BufferSize = 0;
	return TempHand;			/* Return the handle */
}


/**********************************

	When you are done saving data into a StreamHandle_t
	Call this routine to truncate the data to the current mark

**********************************/

void BURGERCALL StreamHandleEndSave(StreamHandle_t *Input)
{
	if (!Input->ErrorFlag) {
		Input->EOFMark = Input->Mark;		/* Perform the truncation */
	}
}

/**********************************

	Saves the contents of a stream handle record to a file
	return FALSE if OK, or TRUE if an error occured

**********************************/

Word BURGERCALL StreamHandleSaveFile(StreamHandle_t *Input,const char *FileName)
{
	Word8 **TheHand;
	Word Result;
	Word32 Length;

	Result = TRUE;
	if (!Input->ErrorFlag) {		/* Error occured? */
		if ((Length = Input->EOFMark)!=0) {		/* Get the saved length */
			if ((TheHand = Input->DataHandle)!=0) {		/* Valid handle? */
				Result = !SaveAFile(FileName,LockAHandle((void **)TheHand),Length);	/* Open the output */
				UnlockAHandle((void **)TheHand);	/* Unlock the handle */
			}
		}
	}
	return Result;		/* Return the result code! */
}

/**********************************

	Saves the contents of a stream handle record to a file
	return FALSE if OK, or TRUE if an error occured
	Save as a TEXT

**********************************/

Word BURGERCALL StreamHandleSaveTextFile(StreamHandle_t *Input,const char *FileName)
{
	Word8 **TheHand;
	Word Result;
	Word32 Length;

	Result = TRUE;
	if (!Input->ErrorFlag) {		/* Error occured? */
		if ((Length = Input->EOFMark)!=0) {		/* Get the saved length */
			if ((TheHand = Input->DataHandle)!=0) {		/* Valid handle? */
				Result = !SaveATextFile(FileName,LockAHandle((void **)TheHand),Length);	/* Open the output */
				UnlockAHandle((void **)TheHand);	/* Unlock the handle */
			}
		}
	}
	return Result;		/* Return the result code! */
}

/**********************************

	Get a byte of data from a stream

**********************************/

Word BURGERCALL StreamHandleGetByte(StreamHandle_t *Input)
{
	Word32 Offset;

	Offset = Input->Mark;		/* Cache the offset */
	if (Offset<Input->EOFMark) {
		Input->Mark = Offset+1;
	 	return ((Word8 *)(Input->DataHandle[0]+Offset))[0];
	}
	Input->ErrorFlag = TRUE;		/* Overflow */
	return 0;		/* Invalid data */
}

/**********************************

	Get 2 bytes of data from a stream (Little Endian)

**********************************/

Word BURGERCALL StreamHandleGetShort(StreamHandle_t *Input)
{
	Word32 Offset;
	Word32 NewOffset;
	
	Offset = Input->Mark;
	NewOffset = Offset+2;
	if (Input->EOFMark>=NewOffset) {
		Word8 *WorkPtr;
		Input->Mark = NewOffset;
		WorkPtr = Input->DataHandle[0]+Offset;		/* Make a pointer */
#if defined(__INTEL__)
		return Burger::LoadLittle(((Word16 *)WorkPtr)[0]);		/* No problem with bad alignment */
#else
		return WorkPtr[0]+((Word)WorkPtr[1]<<8);	/* Other CPU's hate bad alignment */
#endif
	}
	Input->ErrorFlag = TRUE;
	return 0;
}

/**********************************

	Get a 4 bytes of data from a stream (Little Endian)

**********************************/

Word32 BURGERCALL StreamHandleGetLong(StreamHandle_t *Input)
{
	Word32 Offset;
	Word32 NewOffset;
	
	Offset = Input->Mark;
	NewOffset = Offset+4;
	if (Input->EOFMark>=NewOffset) {
		Word8 *WorkPtr;
		Input->Mark = NewOffset;
		WorkPtr = Input->DataHandle[0]+Offset;		/* Make a pointer */
#if defined(__INTEL__) 
		return Burger::LoadLittle(((Word32 *)WorkPtr)[0]);		/* No problem with bad alignment */
#else
		return WorkPtr[0]+((Word32)WorkPtr[1]<<8)+
			((Word32)WorkPtr[2]<<16)+((Word32)WorkPtr[3]<<24);
#endif
	}
	Input->ErrorFlag = TRUE;
	return 0;
}

/**********************************

	Get 4 bytes of data from a stream (Little Endian)

**********************************/

float BURGERCALL StreamHandleGetFloat(StreamHandle_t *Input)
{
	Word32 Offset;
	Word32 NewOffset;
	
	Offset = Input->Mark;
	NewOffset = Offset+4;
	if (Input->EOFMark>=NewOffset) {
		Word8 *WorkPtr;
		Word32 MemFloat;
		Input->Mark = NewOffset;
		WorkPtr = Input->DataHandle[0]+Offset;		/* Make a pointer */
#if defined(__INTEL__)
		MemFloat = Burger::LoadLittle(((Word32 *)WorkPtr)[0]);
#else
		MemFloat = WorkPtr[0]+((Word32)WorkPtr[1]<<8)+
			((Word32)WorkPtr[2]<<16)+((Word32)WorkPtr[3]<<24);
#endif
		return ((float *)&MemFloat)[0];		/* Return it as a floating point value */
	}
	Input->ErrorFlag = TRUE;
	return 0.0f;
}

/**********************************

	Get 8 bytes of data from a stream (Little Endian)

**********************************/

double BURGERCALL StreamHandleGetDouble(StreamHandle_t *Input)
{
	Word32 Offset;
	Word32 NewOffset;
	
	Offset = Input->Mark;
	NewOffset = Offset+8;
	if (Input->EOFMark>=NewOffset) {
		Word8 *WorkPtr;
		Word32 MemFloat[2];
		Input->Mark = NewOffset;

		WorkPtr = Input->DataHandle[0]+Offset;		/* Make a pointer */
#if defined(__INTEL__)
		MemFloat[0] = Burger::LoadLittle(((Word32*)WorkPtr)[0]);
		MemFloat[1] = Burger::LoadLittle(((Word32*)WorkPtr)[1]);
			
#elif defined(__LITTLEENDIAN__)
		MemFloat[0] = WorkPtr[0]+((Word32)WorkPtr[1]<<8)+
			((Word32)WorkPtr[2]<<16)+((Word32)WorkPtr[3]<<24);
		MemFloat[1] = WorkPtr[4]+((Word32)WorkPtr[5]<<8)+
			((Word32)WorkPtr[6]<<16)+((Word32)WorkPtr[7]<<24);
#else
		MemFloat[1] = WorkPtr[0]+((Word32)WorkPtr[1]<<8)+
			((Word32)WorkPtr[2]<<16)+((Word32)WorkPtr[3]<<24);
		MemFloat[0] = WorkPtr[4]+((Word32)WorkPtr[5]<<8)+
			((Word32)WorkPtr[6]<<16)+((Word32)WorkPtr[7]<<24);
#endif
		return ((double *)(&MemFloat[0]))[0];	
	}
	Input->ErrorFlag = TRUE;
	return 0;
}

/**********************************

	Convenience routine to save a 2D floating point matrix

**********************************/

void BURGERCALL StreamHandleGetVector2D(StreamHandle_t *Input,Vector2D_t *Output)
{
	Output->x = StreamHandleGetFloat(Input);
	Output->y = StreamHandleGetFloat(Input);
}

/**********************************

	Convenience routine to save a 3D floating point matrix

**********************************/

void BURGERCALL StreamHandleGetVector3D(StreamHandle_t *Input,Vector3D_t *Output)
{
	Output->x = StreamHandleGetFloat(Input);
	Output->y = StreamHandleGetFloat(Input);
	Output->z = StreamHandleGetFloat(Input);
}

/**********************************

	Convenience routine to save a 3D floating point matrix

**********************************/

void BURGERCALL StreamHandleGetMatrix3D(StreamHandle_t *Input,Matrix3D_t *Output)
{
	Output->x.x = StreamHandleGetFloat(Input);
	Output->x.y = StreamHandleGetFloat(Input);
	Output->x.z = StreamHandleGetFloat(Input);
	Output->y.x = StreamHandleGetFloat(Input);
	Output->y.y = StreamHandleGetFloat(Input);
	Output->y.z = StreamHandleGetFloat(Input);
	Output->z.x = StreamHandleGetFloat(Input);
	Output->z.y = StreamHandleGetFloat(Input);
	Output->z.z = StreamHandleGetFloat(Input);
}

/**********************************

	Convenience routine to get a euler angle record
	
**********************************/

void BURGERCALL StreamHandleGetEuler(StreamHandle_t *Input,struct Euler_t *Output)
{
	Output->x = (short)StreamHandleGetShort(Input);
	Output->y = (short)StreamHandleGetShort(Input);
	Output->z = (short)StreamHandleGetShort(Input);
}

/**********************************

	Get 2 bytes of data from a stream (Big Endian)

**********************************/

Word BURGERCALL StreamHandleGetShortMoto(StreamHandle_t *Input)
{
	Word32 Offset;
	Word32 NewOffset;
	
	Offset = Input->Mark;
	NewOffset = Offset+2;
	if (Input->EOFMark>=NewOffset) {
		Word8 *WorkPtr;
		Input->Mark = NewOffset;
		WorkPtr = Input->DataHandle[0]+Offset;		/* Make a pointer */
#if defined(__INTEL__)
		return Burger::LoadBig(((Word16 *)WorkPtr)[0]);
#else
		return ((Word)WorkPtr[0]<<8)+WorkPtr[1];
#endif
	}
	Input->ErrorFlag = TRUE;
	return 0;
}

/**********************************

	Get 4 bytes of data from a stream (Big Endian)

**********************************/

Word32 BURGERCALL StreamHandleGetLongMoto(StreamHandle_t *Input)
{
	Word32 Offset;
	Word32 NewOffset;
	
	Offset = Input->Mark;
	NewOffset = Offset+4;
	if (Input->EOFMark>=NewOffset) {
		Word8 *WorkPtr;
		Input->Mark = NewOffset;
		WorkPtr = Input->DataHandle[0]+Offset;		/* Make a pointer */
#if defined(__INTEL__)
		return Burger::LoadBig(((Word32 *)WorkPtr)[0]);
#else
		return ((Word32)WorkPtr[0]<<24)+((Word32)WorkPtr[1]<<16)+
			((Word32)WorkPtr[2]<<8)+WorkPtr[3];
#endif
	}
	Input->ErrorFlag = TRUE;
	return 0;
}

/**********************************

	Get 4 bytes of data from a stream (Big Endian)

**********************************/

float BURGERCALL StreamHandleGetFloatMoto(StreamHandle_t *Input)
{
	Word32 Offset;
	Word32 NewOffset;
	
	Offset = Input->Mark;
	NewOffset = Offset+4;
	if (Input->EOFMark>=NewOffset) {
		Word8 *WorkPtr;
		Word32 MemFloat;
		Input->Mark = NewOffset;
		WorkPtr = Input->DataHandle[0]+Offset;		/* Make a pointer */
#if defined(__INTEL__)
		MemFloat = Burger::LoadBig(((Word32 *)WorkPtr)[0]);
#else
		MemFloat = ((Word32)WorkPtr[0]<<24)+((Word32)WorkPtr[1]<<16)+
			((Word32)WorkPtr[2]<<8)+WorkPtr[3];
#endif
		return ((float *)&MemFloat)[0];
	}
	Input->ErrorFlag = TRUE;
	return 0;
}

/**********************************

	Get 8 bytes of data from a stream (Big Endian)

**********************************/

double BURGERCALL StreamHandleGetDoubleMoto(StreamHandle_t *Input)
{
	Word32 Offset;
	Word32 NewOffset;
	
	Offset = Input->Mark;
	NewOffset = Offset+8;
	if (Input->EOFMark>=NewOffset) {
		Word8 *WorkPtr;
		Word32 MemFloat[2];
		Input->Mark = NewOffset;

		WorkPtr = Input->DataHandle[0]+Offset;		/* Make a pointer */
#if defined(__INTEL__)
		MemFloat[1] = Burger::LoadBig(((Word32 *)WorkPtr)[0]);
		MemFloat[0] = Burger::LoadBig(((Word32 *)WorkPtr)[1]);
#elif defined(__LITTLEENDIAN__)
		MemFloat[1] = ((Word32)WorkPtr[0]<<24)+((Word32)WorkPtr[1]<<16)+
			((Word32)WorkPtr[2]<<8)+WorkPtr[3];
		MemFloat[0] = ((Word32)WorkPtr[0]<<24)+((Word32)WorkPtr[5]<<16)+
			((Word32)WorkPtr[6]<<8)+WorkPtr[7];
#else
		MemFloat[0] = ((Word32)WorkPtr[0]<<24)+((Word32)WorkPtr[1]<<16)+
			((Word32)WorkPtr[2]<<8)+WorkPtr[3];
		MemFloat[1] = ((Word32)WorkPtr[0]<<24)+((Word32)WorkPtr[5]<<16)+
			((Word32)WorkPtr[6]<<8)+WorkPtr[7];
#endif
		return ((double *)(&MemFloat[0]))[0];	
	}
	Input->ErrorFlag = TRUE;
	return 0;
}

/**********************************

	Convenience routine to save a 2D floating point matrix
	Big endian
	
**********************************/

void BURGERCALL StreamHandleGetVector2DMoto(StreamHandle_t *Input,Vector2D_t *Output)
{
	Output->x = StreamHandleGetFloatMoto(Input);
	Output->y = StreamHandleGetFloatMoto(Input);
}

/**********************************

	Convenience routine to save a 3D floating point matrix
	Big endian
	
**********************************/

void BURGERCALL StreamHandleGetVector3DMoto(StreamHandle_t *Input,Vector3D_t *Output)
{
	Output->x = StreamHandleGetFloatMoto(Input);
	Output->y = StreamHandleGetFloatMoto(Input);
	Output->z = StreamHandleGetFloatMoto(Input);
}


/**********************************

	Convenience routine to save a 3D floating point matrix
	Big endian
	
**********************************/

void BURGERCALL StreamHandleGetMatrix3DMoto(StreamHandle_t *Input,Matrix3D_t *Output)
{
	Output->x.x = StreamHandleGetFloatMoto(Input);
	Output->x.y = StreamHandleGetFloatMoto(Input);
	Output->x.z = StreamHandleGetFloatMoto(Input);
	Output->y.x = StreamHandleGetFloatMoto(Input);
	Output->y.y = StreamHandleGetFloatMoto(Input);
	Output->y.z = StreamHandleGetFloatMoto(Input);
	Output->z.x = StreamHandleGetFloatMoto(Input);
	Output->z.y = StreamHandleGetFloatMoto(Input);
	Output->z.z = StreamHandleGetFloatMoto(Input);
}

/**********************************

	Convenience routine to get a euler angle record
	
**********************************/

void BURGERCALL StreamHandleGetEulerMoto(StreamHandle_t *Input,struct Euler_t *Output)
{
	Output->x = (short)StreamHandleGetShortMoto(Input);
	Output->y = (short)StreamHandleGetShortMoto(Input);
	Output->z = (short)StreamHandleGetShortMoto(Input);
}

/**********************************

	Get arbitrary data from a stream

**********************************/

void BURGERCALL StreamHandleGetMem(StreamHandle_t *Input,void *DestPtr,Word32 Length)
{
	Word32 Offset;
	Word32 NewOffset;
	
	if (Length) {			/* Any data requested? */
		Offset = Input->Mark;
		NewOffset = Offset+Length;		/* Will it overrun? */
		if (Input->EOFMark>=NewOffset) {	/* Out of bounds? */
			Input->Mark = NewOffset;		/* Save new length */
			FastMemCpy(DestPtr,Input->DataHandle[0]+Offset,Length);	/* Get it */
			return;
		}
		Input->ErrorFlag = TRUE;		/* Overrun error! */
		FastMemSet(DestPtr,0,Length);	/* Zap the dest buffer */
	}
}

/**********************************

	Get a "C" string from a stream
	I can accept EXTRACTSTRDELIMITLF, EXTRACTSTRHANDLE and 
	EXTRACTSTRNOTRANSLATE flags

**********************************/

void * BURGERCALL StreamHandleGetString(StreamHandle_t *Input,Word Flags)
{
	Word32 MaxLen;	/* Length of the input buffer */
	void *DestPtr;		/* Result pointer */
	Word32 Offset;	/* Offset cache */
	Word32 LenInput;	/* Memory variable for pointer reference */

	Offset = Input->Mark;
	MaxLen = Input->EOFMark-Offset;	/* Size of remaining data */
	if (MaxLen) {
		LenInput = MaxLen;			/* Copy to memory */
		DestPtr = ExtractAString((char *)LockAHandle((void **)Input->DataHandle)+Offset,&LenInput,Flags);		/* Pull out the string */
		UnlockAHandle((void **)Input->DataHandle);		/* Unlock the input */
		Input->Mark = Offset+LenInput;		/* Accept the data */
		return DestPtr;		/* Return result of ExtractAString */
	}
	Input->ErrorFlag = TRUE;	/* No data... */
	return 0;
}

/**********************************

	Get a "C" string from a stream
	I can accept EXTRACTSTRDELIMITLF and 
	EXTRACTSTRNOTRANSLATE flags

**********************************/

void BURGERCALL StreamHandleGetString2(StreamHandle_t *Input,Word Flags,char *DestBuffer,Word32 MaxLength)
{
	Word32 MaxLen;	/* Length of the input buffer */
	Word32 Offset;	/* Offset cache */
	Word32 LenInput;	/* Memory variable for pointer reference */

	Offset = Input->Mark;
	MaxLen = Input->EOFMark-Offset;	/* Size of remaining data */
	if (MaxLen) {
		LenInput = MaxLen;			/* Copy to memory */
		ExtractAString2((char *)LockAHandle((void **)Input->DataHandle)+Offset,&LenInput,Flags,DestBuffer,MaxLength);		/* Pull out the string */
		UnlockAHandle((void **)Input->DataHandle);		/* Unlock the input */
		Input->Mark = Offset+LenInput;		/* Accept the data */
		return;			/* Return NOW */
	}
	DestBuffer[0] = 0;			/* Zap the output buffer */
	Input->ErrorFlag = TRUE;	/* No data... */
}

/**********************************

	Save a byte of data into the stream

**********************************/

void BURGERCALL StreamHandlePutByte(StreamHandle_t *Input,Word Val)
{
	Word32 Offset;
	Word32 NewOffset;
	
	Offset = Input->Mark;
	NewOffset = Offset+1;
	do {
		if (Input->BufferSize>=NewOffset) {	/* Enough space? */
			Input->Mark = NewOffset;
			if (Input->EOFMark<NewOffset) {
				Input->EOFMark = NewOffset;
			}
			((Word8 *)(Input->DataHandle[0]+Offset))[0] = (Word8)Val;
			return;
		}
		Input->BufferSize += 8192;		/* Make more space */
	} while ((Input->DataHandle =
		(Word8 **)ResizeAHandle((void **)Input->DataHandle,Input->BufferSize))!=0);
	Input->BufferSize = 0;		/* I am bogus! */
	Input->Mark = 0;
	Input->EOFMark = 0;
	Input->ErrorFlag = TRUE;	/* Oh oh, data is invalid! */
}


/**********************************

	Save 2 bytes of data (little endian) into the stream

**********************************/

void BURGERCALL StreamHandlePutShort(StreamHandle_t *Input,Word Val)
{
	Word32 Offset;
	Word32 NewOffset;
	
	Offset = Input->Mark;
	NewOffset = Offset+2;
	do {
		if (Input->BufferSize>=NewOffset) {	/* Enough space? */
			Word8 *WorkPtr;
			Input->Mark = NewOffset;
			if (Input->EOFMark<NewOffset) {
				Input->EOFMark = NewOffset;
			}
			WorkPtr = Input->DataHandle[0]+Offset;
#if defined(__INTEL__)
			((Word16 *)WorkPtr)[0] = Burger::LoadLittle(static_cast<Word16>(Val));
#else
			WorkPtr[0] = (Word8)Val;
			WorkPtr[1] = (Word8)(Val>>8);
#endif
			return;
		}
		Input->BufferSize += 8192;		/* Make more space */
	} while ((Input->DataHandle =
		(Word8 **)ResizeAHandle((void **)Input->DataHandle,Input->BufferSize))!=0);
	Input->BufferSize = 0;		/* I am bogus! */
	Input->Mark = 0;
	Input->EOFMark = 0;
	Input->ErrorFlag = TRUE;	/* Oh oh, data is invalid! */
}

/**********************************

	Save 4 bytes of data (little endian) into the stream

**********************************/

void BURGERCALL StreamHandlePutLong(StreamHandle_t *Input,Word32 Val)
{
	Word32 Offset;
	Word32 NewOffset;
	
	Offset = Input->Mark;
	NewOffset = Offset+4;
	do {
		if (Input->BufferSize>=NewOffset) {	/* Enough space? */
			Word8 *WorkPtr;
			Input->Mark = NewOffset;
			if (Input->EOFMark<NewOffset) {
				Input->EOFMark = NewOffset;
			}
			WorkPtr = Input->DataHandle[0]+Offset;
#if defined(__INTEL__)
			((Word32 *)WorkPtr)[0] = Burger::LoadLittle(Val);
#else
			WorkPtr[0] = (Word8)Val;
			WorkPtr[1] = (Word8)(Val>>8);
			WorkPtr[2] = (Word8)(Val>>16);
			WorkPtr[3] = (Word8)(Val>>24);
#endif
			return;
		}
		Input->BufferSize += 8192;		/* Make more space */
	} while ((Input->DataHandle =
		(Word8 **)ResizeAHandle((void **)Input->DataHandle,Input->BufferSize))!=0);
	Input->BufferSize = 0;		/* I am bogus! */
	Input->Mark = 0;
	Input->EOFMark = 0;
	Input->ErrorFlag = TRUE;	/* Oh oh, data is invalid! */
}

/**********************************

	Save 4 bytes of data (little endian) into the stream

**********************************/

void BURGERCALL StreamHandlePutFloat(StreamHandle_t *Input,float Val)
{
	Word32 Offset;
	Word32 NewOffset;
	
	Offset = Input->Mark;
	NewOffset = Offset+4;
	do {
		if (Input->BufferSize>=NewOffset) {	/* Enough space? */
			Word8 *WorkPtr;
			Input->Mark = NewOffset;
			if (Input->EOFMark<NewOffset) {
				Input->EOFMark = NewOffset;
			}
			WorkPtr = Input->DataHandle[0]+Offset;
#if defined(__INTEL__)
			((Word32 *)WorkPtr)[0] = Burger::LoadLittle(((Word32 *)(&Val))[0]);
#else
			/* Big endian version */
			Offset = ((Word32 *)(&Val))[0];
			WorkPtr[0] = (Word8)Offset;
			WorkPtr[1] = (Word8)(Offset>>8);
			WorkPtr[2] = (Word8)(Offset>>16);
			WorkPtr[3] = (Word8)(Offset>>24);
#endif	 	
			return;
		}
		Input->BufferSize += 8192;		/* Make more space */
	} while ((Input->DataHandle =
		(Word8 **)ResizeAHandle((void **)Input->DataHandle,Input->BufferSize))!=0);
	Input->BufferSize = 0;		/* I am bogus! */
	Input->Mark = 0;
	Input->EOFMark = 0;
	Input->ErrorFlag = TRUE;	/* Oh oh, data is invalid! */
}


/**********************************

	Save 8 bytes of data (little endian) into the stream

**********************************/

void BURGERCALL StreamHandlePutDouble(StreamHandle_t *Input,double Val)
{
	Word32 Offset;
	Word32 NewOffset;
	
	Offset = Input->Mark;
	NewOffset = Offset+8;
	do {
		if (Input->BufferSize>=NewOffset) {	/* Enough space? */
			Word8 *WorkPtr;
			Input->Mark = NewOffset;
			if (Input->EOFMark<NewOffset) {
				Input->EOFMark = NewOffset;
			}
			WorkPtr = Input->DataHandle[0]+Offset;
#if defined(__INTEL__)
			((double *)WorkPtr)[0] = Val;
#else
			{
				Word32 MemLong[2];
				((double *)&MemLong[0])[0] = Val;
#if defined(__LITTLEENDIAN__)
				Offset = MemLong[0];
				WorkPtr[0] = (Word8)Offset;
				WorkPtr[1] = (Word8)(Offset>>8);
				WorkPtr[2] = (Word8)(Offset>>16);
				WorkPtr[3] = (Word8)(Offset>>24);
				Offset = MemLong[1];
				WorkPtr[4] = (Word8)Offset;
				WorkPtr[5] = (Word8)(Offset>>8);
				WorkPtr[6] = (Word8)(Offset>>16);
				WorkPtr[7] = (Word8)(Offset>>24);
#else
				Offset = MemLong[1];
				WorkPtr[0] = (Word8)Offset;
				WorkPtr[1] = (Word8)(Offset>>8);
				WorkPtr[2] = (Word8)(Offset>>16);
				WorkPtr[3] = (Word8)(Offset>>24);
				Offset = MemLong[0];
				WorkPtr[4] = (Word8)Offset;
				WorkPtr[5] = (Word8)(Offset>>8);
				WorkPtr[6] = (Word8)(Offset>>16);
				WorkPtr[7] = (Word8)(Offset>>24);
#endif
			}
#endif	 	
			return;
		}
		Input->BufferSize += 8192;		/* Make more space */
	} while ((Input->DataHandle =
		(Word8 **)ResizeAHandle((void **)Input->DataHandle,Input->BufferSize))!=0);
	Input->BufferSize = 0;		/* I am bogus! */
	Input->Mark = 0;
	Input->EOFMark = 0;
	Input->ErrorFlag = TRUE;	/* Oh oh, data is invalid! */
}


/**********************************

	Convenience routine to save a 2D floating point vector

**********************************/

void BURGERCALL StreamHandlePutVector2D(StreamHandle_t *Input,const Vector2D_t *Val)
{
	StreamHandlePutFloat(Input,Val->x);
	StreamHandlePutFloat(Input,Val->y);
}


/**********************************

	Convenience routine to save a 3D floating point vector

**********************************/

void BURGERCALL StreamHandlePutVector3D(StreamHandle_t *Input,const Vector3D_t *Val)
{
	StreamHandlePutFloat(Input,Val->x);
	StreamHandlePutFloat(Input,Val->y);
	StreamHandlePutFloat(Input,Val->z);
}

/**********************************

	Convenience routine to save a 3D floating point matrix

**********************************/

void BURGERCALL StreamHandlePutMatrix3D(StreamHandle_t *Input,const Matrix3D_t *Val)
{
	StreamHandlePutFloat(Input,Val->x.x);
	StreamHandlePutFloat(Input,Val->x.y);
	StreamHandlePutFloat(Input,Val->x.z);
	StreamHandlePutFloat(Input,Val->y.x);
	StreamHandlePutFloat(Input,Val->y.y);
	StreamHandlePutFloat(Input,Val->y.z);
	StreamHandlePutFloat(Input,Val->z.x);
	StreamHandlePutFloat(Input,Val->z.y);
	StreamHandlePutFloat(Input,Val->z.z);
}

/**********************************

	Convenience routine to put a euler angle record
	
**********************************/

void BURGERCALL StreamHandlePutEuler(StreamHandle_t *Input,const struct Euler_t *Val)
{
	StreamHandlePutShort(Input,Val->x);
	StreamHandlePutShort(Input,Val->y);
	StreamHandlePutShort(Input,Val->z);
}

/**********************************

	Save 2 bytes of data (big endian) into the stream

**********************************/

void BURGERCALL StreamHandlePutShortMoto(StreamHandle_t *Input,Word Val)
{
	Word32 Offset;
	Word32 NewOffset;
	
	Offset = Input->Mark;
	NewOffset = Offset+2;
	do {
		if (Input->BufferSize>=NewOffset) {	/* Enough space? */
			Word8 *WorkPtr;
			Input->Mark = NewOffset;
			if (Input->EOFMark<NewOffset) {
				Input->EOFMark = NewOffset;
			}
			WorkPtr = Input->DataHandle[0]+Offset;
#if defined(__INTEL__)
			((Word16 *)WorkPtr)[0] = Burger::LoadBig((Word16)Val);
#else
			WorkPtr[0] = (Word8)(Val>>8);
			WorkPtr[1] = (Word8)Val;
#endif
			return;
		}
		Input->BufferSize += 8192;		/* Make more space */
	} while ((Input->DataHandle =
		(Word8 **)ResizeAHandle((void **)Input->DataHandle,Input->BufferSize))!=0);
	Input->BufferSize = 0;		/* I am bogus! */
	Input->Mark = 0;
	Input->EOFMark = 0;
	Input->ErrorFlag = TRUE;	/* Oh oh, data is invalid! */
}


/**********************************

	Save 4 bytes of data (big endian) into the stream

**********************************/

void BURGERCALL StreamHandlePutLongMoto(StreamHandle_t *Input,Word32 Val)
{
	Word32 Offset;
	Word32 NewOffset;
	
	Offset = Input->Mark;
	NewOffset = Offset+4;
	do {
		if (Input->BufferSize>=NewOffset) {	/* Enough space? */
			Word8 *WorkPtr;
			Input->Mark = NewOffset;
			if (Input->EOFMark<NewOffset) {
				Input->EOFMark = NewOffset;
			}
			WorkPtr = Input->DataHandle[0]+Offset;
#if defined(__INTEL__)
			((Word32 *)WorkPtr)[0] = Burger::LoadBig(Val);
#else
			WorkPtr[0] = (Word8)(Val>>24);
			WorkPtr[1] = (Word8)(Val>>16);
			WorkPtr[2] = (Word8)(Val>>8);
			WorkPtr[3] = (Word8)Val;
#endif
			return;
		}
		Input->BufferSize += 8192;		/* Make more space */
	} while ((Input->DataHandle =
		(Word8 **)ResizeAHandle((void **)Input->DataHandle,Input->BufferSize))!=0);
	Input->BufferSize = 0;		/* I am bogus! */
	Input->Mark = 0;
	Input->EOFMark = 0;
	Input->ErrorFlag = TRUE;	/* Oh oh, data is invalid! */
}
/**********************************

	Save 4 bytes of data (big endian) into the stream

**********************************/

void BURGERCALL StreamHandlePutFloatMoto(StreamHandle_t *Input,float Val)
{
	Word32 Offset;
	Word32 NewOffset;
	
	Offset = Input->Mark;
	NewOffset = Offset+4;
	do {
		if (Input->BufferSize>=NewOffset) {	/* Enough space? */
			Word8 *WorkPtr;
			Input->Mark = NewOffset;
			if (Input->EOFMark<NewOffset) {
				Input->EOFMark = NewOffset;
			}
			WorkPtr = Input->DataHandle[0]+Offset;
#if defined(__INTEL__)
			((Word32 *)WorkPtr)[0] = Burger::LoadBig(((Word32 *)(&Val))[0]);
#else
			/* Big endian version */
			Offset = ((Word32 *)(&Val))[0];
			WorkPtr[0] = (Word8)(Offset>>24);
			WorkPtr[1] = (Word8)(Offset>>16);
			WorkPtr[2] = (Word8)(Offset>>8);
			WorkPtr[3] = (Word8)Offset;
#endif	 	
			return;
		}
		Input->BufferSize += 8192;		/* Make more space */
	} while ((Input->DataHandle =
		(Word8 **)ResizeAHandle((void **)Input->DataHandle,Input->BufferSize))!=0);
	Input->BufferSize = 0;		/* I am bogus! */
	Input->Mark = 0;
	Input->EOFMark = 0;
	Input->ErrorFlag = TRUE;	/* Oh oh, data is invalid! */
}

/**********************************

	Save 8 bytes of data (big endian) into the stream

**********************************/

void BURGERCALL StreamHandlePutDoubleMoto(StreamHandle_t *Input,double Val)
{
	Word32 Offset;
	Word32 NewOffset;

	Offset = Input->Mark;
	NewOffset = Offset+8;
	do {
		if (Input->BufferSize>=NewOffset) {	/* Enough space? */
			Word8 *WorkPtr;
			Input->Mark = NewOffset;
			if (Input->EOFMark<NewOffset) {
				Input->EOFMark = NewOffset;
			}
			WorkPtr = Input->DataHandle[0]+Offset;
			{
				Word32 MemLong[2];
				((double *)&MemLong[0])[0] = Val;
#if defined(__LITTLEENDIAN__)
				Offset = MemLong[1];
				WorkPtr[0] = (Word8)(Offset>>24);
				WorkPtr[1] = (Word8)(Offset>>16);
				WorkPtr[2] = (Word8)(Offset>>8);
				WorkPtr[3] = (Word8)Offset;
				Offset = MemLong[0];
				WorkPtr[4] = (Word8)(Offset>>24);
				WorkPtr[5] = (Word8)(Offset>>16);
				WorkPtr[6] = (Word8)(Offset>>8);
				WorkPtr[7] = (Word8)Offset;
#else
				Offset = MemLong[0];
				WorkPtr[0] = (Word8)(Offset>>24);
				WorkPtr[1] = (Word8)(Offset>>16);
				WorkPtr[2] = (Word8)(Offset>>8);
				WorkPtr[3] = (Word8)Offset;
				Offset = MemLong[1];
				WorkPtr[4] = (Word8)(Offset>>24);
				WorkPtr[5] = (Word8)(Offset>>16);
				WorkPtr[6] = (Word8)(Offset>>8);
				WorkPtr[7] = (Word8)Offset;
#endif
			} 	
			return;
		}
		Input->BufferSize += 8192;		/* Make more space */
	} while ((Input->DataHandle =
		(Word8 **)ResizeAHandle((void **)Input->DataHandle,Input->BufferSize))!=0);
	Input->BufferSize = 0;		/* I am bogus! */
	Input->Mark = 0;
	Input->EOFMark = 0;
	Input->ErrorFlag = TRUE;	/* Oh oh, data is invalid! */
}


/**********************************

	Convenience routine to save a 2D floating point vector
	Big Endian

**********************************/

void BURGERCALL StreamHandlePutVector2DMoto(StreamHandle_t *Input,const Vector2D_t *Val)
{
	StreamHandlePutFloatMoto(Input,Val->x);
	StreamHandlePutFloatMoto(Input,Val->y);
}


/**********************************

	Convenience routine to save a 3D floating point vector
	Big Endian
	
**********************************/

void BURGERCALL StreamHandlePutVector3DMoto(StreamHandle_t *Input,const Vector3D_t *Val)
{
	StreamHandlePutFloatMoto(Input,Val->x);
	StreamHandlePutFloatMoto(Input,Val->y);
	StreamHandlePutFloatMoto(Input,Val->z);
}

/**********************************

	Convenience routine to save a 3D floating point matrix
	Big Endian

**********************************/

void BURGERCALL StreamHandlePutMatrix3DMoto(StreamHandle_t *Input,const Matrix3D_t *Val)
{
	StreamHandlePutFloatMoto(Input,Val->x.x);
	StreamHandlePutFloatMoto(Input,Val->x.y);
	StreamHandlePutFloatMoto(Input,Val->x.z);
	StreamHandlePutFloatMoto(Input,Val->y.x);
	StreamHandlePutFloatMoto(Input,Val->y.y);
	StreamHandlePutFloatMoto(Input,Val->y.z);
	StreamHandlePutFloatMoto(Input,Val->z.x);
	StreamHandlePutFloatMoto(Input,Val->z.y);
	StreamHandlePutFloatMoto(Input,Val->z.z);
}

/**********************************

	Convenience routine to put a euler angle record
	
**********************************/

void BURGERCALL StreamHandlePutEulerMoto(StreamHandle_t *Input,const struct Euler_t *Val)
{
	StreamHandlePutShortMoto(Input,Val->x);
	StreamHandlePutShortMoto(Input,Val->y);
	StreamHandlePutShortMoto(Input,Val->z);
}

/**********************************

	Save an arbitrary stream of bytes into the stream

**********************************/

void BURGERCALL StreamHandlePutMem(StreamHandle_t *Input,const void *SrcPtr,Word32 Length)
{
	Word32 Offset;
	Word32 NewOffset;
	
	if (Length) {
		Offset = Input->Mark;
		NewOffset = Offset+Length;
		do {
			if (Input->BufferSize>=NewOffset) {	/* Enough space? */
				Input->Mark = NewOffset;
				if (Input->EOFMark<NewOffset) {
					Input->EOFMark = NewOffset;
				}
				FastMemCpy(Input->DataHandle[0]+Offset,SrcPtr,Length);
				return;
			}
			Input->BufferSize += 8192+Length;		/* Make more space */
		} while ((Input->DataHandle =
			(Word8 **)ResizeAHandle((void **)Input->DataHandle,Input->BufferSize))!=0);
		Input->BufferSize = 0;		/* I am bogus! */
		Input->Mark = 0;
		Input->EOFMark = 0;
		Input->ErrorFlag = TRUE;	/* Oh oh, data is invalid! */
	}
}

/**********************************

	Save an arbitrary stream of bytes into the stream

**********************************/

void BURGERCALL StreamHandlePutString(StreamHandle_t *Input,const void *SrcPtr)
{
	Word32 Length;		/* Length of the string */
	Word32 Offset;
	Word32 NewOffset;
		
	Length = strlen((char *)SrcPtr)+1;		/* Get the length of the string */
	Offset = Input->Mark;	/* Set the offset */
	NewOffset = Offset+Length;
	do {
		if (Input->BufferSize>=NewOffset) {	/* Enough space? */
			Input->Mark = NewOffset;
			if (Input->EOFMark<NewOffset) {	/* Adjust the EOFMark */
				Input->EOFMark = NewOffset;
			}
			FastMemCpy(((Word8 *)*Input->DataHandle)+Offset,SrcPtr,Length);	/* With or without zero */
			return;
		}
		Input->BufferSize += Length+8192;		/* Make more space (Plus extra to reduce overhead */
	} while ((Input->DataHandle = (Word8 **)ResizeAHandle((void **)Input->DataHandle,Input->BufferSize))!=0);		/* Error in resize? */
	Input->BufferSize = 0;		/* I am bogus! */
	Input->Mark = 0;
	Input->EOFMark = 0;
	Input->ErrorFlag = TRUE;	/* Oh oh, data is invalid! */
}

/**********************************

	Save an arbitrary stream of bytes into the stream

**********************************/

void BURGERCALL StreamHandlePutStringNoZero(StreamHandle_t *Input,const void *SrcPtr)
{
	Word32 Length;		/* Length of the string */
	Word32 Offset;
	Word32 NewOffset;
		
	Length = strlen((char *)SrcPtr);		/* Get the length of the string */
	Offset = Input->Mark;	/* Set the offset */
	NewOffset = Offset+Length;
	do {
		if (Input->BufferSize>=NewOffset) {	/* Enough space? */
			Input->Mark = NewOffset;
			if (Input->EOFMark<NewOffset) {
				Input->EOFMark = NewOffset;
			}
			FastMemCpy(((Word8 *)*Input->DataHandle)+Offset,SrcPtr,Length);	/* With or without zero */
			return;
		}
		Input->BufferSize += Length+8192;		/* Make more space (Plus extra to reduce overhead */
	} while ((Input->DataHandle = (Word8 **)ResizeAHandle((void **)Input->DataHandle,Input->BufferSize))!=0);		/* Error in resize? */
	Input->BufferSize = 0;
	Input->Mark = 0;
	Input->EOFMark = 0;
	Input->ErrorFlag = TRUE;		/* Oh oh, fatal error! */
}

/**********************************

	Set the new file position mark
	-1 means end of data
	
**********************************/

void BURGERCALL StreamHandleSetMark(StreamHandle_t *Input,Word32 NewMark)
{
	if (NewMark==(Word32)-1 || Input->EOFMark<NewMark) {	/* Set the mark to the end of the data */
		NewMark = Input->EOFMark;	/* Set to EOF */
	}
	Input->Mark = NewMark;		/* Save the new mark */
}

/**********************************

	Save an arbitrary stream of bytes into the stream

**********************************/

void BURGERCALL StreamHandleSetSize(StreamHandle_t *Input,Word32 Size)
{
	if (Input->BufferSize!=Size) {
		if (Size) {
			if ((Input->DataHandle = (Word8 **)ResizeAHandle((void **)Input->DataHandle,Size))!=0) {
				Input->BufferSize = Size;
				if (Size<Input->EOFMark) {		/* Truncate EOF */
					Input->EOFMark = Size;
				}
				if (Size<Input->Mark) {			/* Truncate Mark */
					Input->Mark = Size;
				}
				return;
			}
		} else {
			DeallocAHandle((void **)Input->DataHandle);
			Input->DataHandle = 0;		/* Sure it's clear */
		}
		Input->BufferSize = 0;		/* I am bogus! */
		Input->Mark = 0;
		Input->EOFMark = 0;
		Input->ErrorFlag = TRUE;	/* Oh oh, data is invalid! */
	}
}

/**********************************

	Adjust the data relative to the current mark.
	Negative numbers go backwards

**********************************/

void BURGERCALL StreamHandleSkip(StreamHandle_t *Input,long Offset)
{
	Offset += Input->Mark;	/* Skip the pointer */
	if (Offset<0) {			/* Less than zero? */
		Offset = 0;			/* Set to the beginning */
	}
	if ((long)Input->EOFMark<Offset) {	/* Set the mark to the end of the data */
		Offset = Input->EOFMark;	/* Set to EOF */
	}
	Input->Mark = Offset;		/* Save the new mark */
}

/**********************************

	Pretend I read a string.

**********************************/

void BURGERCALL StreamHandleSkipString(StreamHandle_t *Input)
{
	char Temp[4];		/* Meaningless buffer */
	
	StreamHandleGetString2(Input,0,Temp,sizeof(Temp));	/* Read a string but do nothing with it */
}


/**********************************

	Locks down the input stream and returns
	a pointer at the current mark.
	A NULL is returned in case of bad data

**********************************/

void *BURGERCALL StreamHandleLock(StreamHandle_t *Input)
{	
	if (!Input->ErrorFlag && Input->DataHandle) {		/* Valid handle? */
		return (void *)(((Word8 *)LockAHandle((void **)Input->DataHandle))+Input->Mark);
	}
	return 0;
}


/**********************************

	Reserve some memory and locks down the input stream and returns
	a pointer at the current mark.
	A NULL is returned in case of error or bad data

**********************************/

void * BURGERCALL StreamHandleLockExpand(StreamHandle_t *Input,Word32 Size)
{
	StreamHandleExpand(Input,Size);				/* Reserve the memory */
	if (!Input->ErrorFlag && Input->DataHandle) {		/* Valid handle? */
		return (void *)(((Word8 *)LockAHandle((void **)Input->DataHandle))+Input->Mark);
	}
	return 0;		/* Bad!! */

}

/**********************************

	The user directly stored data into the stream.
	Regain control and set my flags.

**********************************/

void BURGERCALL StreamHandleUnlock(StreamHandle_t *Input,const void *EndPtr)
{
	Word32 NewOffset;
	const Word8 *StartPtr;
			
	StartPtr = Input->DataHandle[0];		/* Get the buffer starting pointer */
	NewOffset = (Word8 *)EndPtr - StartPtr;	/* Adjust from the end pointer */
	UnlockAHandle((void **)Input->DataHandle);		/* Unlock the handle */
	if (Input->BufferSize>=NewOffset) {		/* Enough space? */
		Input->Mark = NewOffset;			/* Set the new file mark */
		if (Input->EOFMark<NewOffset) {		/* Adjust the EOF */
			Input->EOFMark = NewOffset;
		}
		return;				/* Exit ok */
	}
	Input->ErrorFlag = TRUE;		/* Oh man, you really screwed up! */
}

/**********************************

	Grow the buffer to accomdate future data.

**********************************/

void BURGERCALL StreamHandleExpand(StreamHandle_t *Input,Word32 Size)
{	
	if (Size) {			/* Any data to grow? */
		Size = Input->Mark+Size;		/* Get the possible end of buffer */
		if (Input->BufferSize<Size) {	/* Enough space? */
			Input->BufferSize = Size+8192;		/* Make more space */
			if ((Input->DataHandle =
				(Word8 **)ResizeAHandle((void **)Input->DataHandle,Input->BufferSize))==0) {
				Input->BufferSize = 0;		/* I am bogus! */
				Input->Mark = 0;
				Input->EOFMark = 0;
				Input->ErrorFlag = TRUE;	/* Oh oh, data is invalid! */
			}
		}
	}
}

