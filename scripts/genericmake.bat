SETLOCAL ENABLEDELAYEDEXPANSION

SET CMakeBuildType=%1

IF "!CMakeBuildType!" == "Release" (
    SET EnableTests=OFF
) ELSE (
    SET CMakeBuildType=Debug
    SET EnableTests=ON
)

FOR %%X IN (ninja.exe) DO (
    SET NinjaFound=%%~$PATH:X
)

IF DEFINED NinjaFound (
    SET Generator=Ninja
) ELSE (
    SET Generator=JOM
)

IF "!CMakeBuildType!" == "Release" (
    SET TitleTests=
) ELSE (
    SET TitleTests= and its tests
)

TITLE Making OpenCOR!TitleTests! (using !Generator!)...

CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\amd64\vcvars64.bat"

CD build

IF DEFINED NinjaFound (
    SET CMakeGenerator=Ninja
) ELSE (
    SET CMakeGenerator=NMake Makefiles JOM
)

cmake -G "!CMakeGenerator!" -DCMAKE_BUILD_TYPE=!CMakeBuildType! -DENABLE_TESTS=!EnableTests! ..

SET ExitCode=!ERRORLEVEL!

IF !ExitCode! EQU 0 (
    FOR /F "TOKENS=1,* DELIMS= " %%X IN ("%*") DO (
        SET Args=%%Y
    )

    IF DEFINED NinjaFound (
        ninja !Args!

        SET ExitCode=!ERRORLEVEL!
    ) ELSE (
        jom !Args!

        SET ExitCode=!ERRORLEVEL!
    )
)

CD ..

EXIT /B !ExitCode!
