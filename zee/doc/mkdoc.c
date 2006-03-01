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

#define NAME "mkdoc"

struct fentry {
  astr name;
  astr doc;
};

static vector *ftable;
static size_t fentries = 0;

static void fdecl(FILE *fp, astr name)
{
  int state = 0;
  astr doc = astr_new(""), line;

  while ((line = astr_fgets(fp)) != NULL) {
    if (state == 1) {
      if (astr_ncmp(line, astr_new("\")"), 3) == 0
          || astr_ncmp(line, astr_new("\","), 3) == 0) {
        state = 2;
        break;
      }
      astr_cat(doc, line);
      astr_cat_char(doc, '\n');
    } else if (astr_ncmp(line, astr_new("\"\\"), 3) == 0)
      state = 1;
  }

  if (astr_len(doc) == 0) {
    fprintf(stderr, NAME ": no docstring for %s\n", astr_cstr(name));
    exit(1);
  } else if (state == 1) {
    fprintf(stderr, NAME ": unterminated docstring for %s\n", astr_cstr(name));
    exit(1);
  }

  vec_item(ftable, fentries++, struct fentry) = (struct fentry){astr_dup(name), doc};
}

static void get_funcs(FILE *fp)
{
  astr buf;

  while ((buf = astr_fgets(fp)) != NULL) {
    if (!astr_ncmp(buf, astr_new("DEF("), 4) ||
        !astr_ncmp(buf, astr_new("DEF_ARG("), 8)) {
      const char *s = astr_cstr(buf);
      char *p = strchr(s, '(');
      char *q = strrchr(s, ',');
      if (p == NULL || q == NULL || p == q) {
        fprintf(stderr, NAME ": invalid DEF[_ARG]() syntax `%s'\n", s);
        exit(1);
      }
      fdecl(fp, astr_sub(buf, (p - s) + 1, q - s));
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
    fprintf(fp, "@item %s\n%s", astr_cstr(vec_item(ftable, i, struct fentry).name),
            astr_cstr(vec_item(ftable, i, struct fentry).doc));
    fprintf(stdout, "X(%s,\n\"\\\n%s\")\n", astr_cstr(vec_item(ftable, i, struct fentry).name),
            astr_cstr(vec_item(ftable, i, struct fentry).doc));
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
    astr doc = astr_new(vtable[i].doc);
    if (!doc || astr_len(doc) == 0) {
      fprintf(stderr, NAME ": no docstring for %s\n", vtable[i].name);
      exit(1);
    }
    fprintf(fp, "@item %s\n%s\n", vtable[i].name, astr_cstr(doc));
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
    astr doc = astr_new(otable[i].doc);
    if (!doc || astr_len(doc) == 0) {
      fprintf(stderr, NAME ": no docstring for --%s\n", otable[i].longname);
      exit(1);
    }
    fprintf(fp, "@item --%s\n%s\n", otable[i].longname, astr_cstr(doc));
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
