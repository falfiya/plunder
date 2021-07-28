mode := release

ifeq ($(mode), debug)
	cflags += -g
else
	clags += -Ofast
endif

cflags += -nostdlib
cflags += -ffreestanding
cflags += -fno-stack-check
cflags += -fno-stack-protector
cflags += -mno-stack-arg-probe
cflags += -fuse-ld=lld-link
cflags += -lkernel32
cflags += -lshell32
cflags += -lntdll
cflags += -Xlinker /entry:start
cflags += -Xlinker /nodefaultlib
cflags += -Xlinker /subsystem:console
cflags += -Xlinker "/libpath:C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0\um\x64"

bin/clearicns.exe: src/main.c bin Makefile
	clang $< $(cflags) -o $@

run: bin/clearicns.exe
	$<

bin:
	mkdir bin

.PHONY: run
