/**********************************

	Parse out a PSD file

**********************************/

#include "ImImage.h"
#include <BREndian.hpp>
#include "ClStdlib.h"
#include "MmMemory.h"

/**********************************

	All this was taken straight from the psd docs.
	Note: this reader does not read the image resources block.
	All records are in BIG ENDIAN format!

**********************************/

/* Master file header */

typedef struct PSDHeader_t {
	char Signature[4];	// Should be "8BPS"
	Word16 Version;		// Should be 1.
	char Reserved1[6];	// Padding 
	Word NumChannels;	// Number of data channels
	Word32 Width;		// Width in pixels
	Word32 Height;	// Height in pixels
	Word16 BitsPerChannel;	// Bits per pixel (Usually 8)
	Word16 Mode;			// Type of PSD file
} PSDHeader_t;

typedef struct PSDColorMode_t {
	Word32 Length;			/* Size of the data */
	Word8 *ColorModeDataPtr;		/* Color mode data */
} PSDColorMode_t;

typedef struct PSDChannelLengthInfo_t {
	int ChannelID;			/* ID of the channel represented (signed) */
	Word32 Length;			/* Length of the info */
} PSDChannelLengthInfo_t;

typedef struct PSDLayerMaskData_t {
	Word32 Length;			/* Size of the structure (20 or 36) */
	Word32 Top, Left, Bottom, Right;	/* Dimensions of the mask */
	char DefaultColor;			/* Base color index */
	char Flags;					/* Mask flags */
	char Padd[2];				/* Not used */
} PSDLayerMaskData_t;

typedef struct PSDLayerChannelBlendRange_t {
	Word32 Src;		/* Start range */
	Word32 Dst;		/* Dest range */
} PSDLayerChannelBlendRange_t;

typedef struct PSDLayerBlendingRanges_t {
	Word32 Length;						/* Number of ranges present */
	PSDLayerChannelBlendRange_t GrayBlend;	/* Base greyscale blend */
	PSDLayerChannelBlendRange_t	*ChannelBlendRanges;	/* Ranges for each channel */
} PSDLayerBlendingRanges_t;

typedef struct PSDChannelImageData_t {
	int Compression;
	Word8 *Data;
} PSDChannelImageData_t;

typedef struct PSDLayerRecord_t {
	int Top, Left, Bottom, Right;
	int NumChannels;
	PSDChannelLengthInfo_t*	ChannelLengthInfoPtr;
	char BlendModeSig[4];
	int BlendModeKey;
	Word8 Opacity;
	Word8 Clipping;
	Word8 Flags;
	Word8 Pad1;
	int	ExtraDataSize;
	PSDLayerMaskData_t LayerMaskData;
	PSDLayerBlendingRanges_t LayerBlendingRanges;
	char* Name;
	// Tacked our channel data here.
	PSDChannelImageData_t*	ChannelImageData;
} PSDLayerRecord_t;

typedef struct PSDLayerInfoSection_t {
	int Length;
	int NumLayers;
	PSDLayerRecord_t *LayerRecordsPtr;
} PSDLayerInfoSection_t;

static const char PSDFileHeader[] = "8BPS";		/* Main header for a PSD file */

/**********************************

	Extract the header of a PSD file

**********************************/

static const Word8 *PSDReadHeader(PSDHeader_t* Header,const Word8 *Stream)
{
	const char *Msg;
	
	FastMemSet(Header,0,sizeof(PSDHeader_t));		/* Init the buffer */
	
	((Word32 *)&Header->Signature[0])[0] = ((Word32 *)Stream)[0];	/* Copy 4 bytes */
	Header->Version = Burger::LoadBig((Word16*)(Stream+4));
	Header->NumChannels = Burger::LoadBig((Word16*)(Stream+12));
	Header->Width = Burger::LoadBig((Word32*)(Stream+14));
	Header->Height = Burger::LoadBig((Word32*)(Stream+18));
	Header->BitsPerChannel = Burger::LoadBig((Word16*)(Stream+22));
	Header->Mode = Burger::LoadBig((Word16*)(Stream+24));
	
	/* Now check the header for validity */
	
	if (memcmp(Header->Signature,PSDFileHeader,4)) {					/* String match? */
		Msg = "PSD Head is bad";
	} else if (Header->Version != 1) {
		Msg = "PSD Version is not 1";
	} else if (Header->BitsPerChannel != 8) {
		Msg = "PSD is not 8 bits per channel";
	} else {
		return Stream+26;
	}
	NonFatal(Msg);
	return 0;
}

