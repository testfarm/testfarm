/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display control buffer - Display window geometry                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 25-JUN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 957 $
 * $Date: 2008-02-25 14:10:30 +0100 (lun., 25 févr. 2008) $
 */

#ifndef __TVU_FRAME_GEOMETRY_H__
#define __TVU_FRAME_GEOMETRY_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
  int x, y;                   /* Window position */
  unsigned int width, height; /* Window size */
} frame_geometry_t;

#define FRAME_GEOMETRY_NULL {0,0,0,0}
extern const frame_geometry_t frame_geometry_null;

extern int frame_geometry_parse_position(char *s, frame_geometry_t *g);
extern int frame_geometry_parse(char *s, frame_geometry_t *g);
extern void frame_geometry_clip(frame_geometry_t *g, frame_geometry_t *g0);

extern int frame_geometry_intersect(frame_geometry_t *g1, frame_geometry_t *g2, frame_geometry_t *gi);
extern int frame_geometry_overlaps( frame_geometry_t *g1, frame_geometry_t *g2);

extern void frame_geometry_union(frame_geometry_t *g, frame_geometry_t *g2);

extern char *frame_geometry_str(frame_geometry_t *g);

#ifdef __cplusplus
}
#endif

#endif /* __TVU_FRAME_GEOMETRY_H__ */
