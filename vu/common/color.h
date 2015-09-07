/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* RGB Color specification                                                  */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 29-AUG-2007                                                    */
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

#ifndef __TVU_COLOR_H__
#define __TVU_COLOR_H__

typedef unsigned char color_rgb_t[3];

extern int color_parse(char *spec, unsigned long *pvalue);
extern void color_set_rgb(unsigned long value, color_rgb_t rgb);

extern void color_fill(unsigned long color, unsigned int width, unsigned int height, unsigned char **pbuf);
extern int color_fill_from_spec(char *spec, unsigned int *pwidth, unsigned int *pheight, unsigned char **pbuf);

#endif /* __TVU_COLOR_H__ */