/**********************************

	Skip past the color data structure

**********************************/

static INLINECALL const Word8 *PSDSkipColorData(const Word8 *Stream)
{
	Word32 Size;
	
	Size = Burger::LoadBig((Word32*)Stream);	/* Get the size of this chunk */
	return &Stream[Size+4];
}

/**********************************

	Skip past the image resource data structure

**********************************/

static INLINECALL const Word8 * PSDSkipImageResources(const Word8 *Stream)
{
	Word32 Size;
	
	Size = Burger::LoadBig((Word32*)Stream);	/* Get the size of this chunk */
	return &Stream[Size+4];
}

/**********************************

	Skip past the image resource data structure

**********************************/

static INLINECALL const Word8 *PSDReadChannelLengthInfo(PSDChannelLengthInfo_t* ChannelInfo,const Word8 *Stream)
{
	ChannelInfo->ChannelID = Burger::LoadBig((short*)Stream);
	ChannelInfo->Length = Burger::LoadBig((Word32*)(Stream+2));
	return Stream+6;
}

/**********************************

	Parse the mask range data

**********************************/

static const Word8 *PSDReadLayerMaskData(PSDLayerMaskData_t* LayerMask,const Word8 *Stream)
{
	Word32 Length;
	
	FastMemSet(LayerMask,0,sizeof(PSDLayerMaskData_t));
	
	Length = Burger::LoadBig((Word32*)Stream);
	if (!Length) {
		return Stream+4;
	}
	LayerMask->Length = Length;
	// Load other fields.
	if (Length != 36 && Length != 20) {
		NonFatal("PSD Bad layer mask record size");
		return 0;
	}
	LayerMask->Top = Burger::LoadBig((Word32*)(Stream+4));
	LayerMask->Left = Burger::LoadBig((Word32*)(Stream+8));
	LayerMask->Bottom = Burger::LoadBig((Word32*)(Stream+12));
	LayerMask->Right = Burger::LoadBig((Word32*)(Stream+16));
	LayerMask->DefaultColor = Stream[20];
	LayerMask->Flags = Stream[21];
	return Stream+Length+4;
}

/**********************************

	Extract a pascal string from the data stream

**********************************/

static const Word8 *PSDReadPascalString(char** StringPtr,const Word8 *Stream)
{
	char* String;
	Word Length;
	
	Length = Stream[0]+1;					/* Get the buffer length */
	String = (char*)AllocAPointer(Length);
	StringPtr[0] = String;					/* Return the pointer */
	if (!String) {
		return 0;							/* Crap */
	}
	PStr2CStr(String,(const char *)Stream);	/* Convert to "C" string */
	Stream+=Length;							/* Parse past the string */
	return Stream;							/* Return the new index */
}

/**********************************

	Extract the blending ranges for this layer

**********************************/

