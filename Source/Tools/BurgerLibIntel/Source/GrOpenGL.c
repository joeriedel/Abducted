/**********************************

	Burgerlib OpenGL

**********************************/

#include <BRTypes.h>

#if defined(__WIN32__) || defined(__MAC__)

#define WIN32_LEAN_AND_MEAN
#include "GrGraphics.h"

#if defined(__WIN32__)
#include <windows.h>
#include "W9Win95.h"
#endif

#include <BRGlut.h>
#include "ClStdLib.h"
#include "MmMemory.h"
#include "PlPalette.h"
#include "ShStream.h"
#include "ImImage.h"
#include "LrRect.h"
#include "SsScreenShape.h"

/**********************************

	Initialize the OpenGL context to a default state

**********************************/

void BURGERCALL OpenGLInit(void)
{
//	const char *ExtPtr;			/* Pointer to supported extensions */
	glViewport(0,0,ScreenWidth,ScreenHeight);
//	glLoadIdentity();
//	glOrtho(0,ScreenWidth,0,ScreenHeight,-1,1);
//	ExtPtr = glGetString(GL_EXTENSIONS);
//	if (ExtPtr) {
//	}
}

/**********************************

	Draw a solid rect in OpenGL mode

**********************************/

void BURGERCALL OpenGLSolidRect(int x,int y,Word Width,Word Height,Word32 Color)
{
	ScreenUse2DCoords(TRUE);
	ScreenSetTexture(0);
	glBegin(GL_QUADS);
	glColor4ub((GLubyte) ((Color>>16)&0xFF),(GLubyte) ((Color>>8)&0xFF),(GLubyte)(Color&0xff),(GLubyte)((Color>>24)&0xFF));
	glVertex2i(x,y);
	glVertex2i(x+Width,y);
	glVertex2i(x+Width,y+Height);
	glVertex2i(x,y+Height);
	glEnd();
}

/**********************************

	Draw a texture in 2D

**********************************/

void BURGERCALL OpenGLTextureDraw2D(Word TexNum,int x,int y,Word Width,Word Height)
{
	ScreenUse2DCoords(TRUE);
	ScreenSetTexture(TexNum);
	glBegin(GL_QUADS);
	glColor4ub((GLubyte)0xFF,(GLubyte) 0xFF,(GLubyte)0xFF,(GLubyte)0xFF);
	glTexCoord2f(0.0f,0.0f);
	glVertex2i(x,y);
	glTexCoord2f(1.0f,0.0f);
	glVertex2i(x+Width,y);
	glTexCoord2f(1.0f,1.0f);
	glVertex2i(x+Width,y+Height);
	glTexCoord2f(0.0f,1.0f);
	glVertex2i(x,y+Height);
	glEnd();
}

/**********************************

	Draw a texture in 2D

**********************************/

void BURGERCALL OpenGLTextureDraw2DSubRect(Word TexNum,int x,int y,Word Width,Word Height,const float *UVPtr)
{
	float y1,y2;
	y1=UVPtr[1];
	y2=UVPtr[3];
	ScreenUse2DCoords(TRUE);
	ScreenSetTexture(TexNum);
	glBegin(GL_QUADS);
	glColor4ub((GLubyte)0xFF,(GLubyte) 0xFF,(GLubyte)0xFF,(GLubyte)0xFF);
	glTexCoord2f(UVPtr[0],y1);
	glVertex2i(x,y);
	glTexCoord2f(UVPtr[2],y1);
	glVertex2i(x+Width,y);
	glTexCoord2f(UVPtr[2],y2);
	glVertex2i(x+Width,y+Height);
	glTexCoord2f(UVPtr[0],y2);
	glVertex2i(x,y+Height);
	glEnd();
}

/**********************************

	Upload a texture.
	I try to hint to OpenGL to use the least amount of
	memory to describe the texture

	Note : OpenGL uses these constants for internal format hinting...
	2 = Alpha only
	3 = Color only
	4 = Alpha + Color

**********************************/

Word BURGERCALL OpenGLLoadTexture(Word *TextureNum,const Image_t *ImagePtr)
{
	GLuint TexNum;
	Image_t MyImage;
	Word NewWidth,NewHeight;
	GLint MaxWidth;

	/* Perform a sanity check on the new texture size */

	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&MaxWidth);		/* What's the largest texture I can use? */

	NewWidth = PowerOf2(ImagePtr->Width);		/* First convert the size to a power of 2 */
	NewHeight = PowerOf2(ImagePtr->Height);

	if (NewWidth>(Word)MaxWidth) {		/* Make sure I am not too big!! */
		NewWidth = MaxWidth;
	}
	if (NewHeight>(Word)MaxWidth) {
		NewHeight = MaxWidth;
	}

	/* Allocate a unique texture number */

	glGenTextures(1,&TexNum);
	TextureNum[0] = TexNum;
	glBindTexture(GL_TEXTURE_2D,TexNum);

	/* Now, based on the input texture type, perform the proper upload */

	switch (ImagePtr->DataType) {

	/* Upload a solid texture with no alpha */

	case IMAGE888:		/* Already the right format? */
		if (NewWidth==ImagePtr->Width && NewHeight==ImagePtr->Height) {		/* No resizing? */

			/* Upload right now! */
			glTexImage2D(GL_TEXTURE_2D,0,3,NewWidth,NewHeight,0,GL_RGB,GL_UNSIGNED_BYTE,ImagePtr->ImagePtr);
			break;
		}

	/* Resize and convert to 24 bit color */

	case IMAGE8_PAL:		/* Non alpha types */
	case IMAGE332:
	case IMAGE555:
	case IMAGE565:
		if (ImageInit(&MyImage,NewWidth,NewHeight,IMAGE888)) {		/* Convert to 24 bit color */
			return TRUE;
		}
		ImageStore888(&MyImage,ImagePtr);
		glTexImage2D(GL_TEXTURE_2D,0,3,NewWidth,NewHeight,0,GL_RGB,GL_UNSIGNED_BYTE,MyImage.ImagePtr);
		ImageDestroy(&MyImage);
		break;

	/* Upload a texture with alpha */

	/* Already ready for 32 bit color? */

	case IMAGE8888:
		if (NewWidth==ImagePtr->Width && NewHeight==ImagePtr->Height) {		/* No resizing? */

			/* Upload right now! */
			glTexImage2D(GL_TEXTURE_2D,0,4,NewWidth,NewHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,ImagePtr->ImagePtr);
			break;
		}

	/* Resize and convert to 32 bit color */

	default:
		if (ImageInit(&MyImage,NewWidth,NewHeight,IMAGE8888)) {
			return TRUE;
		}
		ImageStore8888(&MyImage,ImagePtr);
		glTexImage2D(GL_TEXTURE_2D,0,4,NewWidth,NewHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,MyImage.ImagePtr);
		ImageDestroy(&MyImage);
	}
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	return FALSE;		/* I'm ok! */
}


#endif
