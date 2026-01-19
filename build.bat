:: build.bat
:: last updated: 18/01/2026 <d/m/y>
@echo off
setlocal enabledelayedexpansion
set CC=gcc
set INCLUDES=-Isrc -Isrc/c_header_files
set WARNINGS=-Wall -Wextra
set LIBS=-lssl -lcrypto -largon2 -ladvapi32
set SRC_DIR=src\c_src_files
set OBJ_DIR=build
set DIST_DIR=dist

if "%1"=="clean" goto :clean
if "%1"=="quick" goto :quick
if "%1"=="rebuild" (
    call :clean
    goto :interactive
)
:interactive
echo ------------------------------------------------------------
echo   build.bat                                                |
echo   You may not question my methods.                         |
echo ------------------------------------------------------------
echo.                                                           |
echo You will be asked a series of questions / preferences.     |
echo Type 1 for "yeah" or 0 for "fuh nah"; starting now.        |
echo.                                                           |
echo ------------------------------------------------------------
:ask_keep_objs
set /p KEEP_OBJS="Do you wish to keep the [.o] files after compiling? >> "
if "%KEEP_OBJS%"=="0" goto :ask_keep_objs_ok
if "%KEEP_OBJS%"=="1" goto :ask_keep_objs_ok
echo Invalid input. Enter 1 or 0.
goto :ask_keep_objs
:ask_keep_objs_ok
echo OK.
echo.
:ask_verbose
set /p VERBOSE="Do you want verbose compilation output? >> "
if "%VERBOSE%"=="0" goto :ask_verbose_ok
if "%VERBOSE%"=="1" goto :ask_verbose_ok
echo Invalid input. Enter 1 or 0.
goto :ask_verbose
:ask_verbose_ok
echo Noted.
echo.
:ask_optimize
set /p OPTIMIZE="Enable aggressive optimizations (-O3)? >> "
if "%OPTIMIZE%"=="0" goto :ask_optimize_ok
if "%OPTIMIZE%"=="1" goto :ask_optimize_ok
echo Invalid input. Enter 1 or 0.
goto :ask_optimize
:ask_optimize_ok
echo Understood.
echo.
:ask_static
set /p STATIC="Create a static build (dependencies at RT; 6MB)? >> "
if "%STATIC%"=="0" goto :ask_static_ok
if "%STATIC%"=="1" goto :ask_static_ok
echo Invalid input. Enter 1 or 0.
goto :ask_static
:ask_static_ok
echo Acknowledged.
echo.

echo ------------------------------------------------------------
echo Processing your choices...
goto :build
:quick
set KEEP_OBJS=0
set VERBOSE=0
set OPTIMIZE=1
set STATIC=0
echo Quick build mode (no questions asked)...
goto :build
:build
echo.
if "%OPTIMIZE%"=="1" (
    set OPT=-O3
    echo [CONFIG] Optimization: -O3 ^(aggressive^)
) else (
    set OPT=-O2
    echo [CONFIG] Optimization: -O2 ^(standard^)
)

if "%STATIC%"=="1" (
    set STATIC_FLAG=-static
    set LIBS=-lssl -lcrypto -largon2 -ladvapi32 -lws2_32 -lcrypt32 -lbcrypt -lgdi32 -luser32 -ldwmapi -lcomctl32 -luxtheme -lcomdlg32
    echo [CONFIG] Static build: enabled ^(this might take a bit lolz^).
    echo [CONFIG] Extra libs: ws2_32, crypt32, bcrypt
) else (
    set STATIC_FLAG=
    set LIBS=-lssl -lcrypto -largon2 -ladvapi32 -lgdi32 -luser32 -ldwmapi -lcomctl32 -luxtheme -lcomdlg32
    echo [CONFIG] Static build: disabled.
)

if "%VERBOSE%"=="1" (
    echo [CONFIG] Verbose output: enabled.
) else (
    echo [CONFIG] Verbose output: disabled.
)
echo.
if not exist %OBJ_DIR% mkdir %OBJ_DIR%
if not exist %DIST_DIR% mkdir %DIST_DIR%
set OBJS=
set COUNT=0
for %%f in (%SRC_DIR%\*.c) do (
    set /a COUNT+=1
    set SOURCE=%%f
    set BASENAME=%%~nf
    if "%VERBOSE%"=="1" (
        echo [CC] !SOURCE! -^> %OBJ_DIR%\!BASENAME!.o
        %CC% %INCLUDES% %WARNINGS% %OPT% -c !SOURCE! -o %OBJ_DIR%\!BASENAME!.o
    ) else (
        echo [CC] !BASENAME!.c
        %CC% %INCLUDES% %WARNINGS% %OPT% -c !SOURCE! -o %OBJ_DIR%\!BASENAME!.o 2>nul
    )
    if errorlevel 1 (
        echo.
        echo [ERROR] Compilation failed for !SOURCE!
        exit /b 1
    )
    set OBJS=!OBJS! %OBJ_DIR%\!BASENAME!.o
)
echo.
echo [LINK] Linking final executable...
if "%VERBOSE%"=="1" (
    %CC% %OBJS% -o %DIST_DIR%\binf.exe %LIBS% %STATIC_FLAG%
) else (
    %CC% %OBJS% -o %DIST_DIR%\binf.exe %LIBS% %STATIC_FLAG% 2>nul
)
if errorlevel 1 (
    echo ERROR: Linking failed!
    exit /b 1
)
if "%KEEP_OBJS%"=="0" (
    echo.
    echo [CLEAN] Removing object files...
    rmdir /s /q %OBJ_DIR%
    echo [CLEAN] Build directory cleaned.
) else (
    echo.
    echo [KEEP] Object files preserved in %OBJ_DIR%\
)
echo.
echo ------------------------------------------------------------
echo   Finished compiling                                       |
echo ------------------------------------------------------------
echo   Output: %DIST_DIR%\binf.exe                              |
for %%A in (%DIST_DIR%\binf.exe) do echo   Size: %%~zA bytes    |
echo ------------------------------------------------------------
exit /b 0
:clean
echo Purging all build artifacts...
if exist %OBJ_DIR% rmdir /s /q %OBJ_DIR%
if exist %DIST_DIR% rmdir /s /q %DIST_DIR%
echo [CLEAN] Cleaned.
exit /b 0

:: end