static const Word8 *PSDReadLayerBlendingRanges(PSDLayerBlendingRanges_t* BlendRanges, PSDLayerRecord_t* LayerRecord,const Word8* Stream)
{
	int i;
	Word NumChannels;
	PSDLayerChannelBlendRange_t *DestPtr;
	const Word8 *ReturnValue;		/* Pointer to the end of this data */
	
	FastMemSet(BlendRanges,0,sizeof(PSDLayerBlendingRanges_t));		/* Init my structure */
	NumChannels = 0;			/* Init the channel count */
	for(i = 0; i < LayerRecord->NumChannels; i++) {
		if (LayerRecord->ChannelLengthInfoPtr[i].ChannelID != -2) {		/* Not invalid? */
			NumChannels++;		/* Increase the count */
		}
	}
	if (!NumChannels) {
		NonFatal("PSD Bad blending channel number");
		return 0;
	}
	DestPtr = (PSDLayerChannelBlendRange_t*)AllocAPointerClear(sizeof(PSDLayerChannelBlendRange_t)*NumChannels);
	if (!DestPtr) {		/* No memory?? */
		return 0;								/* I am screwed */
	}
	BlendRanges->ChannelBlendRanges = DestPtr;
	BlendRanges->Length = Burger::LoadBig((Word32*)Stream);
	BlendRanges->GrayBlend.Src = Burger::LoadBig((Word32*)(Stream+4));
	BlendRanges->GrayBlend.Dst = Burger::LoadBig((Word32*)(Stream+8));
	ReturnValue = Stream+BlendRanges->Length+4;		/* End of the data (May have extra stuff I don't care about)*/
	Stream+=12;

	/* Load each channel blend. */
	do {
		DestPtr->Src = Burger::LoadBig((Word32*)Stream);		/* Copy the range */
		DestPtr->Dst = Burger::LoadBig((Word32*)(Stream+4));
		++DestPtr;			/* Next structure entry */
		Stream+=8;			/* 8 bytes accepted */
	} while (--NumChannels);
	return ReturnValue;			/* Return the new pointer */
}

/**********************************

	Copy literal data

**********************************/

static INLINECALL const Word8 * PSDDecodeUncompressedBlock(Word8* Output,Word32 BlockLength,const Word8* Stream)
{
	FastMemCpy(Output, Stream, BlockLength);
	return Stream + BlockLength;
}

/**********************************

	Decompress data using RLE compression

**********************************/

static const Word8 *PSDDecodeRLEBlock(PSDLayerRecord_t* LayerRecord,Word8* Output, int BlockLength, int ChannelID,const Word8* Stream)
{
	// Decodes an RLE block.
	int i, y;
	int width, height;
	int inscans;
	Word martin;
	Word replicant;
	Word16 *scancounts;		/* Number of bytes per scan line */
	Word16 TempScan[1024];	/* Fast buffer */

	/* Get the dimensions of this record */
	if (ChannelID != -2) {
		height = LayerRecord->Bottom-LayerRecord->Top;
		width  = LayerRecord->Right-LayerRecord->Left;
	} else {
		height = LayerRecord->LayerMaskData.Bottom-LayerRecord->LayerMaskData.Top;
		width  = LayerRecord->LayerMaskData.Right-LayerRecord->LayerMaskData.Left;
	}

	/* Ok, am I screwed? */
	
	if (height<=0 || width<=0) {
		return Stream+BlockLength;		/* Not an error, but I will skip the bad data */
	}

	if (height>1024) {					/* Too big for internal stack */
		scancounts = (Word16*)AllocAPointer(sizeof(Word16)*height);
		if (!scancounts) {
			return 0;			/* I must be allocating a god-awful buffer! */
		}
	} else {
		scancounts = TempScan;
	}
	i = 0;
	do {
		scancounts[i] = Burger::LoadBig((Word16*)Stream);
		Stream+=2;
	} while (++i<height);
	
	y = 0;
	do {
		int current; 
		inscans=0;		/* Reset the counters */
		current = scancounts[y];
		if (inscans < current) {
			do {
				martin = Stream[0];		/* Get the token */
				++Stream;				/* Accept it */
				++inscans;				/* Accept from the stream */
				if (martin < 128) {
					++martin;			/* 1-128 */
					inscans+=martin;
					do {
						Output[0] = Stream[0];	/* Explicit copy */
						++Stream;
						++Output;
					} while (--martin);			/* All done? */
				} else {
					replicant = Stream[0];		/* Word8 to copy */
					++Stream;
					++inscans;
					martin = 257-martin;		/* 2-129 */
					do {
						Output[0] = static_cast<Word8>(replicant);	/* Fill the data */
						++Output;
					} while (--martin);			/* All done? */
				}
			} while (inscans < current);
		}
		if (inscans > ((current+1)&~1)) {
			NonFatal("PSD Bad scan length consistency");
			if (scancounts!=TempScan) {
				DeallocAPointer(scancounts);		/* Release my buffer */
			}
			return 0;			/* You are screwed */
		}
	} while (++y<height);
	if (scancounts!=TempScan) {
		DeallocAPointer(scancounts);				/* Relese my buffer */
	}
	return Stream;									/* Return current running pointer */
}

