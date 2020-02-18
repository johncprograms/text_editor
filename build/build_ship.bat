
call cls

@echo off
call where build_ship.bat 2> nul
if %errorlevel% NEQ 0 (
  echo we expect the current working directory to be the /build/ directory! please cd there, first.
  echo FAILED
  goto :EOF
)
@echo on

call mkdir exe

call copy ..\text_editor.config exe\text_editor_ship.config

call %DMAIN_CL% ^
/arch:AVX2 ^
/c ^
/D_USING_V110_SDK71_ ^
/D_MBCS ^
/DDBG=0 ^
/errorReport:none ^
/FAs ^
/Faexe\ ^
/Foexe\ ^
/FC ^
/fp:precise ^
/Gd ^
/GF ^
/GL ^
/Gm- ^
/GR- ^
/GS ^
/guard:cf ^
/Gw ^
/Gy ^
/J ^
/MT ^
/nologo ^
/Ob2 ^
/Og ^
/Oi ^
/Ot ^
/Qfast_transcendentals ^
/sdl ^
/W4 ^
/WX- ^
/wd4100 ^
/wd4127 ^
/wd4201 ^
/Zc:inline ^
/Zc:wchar_t ^
/Z7 ^
../text_editor.cpp

@echo off
if %errorlevel% NEQ 0 (
  echo FAILED
  goto :EOF
)
@echo on

call %DMAIN_LK% ^
/CGTHREADS:8 ^
/DEBUG ^
/DYNAMICBASE ^
/ERRORREPORT:NONE ^
/GUARD:CF ^
/HIGHENTROPYVA ^
/INCREMENTAL:NO ^
/LTCG:STATUS ^
/MACHINE:X64 ^
/MANIFEST:NO ^
/NOLOGO ^
/NXCOMPAT ^
/OPT:REF,ICF=4 ^
/OUT:"exe\text_editor_ship.exe" ^
/PDB:"exe\text_editor_ship.pdb" ^
/SUBSYSTEM:WINDOWS",5.02" ^
exe/text_editor.obj

@echo.
@echo SUCCESS