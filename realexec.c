/* realexec - a real exec(3) exec not the fake exec(n) exec for TCL */

#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <tcl.h>

extern char **environ;

static const char *const options[] = {
    "-env", "-name", "--", NULL
};

enum options {
    REXEC_ENV, REXEC_NAME, REXEC_LAST
};

int Realexec_Init(Tcl_Interp * interp);
static int real_exec(ClientData clientData, Tcl_Interp * interp, int objc,
                     Tcl_Obj * CONST objv[]);

int Realexec_Init(Tcl_Interp * interp)
{
#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, "8", 0) == NULL)
        return TCL_ERROR;
#endif
    if (Tcl_PkgProvide(interp, "realexec", "1.0.0") != TCL_OK)
        return TCL_ERROR;
    Tcl_CreateObjCommand(interp, "real_exec",
                         (Tcl_ObjCmdProc *) real_exec, (ClientData) NULL,
                         (Tcl_CmdDeleteProc *) NULL);
    return TCL_OK;
}

static int real_exec(ClientData clientData, Tcl_Interp * interp, int objc,
                     Tcl_Obj * CONST objv[])
{
    char **args, *command, *name, **newenv = NULL, *progname = NULL;
    int i, j, envLength, index, ret;
    Tcl_Obj **envPtr;

    for (i = 1; i < objc; i++) {
        name = Tcl_GetString(objv[i]);
        if (name[0] != '-')
            break;

        ret =
            Tcl_GetIndexFromObj(interp, objv[i], options, "option", TCL_EXACT,
                                &index);
        if (ret != TCL_OK)
            return ret;

        switch ((enum options) index) {
        case REXEC_ENV:
            if (++i >= objc)
                goto DONE_OPTS;
            ret = Tcl_ListObjGetElements(interp, objv[i], &envLength, &envPtr);
            if (ret != TCL_OK)
                return ret;
            if (envLength > 0) {
                if ((newenv = calloc(envLength + 1, sizeof(char **))) == NULL)
                    err(1, "calloc failed");
                for (j = 0; j < envLength; j++)
                    newenv[j] = Tcl_GetString(envPtr[j]);
            }
            break;
        case REXEC_NAME:
            if (++i >= objc)
                goto DONE_OPTS;
            progname = Tcl_GetString(objv[i]);
            break;
        case REXEC_LAST:
            i++;
            goto DONE_OPTS;
        }
    }

  DONE_OPTS:
    if (i >= objc) {
        Tcl_WrongNumArgs(interp, 1, objv,
                         "?-env ...? ?-name ...? command ?arg ..?]");
        return TCL_ERROR;
    }
    objc -= i;
    objv += i;

    if ((args = calloc(objc + 1, sizeof(char **))) == NULL)
        err(1, "calloc failed");
    for (i = 0; i < objc; i++)
        args[i] = Tcl_GetString(objv[i]);

    command = args[0];
    if (progname != NULL)
        args[0] = progname;

    if (newenv != NULL)
        environ = newenv;

    execvp(command, args);
    err(1, "execvp failed");
}
