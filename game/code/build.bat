@echo off

set CommonCompilerFlags=-MTd -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4456 -wd4505 -wd4996 -wd4312 -DSNAKE_INTERNAL=1 -DSNAKE_SLOW=1 -DSNAKE_WIN32=1 -DSAVE_IMAGES=1 -FC -Z7
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

REM TODO - can we just build both with one exe?

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

REM 32-bit build
REM cl %CommonCompilerFlags% ..\snake\code\win32_snake.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%

REM 64-bit build
del *.pdb > NUL 2> NUL
REM cl %CommonCompilerFlags% ..\game\code\snake.cpp -Fmsnake.map -LD /link -incremental:no -opt:ref -PDB:snake_%random%.pdb -EXPORT:GameGetSoundSamples -EXPORT:GameUpdateAndRender
cl %CommonCompilerFlags% ..\game\code\win32_snake.cpp -Fmwin32_snake.map /link %CommonLinkerFlags%
REM cl %CommonCompilerFlags% ..\game\code\testserver.cpp -Fmtestserver.map /link %CommonLinkerFlags%
popd
