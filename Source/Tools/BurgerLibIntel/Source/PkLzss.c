#include "PkPack.h"
#include <BREndian.h>
#include "MmMemory.h"
#include "ClStdLib.h"
#include <string.h>

#define N 4096			/* size of ring buffer */
#define F 18			/* upper limit for MatchSize */
#define THRESHOLD 2		/* encode string into position and length */
						/* if MatchSize is greater than this */
#define NOTUSED N			/* index for root of binary search trees */

typedef struct LZSSState_t {
	Word8 RingBuffer[N+F-1];		/* ring buffer of size N, */
						/* With extra F-1 bytes to facilitate string comparison */

	Word MatchOffset;	/* Offset of string match */
	Word MatchSize;		/* Length of string match 0-18 */
	/* of longest match. These are set by the InsertNode() procedure. */

	Word LeftBranch[N + 1];		/* Left child */
	Word RightBranch[N + 1 + 256];	/* Right child / Hash table */
	Word RootBranch[N + 1];		/* Roots for each binary tree */
	/* left & right children & parents -- These constitute binary search trees. */
} LZSSState_t;	

/**********************************

	Init the binary tree needed for the compression system

	For i = 0 to N - 1, RightBranch[i] and LeftBranch[i] will be the right and
	left children of node i. These nodes need not be initialized.
	Also, RootBranch[i] is the parent of node i. These are initialized to
	NOTUSED (= N), which stands for 'not used.'
	For i = 0 to 255, RightBranch[N + i + 1] is the root of the tree
	for strings that begin with character i. These are initialized
	to NOTUSED. Note there are 256 trees.

**********************************/

static INLINECALL void InitTree(LZSSState_t *Input)
{
	Word Count;
	Word *BranchPtr;

	Count = 256;
	BranchPtr = &Input->RightBranch[N+1];
	do {
		BranchPtr[0] = NOTUSED;		/* Zap the hash table */
		++BranchPtr;
	} while (--Count);

	BranchPtr = &Input->RootBranch[0];
	Count = N;
	do {
		BranchPtr[0] = NOTUSED;		/* Kill the root indexs */
		++BranchPtr;
	} while (--Count);
}

/**********************************

	Removes a node from the binary tree

**********************************/

static void BURGERCALL DeleteNode(LZSSState_t *Input,Word NodeNum)
{
	Word ClipNum;
	Word Cache,Cache2;

	if (Input->RootBranch[NodeNum] == NOTUSED) {		/* Valid node? */
		return;				/* not in tree */
	}
	if (Input->RightBranch[NodeNum] == NOTUSED) {	/* Is there a right branch? */
		ClipNum = Input->LeftBranch[NodeNum];		/* Clip just the left branch */
	} else if (Input->LeftBranch[NodeNum] == NOTUSED) {	/* Is there a left branch? */
		ClipNum = Input->RightBranch[NodeNum];		/* Clip just the right branch */
	} else {
		ClipNum = Input->LeftBranch[NodeNum];		/* Dual clip tree */
		Cache = Input->RightBranch[ClipNum];
		if (Cache != NOTUSED) {		/* Shall I prune the tree? */
			do {
				ClipNum = Cache;		/* Follow to the end of the tree */
				Cache = Input->RightBranch[ClipNum];
			} while (Cache != NOTUSED);

			Cache = Input->LeftBranch[ClipNum];
			Cache2 = Input->RootBranch[ClipNum];
			Input->RightBranch[Cache2] = Cache;	/* Set the new right parent */
			Input->RootBranch[Cache] = Cache2;	/* Set the new root */

			Cache = Input->LeftBranch[NodeNum];
			Input->LeftBranch[ClipNum] = Cache;		/* Clip the left */
			Input->RootBranch[Cache] = ClipNum;		/* Set the left's new parent */
		}
		Cache = Input->RightBranch[NodeNum];
		Input->RightBranch[ClipNum] = Cache;		/* Attach my tree to the right's end */
		Input->RootBranch[Cache] = ClipNum;		/* Set the new parent */
	}

	/* ClipNum has the branch to remove */

	Cache = Input->RootBranch[NodeNum];		/* Cache a value for speed */

	Input->RootBranch[ClipNum] = Cache;		/* Reset the parent */
	if (Input->RightBranch[Cache] == NodeNum) {	/* Finish up */
		Input->RightBranch[Cache] = ClipNum;		/* Trim the right */
		Input->RootBranch[NodeNum] = NOTUSED;			/* Zap this tree */
		return;
	}
	Input->LeftBranch[Cache] = ClipNum;		/* Trim the left */
	Input->RootBranch[NodeNum] = NOTUSED;			/* Zap this tree */
}


