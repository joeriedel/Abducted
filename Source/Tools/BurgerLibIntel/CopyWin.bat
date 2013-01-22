@echo off
@xcopy /y /q PublicHeaders\*.h c:\Burgerlib\DOS
@xcopy /y /q PublicHeaders\*.hpp c:\Burgerlib\DOS
@xcopy /y /q PublicHeaders\*.h c:\Burgerlib\Win95
@xcopy /y /q PublicHeaders\*.hpp c:\Burgerlib\Win95
@xcopy /y win95msc\*.lib c:\burgerlib\win95
@xcopy /y source\BRString.h c:\burgerlib\win95
@xcopy /y source\BRString.h c:\burgerlib\DOS
