@echo off

set CommonCompilerFlags=-MTd -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4456 -wd4505 -wd4996 -wd4312 -wd4661 -wd4127 -DQLIB_INTERNAL=1 -DQLIB_SLOW=0 -DSAVE_IMAGES=1 -DQLIB_WIN32=1 -DVSYNC=0 -FC -Z7

set CommonLinkerFlags= -incremental:no -opt:ref Dxgi.lib %CD%\qlib\sdl-vc\lib\x64\SDL2.lib %CD%\qlib\sdl-vc\lib\x64\SDL2main.lib

REM copy %CD%\qlib\sdl-vc\lib\x64\SDL2.dll %CD%\..\..\build

pushd %CD%
IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

cl %CommonCompilerFlags% ..\game\code\application.cpp /link %CommonLinkerFlags%

REM cl %CommonCompilerFlags% ..\game\code\server.cpp /link %CommonLinkerFlags%

popd