/**********************************

	Decompress a channel.
	Can use RLE or literal data

**********************************/

static const Word8 *PSDDecodeChannel(PSDHeader_t* Header, PSDLayerRecord_t* LayerRecord, PSDChannelImageData_t* ChannelImage, PSDChannelLengthInfo_t* ChannelInfo,const Word8* Stream)
{
	int size;
	int type;

	ChannelImage->Data = 0;			/* Init the pointer */
	if (ChannelInfo->ChannelID != -2) {
		size = (LayerRecord->Bottom-LayerRecord->Top)*(LayerRecord->Right-LayerRecord->Left)*((Header->BitsPerChannel+7)>>3);
	} else {
		size = (LayerRecord->LayerMaskData.Bottom-LayerRecord->LayerMaskData.Top)*(LayerRecord->LayerMaskData.Right-LayerRecord->LayerMaskData.Left)*((Header->BitsPerChannel+7)>>3);
	}

	if(size > 0) {
		ChannelImage->Data = (Word8*)AllocAPointer(size);
		if (!ChannelImage->Data) {
			return 0;
		}
	}
	type = Burger::LoadBig((short*)Stream);
	Stream+=2;
	if (!size) {
		return Stream;		/* Don't bother */
	}

	switch(type) {
	case 0:			// Uncompressed;
		Stream = PSDDecodeUncompressedBlock(ChannelImage->Data,ChannelInfo->Length-2,Stream);
		break;
	case 1:			/* RLE compression */
		Stream = PSDDecodeRLEBlock(LayerRecord, ChannelImage->Data, ChannelInfo->Length-2, ChannelInfo->ChannelID,Stream);
		break;
	default:
		NonFatal("PSD Unsupported compression");
		return 0;
	}
	return Stream;
}

/**********************************

	Convert an RGB pixel array into a Burgerlib format

**********************************/

static int PSDCodeRGBChannels(PSDHeader_t* Header, PSDLayerRecord_t* LayerRecord, PSDImageLayer_t* PSDLayer)
{
	// find the 3 channel, r=0, g=1, b=2.
	Word8* r, *g, *b, *rgb;
	int i;
	int numbytes;

	if (Header->BitsPerChannel != 8) {
		NonFatal("PSD Bad channel depth (Not 8 bits)");
		return TRUE;
	}

	if (!LayerRecord->ChannelImageData) {		/* No data? */
		return FALSE;							/* It's ok */
	}

	/* Now, let's find the R,G and B channels */
	/* They are not always in the same order */
	
	r=g=b=0;
	for(i = 0; i < LayerRecord->NumChannels; i++) {
		switch(LayerRecord->ChannelLengthInfoPtr[i].ChannelID) {
		case 0:
			r = LayerRecord->ChannelImageData[i].Data;
			break;
		case 1:
			g = LayerRecord->ChannelImageData[i].Data;
			break;
		case 2:
			b = LayerRecord->ChannelImageData[i].Data;
			break;
		}
		if (r&&g&&b) {		/* Did I find them already? */
			break;
		}
	}

	if(!r||!g||!b) {
		NonFatal("PSD Missing an R,G or B channel");
		return TRUE;
	}
	// This image will be 24 bit color because we force things to be 8 bits per channel.
	
	numbytes = PSDLayer->Width*PSDLayer->Height;
	rgb = (Word8*)AllocAPointer(numbytes*3);		/* If numbytes==0, then the pointer is zero! */
	PSDLayer->RGB = rgb;
	if (!rgb) {
		return TRUE;		/* You are screwed again */
	}
	
	do {
		rgb[0] = r[0];		/* Merge the three layers into a single 24 bit image */
		rgb[1] = g[0];
		rgb[2] = b[0];
		++r;
		++g;
		++b;
		rgb+=3;
	} while (--numbytes);
	return FALSE;
}

