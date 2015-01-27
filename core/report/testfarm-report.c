/****************************************************************************/
/* TestFarm                                                                 */
/* Test Report Generator                                                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 28-MAR-2001                                                    */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <glib.h>

#include "output.h"
#include "output_xml_load.h"
#include "md5.h"
#include "report_log.h"
#include "report.h"

#define NAME "testfarm-report"
#define TITLE "TestFarm Report Generator -- Version " VERSION " (" __DATE__ ")"


static void verbose_options(report_config_t *rc)
{
  if ( rc->fname != NULL )
    printf("  Option file: %s\n", rc->fname);

  printf("  Show IN_TITLE information: %s\n", (rc->conf & REPORT_CONFIG_IN_TITLE) ? "yes":"no");
  printf("  Show IN_HEADER information: %s\n", (rc->conf & REPORT_CONFIG_IN_HEADER) ? "yes":"no");
  printf("  Show IN_VERDICT information: %s\n", (rc->conf & REPORT_CONFIG_IN_VERDICT) ? "yes":"no");
  printf("  Show durations: %s\n", (rc->conf & REPORT_CONFIG_DURATION) ? "yes":"no");
  printf("  Show verdict summary:");
  if ( rc->conf & (REPORT_CONFIG_SCENARIO | REPORT_CONFIG_CASE) ) {
    printf(" (");
    if ( rc->conf & REPORT_CONFIG_SCENARIO ) {
      printf("Scenario");
      if ( rc->conf & REPORT_CONFIG_CASE )
        printf(" and ");
    }
    if ( rc->conf & REPORT_CONFIG_CASE )
      printf("Test Case");
    printf("):");

    if ( rc->conf & REPORT_CONFIG_VERDICT_ANY ) {
      if ( rc->conf & REPORT_CONFIG_VERDICT_PASSED ) printf(" PASSED");
      if ( rc->conf & REPORT_CONFIG_VERDICT_FAILED ) printf(" FAILED");
      if ( rc->conf & REPORT_CONFIG_VERDICT_INCONCLUSIVE ) printf(" INCONCLUSIVE");
      if ( rc->conf & REPORT_CONFIG_VERDICT_SKIP ) printf(" SKIP");
    }
  }
  else {
    printf("no");
  }
  printf("\n");

  printf("  Show detailed output dump:");
  if ( (rc->conf & REPORT_CONFIG_DUMP) && (rc->conf & REPORT_CONFIG_DUMP_ANY) ) {
    if ( rc->conf & REPORT_CONFIG_DUMP_PASSED ) printf(" PASSED");
    if ( rc->conf & REPORT_CONFIG_DUMP_FAILED ) printf(" FAILED");
    if ( rc->conf & REPORT_CONFIG_DUMP_INCONCLUSIVE ) printf(" INCONCLUSIVE");
    if ( rc->conf & REPORT_CONFIG_DUMP_SKIP ) printf(" SKIP");
  }
  else {
    printf("no");
  }
  printf("\n");

  printf("  Generate HTML Test Log files: %s\n", (rc->conf & REPORT_CONFIG_LOG) ? "yes":"no");
}


static int verbose_output(char *out_name)
{
  output_t *out;
  time_t t;
  char date[32];

  /* Load Test Output file */
  out = output_xml_load(out_name);
  if ( out == NULL )
    return -1;

  printf("  Test Suite: %s\n", out->tree.name);

  t = (time_t) (out->tree.begin / 1000);
  strftime(date, sizeof(date), "%d-%b-%Y %H:%M:%S", localtime(&t));
  printf("  Date: %s\n", date);

  if ( out->tree.description != NULL )
    printf("  Description: %s\n", out->tree.description);
  if ( out->tree.reference != NULL )
    printf("  Reference: %s\n", out->tree.reference);
  if ( out->tree.operator != NULL )
    printf("  Operator: %s\n", out->tree.operator);
  printf("  Number of Test Cases: %d\n", out->stat.ncase);
  printf("    Executed=%d Significant=%d\n", out->stat.executed, out->stat.passed + out->stat.failed + out->stat.inconclusive);
  printf("    PASSED=%d FAILED=%d INCONCLUSIVE=%d SKIP=%d\n", out->stat.passed, out->stat.failed, out->stat.inconclusive, out->stat.skip);
  printf("  Duration: %s\n", report_elapsed(out->stat.elapsed_time));

  /* Free Test Output file descriptor */
  output_destroy(out);

  return 0;
}


/*******************************************************/
/* Program body                                        */
/*******************************************************/

static void usage(void)
{
  fprintf(stderr, TITLE "\n");
  fprintf(stderr, "Usage: " NAME " [-c <config>] [-v] [-n] [-o <directory>] <result-file>\n");
  fprintf(stderr, "  -c <config>, --config=<config>: Specify Report Configuration name\n");
  fprintf(stderr, "  -v, --verbose: Print report summary on standard output\n");
  fprintf(stderr, "  -n, --no-html: Do not generate HTML Test Report file\n");
  fprintf(stderr, "  -o <directory>, --output=<directory>: Specify HTML Test Report target directory\n");
  fprintf(stderr, "  <result-file>: input file (XML test output file)\n");

  exit(2);
}


