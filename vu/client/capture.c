/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Capture Device Management                                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 25-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1036 $
 * $Date: 2008-12-07 14:18:30 +0100 (dim., 07 déc. 2008) $
 */

#include <stdio.h>
#include <gmodule.h>

#include "error.h"
#include "capture.h"


#define CAPTURE_INTERFACE_DIR "/opt/testfarm/lib/vu"

#define CAPTURE_INTERFACE_CALL_VOID(func, args...) if ( capture_interface && capture_interface->func ) { capture_interface->func(args); }
#define CAPTURE_INTERFACE_CALL_RET(func, ret, args...) return (capture_interface && capture_interface->func) ? capture_interface->func(args) : (ret)


static GModule *capture_module = NULL;
static capture_interface_t *capture_interface = NULL;


capture_t *capture_open(char *device, capture_opt_t opt, capture_update_fn *update, void *data)
{
  CAPTURE_INTERFACE_CALL_RET(open, NULL, device, opt, update, data);
}


void capture_close(capture_t *capture)
{
  CAPTURE_INTERFACE_CALL_VOID(close, capture);
}


int capture_set_window(capture_t *capture, frame_geometry_t *g)
{
  CAPTURE_INTERFACE_CALL_RET(set_window, -1, capture, g);
}


int capture_get_window(capture_t *capture, frame_geometry_t *g)
{
  CAPTURE_INTERFACE_CALL_RET(get_window, -1, capture, g);
}


long capture_set_period(capture_t *capture, long delay)
{
  CAPTURE_INTERFACE_CALL_RET(set_period, -1, capture, delay);
}


long capture_get_period(capture_t *capture)
{
  CAPTURE_INTERFACE_CALL_RET(get_period, -1, capture);
}


int capture_refresh(capture_t *capture)
{
  CAPTURE_INTERFACE_CALL_RET(refresh, -1, capture);
}


void capture_action_key(capture_t *capture, int down, unsigned long key)
{
  CAPTURE_INTERFACE_CALL_VOID(action_key, capture, down, key);
}


void capture_action_pointer(capture_t *capture, unsigned char buttons, unsigned int x, unsigned int y)
{
  CAPTURE_INTERFACE_CALL_VOID(action_pointer, capture, buttons, x, y);
}


void capture_action_scroll(capture_t *capture, unsigned char direction)
{
  CAPTURE_INTERFACE_CALL_VOID(action_scroll, capture, direction);
}


int capture_attr_set(capture_t *capture, char *key, char *value)
{
  CAPTURE_INTERFACE_CALL_RET(attr_set, -1, capture, key, value);
}


capture_attr_t *capture_attr_get(capture_t *capture, char *key, int *nmemb)
{
  CAPTURE_INTERFACE_CALL_RET(attr_get, NULL, capture, key, nmemb);
}


void capture_show_status(capture_t *capture, char *hdr)
{
  printf("%s  Capabilities:", hdr);
  if ( capture->cap & CAPTURE_CAP_VIDEO )
    printf(" VIDEO");
  if ( capture->cap & CAPTURE_CAP_KEY )
    printf(" KEYBOARD");
  if ( capture->cap & CAPTURE_CAP_POINTER )
    printf(" POINTER");
  if ( capture->cap & CAPTURE_CAP_SCROLL )
    printf(" SCROLL");
  printf("\n");

  CAPTURE_INTERFACE_CALL_VOID(show_status, capture, hdr);
}


int capture_init(char *method)
{
  char filename[256];
  GModule *module;
  gpointer ptr;

  if ( ! g_module_supported() ) {
    fprintf(stderr, NAME ": device init: dynamic modules not supported\n");
    return -1;
  }

  if ( capture_module != NULL )
    g_module_close(capture_module);
  capture_module = NULL;

  snprintf(filename, sizeof(filename), CAPTURE_INTERFACE_DIR "/%s." G_MODULE_SUFFIX, method);
  module = g_module_open(filename, G_MODULE_BIND_LAZY);
  if ( module == NULL ) {
    fprintf(stderr, NAME ": device open: %s\n", g_module_error());
    return -1;
  }

  ptr = NULL;
  if ( !g_module_symbol(module, "_capture_interface", &ptr) ) {
    fprintf(stderr, NAME ": device lookup: %s\n", g_module_error());
    g_module_close(module);
    return -1;
  }

  if ( ptr == NULL ) {
    fprintf(stderr, NAME ": device link: %s: Cannot find a valid device descriptor\n", method);
    g_module_close(module);
    return -1;
  }

  capture_module = module;
  capture_interface = ptr;

  return 0;
}
