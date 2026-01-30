@ECHO OFF

VERIFY OTHER 2>nul
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION
IF NOT ERRORLEVEL 0 (
  echo Unable to enable extensions
)

FOR /F "delims=" %%A IN ('cd') DO SET "ORIGINAL_DIR=%%A"
ECHO orig dir: %ORIGINAL_DIR%

SET "logFolder=.\_build_logs"

GOTO :main

:doCommand
SET "logFile=%logFolder%\%~1.log"
SET "command=%~2"
ECHO %command%>"%logFile%" 2>&1
%command%>>"%logFile%" 2>&1
EXIT /B %ERRORLEVEL%

:main

cd "%SCRIPT_DIR%"

RMDIR /S /Q "%ORIGINAL_DIR%\%logFolder%"
RMDIR /S /Q "%ORIGINAL_DIR%\_dest"
RMDIR /S /Q "%ORIGINAL_DIR%\.xmake"
RMDIR /S /Q "%ORIGINAL_DIR%\build"

MKDIR "%ORIGINAL_DIR%\%logFolder%"

CALL :doCommand "00_made_build_logs" "echo we did it" && cd>NUL || Goto :END

CALL :doCommand "01_xmake_set_theme" "xmake global --theme=plain" && cd>NUL || Goto :END

CALL :doCommand "02_xmake_configure_debug" "xmake config -vD --plat=windows --arch=x64 --kind=static --mode=debug --runtimes=MD --yes --policies=package.precompiled:n" && cd>NUL || Goto :END

CALL :doCommand "03_xmake_build_debug" "xmake build -a -vD" && cd>NUL || Goto :END

CALL :doCommand "04_xmake_configure_release" "xmake config -vD --plat=windows --arch=x64 --kind=static --mode=release --runtimes=MD --yes --policies=package.precompiled:n" && cd>NUL || Goto :END

CALL :doCommand "05_xmake_build_release" "xmake build -a -vD" && cd>NUL || Goto :END

CALL :doCommand "06_xmake_run" "xmake run -vD SfgGenerator '%ORIGINAL_DIR%'" && cd>NUL || Goto :END

REM CALL :doCommand "10_xmake_test" "xmake test -vD SfgGenerator/*" && cd>NUL || Goto :END

CALL :doCommand "99_copy_built_clap" "copy /B /Y %ORIGINAL_DIR%\build\windows\x64\release\SfgGenerator.clap 'C:\Program Files\Common Files\CLAP\SfgGenerator.clap'" && cd>NUL || Goto :END

:END
cd %ORIGINAL_DIR%
ENDLOCAL
EXIT /B %ERRORLEVEL%