/**********************************

	Convert the alpha channel from the PSD to my data

**********************************/

static int PSDCodeAlphaChannel(PSDHeader_t* /* Header */, PSDLayerRecord_t* LayerRecord, PSDImageLayer_t* PSDLayer)
{
	int i;
	Word8* alpha;
	int numbytes;

	if (!LayerRecord->ChannelImageData) {		/* No alpha? */
		return FALSE;
	}

	// find the alpha channel.
	alpha = 0;
	for(i = 0; i < LayerRecord->NumChannels; i++) {
		switch(LayerRecord->ChannelLengthInfoPtr[i].ChannelID) {	/* Scan for Alpha */
		case -1:
			alpha=LayerRecord->ChannelImageData[i].Data;			/* Ah ha! */
			break;
		}
	}
	if(!alpha) {
		NonFatal("PSD Missing alpha channel");
		return TRUE;
	}

	// direct copy.
	numbytes = PSDLayer->Width*PSDLayer->Height;
	PSDLayer->Alpha = (Word8*)AllocAPointer(numbytes);
	if (!PSDLayer->Alpha) {
		return TRUE;
	}
	FastMemCpy(PSDLayer->Alpha, alpha, numbytes);		/* Copy the data */
	return FALSE;
}

/**********************************

	Convert the mask channel from the PSD to my data

**********************************/

static int PSDCodeMaskChannel(PSDHeader_t* /* Header */, PSDLayerRecord_t* LayerRecord, PSDImageLayer_t* PSDLayer)
{
	int i;
	Word8* mask;
	int numbytes;

	if (!LayerRecord->ChannelImageData) {		/* No mask found? */
		return FALSE;
	}

	// find mask channel.
	mask = 0;
	for(i = 0; i < LayerRecord->NumChannels; i++) {
		switch(LayerRecord->ChannelLengthInfoPtr[i].ChannelID) {
		case -2:
			mask=LayerRecord->ChannelImageData[i].Data;		/* Ah ha! */
			break;
		}
	}
	if (!mask) {
		NonFatal("PSD Missing the mask channel");
		return TRUE;
	}
	// direct copy.
	numbytes = PSDLayer->MaskWidth*PSDLayer->MaskHeight;
	PSDLayer->Mask = (Word8*)AllocAPointer(numbytes);
	if (!PSDLayer->Mask) {
		return TRUE;
	}
	FastMemCpy(PSDLayer->Mask, mask, numbytes);
	return FALSE;
}

/**********************************

	Grab bitmap data from the PSD image

**********************************/

static const Word8 *PSDReadChannelImageData(PSDHeader_t* Header, PSDLayerInfoSection_t* LayerSection,const Word8* Stream)
{
	/* 
	** The PSD contains channel image data for each channel in order of
	** each layer.
	*/

	int i;
	int channel;
	PSDLayerRecord_t* LayerRecord;

	for (i = 0; i < LayerSection->NumLayers; i++) {
		LayerRecord = &LayerSection->LayerRecordsPtr[i];
		LayerRecord->ChannelImageData = (PSDChannelImageData_t*)AllocAPointer(sizeof(PSDChannelImageData_t)*LayerRecord->NumChannels);
		if (!LayerRecord->ChannelImageData) {
			return 0;
		}

		FastMemSet(LayerRecord->ChannelImageData, 0, sizeof(PSDChannelImageData_t)*LayerRecord->NumChannels);

		// Read each channel.
		for(channel = 0; channel < LayerRecord->NumChannels; channel++) {
			Stream = PSDDecodeChannel(Header, LayerRecord, &LayerRecord->ChannelImageData[channel], &LayerRecord->ChannelLengthInfoPtr[channel],Stream);
			if (!Stream) {
				return Stream;
			}
		}
	}
	return Stream;
}

/**********************************

	Get a layer record from the PSD stream

**********************************/

