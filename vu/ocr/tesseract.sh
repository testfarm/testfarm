#!/bin/sh  -f

## TestFarm Virtual User
## Tesseract OCR agent launcher
##
## (c) Basil Dev 2008 - www.basildev.com
## $Revision: 961 $
## $Date: 2008-02-25 18:53:49 +0100 (lun., 25 févr. 2008) $

shmid=$1
shift

exec tesseract $shmid - $* batch.nochop server
