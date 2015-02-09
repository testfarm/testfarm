/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Filter Agent program body                                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 02-JAN-2008                                                    */
/****************************************************************************/

/*
 * $Revision: 888 $
 * $Date: 2008-01-07 15:30:49 +0100 (lun., 07 janv. 2008) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "filter_agent.h"


void filter_agent_register(char *class_id)
{
  printf("*%s\n", class_id);
}


void filter_agent_added(unsigned long num)
{
  printf("+%lu\n", num);
}


void filter_agent_removed(unsigned long num)
{
  printf("-%lu\n", num);
}


void filter_agent_shown(unsigned long num, char *msg)
{
  printf("?%lu %s\n", num, msg);
}


void filter_agent_applied(unsigned long num)
{
  printf("!%lu\n", num);
}


int main(int argc, char *argv[])
{
  GHashTable *fbs;
  GIOChannel *channel;
  GIOStatus status;
  GError *error = NULL;
  gchar *buf;
  gsize size;

  if ( filter_agent_init(argc, argv) ) {
    fprintf(stderr, "TestFarm Virtual User Filter Agent\n");
    fprintf(stderr, "%s\n", filter_agent_desc);
    return 1;
  }

  setbuf(stdout, NULL);

  fbs = g_hash_table_new(NULL, NULL);

  channel = g_io_channel_unix_new(fileno(stdin));

  while ( (status = g_io_channel_read_line(channel, &buf, &size, NULL, &error)) == G_IO_STATUS_NORMAL ) {
    if ( buf != NULL ) {
      char op = buf[0];
      char *s_num = buf + 1;
      char *str = strchr(s_num, ' ');
      unsigned long num;

      if ( str != NULL ) {
	*(str++) = '\0';
	g_strstrip(str);
      }

      num = strtoul(s_num, NULL, 0);

      switch ( op ) {
      case '!':
	if ( str != NULL ) {
	  int shmid = atoi(str);
	  frame_buf_t *fb = g_hash_table_lookup(fbs, GINT_TO_POINTER(shmid));

	  if ( fb == NULL ) {
	    fb = frame_buf_map(shmid, 0);
	    if ( fb != NULL ) {
	      g_hash_table_insert(fbs, GINT_TO_POINTER(shmid), fb);
	      fprintf(stderr, "[INFO ] APPLY %lu: new shmid=%d %ux%u\n", num, shmid,
		      fb->rgb.width, fb->rgb.height);
	    }
	    else {
	      fprintf(stderr, "[ERROR] APPLY %lu: cannot map shmid=%d\n", num, shmid);
	    }
	  }

	  if ( fb != NULL ) {
	    filter_agent_apply(num, fb);
	  }
	  else {
	    filter_agent_applied(num);
	  }
	}
	else {
	  fprintf(stderr, "[ERROR] APPLY %lu: missing parameter <shmid>\n", num);
	  filter_agent_applied(num);
	}
	break;
      case '?':
	filter_agent_show(num);
	break;
      case '+':
	{
	  char *class_id = NULL;
	  GList *options = NULL;

	  while ( str != NULL ) {
	    char *str2 = strchr(str, ' ');

	    if ( str2 != NULL ) {
	      *(str2++) = '\0';
	      while ( (*str2 != '\0') && (*str2 < ' ')  )
		str2++;
	    }

	    if ( *str != '\0' ) {
	      if ( class_id == NULL )
		class_id = str;
	      else
		options = g_list_append(options, str);
	    }

	    str = str2;
	  }

	  fprintf(stderr, "[INFO ] ADD %lu: class='%s'\n", num, class_id);

	  filter_agent_add(num, class_id, options);

	  g_list_free(options);
	}
	break;
      case '-':
	filter_agent_remove(num);
	break;
      }

      g_free(buf);
    }
  }

  if ( status == G_IO_STATUS_ERROR ) {
    fprintf(stderr, "[ERROR] read error: %s\n", error->message);
  }

  g_hash_table_destroy(fbs);
  g_io_channel_unref(channel);

  return 0;
}
