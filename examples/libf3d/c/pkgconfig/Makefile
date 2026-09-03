# Build against libf3d_c_api using pkg-config, without CMake.
# If VTK isn't in a standard system location, export LD_LIBRARY_PATH=/path/to/vtk/lib
# before building and running.
CC ?= cc
CFLAGS += $(shell pkg-config --cflags f3d_c_api)
LDLIBS += $(shell pkg-config --libs f3d_c_api) -Wl,-rpath,$(shell pkg-config --variable=libdir f3d_c_api)

check-engine-pkgconfig: main.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@

.PHONY: clean
clean:
	rm -f check-engine-pkgconfig
