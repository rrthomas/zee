/* Table of command-line options */

/*
 * The first column specifies the long name.
 * The second column is the documentation.
 * The third column specifies the short namee.
 * The fourth column specifies whether it takes a parameter.
 */

X("batch", "            run non-interactively\n",
  0)
X("eval", " CMD         execute command CMD\n",
  1)
X("help", "             display this help message and exit\n",
  0)
X("load", " FILE        load file of commands using the load function\n",
  1)
X("no-init-file", "     do not load ~/." PACKAGE_NAME "\n",
  0)
X("version", "          display version information and exit\n",
  0)
