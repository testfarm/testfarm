/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display Tool - Pixbufs and Icons                                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 08-JAN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 814 $
 * $Date: 2007-11-27 11:33:11 +0100 (mar., 27 nov. 2007) $
 */

#include <gtk/gtk.h>

#include "support.h"
#include "xpm.h"


GdkPixbuf *pixbuf_blank = NULL;
static char *xpm_blank_d[] = {
  "16 16 1 1",
  "  c None",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
};


GdkPixbuf *pixbuf_connect_creating = NULL;
static char *xpm_connect_creating_d[] = {
/* columns rows colors chars-per-pixel */
"16 16 10 1",
"  c None",
". c #585858",
"X c #808000",
"o c #c0c000",
"O c yellow",
"+ c #808080",
"@ c #a0a0a0",
"# c #c3c3c3",
"$ c gainsboro",
"% c white",
/* pixels */
"            OOOO",
"         OOOoooX",
"         ooooXX ",
"         Xooo   ",
"          Xooo  ",
"   %#     Ooooo ",
"  %$$#   OoooXX ",
" +#$$$# OooXX   ",
" .+#$$$# XX     ",
" ..+#$$$#       ",
"  .#+#$$#@      ",
"   %@+#$@+      ",
"  %@+.+@+.      ",
" %@  .++.       ",
"%@    ..        ",
"@               "
};


GdkPixbuf *pixbuf_connect_established = NULL;
static char *xpm_connect_established_d[] = {
/* columns rows colors chars-per-pixel */
"16 16 8 1",
"  c None",
". c gray19",
"X c #585858",
"o c #808080",
"O c #a0a0a0",
"+ c #c3c3c3",
"@ c gainsboro",
"# c white",
/* pixels */
"              #O",
"             #O ",
"      ###   #O  ",
"     #@@@# #O   ",
"      O@@@#.    ",
"   #+  O@@@#.   ",
"  #@@+ .O@@@#   ",
" o+@@@+  O@@+   ",
" Xo+@@@+ .O+o   ",
" XXo+@@@+  oX   ",
"  X+o+@@+O X    ",
"   #Oo+@Oo      ",
"  #OoXoOoX      ",
" #O  XooX       ",
"#O    XX        ",
"O               "
};


GdkPixbuf *pixbuf_connect_no = NULL;
static char *xpm_connect_no_d[] = {
/* columns rows colors chars-per-pixel */
"16 16 10 1",
"  c None",
". c #404000",
"X c #585858",
"o c #c0c000",
"O c yellow",
"+ c #808080",
"@ c #a0a0a0",
"# c #c3c3c3",
"$ c gainsboro",
"% c white",
/* pixels */
"                ",
"                ",
"                ",
"        Oo      ",
"       Oo.      ",
"   %# Oo.       ",
"  %$$# .   Oo   ",
" +#$$$#   Oo.   ",
" X+#$$$# Oo.    ",
" XX+#$$$# .     ",
"  X#+#$$#@      ",
"   %@+#$@+      ",
"  %@+X+@+X      ",
" %@  X++X       ",
"%@    XX        ",
"@               "
};


GdkPixbuf *pixbuf_frame = NULL;


void xpm_init(GtkWidget *window)
{
  pixbuf_blank = gdk_pixbuf_new_from_xpm_data((const char **) xpm_blank_d);
  pixbuf_connect_creating = gdk_pixbuf_new_from_xpm_data((const char **) xpm_connect_creating_d);
  pixbuf_connect_established = gdk_pixbuf_new_from_xpm_data((const char **) xpm_connect_established_d);
  pixbuf_connect_no = gdk_pixbuf_new_from_xpm_data((const char **) xpm_connect_no_d);

  pixbuf_frame = create_pixbuf("testfarm-vu-frame.png");
}


void xpm_done(void)
{
}
