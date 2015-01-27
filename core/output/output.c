/****************************************************************************/
/* TestFarm                                                                 */
/* Test Output management                                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 19-FEB-2001                                                    */
/****************************************************************************/

/*
 * $Revision: 772 $
 * $Date: 2007-10-11 15:58:52 +0200 (jeu., 11 oct. 2007) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/time.h>
#include <glib.h>

#include "useful.h"
#include "codegen.h"

#include "validate.h"
#include "output.h"

#include "output_xml.h"


/*---------------------------------------------------*/
/* Getting current date in milliseconds              */
/*---------------------------------------------------*/

static long long output_date(void)
{
  struct timeval tv;

  gettimeofday(&tv, NULL);

  return (((long long) tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}


/*---------------------------------------------------*/
/* Writing Test Cases node                           */
/*---------------------------------------------------*/

void output_write_case(output_t *out, tree_object_t *object)
{
  char *name = object->parent_item->name;
  char *comment = object->parent_item->comment;
  tree_object_t *parent_seq;
  output_sequence_t *oseq;
  output_case_t *ocase;
  validate_t *v;
  int checked;

  if ( out == NULL )
    return;
  if ( object->type != TYPE_CASE )
    return;

  /* Check node is healthy */
  if ( object->parent_seq == NULL ) {
    fprintf(stderr, "*PANIC* Orphan Test Case detected\n");
    return;
  }

  /* Create parent sequence if not already done */
  parent_seq = object->parent_seq;
  oseq = output_sequence_find(out->sequences, parent_seq->parent_item->name);
  if ( oseq == NULL ) {
    oseq = output_sequence_alloc(parent_seq->parent_item->name,
                                 parent_seq->parent_item->comment,
                                 tree_object_has_seq(parent_seq) ? 0:1);
    out->sequences = g_list_append(out->sequences, oseq);
  }

  /* Build string with test case number and name */
  if ( name == NULL )
    name = "";
  if ( comment == NULL )
    comment = "";

  /* Perform validation check */
  v = validate_object_data(object);
  checked = validate_check(v, object);

  /* Retrieve / Alloc Test Case entry */
  ocase = output_case_find(oseq->cases, name);
  if ( ocase == NULL ) {
    ocase = output_case_alloc(name, comment);
    out->cases = g_list_append(out->cases, ocase);
    oseq->cases = g_list_append(oseq->cases, ocase);
    ocase->seq = oseq;
  }

  /* Init Test Case info */
  ocase->default_criticity = object->d.Case->criticity;
  ocase->info.verdict = VERDICT_UNEXECUTED;
  ocase->info.criticity = CRITICITY_NONE;
  ocase->info.validated = checked && (v->level == validate_max);

  /* Set Test Case as current */
  out->cur_case = ocase;

  /* Clear test case timer */
  ocase->begin = output_date();

  /* Write Test Case to Test Output file */
  ocase->offset = output_xml_case(OUTPUT_XML(out->file), object);
  if ( checked )
    output_xml_validated(OUTPUT_XML(out->file), v);
}


/*---------------------------------------------------*/
/* Writing Verdicts                                  */
/*---------------------------------------------------*/

static void output_compute_stat_item(output_case_t *ocase, output_stat_t *stat)
{
  output_stat_verdict(stat, ocase->info.verdict, ocase->info.criticity);
}


static void output_compute_stat(output_t *out)
{
  output_stat_clear(&(out->stat));
  g_list_foreach(out->cases, (GFunc) output_compute_stat_item, &(out->stat));
}


void output_write_verdict(output_t *out, int verdict, int criticity)
{
  long long now = output_date();

  if ( out == NULL )
    return;

  /* Update current test case */
  if ( out->cur_case != NULL ) {
    out->cur_case->end = now;
    out->cur_case->info.verdict = verdict;
    out->cur_case->info.elapsed = now - out->cur_case->begin;
    if ( criticity < 0 )
      criticity = out->cur_case->default_criticity;
    out->cur_case->info.criticity = criticity;
  }

  /* Update total elapsed time */
  output_stat_elapsed(&(out->stat), (now - out->tree.begin));

  /* Update verdict statistics */
  output_compute_stat(out);

  /* Write Test Output file */
  output_xml_verdict(OUTPUT_XML(out->file), verdict, criticity);
}


/*---------------------------------------------------*/
/* Writing Standard I/O's                            */
/*---------------------------------------------------*/

void output_write_stdio(output_t *out, int channel, char *str)
{
  if ( out == NULL )
    return;

  output_xml_stdio(OUTPUT_XML(out->file), channel, str);
}


/*---------------------------------------------------*/
/* Test Output file write                            */
/*---------------------------------------------------*/

void output_write_clear(output_t *out)
{
  if ( out == NULL )
    return;

  /* Close output file if needed */
  output_write_close(out);

  /* Clear verdict statistics */
  output_stat_clear(&(out->stat));

  /* Free list of sequences */
  if ( out->sequences != NULL ) {
    g_list_foreach(out->sequences, (GFunc) output_sequence_free, NULL);
    g_list_free(out->sequences);
    out->sequences = NULL;
  }

  /* Free list of test cases */
  if ( out->cases != NULL ) {
    g_list_foreach(out->cases, (GFunc) output_case_free, NULL);
    g_list_free(out->cases);
    out->cases = NULL;
  }

  out->cur_case = NULL;
}


int output_write_open(output_t *out)
{
  if ( out == NULL )
    return -1;

  /* Clear Test Output file management */
  output_write_clear(out);

  /* Init global timer */
  out->tree.begin = output_date();

  /* Open Test Output file */
  return output_xml_open(OUTPUT_XML(out->file), &(out->tree));
}


void output_write_close(output_t *out)
{
  if ( out == NULL )
    return;

  /* Close Test Output file */
  output_xml_close(OUTPUT_XML(out->file));
}


/*---------------------------------------------------*/
/* Reading Standard I/O's                            */
/*---------------------------------------------------*/

void output_read_node_stdio(output_t *out, char *name,
			    output_file_func_t *func, void *arg,
			    char *filter)
{
  output_case_t *ocase = NULL;

  /* Find node if name is specified */
  if ( name != NULL ) {
    ocase = output_case_find(out->cases, name);

    /* Abort if Test Case not found */
    if ( ocase == NULL )
      return;
  }

  output_xml_read(OUTPUT_XML(out->file), ocase, func, arg, filter);
}


/*---------------------------------------------------*/
/* Output merge with log file                        */
/*---------------------------------------------------*/

int output_merge(output_t *out, char *log_name)
{
  int ret;

  if ( out == NULL )
    return -1;

  /* Merge log file to Test Output file */
  ret = output_xml_merge(OUTPUT_XML(out->file), log_name);

  return ret;
}


/*---------------------------------------------------*/
/* Output file name suffix                           */
/*---------------------------------------------------*/

char *output_suffix(char *filename)
{
  char *suffix = strrchr(filename, '.');
  if ( (suffix == NULL) || (strcmp(suffix, ".out") != 0) )
    suffix = filename + strlen(filename);
  return suffix;
}



/*---------------------------------------------------*/
/* Output descriptor init/destroy                    */
/*---------------------------------------------------*/

char *output_get_current_node_name(output_t *out)
{
  char *name;

  if ( out->cur_case == NULL )
    name = out->tree.name;
  else
    name = out->cur_case->info.name;

  return name;
}


void output_set_operator(output_t *out, char *name)
{
  output_tree_operator(&(out->tree), name);
}


void output_set_release(output_t *out, char *name)
{
  output_tree_release(&(out->tree), name);
}


void output_set_filename(output_t *out, char *filename)
{
  /* Set new Test Output file name */
  output_xml_filename(OUTPUT_XML(out->file), filename);
}


output_t *output_alloc(tree_t *codegen_tree)
{
  /* Alloc output file management descriptor */
  output_t *out = (output_t *) malloc(sizeof(output_t));

  /* Init tree settings */
  output_tree_init(&(out->tree), codegen_tree);

  /* Clear verdict statistics */
  output_stat_init(&(out->stat), codegen_tree);

  /* Clear list of sequences */
  out->sequences = NULL;
  out->cases = NULL;
  out->validates = NULL;
  out->cur_case = NULL;

  /* Alloc Test Output file */
  out->file = (output_file_t *) output_xml_alloc();

  return out;
}


void output_destroy(output_t *out)
{
  if ( out == NULL )
    return;

  /* Clear Test Output file */
  output_write_clear(out);

  /* Free Test Output file */
  output_xml_free(OUTPUT_XML(out->file));

  output_tree_clear(&(out->tree));

  /* Free the Validation State descriptors that were allocated by output file load */
  g_list_foreach(out->validates, (GFunc) validate_free, NULL);
  g_list_free(out->validates);

  free(out);
}