static const Word8 *PSDReadLayerRecord(PSDLayerRecord_t* LayerRecord,const Word8 *Stream)
{
	int i;
	int read_so_far;
	const Word8 *mark1;

	LayerRecord->Top = Burger::LoadBig((Word32*)Stream);
	LayerRecord->Left = Burger::LoadBig((Word32*)(Stream+4));
	LayerRecord->Bottom = Burger::LoadBig((Word32*)(Stream+8));
	LayerRecord->Right = Burger::LoadBig((Word32*)(Stream+12));
	LayerRecord->NumChannels = Burger::LoadBig((short*)(Stream+16));
	Stream += 18;
	
	LayerRecord->ChannelLengthInfoPtr = (PSDChannelLengthInfo_t*)AllocAPointerClear(sizeof(PSDChannelLengthInfo_t)*LayerRecord->NumChannels);
	if (!LayerRecord->ChannelLengthInfoPtr) {
		return 0;
	}
	for (i = 0; i < LayerRecord->NumChannels; i++) {
		Stream = PSDReadChannelLengthInfo(&LayerRecord->ChannelLengthInfoPtr[i],Stream);
		if (!Stream) {
			return Stream;
		}
	}

	((Word32 *)&LayerRecord->BlendModeSig[0])[0] = ((Word32 *)Stream)[0];
	if (memcmp(LayerRecord->BlendModeSig,"8BIM",4)) {
		NonFatal("PSD BAD Blend mode signature");
		return 0;
	}
	LayerRecord->BlendModeKey = Burger::LoadBig((Word32*)(Stream+4));
	LayerRecord->Opacity = Stream[8];
	LayerRecord->Clipping = Stream[9];
	LayerRecord->Flags = Stream[10];
	LayerRecord->ExtraDataSize = Burger::LoadBig((Word32*)(Stream+12));
	Stream += 16;	
	mark1 = Stream;
	// Read mask data.
	Stream = PSDReadLayerMaskData(&LayerRecord->LayerMaskData,Stream);
	if (Stream) {
		// Read blend ranges.
		Stream = PSDReadLayerBlendingRanges(&LayerRecord->LayerBlendingRanges, LayerRecord, Stream);
		if (Stream) {
			// Name
			Stream = PSDReadPascalString(&LayerRecord->Name,Stream);
			if (Stream) {
				read_so_far = Stream-mark1;

				if (read_so_far > LayerRecord->ExtraDataSize) {
					NonFatal("PSD Layer data overrun");
					Stream = 0;
				} else if (read_so_far < LayerRecord->ExtraDataSize) {
					Stream += (LayerRecord->ExtraDataSize-read_so_far);
				}
			}
		}
	}
	return Stream;
}

/**********************************

	Grab a layer's information section

**********************************/

static const Word8 *PSDReadLayerInfoSection(PSDLayerInfoSection_t* LayerSection,const Word8* Stream)
{
	int n;
	int i;
	int LayerInfoLength;
	int SectionLength;

	SectionLength = Burger::LoadBig((Word32*)Stream);
	// Load the layer info block.
	LayerInfoLength = Burger::LoadBig((Word32*)(Stream+4));
	n = Burger::LoadBig((short*)(Stream+8));

	Stream+=10;
	// Abs?
	if(n < 0) {
		n = -n;
	}
	LayerSection->NumLayers = n;
	LayerSection->LayerRecordsPtr = (PSDLayerRecord_t*)AllocAPointerClear(sizeof(PSDLayerRecord_t)*LayerSection->NumLayers);
	if (!LayerSection->LayerRecordsPtr) {
		return 0;
	}
	for(i = 0; i < LayerSection->NumLayers; i++) {
		Stream = PSDReadLayerRecord(&LayerSection->LayerRecordsPtr[i],Stream);
		if (!Stream) {
			break;
		}
	}
	return Stream;
}

/**********************************

	Dispose of a layer's information section

**********************************/

