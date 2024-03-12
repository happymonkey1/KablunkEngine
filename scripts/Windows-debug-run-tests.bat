@echo off
pushd %~dp0\..\
call "bin/Debug-windows-x86_64/KablunkEngineTests/KablunkEngineTests.exe"
popd
pause