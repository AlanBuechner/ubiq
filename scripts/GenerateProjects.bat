@echo off

pushd ..
call "vendor/python/python.exe" scripts/Build.py -gs
popd

PAUSE
