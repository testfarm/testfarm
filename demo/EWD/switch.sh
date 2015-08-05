#!/bin/bash

CONF="EWDsystem"

if [ -z $1 ]; then
  echo "TestFarm System Config switcher"
  echo "Usage: switch.sh <MEDIA>"
  exit 2
fi

XML=$CONF-$1.xml
echo "Switching to config $XML"

ln -sf $XML $CONF.xml
twiz-conf $CONF

