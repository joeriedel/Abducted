/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

*******************************/

#include "JError.hpp"
#include "JVersion.hpp"

namespace JPeg70 {

const char *const StandardMessageTable[JMSG_LASTMSGCODE] = {
	"Bogus message code %d",
	"Sorry, there are legal restrictions on arithmetic coding",
	"ALIGN_TYPE is wrong, please fix",
	"MAX_ALLOC_CHUNK is wrong, please fix",
	"Bogus buffer control mode",
	"Invalid component ID %d in SOS",
	"DCT coefficient out of range",
	"IDCT output block size %d not supported",
	"Bogus Huffman table definition",
	"Bogus input colorspace",
	"Bogus JPEG colorspace",
	"Bogus marker length",
	"Wrong JPEG library version: library is %d, caller expects %d",
	"Sampling factors too large for interleaved scan",
	"Invalid memory pool code %d",
	"Unsupported JPEG data precision %d",
	"Invalid progressive parameters Ss=%d Se=%d Ah=%d Al=%d",
	"Invalid progressive parameters at scan script entry %d",
	"Bogus sampling factors",
	"Invalid scan script at entry %d",
	"Improper call to JPEG library in state %d",
	"JPEG parameter struct mismatch: library thinks size is %u, caller expects %u",
	"Bogus virtual array access",
	"Buffer passed to JPEG library is too small",
	"Suspension not allowed here",
	"CCIR601 sampling not implemented yet",
	"Too many color components: %d, max %d",
	"Unsupported color conversion request",
	"Bogus DAC index %d",
	"Bogus DAC value 0x%x",
	"Bogus DHT index %d",
	"Bogus DQT index %d",
	"Empty JPEG image (DNL not supported)",
	"Read from EMS failed",
	"Write to EMS failed",
	"Didn't expect more than one scan",
	"Input file read error",
	"Output file write error --- out of disk space?",
	"Fractional sampling not implemented yet",
	"Huffman code size table overflow",
	"Missing Huffman code table entry",
	"Maximum supported image dimension is %u pixels",
	"Empty input file",
	"Premature end of input file",
	"Cannot transcode due to multiple use of quantization table %d",
	"Scan script does not transmit all data",
	"Invalid color quantization mode change",
	"Not implemented yet",
	"Requested feature was omitted at compile time",
	"Backing store not supported",
	"Huffman table 0x%02x was not defined",
	"JPEG datastream contains no image",
	"Quantization table 0x%02x was not defined",
	"Not a JPEG file: starts with 0x%02x 0x%02x",
	"Insufficient memory (case %d)",
	"Cannot quantize more than %d color components",
	"Cannot quantize to fewer than %d colors",
	"Cannot quantize to more than %d colors",
	"Invalid JPEG file structure: two SOF markers",
	"Invalid JPEG file structure: missing SOS marker",
	"Unsupported JPEG process: SOF type 0x%02x",
	"Invalid JPEG file structure: two SOI markers",
	"Invalid JPEG file structure: SOS before SOF",
	"Failed to create temporary file %s",
	"Read failed on temporary file",
	"Seek failed on temporary file",
	"Write failed on temporary file --- out of disk space?",
	"Application transferred too few scanlines",
	"Unsupported marker type 0x%02x",
	"Virtual array controller messed up",
	"Image too wide for this implementation",
	"Read from XMS failed",
	"Write to XMS failed",
	JCopyright,
	JVersion,
	"Caution: quantization tables are too coarse for baseline JPEG",
	"Adobe APP14 marker: version %d, flags 0x%04x 0x%04x, transform %d",
	"Unknown APP0 marker (not JFIF), length %u",
	"Unknown APP14 marker (not Adobe), length %u",
	"Define Arithmetic Table 0x%02x: 0x%02x",
	"Define Huffman Table 0x%02x",
	"Define Quantization Table %d  precision %d",
	"Define Restart Interval %u",
	"Freed EMS handle %u",
	"Obtained EMS handle %u",
	"End Of Image",
	"        %3d %3d %3d %3d %3d %3d %3d %3d",
	"JFIF APP0 marker: version %d.%02d, density %dx%d  %d",
	"Warning: thumbnail image size does not match data length %u",
	"JFIF extension marker: type 0x%02x, length %u",
	"    with %d x %d thumbnail image",
	"Miscellaneous marker 0x%02x, length %u",
	"Unexpected marker 0x%02x",
	"        %4u %4u %4u %4u %4u %4u %4u %4u",
	"Quantizing to %d = %d*%d*%d colors",
	"Quantizing to %d colors",
	"Selected %d colors for quantization",
	"At marker 0x%02x, recovery action %d",
	"RST%d",
	"Smoothing not supported with nonstandard sampling ratios",
	"Start Of Frame 0x%02x: width=%u, height=%u, components=%d",
	"    Component %d: %dhx%dv q=%d",
	"Start of Image",
	"Start Of Scan: %d components",
	"    Component %d: dc=%d ac=%d",
	"  Ss=%d, Se=%d, Ah=%d, Al=%d",
	"Closed temporary file %s",
	"Opened temporary file %s",
	"JFIF extension marker: JPEG-compressed thumbnail image, length %u",
	"JFIF extension marker: palette thumbnail image, length %u",
	"JFIF extension marker: RGB thumbnail image, length %u",
	"Unrecognized component IDs %d %d %d, assuming YCbCr",
	"Freed XMS handle %u",
	"Obtained XMS handle %u",
	"Unknown Adobe color transform code %d",
	"Inconsistent progression sequence for component %d coefficient %d",
	"Corrupt JPEG data: %u extraneous bytes before marker 0x%02x",
	"Corrupt JPEG data: premature end of data segment",
	"Corrupt JPEG data: bad Huffman code",
	"Warning: unknown JFIF revision number %d.%02d",
	"Premature end of JPEG file",
	"Corrupt JPEG data: found marker 0x%02x instead of RST%d",
	"Invalid SOS parameters for sequential JPEG",
	"Application transferred too many scanlines"
};

/* Code in storage */

#if 0
/*
 * Error exit handler: must not return to caller.
 *
 * Applications may override this if they want to get control back after
 * an error.  Typically one would longjmp somewhere instead of exiting.
 * The setjmp buffer can be made a private field within an expanded error
 * handler object.  Note that the info needed to generate an error message
 * is stored in the error object, so you can generate the message now or
 * later, at your convenience.
 * You should make sure that the JPEG object is cleaned up (with jpeg_abort
 * or jpeg_destroy) at some point.
 */

static void BURGERCALL FatalError(JPeg70::CCommonManager * cinfo)
{
	/* Always display the message */
	cinfo->OutputMessage();

	/* Let the memory manager delete any temp files before we die */
	jpeg_destroy(cinfo);

	exit(10);
}

/*
 * Actual output of an error or trace message.
 * Applications may override this method to send JPEG messages somewhere
 * other than stderr.
 *
 * On Windows, printing to stderr is generally completely useless,
 * so we provide optional code to produce an error-dialog popup.
 * Most Windows applications will still prefer to override this routine,
 * but if they don't, it'll do something at least marginally useful.
 *
 * NOTE: to use the library in an environment that doesn't support the
 * C stdio library, you may have to delete the call to fprintf() entirely,
 * not just not use this routine.
 */

static void BURGERCALL OutputMessage (JPeg70::CCommonManager * /* cinfo */)
{
#if 0
  char buffer[JMSG_LENGTH_MAX];

  /* Create the message */
  cinfo->FormatMessage(buffer,sizeof(buffer));

#ifdef USE_WINDOWS_MESSAGEBOX
  /* Display it in a message dialog box */
  MessageBox(GetActiveWindow(), buffer, "JPEG Library Error",
	     MB_OK | MB_ICONERROR);
#else
  /* Send it to stderr, adding a newline */
  fprintf(stderr, "%s\n", buffer);
#endif
#endif
}


/*
 * Format a message string for the most recent JPEG error or message.
 * The message is stored into buffer, which should be at least JMSG_LENGTH_MAX
 * characters.  Note that no '\n' character is added to the string.
 * Few applications should need to override this method.
 */

static void BURGERCALL FormatMessage (JPeg70::CCommonManager * cinfo, char * buffer,Word BufferSize)
{
  JPeg70::CErrorManager * err = cinfo->err;
  int msg_code = cinfo->m_ErrMsgCode;
  const char * msgtext = NULL;
  const char * msgptr;
  char ch;
  Word8 isstring;

  /* Look up message string in proper table */
  if (msg_code > 0 && msg_code <= cinfo->m_LastMessage) {
    msgtext = cinfo->m_MessageTablePtr[msg_code];
  } else if (cinfo->m_AddonMessageTablePtr != NULL &&
	     msg_code >= cinfo->m_FirstAddonMessage &&
	     msg_code <= cinfo->m_LastAddonMessage) {
    msgtext = cinfo->m_AddonMessageTablePtr[msg_code - cinfo->m_FirstAddonMessage];
  }

  /* Defend against bogus message number */
  if (msgtext == NULL) {
    cinfo->m_ErrMsgParms.i[0] = msg_code;
    msgtext = cinfo->m_MessageTablePtr[0];
  }

  /* Check for string parameter, as indicated by %s in the message text */
  isstring = FALSE;
  msgptr = msgtext;
  while ((ch = *msgptr++) != '\0') {
    if (ch == '%') {
      if (*msgptr == 's') isstring = TRUE;
      break;
    }
  }

  /* Format the message into the passed buffer */
  if (isstring)
    sprintf(buffer, msgtext, cinfo->m_ErrMsgParms.s);
  else
    sprintf(buffer, msgtext,
	    cinfo->m_ErrMsgParms.i[0], cinfo->m_ErrMsgParms.i[1],
	    cinfo->m_ErrMsgParms.i[2], cinfo->m_ErrMsgParms.i[3],
	    cinfo->m_ErrMsgParms.i[4], cinfo->m_ErrMsgParms.i[5],
	    cinfo->m_ErrMsgParms.i[6], cinfo->m_ErrMsgParms.i[7]);
}
#endif
}