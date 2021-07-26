vcvars := "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" &&
rcedit := misc/rcedit-x64.exe

# cflags += -Oz
cflags += -g
cflags += -nostdlib
cflags += -ffreestanding
cflags += -fno-stack-check
cflags += -fno-stack-protector
cflags += -mno-stack-arg-probe
cflags += -fuse-ld=lld-link
cflags += -lshell32
cflags += -Xlinker Kernel32.lib
# cflags += -Xlinker Shell32.lib
cflags += -Xlinker /entry:start
cflags += -Xlinker /nodefaultlib
cflags += -Xlinker /subsystem:console
cflags += -Xlinker "/libpath:C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0\um\x64"

bin/clearicns.exe: src/main.c bin
	clang $< $(cflags) -o $@

run: bin/clearicns.exe
	$<

bin:
	mkdir bin

# 	$(rcedit) $@ --set-icon misc/icon.ico
.PHONY: run
