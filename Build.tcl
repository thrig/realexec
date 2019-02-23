package require Tcl 8.5

proc build {} {
    global env
    set ext $env(TCL_SHLIB_SUFFIX)
    if {$ext eq ".so"} {
        set link "-shared -Wl,-soname,librealexec${ext}.1"
    } elseif {$ext eq ".dylib"} {
        set link "-dynamiclib -prebind -current_version 1.0.0 -compatibility_version 1.0.0"
    } else {
        puts stderr "$argv0: unknown extension $ext"
        exit 1
    }
    exec {*}"$env(TCL_CC) $env(TCL_CFLAGS_OPTIMIZE) $env(TCL_INCLUDE_SPEC) -Wall -pedantic -c realexec.c -o realexec.o"
    exec {*}"$env(TCL_CC) $env(TCL_SHLIB_CFLAGS) $env(TCL_LIB_SPEC) $env(TCL_LD_FLAGS) $link realexec.o -o librealexec$ext"
}

proc clean {} {
    foreach file [glob -nocomplain *.o *.so *.dylib] { file delete $file }
    foreach file {progname testexec} { file delete $file }
}

proc install {} {
    global env
    set ext $env(TCL_SHLIB_SUFFIX)
    if {![file isfile librealexec$ext]} { build }
    set dest ~/lib/tcl/realexec
    file mkdir $dest
    file copy -force librealexec$ext $dest
    file copy -force pkgIndex.tcl $dest
}

proc test {} {
    global env
    set ext $env(TCL_SHLIB_SUFFIX)
    if {![file isfile librealexec$ext]} { build }
    exec $env(TCL_CC) progname.c -o progname
    exec -ignorestderr {*}"$env(TCL_CC) $env(TCL_CFLAGS_OPTIMIZE) $env(TCL_INCLUDE_SPEC) $env(TCL_LIB_SPEC) -o testexec testexec.c"
    puts [exec ./testexec]
}

set cmd [lindex $argv 0]
if {$cmd eq ""} { set cmd build }
if {$cmd in {build clean install test}} { $cmd; exit 0 }
puts stderr "$argv0: unknown command: $cmd"; exit 1
