all: build

build: src/zx.c
	gcc $(shell pkg-config xcb --libs --cflags) -c src/xcb_helper.c -o src/xcb_helper.o -Iinclude
	gcc $(shell pkg-config xcb glib-2.0 i3ipc-glib-1.0 --libs --cflags) src/xcb_helper.o -Iinclude src/zx.c -o zx

install: build
	$(shell cp ./zx /usr/bin/zx)
