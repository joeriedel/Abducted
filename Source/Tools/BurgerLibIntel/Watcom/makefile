# Build burgerlib for DOS and Windows using
# the OPENWATCOM compiler

# Sanity check

!ifndef %BurgerDir
!error You haven't set a target. Invoke SetX32, SetW32 or SetDos4gw
!endif

# Set the default target

!ifdef BurgerTarget
Target=$(BurgerTarget)
!else ifdef %BurgerTarget
Target=$(%BurgerTarget)
!else
Target=Release
!endif

.extensions:
.extensions: .exe .lib .obj .h .x86 .c .cpp

# Set the work directories for the source

sourcedir = ..\Source\;..\DosSource\;..\Win95Source;..\JPegSource

# To debug, or not to debug, that is the question.

!ifeq Target Debug
copt = $(%BurgerFlagsCDebug)
aopt = $(%BurgerFlagsADebug)
!else ifeq Target Release
copt = $(%BurgerFlagsCRelease)
aopt = $(%BurgerFlagsARelease)
!else ifeq Target DebugFast
copt = $(%BurgerFlagsCDebugFast)
aopt = $(%BurgerFlagsADebugFast)
!else
!error BurgerTarget is not Release, Debug or DebugFast
!endif

# outdir = Name of directory for object files
# libbin = Name of Burgerlib output file
# jpegbin = Name of JPeg6lib output file

outdir = $(%BurgerSuffix)$(Target)
libbin = Burger$(%BurgerSuffix)$(Target).lib
jpegbin = JPeg6b$(%BurgerSuffix)$(Target).lib

# Tell WMAKE where to find the files to work with

.c: $(sourcedir)
.cpp: $(sourcedir)
.x86: $(sourcedir)
.obj: $(outdir)

# Now, set the compiler flags
# Enums = int, chars are signed, structure alignment = 1

allc= $(copt) -i=..\PublicHeaders -i=$(sourcedir)
CL=$(%BurgerC) $(allc) $(%BurgerCFlags)
CP=$(%BurgerCPP) $(allc) $(%BurgerCFlags)
ASM=$(%BurgerAsm) $(aopt) $(%BurgerAFlags)

# Object files to work with for the library

A=$(outdir)\
OBJS=$(A)BRString.obj $(A)BREndian.obj &
	$(A)PkADPCM.obj $(A)PkALaw.obj $(A)PkDeflate.obj $(A)PkDlzss.obj $(A)PkDlzssF.obj &
	$(A)PkDRle.obj $(A)PkInflate.obj $(A)PkLzss.obj $(A)PkMACE.obj $(A)PkOggBitwise.obj &
	$(A)PkOggFraming.obj $(A)PkRle.obj $(A)PkRleF.obj $(A)PkULaw.obj &
	$(A)PkVorbisAnalysis.obj $(A)PkVorbisBitrate.obj $(A)PkVorbisBlock.obj $(A)PkVorbisCodebook.obj &
	$(A)PkVorbisEnvelope.obj $(A)PkVorbisFloor0.obj $(A)PkVorbisFloor1.obj $(A)PkVorbisIIR.obj &
	$(A)PkVorbisInfo.obj $(A)PkVorbislpc.obj $(A)PkVorbislsp.obj $(A)PkVorbisMdct.obj &
	$(A)PkVorbisPsy.obj $(A)PkVorbisRegistry.obj $(A)PkVorbisMapping0.obj $(A)PkVorbisRes0.obj &
	$(A)PkVorbisSharedBook.obj $(A)PkVorbisSmallft.obj $(A)PkVorbisTime0.obj $(A)PkVorbisSynthesis.obj $(A)PkVorbisWindow.obj &
	$(A)DbDebugManager.obj $(A)DmDialogControl.obj $(A)FmFileManager.obj $(A)FmMSDosCode.obj &
	$(A)RsReadStream.obj $(A)FxFixedPoint.obj $(A)FxWatcom.obj $(A)FpFloatingPoint.obj $(A)FpWatcom.obj &
	$(A)FnFontManager.obj $(A)GrGraphicsManager.obj $(A)GrMSDosCode.obj $(A)GrWatcom.obj &
	$(A)GrDraw1.obj $(A)GrDraw2.obj $(A)GrSVGAOn.obj $(A)GrUpdateSVga.obj $(A)eraseshp.obj &
	$(A)GuGuidManager.obj $(A)ImImageManager.obj $(A)ImImage2BMPFile.obj $(A)ImImage2LBMFile.obj &
	$(A)ImImage2PBMFile.obj $(A)ImImage2PCXFile.obj $(A)ImImage2TGAFile.obj $(A)ImImageParseBMP.obj &
	$(A)ImImageParseGIF.obj $(A)ImImageParseJPeg.obj $(A)ImImageParseLBM.obj $(A)ImImageParsePCX.obj $(A)ImImageParsePPat.obj &
	$(A)ImImageParseCicn.obj $(A)ImImageParsePSD.obj $(A)ImImageParseTGA.obj $(A)ImImageParsePict.obj &
	$(A)InForceFeedback.obj $(A)InJoystick.obj $(A)InJoystickMSDOS.obj $(A)InKeyboard.obj &
	$(A)InKeyboardMSDOS.obj $(A)InMouse.obj $(A)InMouseMSDOS.obj &
	$(A)LkLinkListManager.obj $(A)LsListManager.obj $(A)LlLongLong.obj &
	$(A)MMMemoryManager.obj $(A)MMMemoryGeneric.obj $(A)MMMemPool.obj $(A)MMMSDos.obj &
	$(A)MvFlic.obj $(A)MvDPaintAnim.obj $(A)NtNetManager.obj $(A)OCOSCursorManager.obj &
	$(A)PlPaletteManager.obj $(A)PlMSDosCode.obj $(A)PlRGBHSL.obj $(A)PfPrefsManager.obj $(A)PgPrefFile.obj &
	$(A)PrProfileManager.obj $(A)PrWin95Code.obj &
	$(A)RnRandomManager.obj $(A)LrRectManager.obj $(A)RbRedbookManager.obj $(A)RbMSDosCode.obj &
	$(A)RzResourceManager.obj $(A)RQRunQueue.obj $(A)SSScreenShape.obj &
	$(A)SnMADDosCode.obj $(A)SnMADIT.obj $(A)SnMADMADI.obj $(A)SnMADMainDriver.obj &
	$(A)SNMadS3M.obj $(A)SnMADXM.obj $(A)SnDigitalSong.obj $(A)SnSngDosCode.obj &
	$(A)SnDOSCode.obj $(A)SnSoundManager.obj &
	$(A)ClStdLib.obj $(A)ClMSDosAsm.obj $(A)ClMSDosCode.obj &
	$(A)ClFastMemCpy.obj $(A)ClFastMemSet.obj $(A)ClFastMemSet16.obj &
	$(A)ClFastMemSetIntel.obj $(A)ClFastMemSet16Intel.obj &
	$(A)ShStreamHandle.obj $(A)StStringManager.obj &
	$(A)ThThreads.obj $(A)ThWatcom.obj &
	$(A)TkTimeManager.obj $(A)TkMSDosCode.obj $(A)TkRD2X32.obj $(A)X32Base.obj &
	$(A)ClWinCode.obj $(A)DbWin95Code.obj $(A)FmWin95Code.obj $(A)GrWin95Code.obj &
	$(A)InForceFeedbackWin95.obj $(A)InJoystickWin95.obj $(A)InKeyboardWin95.obj &
	$(A)InMouseWin95.obj $(A)NtWinCode.obj $(A)OCWin95Code.obj $(A)PlWin95Code.obj &
	$(A)RsWinCode.obj $(A)SnMADWinCode.obj $(A)SnSngWinCode.obj &
	$(A)SnWinCode.obj $(A)StStringWin95.obj $(A)ThWinCode.obj $(A)TkWin95Code.obj &
	$(A)W9Win95Code.obj $(A)GrOpenGL.obj