int main(int argc, char *argv[])
{
  int opt_verbose = 0;
  int opt_html = 1;
  char *out_name = NULL;
  char *out_dir = NULL;
  int out_type;
  char *html_dir = NULL;
  char *conf_name = NULL;
  report_config_t *rc;
  unsigned char md5sum[MD5_SIGNATURE_LENGTH];
  int ret = 0;
  char *p;
  int i;

  /* Retrieve command arguments */
  for (i = 1; i < argc; i++) {
    char *str = argv[i];

    if ( str[0] == '-' ) {
      if ( (strcmp(str, "-v") == 0) || (strcmp(str, "--verbose") == 0) ) {
        opt_verbose = 1;
      }
      else if ( (strcmp(str, "-n") == 0) || (strcmp(str, "--no-html") == 0) ) {
        opt_html = 0;
      }
      else if ( strcmp(str, "-o") == 0 ) {
        i++;
        if ( i < argc ) {
          html_dir = argv[i];
        }
        else {
          usage();
        }
      }
      else if ( strncmp(str, "--output=", 9) == 0 ) {
	html_dir = &(argv[i][9]);
      }
      else if ( strcmp(str, "-c") == 0 ) {
        i++;
        if ( i < argc ) {
          conf_name = argv[i];
        }
        else {
          usage();
        }
      }
      else if ( strncmp(str, "--config=", 9) == 0 ) {
	conf_name = &(argv[i][9]);
      }
      else {
        usage();
      }
    }
    else {
      if ( out_name == NULL ) {
        out_name = str;
      }
      else {
        usage();
      }
    }
  }

  if ( out_name == NULL )
    usage();

  /* Construct Test Output file name */
  if ( access(out_name, R_OK) != 0 ) {
    fprintf(stderr, NAME ": %s: Cannot read Test Output file\n", out_name);
    exit(1);
  }

  /* Check Test Output file format */
  out_type = output_file_type(out_name);
  if ( out_type != OUTPUT_FILE_XML ) {
    fprintf(stderr, NAME ": %s: Cannot recognize format of Test Output file\n", out_name);
    exit(1);
  }

  /* Compute Test Output MD5 checksum */
  md5_sign(out_name, md5sum);

  /* Load report configuration file */
  rc = report_config_alloc();
  p = report_config_load(rc, conf_name);
  if ( p != NULL )
    fprintf(stderr, "Report Configuration file: %s\n", p);
  else if ( conf_name != NULL )
    fprintf(stderr, "Warning: Report Configuration '%s' not found: using standard layout\n", conf_name);

  /* Get output file directory */
  out_dir = g_path_get_dirname(out_name);

  /* Create target directory (if required) */
  if ( opt_html ) {
    if ( html_dir == NULL )
      html_dir = out_dir;

    ret = mkdir(html_dir, 0755);
    if ( ret && (errno != EEXIST) ) {
      fprintf(stderr, "Cannot create target directory %s: %s\n", html_dir, strerror(errno));
      exit(1);
    }
  }

  /* Show files name */
  if ( opt_verbose ) {
    printf(TITLE "\n");

    printf("Test Report options:\n");
    verbose_options(rc);

    printf("Test Output file: %s\n", out_name);

    if ( md5sum[0] != '\0' )
      printf("  MD5 Checksum: %s\n", md5sum);

    if ( verbose_output(out_name) ) {
      fprintf(stderr, NAME ": %s: Error opening Test Output file\n", out_name);
      exit(1);
    }
  }

  if ( opt_html ) {
    char signature[strlen((char *) md5sum)+40];
    struct stat st;

    /* Build Test Report signature */
    p = signature;
    p += sprintf(p, "%s", md5sum);

    if ( stat(out_name, &st) == 0 )
      p += sprintf(p, " %ld", st.st_mtime);

    /* Generate HTML Test Log files */
    if ( rc->conf & REPORT_CONFIG_LOG ) {
      /* Log may be appended to the .out file or in a separate .log file */
      if ( report_log_build(out_name, signature, 1, html_dir) != 0 ) {
	char *log_name;
	int size;

	size = strlen(out_dir) + 16;
	log_name = (char *) malloc(size);
	snprintf(log_name, size, "%s" G_DIR_SEPARATOR_S "global.log", out_dir);

        if ( report_log_build(log_name, signature, 0, html_dir) != 0 )
          rc->conf &= ~REPORT_CONFIG_LOG;

	free(log_name);
      }
    }

    /* Genarate HTML Test Report file */
    if ( opt_verbose )
      printf("HTML Test Report target directory: %s\n", html_dir);

    ret = report_xslt(out_name, signature, rc, html_dir);
    if ( ret < 0 )
      ret = 99;
  }

  /* Free string buffers */
  if ( out_dir != NULL )
    free(out_dir);

  report_config_destroy(rc);

  exit(ret);
  return 0;
}
