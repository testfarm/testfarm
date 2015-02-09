#!/bin/sh -f

## TestFarm Virtual User
## GOCR OCR agent launcher
##
## (c) Basil Dev 2008 - www.basildev.com
## $Revision: 1013 $
## $Date: 2008-07-19 16:01:47 +0200 (sam., 19 juil. 2008) $

shmid=$1
shift

exec gocr -f XML -F 10 -S $shmid $*
