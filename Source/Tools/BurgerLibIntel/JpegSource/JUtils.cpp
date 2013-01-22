/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

*******************************/

#include "JUtils.hpp"
#include "JError.hpp"
#include <jpeglib.h>
#include <Burger.h>
namespace JPeg70 {

/*******************************

	NaturalOrder[i] is the natural-order position of the i'th element
	of zigzag order.

	When reading corrupted data, the Huffman decoders could attempt
	to reference an entry beyond the end of this array (if the decoded
	zero run length reaches past the end of the block).  To prevent
	wild stores without adding an inner-loop test, we put some extra
	"63"s after the real entries.  This will cause the extra coefficient
	to be stored in location 63 of the block, not somewhere random.
	The worst case would be a run-length of 15, which means we need 16
	fake entries.

*******************************/

const Word NaturalOrder[DCTSIZE2+16] = {
	 0,  1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63,
	63, 63, 63, 63, 63, 63, 63, 63, /* extra entries for safety in decoder */
	63, 63, 63, 63, 63, 63, 63, 63
};

/*******************************

	Compute a/b rounded up to the next integer, I.E. ceil(a/b)
	Assumes that a >=0, b>0 and both are unsigned

*******************************/

Word32 BURGERCALL DivRoundUp(Word32 a,Word32 b)
{
	return (a + b - 1) / b;
}

/*******************************

	Compute a rounded up to next multiple of b, ie, ceil(a/b)*b
	Assumes a >= 0, b > 0 and both are unsigned

*******************************/

Word32 BURGERCALL RoundUp(Word32 a,Word32 b)
{
	a = a + b - 1;			/* Round up the next fraction */
	return a - (a % b);		/* Get the next value */
}

/*******************************

	Copy some rows of samples from one place to another.
	num_rows rows are copied from input_array[source_row++]
	to output_array[dest_row++]; these areas may overlap for duplication.
	The source and destination arrays must be at least as wide as num_cols.

*******************************/

void BURGERCALL CopySampleRows(JSample_t **input_array,JSample_t **output_array,int Height,Word Width)
{
	if (Height>0 && Width) {
		Width = (Width * sizeof(JSample_t));
		do {
			FastMemCpy(output_array[0],input_array[0],Width);
			++input_array;
			++output_array;
		} while (--Height);
	}
}

/*******************************

	Handle the common dispatcher for compression
	and decompression

*******************************/

/*******************************

	Base constructor

*******************************/

CCommonManager::CCommonManager() :
	m_IsDecompressor(TRUE),
	m_GlobalState(CSTATE_NONE),
	m_ErrMsgCode(0),
	m_MessageTablePtr(StandardMessageTable),
	m_LastMessage(JMSG_LASTMSGCODE - 1),
	m_AddonMessageTablePtr(0),
	m_FirstAddonMessage(0),
	m_LastAddonMessage(0),
	m_TraceLevel(0),
	m_NumWarnings(0),
	m_PassCounter(0),
	m_PassLimit(0),
	m_CompletedPasses(0),
	m_TotalPasses(0)
{
}

/*******************************

	Base destructor

*******************************/

CCommonManager::~CCommonManager()
{
}

/*******************************

	Display the current progress of
	the compression/decompression.
	
	Intended to be derived for the user

*******************************/

void BURGERCALL CCommonManager::ShowProgress(void)
{
}

/*******************************

	Reset the error manager

*******************************/

void BURGERCALL CCommonManager::ResetErrorManager(void)
{
	m_NumWarnings = 0;		/* Reset the warnings */
	m_ErrMsgCode = 0;		/* Zap the error message */
}

/*******************************

	Print any pending text output

*******************************/

void BURGERCALL CCommonManager::OutputMessage(void)
{
}

/*******************************

	Decide whether to emit a trace or warning message.
	msg_level is one of:

	-1: recoverable corrupt-data warning, may want to abort.
	0: important advisory messages (always display to user).
	1: first level of tracing detail.
	2,3,...: successively more detailed tracing messages.
	An application might override this method if it wanted to abort on warnings
	or change the policy about which messages to display.

*******************************/

void BURGERCALL CCommonManager::EmitMessage(int MsgLevel,int ErrorCode)
{
	m_ErrMsgCode = ErrorCode;		/* Save the code */
	if (MsgLevel < 0) {
		
		/* It's a warning message.  Since corrupt files may generate many warnings,
		* the policy implemented here is to show only the first warning,
		* unless trace_level >= 3. */
		
		if (!m_NumWarnings || m_TraceLevel >= 3) {
			OutputMessage();
		}
		/* Always count warnings in num_warnings. */
		++m_NumWarnings;
	} else {
		/* It's a trace message.  Show it if trace_level >= msg_level. */
		if (m_TraceLevel >= MsgLevel) {
			OutputMessage();
		}
	}
}

/*******************************

	Applications may override this if they want to get control back after
	an error.  Typically one would longjmp somewhere instead of exiting.
	The setjmp buffer can be made a private field within an expanded error
	handler object.  Note that the info needed to generate an error message
	is stored in the error object, so you can generate the message now or
	later, at your convenience.
	You should make sure that the JPEG object is cleaned up (with jpeg_abort
	or jpeg_destroy) at some point.

*******************************/

void BURGERCALL CCommonManager::FatalError(int ErrorCode)
{
	m_ErrMsgCode = ErrorCode;		/* Save the code */

	/* Always display the message */
	OutputMessage();

	/* Let the memory manager delete any temp files before we die */
	jpeg_destroy(this);
}


/*******************************

	Format a message string for the most recent JPEG error or message.
	The message is stored into buffer, which should be at least JMSG_LENGTH_MAX
	characters.  Note that no '\n' character is added to the string.
	Few applications should need to override this method.

*******************************/

void BURGERCALL CCommonManager::FormatMessage(char *TextBuffer,Word /* BufferSize */)
{
	int msg_code = m_ErrMsgCode;
	const char *msgtext = 0;

	/* Look up message string in proper table */
	if (msg_code > 0 && msg_code <= m_LastMessage) {
		msgtext = m_MessageTablePtr[msg_code];
	} else if (m_AddonMessageTablePtr &&
		msg_code >= m_FirstAddonMessage &&
		msg_code <= m_LastAddonMessage) {
		msgtext = m_AddonMessageTablePtr[msg_code - m_FirstAddonMessage];
	}

	/* Defend against bogus message number */
	/* Also guards against NULL pointers in the tables */

	if (!msgtext) {
		m_ErrMsgParms.i[0] = msg_code;
		msgtext = m_MessageTablePtr[0];		/* Must be non-null! */
	}

	/* Check for string parameter, as indicated by %s in the message text */
	Bool isstring = FALSE;
	const char *msgptr = msgtext;
	char ch;
	while ((ch = msgptr[0]) != 0) {
		++msgptr;
		if (ch == '%') {
			if (msgptr[0] == 's') {
				isstring = TRUE;
			}
			break;
		}
	}

	/* Format the message into the passed buffer */
	if (isstring) {
		sprintf(TextBuffer,msgtext, m_ErrMsgParms.s);
	} else {
		sprintf(TextBuffer,msgtext,
			m_ErrMsgParms.i[0],m_ErrMsgParms.i[1],
			m_ErrMsgParms.i[2],m_ErrMsgParms.i[3],
			m_ErrMsgParms.i[4],m_ErrMsgParms.i[5],
			m_ErrMsgParms.i[6],m_ErrMsgParms.i[7]);
	}
}

/*******************************

	Allocate a pixel buffer with a header that
	has a YTable

*******************************/

JSample_t **CCommonManager::AllocSArray(Word Width,Word Height)
{
	/* Ensure that the buffer has some sort of size */
	if (!Width) {
		Width = 1;
	}
	if (!Height) {
		Height = 1;
	}
	
	/* Calculate the memory needed */
	
	Word Size = (((Width*Height*sizeof(JSample_t))+(sizeof(JSample_t*)-1))/sizeof(JSample_t*))+Height;
	JSample_t **Result = new JSample_t*[Size];			/* Get the memory from the proper array for the delete to match */
	if (!Result) {
		FatalError(JERR_OUT_OF_MEMORY,static_cast<int>(Size*sizeof(JSample_t *)));
		return 0;
	}
	
	/* Generate the YTable */
	
	JSample_t *WorkPtr = reinterpret_cast<JSample_t *>(&Result[Height]);
	Word i = 0;
	do {
		Result[i] = WorkPtr;
		WorkPtr += Width;		/* Next line down */
	} while (++i<Height);
	return Result;		/* Return the finished buffer (Dispose with delete[]) */
}

}