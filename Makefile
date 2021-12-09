n := plunder
mode := release

ifeq ($(mode), debug)
	cflags += -g
endif

ifeq ($(mode), release)
	cflags += -Ofast
endif

libpath := $(shell misc/winsdk.exe --type:lib --kit:um --arch:x64)

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
cflags += -Xlinker /libpath:"$(libpath)"

bin/$n.exe: src/$n.c bin Makefile
	clang $< $(cflags) -o $@
ifeq ($(mode), release)
	llvm-strip $@
endif

run: bin/$n.exe
	$<

bin:
	mkdir bin

.PHONY: run
