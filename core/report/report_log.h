/****************************************************************************/
/* TestFarm                                                                 */
/* HTML Log generator                                                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 09-JUL-2002                                                    */
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

#ifndef __REPORT_LOG_H__
#define __REPORT_LOG_H__

extern int report_log_build(char *log_name, char *signature, int xml_required, char *html_dir);

#endif /* __REPORT_LOG_H__ */
