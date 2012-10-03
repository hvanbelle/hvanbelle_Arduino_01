#!/bin/sh

if [ -z $1 ];then
    export START_DIR=`pwd` # no input so decide ourself
else     
    export START_DIR="$1"  # this should be the starting place for the script
fi 

logger send arduino-sensors data to cosm-site

$START_DIR/get_sensors_arduino.sh > $START_DIR/cosm_arduino.json

curl --request PUT \
     --data-binary @$START_DIR/cosm_arduino.json \
     --header "X-ApiKey: YOUR_API_KEY_HOULD_GO_HERE" \
     --verbose \
     http://api.cosm.com/v2/feeds/YOUR_COSM_FEED_NBR_SHOULD_GO_HERE

