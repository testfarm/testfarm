/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* OCR Agent for text patterns                                              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 27-JUN-2007                                                    */
/****************************************************************************/

/*
    This file is part of TestFarm,
    the Test Automation Tool for Embedded Software.
    Please visit http://www.testfarm.org.

    TestFarm is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TestFarm is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
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
