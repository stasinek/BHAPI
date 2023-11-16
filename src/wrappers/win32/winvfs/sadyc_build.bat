@set my_target=W2K
@if "%SystemRoot%" == "C:\WINDOWS" set my_target=WXP
@
@set MY_DIR=%CD%
call c:\winddk\3790\bin\setenv.bat c:\winddk\3790 chk %my_target%
@cd /D %MY_DIR%
build
