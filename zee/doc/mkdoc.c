/*
 * Produce various documentation and header files
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Stub to make zrealloc happy */
#define die exit

/* #include other sources so this program can be easily built on the
   build host when cross-compiling */
#include "zmalloc.c"
#include "astr.c"
#include "vector.c"
#include "rblist.c"

#define NAME "mkdoc"


/* FIXME: Far too many "char *"s. */

struct fentry {
  rblist name;
  rblist doc;
};

static vector *ftable;
static size_t fentries = 0;

static void fdecl(FILE *fp, rblist name)
{
  int state = 0;
  rblist doc = rblist_empty, line;

  while ((line = astr_fgets(fp)) != NULL) {
    if (state == 1) {
      if (rblist_ncompare(line, rblist_from_string("\")"), 3) == 0
          || rblist_ncompare(line, rblist_from_string("\","), 3) == 0) {
        state = 2;
        break;
      }
      doc = rblist_concat(doc, rblist_concat_char(line, '\n'));
    } else if (rblist_ncompare(line, rblist_from_string("\"\\"), 3) == 0)
      state = 1;
  }

  if (rblist_length(doc) == 0) {
    fprintf(stderr, NAME ": no docstring for %s\n", astr_to_string(name));
    exit(1);
  } else if (state == 1) {
    fprintf(stderr, NAME ": unterminated docstring for %s\n", astr_to_string(name));
    exit(1);
  }

  vec_item(ftable, fentries++, struct fentry) = (struct fentry){name, doc};
}

static void get_funcs(FILE *fp)
{
  rblist buf;

  while ((buf = astr_fgets(fp)) != NULL) {
    if (!rblist_ncompare(buf, rblist_from_string("DEF("), 4) ||
        !rblist_ncompare(buf, rblist_from_string("DEF_ARG("), 8)) {
      const char *s = astr_to_string(buf);
      char *p = strchr(s, '(');
      char *q = strrchr(s, ',');
      if (p == NULL || q == NULL || p == q) {
        fprintf(stderr, NAME ": invalid DEF[_ARG]() syntax `%s'\n", s);
        exit(1);
      }
      fdecl(fp, rblist_sub(buf, (size_t)((p - s) + 1), (size_t)(q - s)));
    }
  }
}

static void dump_funcs(void)
{
  FILE *fp = fopen("zee_funcs.texi", "w");

  assert(fp);
  fprintf(fp,
          "@c Automatically generated file: DO NOT EDIT!\n"
          "@table @code\n");

  fprintf(stdout,
          "/*\n"
          " * Automatically generated file: DO NOT EDIT!\n"
          " * Table of commands (name, doc)\n"
          " */\n"
          "\n");

  for (size_t i = 0; i < fentries; ++i) {
    fprintf(fp, "@item %s\n%s", astr_to_string(vec_item(ftable, i, struct fentry).name),
            astr_to_string(vec_item(ftable, i, struct fentry).doc));
    fprintf(stdout, "X(%s,\n\"\\\n%s\")\n", astr_to_string(vec_item(ftable, i, struct fentry).name),
            astr_to_string(vec_item(ftable, i, struct fentry).doc));
  }

  fprintf(fp, "@end table");
  fclose(fp);
}

static struct {
  char *name;
  char *fmt;
  char *defval;
  char *doc;
} vtable[] = {
#define X(name, fmt, defval, doc) \
	{name, fmt, defval, doc},
#include "tbl_vars.h"
#undef X
};
#define ventries (sizeof vtable / sizeof vtable[0])

static void dump_vars(void)
{
  FILE *fp = fopen("zee_vars.texi", "w");

  assert(fp);
  fprintf(fp, "@c Automatically generated file: DO NOT EDIT!\n");
  fprintf(fp, "@table @code\n");

  for (size_t i = 0; i < ventries; ++i) {
    rblist doc = rblist_from_string(vtable[i].doc);
    if (!doc || rblist_length(doc) == 0) {
      fprintf(stderr, NAME ": no docstring for %s\n", vtable[i].name);
      exit(1);
    }
    fprintf(fp, "@item %s\n%s\n", vtable[i].name, astr_to_string(doc));
  }

  fprintf(fp, "@end table");
  fclose(fp);
}

static struct {
  char *longname;
  char *doc;
  int opt;
} otable[] = {
#define X(longname, doc, opt) \
        {longname, doc, opt},
#include "tbl_opts.h"
#undef X
};
#define oentries (sizeof otable / sizeof otable[0])

static void dump_opts(void)
{
  FILE *fp = fopen("zee_opts.texi", "w");

  assert(fp);
  fprintf(fp, "@c Automatically generated file: DO NOT EDIT!\n");
  fprintf(fp, "@table @samp\n");

  for (size_t i = 0; i < oentries; ++i) {
    rblist doc = rblist_from_string(otable[i].doc);
    if (!doc || rblist_length(doc) == 0) {
      fprintf(stderr, NAME ": no docstring for --%s\n", otable[i].longname);
      exit(1);
    }
    fprintf(fp, "@item --%s\n%s\n", otable[i].longname, astr_to_string(doc));
  }

  fprintf(fp, "@end table");
  fclose(fp);
}

int main(int argc, char **argv)
{
  FILE *fp = NULL;

  ftable = vec_new(sizeof(struct fentry));
  for (int i = 1; i < argc; i++) {
    if (argv[i] && (fp = fopen(argv[i], "r")) == NULL) {
      fprintf(stderr, NAME ":%s: %s\n",
              argv[i], strerror(errno));
      exit(1);
    }
    get_funcs(fp);
    fclose(fp);
  }
  dump_funcs();

  dump_vars();
  dump_opts();

  return 0;
}
