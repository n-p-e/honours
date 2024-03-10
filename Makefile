all: builddir main

builddir:
	meson setup builddir -Db_sanitize=address

.PHONY: main run clean .FORCE
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
