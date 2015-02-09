#!/bin/sh -f

## TestFarm Virtual User
## Ocrad OCR agent launcher
##
## (c) Basil Dev 2010 - www.basildev.com
## $Revision: 1150 $
## $Date: 2010-06-04 23:13:45 +0200 (ven., 04 juin 2010) $

shmid=$1
shift

exec ocrad -x - --xml -S $shmid $*
