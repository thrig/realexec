/* try to test that the TCL realexec package works */

#include <sys/wait.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

#include <tcl.h>

Tcl_Interp *Interp = NULL;
Tcl_Obj *Script = NULL;

void run_test(void (*testfn) (void), char *msg);
void test_env(void);
void test_exec(void);
void test_load(void);
void test_name(void);

int main(void)
{
    setvbuf(stdout, (char *) NULL, _IOLBF, (size_t) 0);
    run_test(test_load, "simple package require");
    run_test(test_exec, "exec 'echo foo'");
    run_test(test_env, "custom env (only PATH and USER)");
    run_test(test_name, "./progname only named rosebud");
    exit(0);
}

void run_test(void (*testfn) (void), char *msg)
{
    int status;
    pid_t pid;

    printf("# %s\n", msg);

    if ((pid = fork()) < 0)
        err(EX_OSERR, "fork failed");
    if (pid == 0) {
        if ((Interp = Tcl_CreateInterp()) == NULL)
            errx(EX_OSERR, "Tcl_CreateInterp failed");
        if (Tcl_Init(Interp) == TCL_ERROR)
            errx(EX_OSERR, "Tcl_Init failed");
        (*testfn) ();
        abort();
    }
    if (waitpid(pid, &status, 0) < 0)
        err(EX_OSERR, "waitpid failed");
    if (status != 0)
        err(1, "non-zero exit for '%s': %d", msg, status);
}

void test_env(void)
{
    Script =
        Tcl_NewStringObj("set auto_path [linsert $auto_path 0 [pwd]];"
                         "package require realexec;"
                         "real_exec -env {PATH=/usr/bin:/bin USER=jhqdoe} env;"
                         "puts {no bueno}", -1);
    Tcl_IncrRefCount(Script);
    if (Tcl_EvalObjEx(Interp, Script, TCL_EVAL_GLOBAL) != TCL_OK)
        errx(1, "TCL error: %s", Tcl_GetStringResult(Interp));
    exit(42);
}

void test_exec(void)
{
    Script =
        Tcl_NewStringObj("set auto_path [linsert $auto_path 0 [pwd]];"
                         "package require realexec;" "real_exec echo foo;"
                         "puts {no bueno}", -1);
    Tcl_IncrRefCount(Script);
    if (Tcl_EvalObjEx(Interp, Script, TCL_EVAL_GLOBAL) != TCL_OK)
        errx(1, "TCL error: %s", Tcl_GetStringResult(Interp));
    exit(42);
}

void test_load(void)
{
    Script =
        Tcl_NewStringObj("set auto_path [linsert $auto_path 0 [pwd]];"
                         "puts [package require realexec];", -1);
    Tcl_IncrRefCount(Script);
    if (Tcl_EvalObjEx(Interp, Script, TCL_EVAL_GLOBAL) != TCL_OK)
        errx(1, "TCL error: %s", Tcl_GetStringResult(Interp));
    exit(0);
}

void test_name(void)
{
    Script =
        Tcl_NewStringObj("set auto_path [linsert $auto_path 0 [pwd]];"
                         "package require realexec;"
                         "real_exec -name rosebud ./progname;"
                         "puts {no bueno}", -1);
    Tcl_IncrRefCount(Script);
    if (Tcl_EvalObjEx(Interp, Script, TCL_EVAL_GLOBAL) != TCL_OK)
        errx(1, "TCL error: %s", Tcl_GetStringResult(Interp));
    exit(42);
}
