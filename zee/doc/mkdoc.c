/*
 * A Quick & Dirty tool to produce the AUTODOC file.
 */

#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* #include other sources so this program can be easily built on the
   build host when cross-compiling */
#include "strrstr.c"
#include "vasprintf.c"
#include "zmalloc.c"
#include "astr.c"

struct fentry {
  char	*name;
  char	*key1;
  char	*key2;
  astr	doc;
} fentry_table[] = {
#define X0(zee_name, c_name) \
  { zee_name, NULL, NULL, NULL },
#define X1(zee_name, c_name, key1) \
  { zee_name, key1, NULL, NULL },
#define X2(zee_name, c_name, key1, key2) \
  { zee_name, key1, key2, NULL },
#include "tbl_funcs.h"
#undef X0
#undef X1
#undef X2
};

#define fentry_table_size (sizeof fentry_table  / sizeof fentry_table[0])

struct ventry {
  char	*name;
  char	*fmt;
  char	*defvalue;
  char	*doc;
} ventry_table[] = {
#define X(name, fmt, defvalue, doc) \
	{ name, fmt, defvalue, doc },
#include "tbl_vars.h"
#undef X
};

#define ventry_table_size (sizeof ventry_table / sizeof ventry_table[0])

FILE *input_file;

static void fdecl(const char *name)
{
  astr doc = astr_new();
  astr buf;
  unsigned s = 0, i;

  while ((buf = astr_fgets(input_file)) != NULL) {
    if (s == 1) {
      if (!strncmp(astr_cstr(buf), "+*/", 3))
        break;
      astr_cat(doc, buf);
      astr_cat_char(doc, '\n');
    }
    if (!strncmp(astr_cstr(buf), "/*+", 3))
      s = 1;
    else if (astr_cstr(buf)[0] == '{')
      break;
    astr_delete(buf);
  }

  for (i = 0; i < fentry_table_size; ++i)
    if (!strcmp(name, fentry_table[i].name))
      fentry_table[i].doc = doc;
}

static void parse(void)
{
  astr buf;

  while ((buf = astr_fgets(input_file)) != NULL) {
    if (!strncmp(astr_cstr(buf), "DEFUN(", 6) ||
        !strncmp(astr_cstr(buf), "DEFUN_INT(", 10)) {
      int i, j;
      astr sub;
      i = astr_find_cstr(buf, "\"");
      j = astr_rfind_cstr(buf, "\"");
      if (i < 0 || j < 0 || i == j) {
        fprintf(stderr, "mkdoc: invalid DEFUN() syntax\n");
        exit(1);
      }
      sub = astr_substr(buf, i + 1, (size_t)(j - i - 1));
      astr_cpy(buf, sub);
      astr_delete(sub);
      fdecl(astr_cstr(buf));
    }
    astr_delete(buf);
  }
}

static astr texinfo_subst(astr as)
{
  ptrdiff_t i;
  for (i = 0; i < (ptrdiff_t)astr_len(as); i++) {
    int c = *astr_char(as, i);
    if (c == '@' || c == '{' || c == '}') {
      astr_insert_char(as, i, '@');
      i++;
    }
  }
  return as;
}

static void dump_help(void)
{
  unsigned i;
  for (i = 0; i < fentry_table_size; ++i) {
    astr doc = fentry_table[i].doc;
    if (doc)
      fprintf(stdout, "\fF_%s\n%s",
              fentry_table[i].name, astr_cstr(doc));
  }
  for (i = 0; i < ventry_table_size; ++i)
    fprintf(stdout, "\fV_%s\n%s\n%s\n",
            ventry_table[i].name, ventry_table[i].defvalue,
            ventry_table[i].doc);
}

static void dump_key_table(void)
{
  unsigned i;
  FILE *fp = fopen("zee_keys.texi", "w");

  assert(fp);
  fprintf(fp, "@c Automatically generated file: DO NOT EDIT!\n");
  fprintf(fp, "@table @kbd\n");

  for (i = 0; i < fentry_table_size; ++i) {
    astr doc = fentry_table[i].doc;
    if (!doc || astr_len(doc) == 0) {
      fprintf(stderr, "mkdoc: no docstring for %s\n", fentry_table[i].name);
      exit(1);
    }
    if (fentry_table[i].key1) {
      astr key1 = texinfo_subst(astr_cat_cstr(astr_new(), fentry_table[i].key1));
      doc = texinfo_subst(astr_substr(doc, 0, (size_t)astr_find_cstr(doc, "\n")));
      fprintf(fp, "@item");
      fprintf(fp, " %s", astr_cstr(key1));
      astr_delete(key1);
      if (fentry_table[i].key2) {
        astr key2 = texinfo_subst(astr_cat_cstr(astr_new(), fentry_table[i].key2));
        fprintf(fp, ", %s", astr_cstr(key2));
        astr_delete(key2);
      }
      fprintf(fp, "\n%s  Bound to @code{%s}.\n",
              astr_cstr(doc), fentry_table[i].name);
      astr_delete(doc);
    }
  }

  fprintf(fp, "@end table");
  fclose(fp);
}

static void dump_func_table(void)
{
  unsigned i;
  FILE *fp = fopen("zee_funcs.texi", "w");

  assert(fp);
  fprintf(fp, "@c Automatically generated file: DO NOT EDIT!\n");
  fprintf(fp, "@table @code\n");

  for (i = 0; i < fentry_table_size; ++i) {
    astr doc = fentry_table[i].doc;
    if (!doc || astr_len(doc) == 0) {
      fprintf(stderr, "mkdoc: no docstring for %s\n", fentry_table[i].name);
      exit(1);
    }
    fprintf(fp, "@item %s\n%s", fentry_table[i].name, astr_cstr(doc));
  }

  fprintf(fp, "@end table");
  fclose(fp);
}

static void dump_var_table(void)
{
  unsigned i;
  FILE *fp = fopen("zee_vars.texi", "w");

  assert(fp);
  fprintf(fp, "@c Automatically generated file: DO NOT EDIT!\n");
  fprintf(fp, "@table @code\n");

  for (i = 0; i < ventry_table_size; ++i) {
    astr doc = astr_cat_cstr(astr_new(), ventry_table[i].doc);
    if (!doc || astr_len(doc) == 0) {
      fprintf(stderr, "mkdoc: no docstring for %s\n", ventry_table[i].name);
      exit(1);
    }
    fprintf(stderr, "%s\n%s\n", ventry_table[i].name, astr_cstr(doc));
    fprintf(fp, "@item %s\n%s\n", ventry_table[i].name, astr_cstr(doc));
    astr_delete(doc);
  }

  fprintf(fp, "@end table");
  fclose(fp);
}

static void process_file(char *filename)
{
  if (filename != NULL && strcmp(filename, "-") != 0 &&
      (input_file = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "mkdoc:%s: %s\n",
            filename, strerror(errno));
    exit(1);
  }

  parse();

  fclose(input_file);
}

/*
 * Stub to make zmalloc &c. happy.
 */
void zee_exit(int exitcode)
{
  exit(exitcode);
}

int main(int argc, char **argv)
{
  int i;
  for (i = 1; i < argc; i++)
    process_file(argv[i]);

  dump_help();
  dump_key_table();
  dump_func_table();
  dump_var_table();

  return 0;
}
