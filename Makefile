# OpenBSD package name; this may need to instead be "tcl" using MacPorts
TCLPKG=tcl85

realexec.o: realexec.c
	$(CC) -std=c99 `pkg-config --cflags $(TCLPKG)` -c $(CFLAGS) -fPIC -o realexec.o realexec.c

realexec.so: realexec.o
	$(CC) -std=c99 `pkg-config --libs $(TCLPKG)` -lc -shared -fPIC -Wl,-soname,librealexec.so.1 -o realexec.so realexec.o

realexec.dylib: realexec.o
	$(CC) -std=c99 `pkg-config --libs $(TCLPKG)` -dynamiclib -prebind -headerpad_max_install_names -Wl,-search_paths_first -Wl,-single_module -current_version 1.0.0 -compatibility_version 1.0.0 -o realexec.dylib realexec.o

clean:
	@-rm -f *.o *.dylib *.so *.core
