@echo off
echo Setting environment for minimal Visual C++ 6
set MSVCDir=Z:\home\a1ba\projects\Xash3D\mainui_cpp\vsmini
set INCLUDE=%MSVCDir%\include
set LIB=%MSVCDir%\lib
set PATH=%MSVCDir%\bin;%PATH%

echo -- Compiler is MSVC6

set XASH3DSRC=..\..\Xash3D_original
set INCLUDES=-I..\ -I..\font -I..\controls -I..\menus -I%XASH3DSRC% -I%XASH3DSRC%\engine -I%XASH3DSRC%\pm_shared -I%XASH3DSRC%\common -I..\utl
set SOURCES=..\*.cpp ..\controls\*.cpp ..\menus\*.cpp ..\font\*.cpp ..\menus\dynamic\*.cpp
set DEFINES=/DMY_COMPILER_SUCKS /DPRERELEASE_INTERFACE /DMAINUI_USE_CUSTOM_FONT_RENDER /DXASH_DISABLE_FWGS_EXTENSIONS /Dstrcasecmp=stricmp
set LIBS=user32.lib gdi32.lib
set OUTNAME=menu.dll
;set DEBUG=/debug

cl %DEFINES% %LIBS% %SOURCES% %INCLUDES% -o %OUTNAME% /link /dll /out:%OUTNAME% %DEBUG%

echo -- Compile done. Cleaning...

del *.obj *.exp *.lib *.ilk
echo -- Done.
