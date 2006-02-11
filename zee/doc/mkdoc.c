/*
 * A quick & dirty tool to produce various documentation and header files
 */

#include "config.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Stub to make zmalloc etc. happy */
#define die exit

/* #include other sources so this program can be easily built on the
   build host when cross-compiling */
#include "vasprintf.c"
#include "zmalloc.c"
#include "astr.c"
#include "vector.c"

#define NAME "mkdoc"

struct fentry {
  const char *name;
  astr doc;
};

static vector *ftable;

static size_t fentries = 0;

static struct {
  char *name;
  char *fmt;
  char *defvalue;
  char *doc;
} vtable[] = {
#define X(name, fmt, defvalue, doc) \
	{ name, fmt, defvalue, doc },
#include "tbl_vars.h"
#undef X
};

#define ventries (sizeof vtable / sizeof vtable[0])

static struct {
  char *longname;
  char *doc;
  char shortname;
} otable[] = {
#define X(longname, doc, shortname, opt) \
        { longname, doc, shortname },
#include "tbl_opts.h"
#undef X
};

#define oentries (sizeof otable / sizeof otable[0])

static FILE *input_file;

static void add_func(const char *name, astr doc)
{
  struct fentry func;
  func.name = zstrdup(name);
  func.doc = doc;
  vec_item(ftable, fentries++, struct fentry) = func;
}

static void fdecl(const char *name)
{
  astr doc = astr_new();
  astr buf;
  size_t s = 0, i;

  while ((buf = astr_fgets(input_file)) != NULL) {
    if (s == 1) {
      if (!strncmp(astr_cstr(buf), "+*/", 3)) {
        if (doc == NULL || astr_cmp_cstr(doc, "\n") == 0) {
          fprintf(stderr, NAME ": no docstring for %s\n", name);
          exit(1);
        }
        break;
      }
      astr_cat(doc, buf);
      astr_cat_char(doc, '\n');
    }
    if (!strncmp(astr_cstr(buf), "/*+", 3))
      s = 1;
    else if (astr_cstr(buf)[0] == '{')
      break;
    astr_delete(buf);
  }

  /* Check function is not a duplicate */
  for (i = 0; i < fentries; i++)
    if (strcmp(name, vec_item(ftable, i, struct fentry).name) == 0) {
      fprintf(stderr, NAME ": duplicate function %s\n", name);
      exit(1);
    }

  add_func(name, doc);
}

static void parse(void)
{
  astr buf;

  while ((buf = astr_fgets(input_file)) != NULL) {
    if (!strncmp(astr_cstr(buf), "DEFUN(", 6) ||
        !strncmp(astr_cstr(buf), "DEFUN_INT(", 10)) {
      char *p, *q;
      astr sub;
      p = strchr(astr_cstr(buf), '"');
      q = strrchr(astr_cstr(buf), '"');
      if (p == NULL || q == NULL || p == q) {
        fprintf(stderr, NAME ": invalid DEFUN() syntax\n");
        exit(1);
      }
      sub = astr_substr(buf, (p - astr_cstr(buf)) + 1, (size_t)(q - p - 1));
      astr_cpy(buf, sub);
      astr_delete(sub);
      fdecl(astr_cstr(buf));
    }
    astr_delete(buf);
  }
}

static void dump_help(void)
{
  size_t i;
  for (i = 0; i < fentries; ++i) {
    astr doc = vec_item(ftable, i, struct fentry).doc;
    if (doc)
      fprintf(stdout, "\fF_%s\n%s",
              vec_item(ftable, i, struct fentry).name, astr_cstr(doc));
  }
  for (i = 0; i < ventries; ++i)
    fprintf(stdout, "\fV_%s\n%s\n%s\n",
            vtable[i].name, vtable[i].defvalue,
            vtable[i].doc);
}

static void dump_funcs(void)
{
  size_t i;
  FILE *fp1 = fopen("zee_funcs.texi", "w");
  FILE *fp2 = fopen("tbl_funcs.h", "w");

  assert(fp1);
  fprintf(fp1,
          "@c Automatically generated file: DO NOT EDIT!\n"
          "@table @code\n");

  assert(fp2);
  fprintf(fp2,
          "/*\n"
          " * Table of commands (name, C function)\n"
          " */\n"
          "\n");

  for (i = 0; i < fentries; ++i) {
    char *cname = zstrdup(vec_item(ftable, i, struct fentry).name), *p;

    for (p = strchr(cname, '-'); p != NULL; p = strchr(p, '-'))
      *p = '_';

    fprintf(fp1, "@item %s\n%s", vec_item(ftable, i, struct fentry).name, astr_cstr(vec_item(ftable, i, struct fentry).doc));
    fprintf(fp2, "X(\"%s\", %s)\n", vec_item(ftable, i, struct fentry).name, cname);
  }

  fprintf(fp1, "@end table");
  fclose(fp1);
  fclose(fp2);
}

static void dump_vars(void)
{
  size_t i;
  FILE *fp = fopen("zee_vars.texi", "w");

  assert(fp);
  fprintf(fp, "@c Automatically generated file: DO NOT EDIT!\n");
  fprintf(fp, "@table @code\n");

  for (i = 0; i < ventries; ++i) {
    astr doc = astr_cat_cstr(astr_new(), vtable[i].doc);
    if (!doc || astr_len(doc) == 0) {
      fprintf(stderr, NAME ": no docstring for %s\n", vtable[i].name);
      exit(1);
    }
    fprintf(fp, "@item %s\n%s\n", vtable[i].name, astr_cstr(doc));
    astr_delete(doc);
  }

  fprintf(fp, "@end table");
  fclose(fp);
}

static void dump_opts(void)
{
  size_t i;
  FILE *fp = fopen("zee_opts.texi", "w");

  assert(fp);
  fprintf(fp, "@c Automatically generated file: DO NOT EDIT!\n");
  fprintf(fp, "@table @samp\n");

  for (i = 0; i < oentries; ++i) {
    astr doc = astr_cat_cstr(astr_new(), otable[i].doc);
    if (!doc || astr_len(doc) == 0) {
      fprintf(stderr, NAME ": no docstring for --%s\n", otable[i].longname);
      exit(1);
    }
    fprintf(fp, "@item --%s, -%c\n%s\n", otable[i].longname, otable[i].shortname,
            astr_cstr(doc));
    astr_delete(doc);
  }

  fprintf(fp, "@end table");
  fclose(fp);
}

static void process_file(char *filename)
{
  if (filename != NULL && strcmp(filename, "-") != 0 &&
      (input_file = fopen(filename, "r")) == NULL) {
    fprintf(stderr, NAME ":%s: %s\n",
            filename, strerror(errno));
    exit(1);
  }

  parse();

  fclose(input_file);
}

int main(int argc, char **argv)
{
  int i;

  ftable = vec_new(sizeof(struct fentry));

  for (i = 1; i < argc; i++)
    process_file(argv[i]);

  dump_help();
  dump_funcs();
  dump_vars();
  dump_opts();

  vec_delete(ftable);

  return 0;
}
