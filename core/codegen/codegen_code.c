/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : PERL generation                              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/****************************************************************************/

/*
 * $Revision: 374 $
 * $Date: 2007-02-27 19:24:08 +0100 (mar., 27 févr. 2007) $
 */

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "install.h"
#include "codegen_tree.h"
#include "codegen_criticity.h"
#include "codegen_code.h"


static void code_indent(FILE *f, int depth)
{
  while ( depth-- > 0 )
    fprintf(f, "  ");
}

static void code_case_def_params(FILE *f, char *params)
{
  if ( params != NULL )
    fprintf(f, " %s", params);
}


static int code_case_is_generic(tree_case_t *tcase)
{
  if ( tcase->loop > 1 )
    return 0;
  if ( tcase->script->params != NULL )
    return 0;
  return 1;
}

static void code_case_def_function(FILE *f, tree_item_t *item)
{
  tree_case_t *tcase;
  int indent = 0;
  char *pref;

  if ( item->object->type != TYPE_CASE )
    return;
  tcase = item->object->d.Case;

  if ( code_case_is_generic(tcase) )
    return;

  /* Write case description */
  code_indent(f, indent);
  fprintf(f, "##\n");
  code_indent(f, indent);
  fprintf(f, "## Test Case %s", item->name);
  if ( item->comment != NULL )
    fprintf(f, ": \"%s\"", item->comment);
  fprintf(f, "\n");
  code_indent(f, indent);
  fprintf(f, "##\n");

  /* Define case code */
  code_indent(f, indent);
  fprintf(f, "sub _CASE_%s {\n", item->name);
  indent++;

  code_indent(f, indent);
  fprintf(f, "my $test = (defined &%s::EVENTS) ? \\&%s::EVENTS : \\&%s::TEST;\n",
          tcase->script->package, tcase->script->package, tcase->script->package);

  code_indent(f, indent);
  fprintf(f, "my $verdict;\n");
  code_indent(f, indent);
  fprintf(f, "my $criticity;\n");

  /* Test loop */
  pref = (tcase->loop > 1) ? "_":"";
  if ( tcase->loop > 1 ) {
    code_indent(f, indent);
    fprintf(f, "$verdict = %d;\n", VERDICT_PASSED);

    code_indent(f, indent);
    fprintf(f, "for (my $count = 0; ($count < %d) && ($verdict == %d); $count++) {\n", tcase->loop, VERDICT_PASSED);
    indent++;

    code_indent(f, indent);
    fprintf(f, "my $%scriticity;\n", pref);
  }

  /* Test event prologue */
  code_indent(f, indent);
  fprintf(f, "TestFarm::Engine::case(\"%s\");\n", item->name);

  /* Call EVENTS or TEST script function */
  code_indent(f, indent);
  fprintf(f, "($verdict, $%scriticity) = &{$test}(", pref);
  code_case_def_params(f, tcase->script->params);
  fprintf(f, ");\n");

  /* Test event epilogue */
  code_indent(f, indent);
  fprintf(f, "TestFarm::Engine::done();\n");

  /* Call VERDICT script function */
  code_indent(f, indent);
  fprintf(f, "($verdict, $%scriticity) = %s::VERDICT(", pref, tcase->script->package);
  code_case_def_params(f, tcase->script->params);
  fprintf(f, ") if defined &%s::VERDICT;\n", tcase->script->package);

  /* Test loop block end */
  if ( tcase->loop > 1 ) {
    code_indent(f, indent);
    fprintf(f, "if ( (defined $%scriticity) && ((!defined $criticity) || ($criticity < $%scriticity)) ) {\n", pref, pref);
    code_indent(f, indent+1);
    fprintf(f, "$criticity = $%scriticity;\n", pref);
    code_indent(f, indent);
    fprintf(f, "}\n");

    indent--;
    code_indent(f, indent);
    fprintf(f, "}\n");
  }

  code_indent(f, indent);
  fprintf(f, "return ($verdict, $criticity);\n");

  indent--;
  code_indent(f, indent);
  fprintf(f, "}\n\n");
}


