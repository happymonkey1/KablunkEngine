@echo off
pushd %~dp0\..\
cd ..
git submodule update --init --recursive --remote
popd
PAUSE