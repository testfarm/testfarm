/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* OCR Agent for text patterns                                              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 27-JUN-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 962 $
 * $Date: 2008-02-27 09:33:19 +0100 (mer., 27 févr. 2008) $
 */

#ifndef __TVU_OCR_H__
#define __TVU_OCR_H__

#include "frame_geometry.h"

#ifndef __TVU_OCR_C__
typedef struct {
  // No published entries
} ocr_ctx_t;

typedef struct {
  // No published entries
} ocr_t;
#endif

typedef void ocr_reply_t(unsigned long addr, char *str, ocr_ctx_t *ctx, void *data);

extern int ocr_set_agent(ocr_t *ocr, char *ocr_agent, int ocr_argc, char *ocr_argv[]);
extern char *ocr_get_agent(ocr_t *ocr);

extern ocr_t *ocr_alloc(int shmid);
extern void ocr_destroy(ocr_t *ocr);

extern int ocr_enable(ocr_t *ocr, int state);
extern int ocr_request(ocr_t *ocr,
		       unsigned long addr, frame_geometry_t *g, int inverse,
		       ocr_reply_t *reply, void *data);

extern int ocr_get_text_geometry(ocr_ctx_t *ctx, int offset, int len, frame_geometry_t *g);

#endif /* __TVU_OCR_H__ */
