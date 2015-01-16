#!/bin/sh

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
