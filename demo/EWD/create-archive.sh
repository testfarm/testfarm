#!/bin/bash

VERSION=$(../../tools/gitversion.sh)
TARBALL=TestFarm-demo-EWD_$VERSION.tgz

make clean
rm -f *~

SUBDIRS=$(ls */.tree | xargs dirname)
FILES=$(ls *.tree *.wiz *.pl *.pm *.xml)

tar cvfa $TARBALL $SUBDIRS objects wiz passwd $FILES
