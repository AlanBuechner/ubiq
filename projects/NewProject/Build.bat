@ECHO OFF
SETLOCAL

for /f "tokens=1,2 delims==" %%a in (local.ini) do (
    if %%a==EngineLoc set EngineLoc=%%b
)

cd %EngineLoc%
call "vendor/python/python.exe" scripts/Build.py %*