/**********************************

	Inserts string of length F, RingBuffer[r..r+F-1], into one of the
	trees (RingBuffer[r]'th tree) and returns the longest-match position
	and length via the global variables MatchOffset and MatchSize.
	If MatchSize = F, then removes the old node in favor of the new
	one, because the old one will be deleted sooner.
	Note r plays double role, as tree node and position in buffer.

**********************************/

static void BURGERCALL InsertNode(LZSSState_t *Input,Word NodeNum)
{
	Word i;
	Word Parent;	/* Parent node entry */
	int cmp;		/* Branch left or right? (Must be signed!) */
	Word8 *key;		/* Pointer to a text string */

	Input->RightBranch[NodeNum] = NOTUSED;	/* Zap the children nodes now */
	Input->LeftBranch[NodeNum] = NOTUSED;

	key = &Input->RingBuffer[NodeNum];
	Parent = (Word)key[0]+(N+1);	/* Get the parent node number via hash table */
	Input->MatchSize = 0;	/* No match yet */
	cmp = 1;		/* Assume a right branch initially */

	for ( ; ; ) {		/* Stay until a match is found */
		Word8 *TempPtr;

		if (cmp >= 0) {	/* Right branch? */
			i = Input->RightBranch[Parent];	/* Cache */
			if (i == NOTUSED) {	/* Right branch */
				goto RightEnd;
			}
		} else {
			i = Input->LeftBranch[Parent];	/* Left branch */
			if (i == NOTUSED) {		/* Left branch? */
				goto LeftEnd;
			}
		}

		Parent = i;	/* New parent node */
		i = 1;		/* Init string length */
		TempPtr = &Input->RingBuffer[Parent];
		do {
			cmp = (int)key[i];
			cmp = cmp-(int)TempPtr[i];
			if (cmp) {		/* Is it different? */
				break;		/* Get out */
			}
		} while (++i<F);
		if (i > Input->MatchSize) {		/* Larger match? */
			Input->MatchOffset = Parent;	/* Set the mark */
			Input->MatchSize = i;			/* Set the length */
			if (i >= F) {		/* Maximum length! */
				break;		/* Get out of the main loop! */
			}
		}
	}

	/* I have a full match */

	Input->RootBranch[NodeNum] = Input->RootBranch[Parent];	/* Set the new parent */
	Input->LeftBranch[NodeNum] = Input->LeftBranch[Parent];
	Input->RightBranch[NodeNum] = Input->RightBranch[Parent];

	Input->RootBranch[Input->LeftBranch[Parent]] = NodeNum;
	Input->RootBranch[Input->RightBranch[Parent]] = NodeNum;

	i = Input->RootBranch[Parent];
	if (Input->RightBranch[i] == Parent) {
		Input->RightBranch[i] = NodeNum;
		Input->RootBranch[Parent] = NOTUSED;	/* Remove Parent */
		return;
	}
	Input->LeftBranch[i] = NodeNum;
	Input->RootBranch[Parent] = NOTUSED;	/* Remove Parent */
	return;

LeftEnd:;
	Input->LeftBranch[Parent] = NodeNum;		/* Mark the new left branch */
	Input->RootBranch[NodeNum] = Parent;		/* Mark root */
	return;			/* Exit */

RightEnd:;
	Input->RightBranch[Parent] = NodeNum;		/* Set new right branch */
	Input->RootBranch[NodeNum] = Parent;		/* Mark root */
	return;			/* Exit NOW! */
}

/**********************************

	Compress using the LZSS algorithm
	Return the size in bytes of the compressed data

**********************************/

