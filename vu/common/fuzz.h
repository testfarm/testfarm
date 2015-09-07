/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Color fuzzing primitives                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 22-OCT-2007                                                    */
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

#ifndef __TVU_FUZZ_H__
#define __TVU_FUZZ_H__

#include <stdlib.h>

#include "color.h"

typedef struct {
  color_rgb_t color;
} fuzz_t;


#define FUZZ_NULL { color: {0,0,0} }
extern fuzz_t fuzz_null;

extern int fuzz_is_null(fuzz_t *fuzz);

extern int fuzz_parse(fuzz_t *fuzz, char *str);
extern char *fuzz_str(fuzz_t *fuzz);

//extern int fuzz_check(fuzz_t *fuzz, unsigned char *rgb, unsigned char *pix);

#define _fuzz_check_channel(fuzz, rgb, pix, i) \
  if ( abs(((int) rgb[i]) - ((int) pix[i])) > fuzz->color[i] ) return 0

static inline int fuzz_check(fuzz_t *fuzz, unsigned char *rgb, unsigned char *pix)
{
  _fuzz_check_channel(fuzz, rgb, pix, 0);
  _fuzz_check_channel(fuzz, rgb, pix, 1);
  _fuzz_check_channel(fuzz, rgb, pix, 2);
  return 1;
}


#define _fuzz_weight_channel(fuzz, rgb, pix, i) \
  ( abs(((int) rgb[i]) - ((int) pix[i])) - ((int) fuzz_color[i]) )

static inline unsigned short fuzz_weight(color_rgb_t fuzz_color, unsigned char *rgb, unsigned char *pix)
{
  unsigned short acc = 0;
  int v;

  if ( (v = _fuzz_weight_channel(fuzz_color, rgb, pix, 0)) > 0 )
    acc += v;
  if ( (v = _fuzz_weight_channel(fuzz_color, rgb, pix, 1)) > 0 )
    acc += v;
  if ( (v = _fuzz_weight_channel(fuzz_color, rgb, pix, 2)) > 0 )
    acc += v;

  return acc;
}


#ifdef __SSE__
#include <xmmintrin.h>

/* The MMX version.
   Well. Practically, it is not faster than the classical version.
   To be investigated further */
static inline unsigned short fuzz_weight2(color_rgb_t fuzz_color, unsigned char *rgb, unsigned char *pix)
{
  __m64 m_fuzz_color, m_rgb, m_pix;
  __m64 m_r0, m_r1, m_acc;
  __m64 m_zero;

  m_rgb = *((__m64 *) rgb);
  m_pix = *((__m64 *) pix);
  m_r0 = _mm_or_si64(_mm_subs_pu8(m_rgb,m_pix), _mm_subs_pu8(m_pix,m_rgb));

  m_fuzz_color = *((__m64 *) fuzz_color);
  m_r0 = _mm_subs_pu8(m_r0, m_fuzz_color);

  m_r1 = _mm_and_si64(m_r0, (__m64) 0xFFFFFFULL);
  m_zero = _mm_xor_si64(m_r0, m_r0);
  m_acc = _mm_sad_pu8(m_r1, m_zero);

  return (unsigned long long) m_acc;
}
#endif

#endif /* __TVU_FUZZ_H__ */
