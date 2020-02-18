pushd .
call title vs19 64
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\vsdevcmd.bat" ^
-arch=amd64 ^
-host_arch=amd64 ^
-no_logo
popd
set DMAIN_CL="%VCToolsInstallDir%bin\Hostx64\x64\cl.exe"
set DMAIN_LK="%VCToolsInstallDir%bin\Hostx64\x64\link.exe"
set DMAIN_ARCH="64"
