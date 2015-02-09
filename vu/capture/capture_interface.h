/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Capture Device - Application Interface                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 25-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1036 $
 * $Date: 2008-12-07 14:18:30 +0100 (dim., 07 déc. 2008) $
 */

#ifndef __TVU_CAPTURE_INTERFACE_H__
#define __TVU_CAPTURE_INTERFACE_H__

#include "capture_cap.h"
#include "capture_attr.h"
#include "frame_geometry.h"


/*===== Capture device options =====*/
#define CAPTURE_OPT_BIT(n) (1<<(n))
#define CAPTURE_OPT_SHARED CAPTURE_OPT_BIT(0)     /* Allow shared connections */
#define CAPTURE_OPT_DEBUG  CAPTURE_OPT_BIT(1)     /* Enable debug mode */
#define CAPTURE_OPT_ROTATE CAPTURE_OPT_BIT(2)     /* Rotate display 90deg */
#define CAPTURE_OPT_ROTATE_CCW CAPTURE_OPT_BIT(3) /* Rotate dir: 0=clockwise 1=counter-clockwise */

typedef unsigned long capture_opt_t;


/*===== Capture device refresh rate =====*/
#define CAPTURE_PERIOD_MIN  66
#define CAPTURE_PERIOD_MAX  10000


/*===== Generic capture device descriptor =====*/
typedef struct {
  int shmid;                 /* Frame buffer shmid */
  capture_cap_t cap;         /* Capture device capabilities */
  char *name;                /* Source device or connection name */
} capture_t;


/*===== Capture device primitives & handlers =====*/

typedef void capture_update_fn(frame_geometry_t *g, void *data);

typedef struct {
  capture_t * (*open)(char *device, capture_opt_t opt, capture_update_fn *update, void *data);
  void (*close)(capture_t *capture);

  int (*set_window)(capture_t *capture, frame_geometry_t *g);
  int (*get_window)(capture_t *capture, frame_geometry_t *g);

  long (*set_period)(capture_t *capture, long delay);
  long (*get_period)(capture_t *capture);

  int (*refresh)(capture_t *capture);

  void (*action_key)(capture_t *capture, int down, unsigned long key);
  void (*action_pointer)(capture_t *capture, unsigned char buttons, unsigned int x, unsigned int y);
  void (*action_scroll)(capture_t *capture, unsigned char direction);

  int (*attr_set)(capture_t *capture, char *key, char *value);
  capture_attr_t * (*attr_get)(capture_t *capture, char *key, int *nmemb);

  void (*show_status)(capture_t *capture, char *hdr);
} capture_interface_t;

#endif /* __TVU_CAPTURE_INTERFACE_H__ */
