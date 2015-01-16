#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "filelist.h"

int main(int argc, char *argv[])
{
  GList *list;
  GList *l;

  if ( argc != 2 ) {
    fprintf(stderr, "Usage: filelist_test <regex>\n");
    exit(1);
  }

  list = filelist(argv[1]);

  l = list;
  while ( l != NULL ) {
    printf("%s\n", (char *) l->data);
    l = l->next;
  }

  filelist_free(list);

  return 0;
}
