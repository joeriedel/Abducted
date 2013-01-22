#include "StString.h"

/**********************************

	Mac OS specific code

**********************************/

#if defined(__MAC__)

#include "ClStdLib.h"
#include <string.h>
#include <Resources.h>
#include <TextUtils.h>

/* Classic version */

#if !TARGET_API_MAC_CARBON

/**********************************

	Return the name of the computer

**********************************/

void BURGERCALL StrGetComputerName(char* Output,Word OutputSize)
{
	StringHandle MyString;

	if (OutputSize) {	/* Is there a buffer? */

		short OldResFile;
		OldResFile = CurResFile();		/* Get my resource reference */
		UseResFile(0);					/* Use the Mac system reference */
		MyString = GetString(-16413);	/* Get the machine name */
		UseResFile(OldResFile);			/* Restore the resource reference */

		--OutputSize;					/* Valid chars */

		if (!MyString || !MyString[0][0]) {	/* Did I get a name? */

		/* The name wasn't present, use the default */

			strncpy(Output,"Computer",OutputSize);	/* Copy to buffer */
		} else {
			Word Len;
			Len = MyString[0][0];		/* Get the length of the string */
			if (Len<OutputSize) {
				OutputSize = Len;
			}
			FastMemCpy(Output,&MyString[0][1],OutputSize);
		}
		Output[OutputSize] = 0;		/* Make SURE it's zero terminated! */
	}
}

/**********************************

	Return the name of the logged in user

**********************************/

void BURGERCALL StrGetUserName(char* Output,Word OutputSize)
{
	StringHandle MyString;

	if (OutputSize) {	/* Is there a buffer? */

		short OldResFile;
		OldResFile = CurResFile();		/* Get my resource reference */
		UseResFile(0);					/* Use the Mac system reference */
		MyString = GetString(-16096);	/* Get the user name */
		UseResFile(OldResFile);			/* Restore the resource reference */

		--OutputSize;				/* Valid chars */

		if (!MyString || !MyString[0][0]) {	/* Did I get a name? */

		/* The name wasn't present, use the default */

			strncpy(Output,"User",OutputSize);	/* Copy to buffer */
		} else {
			Word Len;
			Len = MyString[0][0];		/* Get the length of the string */
			if (Len<OutputSize) {
				OutputSize = Len;
			}
			FastMemCpy(Output,&MyString[0][1],OutputSize);
		}
		Output[OutputSize] = 0;		/* Make SURE it's zero terminated! */
	}
}

#else

/* Carbon version */

#include "McMac.h"

typedef const struct __SCDynamicStore *	SCDynamicStoreRef;

/**********************************

	Return the name of the computer

**********************************/

void BURGERCALL StrGetComputerName(char* Output,Word OutputSize)
{
	StringHandle MyString;

	if (OutputSize) {	/* Is there a buffer? */
		if (MacOSGetOSVersion()<0x1000) {
			short OldResFile;
			OldResFile = CurResFile();		/* Get my resource reference */
			UseResFile(0);					/* Use the Mac system reference */
			MyString = GetString(-16413);	/* Get the machine name */
			UseResFile(OldResFile);			/* Restore the resource reference */

			--OutputSize;					/* Valid chars */

			if (!MyString || !MyString[0][0]) {	/* Did I get a name? */

			/* The name wasn't present, use the default */

				strncpy(Output,"Computer",OutputSize);	/* Copy to buffer */
			} else {
				Word Len;
				Len = MyString[0][0];		/* Get the length of the string */
				if (Len<OutputSize) {
					OutputSize = Len;
				}
				FastMemCpy(Output,&MyString[0][1],OutputSize);
			}
			Output[OutputSize] = 0;		/* Make SURE it's zero terminated! */
		} else {
			MacOSXFramework_t LibRef;
			if (!MacOSXFrameworkInit(&LibRef,"SystemConfiguration.framework")) {
				CFStringRef (*SCDynamicStoreCopyComputerName)(SCDynamicStoreRef store,CFStringEncoding	*nameEncoding);

				SCDynamicStoreCopyComputerName = (CFStringRef(*)(SCDynamicStoreRef,CFStringEncoding	*))MacOSXFrameworkGetProc(&LibRef,"SCDynamicStoreCopyComputerName");
				if (SCDynamicStoreCopyComputerName) {
					CFStringRef StringRef;
					StringRef = SCDynamicStoreCopyComputerName(0,0);		/* Return the computer name */
					CFStringGetCString(StringRef,Output,OutputSize,kCFStringEncodingMacRoman);	/* Perform the conversion */
					CFRelease(StringRef);									/* Dispose of the string ref */
					if (!Output[0]) {
						strncpy(Output,"Computer",OutputSize);
					}
					Output[OutputSize-1] = 0;
				}
				MacOSXFrameworkDestroy(&LibRef);
			}
		}
	}
}