static void code_case_def(FILE *f, tree_t *tree)
{
  int i;

  for (i = 0; i < tree->nmemb; i++) {
    code_case_def_function(f, tree->items[i]);
  }
}


static GList *code_tree_add_use(GList *modules, char *name)
{
  /* Add module if not found in list */
  if ( g_list_find_custom(modules, name, (GCompareFunc) strcmp) == NULL )
    modules = g_list_append(modules, name);

  return modules;
}


static void code_tree_def_use_item(char *name, FILE *f)
{
  fprintf(f, "  require '%s';\n", name);
}


static void code_tree_def_use(FILE *f, tree_t *tree)
{
  GList *modules = NULL;
  int i;

  /* Build list of modules */
  for (i = 0; i < tree->nmemb; i++) {
    tree_item_t *item = tree->items[i];

    if ( item->object->type == TYPE_CASE ) {
      tree_case_t *tcase = item->object->d.Case;

      if ( tcase->script != NULL )
        modules = code_tree_add_use(modules, tcase->script->name);
    }
  }

  /* Dump list of used modules */
  fprintf(f, "BEGIN {\n");
  g_list_foreach(modules, (GFunc) code_tree_def_use_item, f);
  fprintf(f, "}\n\n");

  /* Free module list */
  g_list_free(modules);
}


static void code_tree_def_tab(FILE *f, tree_t *tree)
{
  int i;

  /* Declare table of test case functions */
  fprintf(f, "BEGIN {\n");
  fprintf(f, "  @_TAB = (\n");
  for (i = 0; i < tree->nmemb; i++) {
    tree_item_t *item = tree->items[i];

    if ( item->object->type == TYPE_CASE ) {
      fprintf(f, "    { 'ID'      => \"%s\"", item->name);

      if ( ! code_case_is_generic(item->object->d.Case) )
        fprintf(f, ",\n      'HANDLER' => \\&_CASE_%s", item->name);

      if ( strcmp(item->name, item->object->d.Case->script->package) )
        fprintf(f, ",\n      'PACKAGE' => \"%s\"", item->object->d.Case->script->package);

      fprintf(f, " }");
      if ( i < (tree->nmemb-1) )
	fprintf(f, ",");
      fprintf(f, "\n");
    }
  }
  fprintf(f, "  );\n");
  fprintf(f, "}\n\n");
}


static void code_tree_def(FILE *f, tree_t *tree)
{
  /* Write test suite description */
  fprintf(f, "##\n");
  fprintf(f, "## TestFarm PERL script\n");
  fprintf(f, "## DO NOT EDIT -- DO NOT EDIT -- DO NOT EDIT -- DO NOT EDIT\n");
  fprintf(f, "## This file is automatically generated by TestFarm code generator\n");
  fprintf(f, "##\n");
  fprintf(f, "## Test Suite %s", tree->head->name);
  if ( tree->head->comment != NULL )
    fprintf(f, ": \"%s\"", tree->head->comment);
  fprintf(f, "\n");
  fprintf(f, "##\n\n");

  /* Import used modules */
  fprintf(f, "use TestFarm::Env;\n");
  fprintf(f, "use TestFarm::Exec;\n");
  if ( tree->system != NULL )
    fprintf(f, "use %s;\n", tree->system);
  fprintf(f, "\n");

  code_tree_def_use(f, tree);

  /* Declare test suite tables */
  code_tree_def_tab(f, tree);
}


static void code_tree_run(FILE *f, tree_t *tree)
{
  fprintf(f, "TestFarm::Exec::MainInit($ARGV[0], $ARGV[1]);\n");

  if ( tree->system != NULL )
    fprintf(f, "START() && exit(1);\n");

  /* Run test suite program */
  fprintf(f, "TestFarm::Exec::MainLoop($ARGV[2], $ARGV[3], \"%s\", @_TAB);\n", (tree->system != NULL) ? tree->system : "");

  /* Shutdown code */
  if ( tree->system != NULL )
    fprintf(f, "STOP();\n");
  fprintf(f, "exit(0);\n");
}


void code_build(FILE *f, tree_t *tree)
{
  if ( tree == NULL )
    return;

  code_tree_def(f, tree);
  code_case_def(f, tree);
  code_tree_run(f, tree);
}