static void FreePSDLayerInfoSectionBlock(PSDLayerInfoSection_t* LayerSection)
{
	int i, k;
	PSDLayerRecord_t* LayerRecord;

	LayerRecord = LayerSection->LayerRecordsPtr;
	for(i = 0; i < LayerSection->NumLayers; i++) {
		// Free all channel data.
		if (LayerRecord->ChannelImageData) {			/* Sanity check */
			for(k = 0; k < LayerRecord->NumChannels; k++) {
				DeallocAPointer(LayerRecord->ChannelImageData[k].Data);
			}
			DeallocAPointer(LayerRecord->ChannelImageData);
		}
		DeallocAPointer(LayerRecord->ChannelLengthInfoPtr);
		DeallocAPointer(LayerRecord->LayerBlendingRanges.ChannelBlendRanges);
		DeallocAPointer(LayerRecord->Name);
		++LayerRecord;
	}
	DeallocAPointer(LayerSection->LayerRecordsPtr);
	FastMemSet(LayerSection,0,sizeof(PSDLayerInfoSection_t));
}

/**********************************

	Taking an image from memory, parse out
	a PSD file and store it in a PSDImage_t struct

**********************************/

Word BURGERCALL PSDImageParse(PSDImage_t* Image, const Word8* InStream)
{
	Word i;
	PSDHeader_t Header;
	PSDLayerInfoSection_t LayerSection;
	PSDImageLayer_t* PSDLayer;
	PSDLayerRecord_t* LayerRecord;
	
	FastMemSet(Image,0,sizeof(PSDImage_t));
	FastMemSet(&LayerSection, 0, sizeof(LayerSection));

	InStream = PSDReadHeader(&Header,InStream);		/* Parse out the master header */
	if (InStream) {
		InStream = PSDSkipColorData(InStream);
		if (InStream) {
			InStream = PSDSkipImageResources(InStream);
			if (InStream) {
				InStream = PSDReadLayerInfoSection(&LayerSection, InStream);
				if(InStream) {
					InStream=PSDReadChannelImageData(&Header, &LayerSection, InStream);
					if (InStream) {
						// Combine all the layers channel into the PSDImage structure.
						Image->Width = Header.Width;
						Image->Height = Header.Height;
						Image->NumLayers = LayerSection.NumLayers;
						Image->Layers = (PSDImageLayer_t*)AllocAPointerClear(sizeof(PSDImageLayer_t)*LayerSection.NumLayers);
						if(Image->Layers) {
							// For each layer extract the channels and put them into something more useful.
							for(i = 0; i < Image->NumLayers; i++) {
								PSDLayer = &Image->Layers[i];
								LayerRecord = &LayerSection.LayerRecordsPtr[i];

								PSDLayer->Top = LayerRecord->Top;
								PSDLayer->Bottom = LayerRecord->Bottom;
								PSDLayer->Left = LayerRecord->Left;
								PSDLayer->Right = LayerRecord->Right;
								PSDLayer->Width = PSDLayer->Right-PSDLayer->Left;
								PSDLayer->Height = PSDLayer->Bottom-PSDLayer->Top;
								
								// mask
								PSDLayer->MaskTop = LayerRecord->LayerMaskData.Top;
								PSDLayer->MaskLeft = LayerRecord->LayerMaskData.Left;
								PSDLayer->MaskRight = LayerRecord->LayerMaskData.Right;
								PSDLayer->MaskBottom = LayerRecord->LayerMaskData.Bottom;
								PSDLayer->MaskWidth = PSDLayer->MaskRight-PSDLayer->MaskLeft;
								PSDLayer->MaskHeight = PSDLayer->MaskBottom-PSDLayer->MaskTop;

								PSDLayer->Name = 0;

								// code the channels.
								PSDCodeRGBChannels(&Header, LayerRecord, PSDLayer);
								PSDCodeAlphaChannel(&Header, LayerRecord, PSDLayer);
								PSDCodeMaskChannel(&Header, LayerRecord, PSDLayer);
							}
						}
					}
				}
				// everything is packed up in PSDImage_t, so kill the layersection to free other mem.
				FreePSDLayerInfoSectionBlock(&LayerSection);
			}
		}
	}
	if (!InStream) {
		PSDImageDestroy(Image);		/* Oh crap */
		return TRUE;
	}
	return FALSE;				/* Return the end pointer or zero */
}

