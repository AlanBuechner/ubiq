@echo off

pushd ..
pushd ..
call "vendor/python/python.exe" scripts/Build.py -gs -g %~dp0
popd

PAUSE