JOBJS=$(A)jcapimin.obj $(A)jcapistd.obj $(A)jccoefct.obj $(A)jccolor.obj $(A)jcdctmgr.obj &
	$(A)jchuff.obj $(A)jcinit.obj $(A)jcmainct.obj $(A)jcmarker.obj $(A)jcmaster.obj &
	$(A)jcomapi.obj $(A)jcparam.obj $(A)jcphuff.obj $(A)jcprepct.obj $(A)jcsample.obj &
	$(A)jctrans.obj $(A)jdapimin.obj $(A)jdapistd.obj &
	$(A)jdcoefct.obj $(A)jdcolor.obj $(A)jddctmgr.obj $(A)jdhuff.obj $(A)jdinput.obj &
	$(A)jdmainct.obj $(A)jdmarker.obj $(A)jdmaster.obj $(A)jdmerge.obj $(A)jdphuff.obj &
	$(A)jdpostct.obj $(A)jdsample.obj $(A)jdtrans.obj $(A)jerror.obj &
	$(A)jidctflt.obj $(A)jidctfst.obj $(A)jidctint.obj &
	$(A)jidctred.obj $(A)jmemmgr.obj $(A)jquant1.obj $(A)jquant2.obj &
	$(A)JFDCTFlt.obj $(A)JFDCTFst.obj $(A)JFDCTSlw.obj &
	$(A)JUtils.obj $(A)JVersion.obj

# Create the output directory, or my compiler will barf.

.before
	@set DOS4g=QUIET
	-@md $(outdir)

# This is the default rule

allfiles : $(libbin) $(jpegbin) .SYMBOLIC

# Build the output directory if it isn't present

$(libbin) : $(objs)
	@SET WOW=$(objs)
	WLIB -b -c -n $(libbin) @WOW
	copy $(libbin) $(%BurgerDir)\$(libbin)

$(jpegbin) : $(JOBJS)
	@SET WOW2=$(JOBJS)
	WLIB -b -c -n $(jpegbin) @WOW2
	copy $(jpegbin) $(%BurgerDir)\$(jpegbin)

# This speeds up the building process for Watcom because it
# keeps the apps in memory and doesn't have to reload for every source file
# Note: There is a bug that if the wlib app is loaded, it will not
# get the proper WOW file if a full build is performed

!ifdef __LOADDLL__
!loaddll wcc386 wccd386
!loaddll wpp386 wppd386
!loaddll wlink wlink
#!loaddll wlib wlibd
!endif

# Build the code with auto dependancies

.x86.obj : .AUTODEPEND
	$(ASM) $[*.x86 -fo=$^*.obj

.c.obj : .AUTODEPEND
	$(CP) $[*.c -fo=$^*.obj

.cpp.obj : .AUTODEPEND
	$(CP) $[*.cpp -fo=$^*.obj
