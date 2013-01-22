#include "PlPalette.h"

/**********************************

	Convert an RGB color to HSL color
	
**********************************/

void BURGERCALL PaletteRGB2HSL(HSL_t *Output,const RGB_t *Input)
{
	float Red,Green,Blue;
	float Hue,Saturation,Luminance;

	Red = Input->Red;			/* Load into registers */
	Green = Input->Green;
	Blue = Input->Blue;

	/* Which color is the brightest? */
	/* I will also pick the luminance */
	/* Note the 6 patterns */
	
	if (Red >= Green) {
		Luminance = Red;					/* Use red */

		if (Blue > Red) {					/* B>R>G */
			Luminance = Blue;				/* Use blue */
			Saturation = 1.0f - Green;		/* Green is saturation */
			Hue = (Red * (1.0f/ 6.0f)) + 0.666667f;
			
		} else if (Blue > Green) {			/* R>B>G */
		
			Saturation = 1.0f - Green;		/* Green is saturation */
			Hue = ((1.0f - Blue) * (1.0f/ 6.0f)) + 0.833333f;
		
		} else {							/* R>G>B */
			Saturation = 1.0f - Blue;		/* Blue is saturation */
			Hue = Green * (1.0f/ 6.0f);
		}
	} else {
		Luminance = Green;					/* Use green */
		if (Blue > Green) {					/* B>G>R */
			Luminance = Blue;				/* Use blue */
			Saturation = 1.0f - Red;		/* Red is saturation */
			Hue = (Blue * (1.0f/ 6.0f)) + 0.333333f;

		} else if (Blue > Red) {			/* G>B>R */
			Saturation = 1.0f - Red;		/* Red is saturation */
			Hue = ((1.0f - Green) * (1.0f/ 6.0f)) + 0.5f;
		} else {							/* G>R>B */
			Saturation = 1.0f - Blue;		/* Blue is saturation */
			Hue = ((1.0f - Red) * (1.0f/ 6.0f)) + 0.166667f;	
		}
	}
	
	/* Save the result */
	
	Output->Hue = Hue;
	Output->Saturation = Saturation;
	Output->Luminance = Luminance;
}

/**********************************

	Convert an HSL color to RGB
	
**********************************/

void BURGERCALL PaletteHSL2RGB(RGB_t *Output,const HSL_t *Input)
{
	float Hue,Saturation,Luminance;
	float Red,Green,Blue;
	
	Hue = Input->Hue;
	Saturation = Input->Saturation;
	Luminance = Input->Luminance;

	/* There are 6 hue parts */
	
	if (Hue < 0.5f) {					/* First half of the color wheel */
		if (Hue < 0.166667f) { 
			Red = 1.0f;					/* R = 1, B = 0, G = H */
			Green = Hue * 6.0f;				/* Green section */
			Blue = 0.0f;
		} else {
			Green = 1.0f;
			if (Hue < 0.33333333f) {	/* R = -H, G = 1, B = 0 */
				Red = 1.0f - ((Hue - 0.166667f) * 6.0f);
				Blue = 0.0f;
			} else { 					/* R = 0, G = 1, B = H */
				Red = 0.0f;
				Blue = (Hue - 0.3333333f) * 6.0f;
			}
		}
	} else {							/* Second half of the color wheel */
		if (Hue >= 0.833333f) {			/* R = 1, G = 0, B = -H */
			Red = 1.0f;
			Green = 0.0f;
			Blue = 1.0f - ((Hue - 0.833333f) * 6.0f);
		} else {
			Blue = 1.0f;
			if (Hue < 0.666667f) {  	/* R = 0, G = -H, B = 1 */
				Red = 0.0f;
				Green = 1.0f - ((Hue - 0.5f) * 6.0f);
			} else {					/* R = H, G = 0, B = 1 */
				Red = (Hue - 0.666667f) * 6.0f;
				Green = 0.0f;
			}
		}
	}

	/* Apply saturation */
	
	Red = 1.0f - (Saturation * (1.0f - Red));
	Green = 1.0f - (Saturation * (1.0f - Green));
	Blue = 1.0f - (Saturation * (1.0f - Blue));

	/* Apply luminosity */

	Output->Red = Red * Luminance;
	Output->Green = Green * Luminance;
	Output->Blue = Blue * Luminance;
}

/**********************************

	Return an inbetween of HSL values.
	0.0 is the first color, 1.0 is the second color
	all Factors between will return the color at that
	point between the two colors
	FALSE on the direction goes from red->green->blue while
	TRUE goes blue->green->red
	
**********************************/

void BURGERCALL PaletteHSLTween(HSL_t *Output,const HSL_t *HSLPtr1,const HSL_t *HSLPtr2,float Factor,Word Dir)
{
	float Temp;
	float Temp2;
	
	/* Tween the hue */

	Temp = HSLPtr1->Hue;		/* Cache */
	Temp2 = HSLPtr2->Hue;
	
	if (!Dir) {					/* Red->Green->Blue */
		if (Temp2 >= Temp) {		/* Increase to tween? */
			Temp = Temp + (Factor * (Temp2 - Temp));
		} else {
			Temp = Temp + (Factor * (1.0f - (Temp - Temp2)));
			if (Temp > 1.0f) {		/* Did it wrap? */
				Temp -= 1.0f;
			}
		}
	} else { 					/* Blue->Green->Red */
		if (Temp >= Temp2) {
			Temp = Temp - (Factor * (Temp - Temp2));
		} else {
			Temp = Temp - (Factor * (1.0f - (Temp2 - Temp)));
			if (Temp < 0.0f) {
				Temp += 1.0f;
			}
		}
	}
	Output->Hue = Temp;

	/* Tween saturation */
	
	Temp = HSLPtr1->Saturation;
	Output->Saturation = Temp + (Factor * (HSLPtr2->Saturation - Temp));

	/* Tween luminosity */

	Temp = HSLPtr1->Luminance;
	Output->Luminance = Temp + (Factor * (HSLPtr2->Luminance - Temp));
}

/**********************************

	Return an inbetween of RGB values.
	Use the HSL system to more correctly follow the color wheel
	
**********************************/

void BURGERCALL PaletteRGBTween(RGB_t *Output,const RGB_t *RGBPtr1,const RGB_t *RGBPtr2,float Factor,Word Dir)
{
	HSL_t HSL2;
	HSL_t HSL1;
	HSL_t NewHSL;

	PaletteRGB2HSL(&HSL1,RGBPtr1);		/* Convert to HSL */
	PaletteRGB2HSL(&HSL2,RGBPtr2);
	PaletteHSLTween(&NewHSL,&HSL1,&HSL2,Factor,Dir);	/* Get the tweened value */
	PaletteHSL2RGB(Output,&NewHSL);		/* Return as RGB */
}