void ** BURGERCALL EncodeLZSS(Word8 *InputBuffer,Word32 InputLength)
{
	Word PipedLen;	/* Number of bytes in current string to match */
	Word8 BitMask;	/* Bit mask for token flag header (Must be BYTE!) */
	Word i;			/* Temp */
	Word SourceIndex;	/* Index for deleting the nodes */
	Word DestIndex;		/* Index for node insertion */
	Word PrevMatchSize;	/* Size of last run */
	Word8 *MaskPtr;		/* Pointer to mask data */
	Word c;
	LZSSState_t *MyState;

	Word8 *OutputBuffer;		/* Pointer to packed data */
	void **OutputHandle;	/* Handle to packed data */

	if (!InputLength) {		/* No input? */
		return 0;			/* Forget it! */
	}

	OutputHandle = AllocAHandle(InputLength+(InputLength>>3)+1);
	if (!OutputHandle) {		/* Can't get memory! */
		return 0;		/* Abort anyway! */
	}
	MyState = (LZSSState_t *)AllocAPointer(sizeof(LZSSState_t));
	if (!MyState) {
		DeallocAHandle(OutputHandle);
		return 0;
	}
	OutputBuffer = (Word8 *)LockAHandle(OutputHandle);	/* Lock it down */
	InitTree(MyState);		/* Initialize the binary trees */

	/* Read F bytes into the last F bytes of the buffer */

	PipedLen = N-F;		/* Ring buffer index */
	do {
		MyState->RingBuffer[PipedLen] = *InputBuffer++;
		++PipedLen;
		if (!--InputLength) {	/* No more data? */
			break;
		}
	} while (PipedLen<N);
	PipedLen = PipedLen-(N-F);	/* Convert back to count */

	MaskPtr = OutputBuffer;
	MaskPtr[0] = 0;
	++OutputBuffer;
	BitMask = 1;
	SourceIndex = 0;
	DestIndex = N - F;		/* Init the ring buffer index */

	/* Finally, insert the whole string just read. The */
	/* global variables MatchSize and MatchOffset are set. */

	InsertNode(MyState,DestIndex);		/* Insert it in */
	do {

	/* MatchSize may be spuriously long near the end of text. */

		if (MyState->MatchSize > PipedLen) {		/* PipedLen<=F */
			MyState->MatchSize = PipedLen;
		}

		if (MyState->MatchSize < (THRESHOLD+1)) {

		/* Not long enough match. Send one byte. */

			MyState->MatchSize = 1;		/* Force 1 byte for future loop */
			MaskPtr[0] |= BitMask;		/* 'send one byte' flag */
			*OutputBuffer++ = MyState->RingBuffer[DestIndex];		/* Send uncoded. */
		} else {

	/* Send position and length pair. Note MatchSize > THRESHOLD. */

			i = MyState->MatchOffset-DestIndex;
			i = i&0xFFF;
			i = i|((MyState->MatchSize-(THRESHOLD+1))<<12);
			((Word16 *)OutputBuffer)[0] = Burger::LoadLittle(static_cast<Word16>(i));
			OutputBuffer += 2;
		}

		BitMask <<=1;	/* Shift mask left one bit. */
		if (!BitMask) {
			MaskPtr = OutputBuffer;		/* Get the new mask byte */
			++OutputBuffer;			/* Inc past it */
			MaskPtr[0] = 0;			/* Init var */
			BitMask = 1;			/* Init mask */
		}

		PrevMatchSize = MyState->MatchSize;	/* Length of the last match */

		i = 0;
		do {
			if (!InputLength) {		/* Any more input? */
				do {		/* After the end of text, */
					DeleteNode(MyState,SourceIndex);		/* no need to read, but */
					++SourceIndex;
					++DestIndex;
					SourceIndex = SourceIndex & (N - 1);
					DestIndex = DestIndex & (N - 1);
					if (PipedLen) {
						--PipedLen;
						InsertNode(MyState,DestIndex);		/* buffer may not be empty. */
					}
				} while (++i<PrevMatchSize);
				break;
			}
			c = *InputBuffer++;		/* Get a byte from the input stream */
			--InputLength;
			DeleteNode(MyState,SourceIndex);		/* Delete old strings and */
			MyState->RingBuffer[SourceIndex] = static_cast<Word8>(c);	/* read new bytes */

	/* If the position is near the end of buffer, extend the buffer to make
		string comparison easier. */

			if (SourceIndex < (F-1)) {
				MyState->RingBuffer[SourceIndex + N] = static_cast<Word8>(c);
			}
	/* Since this is a ring buffer, increment the position modulo N. */

			++SourceIndex;
			++DestIndex;
			SourceIndex = SourceIndex & (N - 1);
			DestIndex = DestIndex & (N - 1);
			InsertNode(MyState,DestIndex);	/* Register the string in RingBuffer[r..r+F-1] */
		} while (++i<PrevMatchSize);
	} while (PipedLen);	/* until length of string to be processed is zero */

	DeallocAPointer(MyState);		/* Get rid of the state */
	
	++MaskPtr;
	if (MaskPtr == OutputBuffer) {	/* No data after token? */
		--OutputBuffer;			/* Remove token */
	}

	InputLength = OutputBuffer-(Word8 *)(*OutputHandle);
	UnlockAHandle(OutputHandle);		/* Unlock for resizing */
	return ResizeAHandle(OutputHandle,InputLength);	/* Will make smaller */
}

