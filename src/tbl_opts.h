// Table of command-line options

/*
 * The first column specifies the long name.
 * The second column specifies whether it takes a parameter.
 * The third column is the documentation.
 * N.B. The options' order must correspond to the code in main.c.
 */

X("batch", 0, "            run non-interactively\n")
X("eval", 1, " CMD         execute command CMD\n")
X("load", 1, " FILE        load file of commands\n")
X("norc", 0, "             do not load ~/.zeerc\n")
X("help", 0, "             display this help message and exit\n")
X("version", 0, "          display version information and exit\n")
