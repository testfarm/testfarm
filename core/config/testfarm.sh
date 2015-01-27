#
# TestFarm Bourne Shell profile
# $Revision: 42 $
# $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
#

TESTFARM_HOME="/opt/testfarm"
test -d $TESTFARM_HOME/bin && which testfarm-launch >&/dev/null || export PATH=$PATH:$TESTFARM_HOME/bin
