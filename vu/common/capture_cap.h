/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Capture Device Capabilities                                              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 06-NOV-2007                                                    */
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

#ifndef __TVU_CAPTURE_CAP_H__
#define __TVU_CAPTURE_CAP_H__

#define CAPTURE_CAP_BIT(n) (1<<(n))
#define CAPTURE_CAP_VIDEO   CAPTURE_CAP_BIT(0)
#define CAPTURE_CAP_KEY     CAPTURE_CAP_BIT(1)
#define CAPTURE_CAP_POINTER CAPTURE_CAP_BIT(2)
#define CAPTURE_CAP_SCROLL  CAPTURE_CAP_BIT(3)
#define CAPTURE_CAP_INPUT   (CAPTURE_CAP_KEY|CAPTURE_CAP_POINTER|CAPTURE_CAP_SCROLL)

typedef unsigned int capture_cap_t;

#endif /* __TVU_CAPTURE_CAP_H__ */
