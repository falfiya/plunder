vcvars := "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" &&
rcedit := misc/rcedit-x64.exe

bin/clearicns.exe: src/main.cxx
	wsl mkdir -p bin
	clang++ -std=c++17 -Wall -fno-rtti -fno-exceptions -Os -fuse-ld=lld-link -lshell32 $< -o $@
	$(rcedit) $@ --set-icon misc/icon.ico

.PHONY: run
