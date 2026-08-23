# Build against libf3d using pkg-config, without CMake.
# If VTK isn't in a standard system location, pass e.g.
# LDFLAGS="-Wl,-rpath-link,/path/to/vtk/lib" and LD_LIBRARY_PATH=/path/to/vtk/lib at runtime.
CXX ?= c++
CXXFLAGS += -std=c++20 $(shell pkg-config --cflags f3d)
LDLIBS += $(shell pkg-config --libs f3d) -Wl,-rpath,$(shell pkg-config --variable=libdir f3d)

check-engine-pkgconfig: main.cxx
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@

.PHONY: clean
clean:
	rm -f check-engine-pkgconfig
