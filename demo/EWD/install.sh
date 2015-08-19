#!/bin/sh

DIR=demo-EWD

BINDIR=$(dirname "$0")
TARBALL=$(ls $BINDIR/TestFarm-demo-EWD_*.tgz | tail -1)

mkdir "$DIR"
cd "$DIR"
tar xfz $TARBALL
