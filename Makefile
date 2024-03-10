LLVM := $(shell test -d /opt/homebrew/opt/llvm/bin && echo /opt/homebrew/opt/llvm/bin)
export CC := $(if $(LLVM),$(LLVM)/clang,$(CC))
export CXX := $(if $(LLVM),$(LLVM)/clang++,$(CXX))

all: builddir main

.PHONY: main run clean rel .FORCE

builddir:
	meson setup builddir -Db_sanitize=address

main: builddir .FORCE
	meson compile -C builddir

builddir-rel:
	meson setup builddir-rel --buildtype=release -Db_lto=true

rel: builddir-rel .FORCE
	meson compile -C builddir-rel

run: main
	@./builddir/main -g dataset/a.txt -k 5

clean:
	rm -rf builddir builddir-rel

.FORCE:
