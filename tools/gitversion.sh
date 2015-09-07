#!/bin/sh

##
##    TestFarm -- GIT version retrieval utility
##
##    This file is part of TestFarm,
##    the Test Automation Tool for Embedded Software.
##    Please visit http://www.testfarm.org.
##
##    TestFarm is free software: you can redistribute it and/or modify
##    it under the terms of the GNU General Public License as published by
##    the Free Software Foundation, either version 3 of the License, or
##    (at your option) any later version.
##
##    TestFarm is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU General Public License for more details.
##
##    You should have received a copy of the GNU General Public License
##    along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
##

unset OPT_TAG OPT_SHORT COMMIT

for args in $*; do
    if [ "$args" = '--tag' ]; then
	OPT_TAG=yes
    elif [ "$args" = '--short' ]; then
	OPT_SHORT=yes
    elif [ ${args##-} = "$args" ]; then
	COMMIT=$args
    fi
done

TAG=$(git log --oneline --decorate=short --first-parent $COMMIT | perl -n -e 'if (/[\( ]tag: (\S+)[\),]/) { print "$1\n"; exit(0); }')

if [ -z "$TAG" ]; then
    echo "No GIT tag found" >&2
    exit 1
fi

if [ -n "$OPT_TAG" ]; then
    echo $TAG
elif [ -n "$OPT_SHORT" ]; then
    echo ${TAG#v}
else
    VERSION=$(git describe --tags --long $COMMIT --match=$TAG | cut -d- -f1,2 | tr - .)
    [ -z "$VERSION" ] && VERSION=0
    echo ${VERSION#v}
fi

exit 0