/**********************************

	Release a PSDImage_t structure

**********************************/

void BURGERCALL PSDImageDestroy(PSDImage_t* Input)
{
	Word i;
	PSDImageLayer_t* layer;
	for (i = 0; i < Input->NumLayers; i++) {
		layer = &Input->Layers[i];
		
		DeallocAPointer(layer->Alpha);
		DeallocAPointer(layer->Mask);
		DeallocAPointer(layer->RGB);
	}
	DeallocAPointer(Input->Layers);
	FastMemSet(Input,0,sizeof(PSDImage_t));
}

/**********************************

	Create a Burgerlib image from a PSD layer

**********************************/

Word BURGERCALL ImageExtractFromPSDImage(Image_t *Output,PSDImageLayer_t* Layer,PSDFlag_e flags)
{
	FastMemSet(Output,0,sizeof(Image_t));
	switch(flags) {
	case PSD_FLAG_RGB:
		if (Layer->RGB) {
			// just get rgb.
			if (!ImageInit(Output,Layer->Width, Layer->Height, IMAGE888)) {
				FastMemCpy(Output->ImagePtr, Layer->RGB, Layer->Width*Layer->Height*3);
				return FALSE;
			}
		}
		break;
	case PSD_FLAG_ALPHA:
		if(Layer->Alpha) {
			// just get alpha.
			if (!ImageInit(Output,Layer->Width, Layer->Height,IMAGE8ALPHA)) {
				FastMemCpy(Output->AlphaPtr, Layer->Alpha, Layer->Width*Layer->Height);
				return FALSE;
			}
		}
		break;
	case PSD_FLAG_MASK:
		if (Layer->Mask) {
			if (!ImageInit(Output,Layer->MaskWidth, Layer->MaskHeight,IMAGE8ALPHA)) {
				FastMemCpy(Output->AlphaPtr, Layer->Mask, Layer->MaskWidth*Layer->MaskHeight);
				return FALSE;
			}
		}
		break;
	case PSD_FLAG_RGBA:
		if (Layer->RGB && Layer->Alpha) {
			int numbytes;
			Word8* rgb, *alpha, *data;
			if (!ImageInit(Output,Layer->Width, Layer->Height,IMAGE8888)) {
				numbytes = Layer->Width*Layer->Height;
				if (numbytes) {
					rgb = Layer->RGB;
					alpha = Layer->Alpha;
					data = Output->ImagePtr;
					do {
						data[0] = rgb[0];
						data[1] = rgb[1];
						data[2] = rgb[2];
						data[3] = alpha[0];
						++alpha;
						rgb+=3;
						data+=4;
					} while (--numbytes);
				}
				return FALSE;
			}
		}
	}
	return TRUE;
}

/**********************************

	Parse the first layer of a PSD

**********************************/

Word BURGERCALL ImageParsePSD(Image_t *Output,const Word8 *Data,Word Layer)
{
	PSDImage_t FooImage;
	Word Result;
	PSDImageLayer_t *LayerPtr;
	
	Result = TRUE;								/* Assume error */
	if (!PSDImageParse(&FooImage,Data)) {
		PSDFlag_e Flag;
		if (Layer<FooImage.NumLayers) {			/* Valid layer? */
			LayerPtr = &FooImage.Layers[Layer];
			if (LayerPtr->RGB) {
				if (LayerPtr->Alpha) {
					Flag = PSD_FLAG_RGBA;
				} else {
					Flag = PSD_FLAG_RGB;
				}
			} else {
				if (LayerPtr->Alpha) {
					Flag = PSD_FLAG_ALPHA;
				} else {
					Flag = PSD_FLAG_MASK;
				}
			}
			Result = ImageExtractFromPSDImage(Output,LayerPtr,Flag);		/* Extract from image */
		}
		PSDImageDestroy(&FooImage);				/* Dispose of the PSD */
	}
	return Result;								/* Return the result */
}
