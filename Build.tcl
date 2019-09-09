# ./Build.tcl - this must be run via the ./Build exec wrapper so that a
# tclConfig.sh can be sourced and relevant environment variables set. I
# asked the Freenode #tcl folks at some point whether TCL could do what
# tclConfig.sh does itself but that did not go anywhere
#
# (this script is modeled somewhat on the Perl Module::Build module)

package require Tcl 8.5

proc die {msg} {
    global argv0
    puts stderr "$argv0: $msg"
    exit 1
}

proc build {} {
    global env
    set ext $env(TCL_SHLIB_SUFFIX)
    if {$ext eq ".so"} {
        set link "-shared -Wl,-soname,librealexec${ext}.1"
    } elseif {$ext eq ".dylib"} {
        set link "-dynamiclib -prebind -current_version 1.0.0 -compatibility_version 1.0.0"
    } else {
        die "unknown extension $ext"
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
    global argv env
    set dir [lindex $argv 1]
    if {$dir eq ""} { die "install requires a target directory" }
    set ext $env(TCL_SHLIB_SUFFIX)
    if {![file isfile librealexec$ext]} { build }
    set dest [file join $dir realexec]
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
die "unknown command: $cmd"
