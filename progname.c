/*

tclsh> real_exec -name blah ./progname
blah

*/
#include <stdio.h>
int main(int argc, char *argv[])
{
    puts(*argv);
    return 0;
}
