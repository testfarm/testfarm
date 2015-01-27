#
# TestFarm C Shell profile
# $Revision: 1.1 $
# $Date: 2005/03/24 14:33:38 $
#

set TESTFARM_HOME=/opt/testfarm
test -d ${TESTFARM_HOME}/bin && which testfarm-launch >&/dev/null || setenv PATH ${PATH}:${TESTFARM_HOME}/bin
