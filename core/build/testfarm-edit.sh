#! /bin/bash -f

## $Revision: 987 $
## $Date: 2008-03-18 15:59:43 +0100 (mar., 18 mars 2008) $

filename=$1
lineno=${2-1}

# Uncomment lines below to enable XEmacs editing
#gnuclient -q $filename &> /dev/null || exec xemacs -eval '(gnuserv-start) (setq gnuserv-frame (selected-frame))' $filename -eval '(goto-line '$lineno')'
#exec gnudoit '(goto-line '$lineno') (raise-frame)' &> /dev/null

# Use gedit by default
gedit $filename