/**********************************

	Return the name of the logged in user

**********************************/

void BURGERCALL StrGetUserName(char* Output,Word OutputSize)
{
	if (OutputSize) {	/* Is there a buffer? */
		char *MyString;
		char Buf[1024];

		MyString = "User";		/* Default */

		if (MacOSGetOSVersion()<0x1000) {
			short OldResFile;
			StringHandle TheString;
			
			OldResFile = CurResFile();		/* Get my resource reference */
			UseResFile(0);					/* Use the Mac system reference */
			TheString = GetString(-16096);	/* Get the user name */
			UseResFile(OldResFile);			/* Restore the resource reference */

			--OutputSize;				/* Valid chars */

			if (TheString && TheString[0][0]) {	/* Did I get a name? */
				Word Len;
				Len = TheString[0][0];		/* Get the length of the string */
				FastMemCpy(Buf,&TheString[0][1],Len);
				Buf[Len] = 0;
				MyString = Buf;
			}
		} else {
			MacOSXFramework_t LibRef;

			if (!MacOSXFrameworkInit(&LibRef,"Foundation.framework")) {
				CFStringRef (*NSFullUserName)(void);

				NSFullUserName = (CFStringRef(*)(void))MacOSXFrameworkGetProc(&LibRef,"NSFullUserName");
				if (NSFullUserName) {
					CFStringRef TempString;

					--OutputSize;
					TempString = NSFullUserName();
					if (TempString) {
						Buf[0] = 0;
						CFStringGetCString(TempString,Buf,sizeof(Buf),kCFStringEncodingMacRoman);	/* Perform the conversion */
						if (Buf[0]) {	/* Did I get a name? */
							MyString = Buf;
						}
					}
				}
				MacOSXFrameworkDestroy(&LibRef);
			}
		}
		strncpy(Output,MyString,OutputSize);		/* Get the length of the string */
		Output[OutputSize] = 0;		/* Make SURE it's zero terminated! */		
	}
}


#endif

/**********************************

	Mac OSX specific code

**********************************/

#elif defined(__MACOSX__)

#include <Foundation/NSPathUtilities.h>
#include <SystemConfiguration/SCDynamicStoreCopySpecific.h>
#include <string.h>

/**********************************

	Return the name of the computer

**********************************/

void BURGERCALL StrGetComputerName(char* Output,Word OutputSize)
{
	CFStringRef StringRef;

	if (OutputSize) {	/* Is there a buffer? */
		StringRef = SCDynamicStoreCopyComputerName(0,0);		/* Return the computer name */
		CFStringGetCString(StringRef,Output,OutputSize,kCFStringEncodingMacRoman);	/* Perform the conversion */
		CFRelease(StringRef);									/* Dispose of the string ref */
		if (!Output[0]) {
			strncpy(Output,"Computer",OutputSize);
		}
		Output[OutputSize-1] = 0;
	}
}

/**********************************

	Return the name of the logged in user

**********************************/

void BURGERCALL StrGetUserName(char* Output,Word OutputSize)
{
	if (OutputSize) {
#if 1
		const char *MyString;
		NSString *TempString;

		--OutputSize;
		TempString = NSFullUserName();
		MyString = [TempString cString];		/* Get the string name */
		if (!MyString || !MyString[0]) {	/* Did I get a name? */

		/* The name wasn't present, use the default */

			MyString = "User";				/* Copy to buffer */
		}
		strncpy(Output,MyString,OutputSize);		/* Get the length of the string */
		Output[OutputSize] = 0;		/* Make SURE it's zero terminated! */
#else

		/* Returns the SHORT user name */
		
		CFStringRef StringRef;
		StringRef = SCDynamicStoreCopyConsoleUser(0,0,0);		/* Return the computer name */
		CFStringGetCString(StringRef,Output,OutputSize,kCFStringEncodingMacRoman);	/* Perform the conversion */
		CFRelease(StringRef);									/* Dispose of the string ref */
		if (!Output[0]) {
			strncpy(Output,"Computer",OutputSize);
		}
		Output[OutputSize-1] = 0;
#endif
	}
}

#endif