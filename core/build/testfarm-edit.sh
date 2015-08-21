#! /bin/bash -f

##
## TestFarm
## Text Editor launch script
##
## This file is part of TestFarm,
## the Test Automation Tool for Embedded Software.
## Please visit http://www.testfarm.org.
##
## TestFarm is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## TestFarm is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
##

filename=$1
lineno=${2-1}

# Uncomment lines below to enable XEmacs editing
#gnuclient -q $filename &> /dev/null || exec xemacs -eval '(gnuserv-start) (setq gnuserv-frame (selected-frame))' $filename -eval '(goto-line '$lineno')'
#exec gnudoit '(goto-line '$lineno') (raise-frame)' &> /dev/null

# Use gedit by default
gedit $filename
