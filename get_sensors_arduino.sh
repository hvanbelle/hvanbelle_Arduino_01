#!/bin/sh

ARDUINO_IP="192.168.2.30"
ARDUINO_PORT="8101"
ARDUINO_GREP="Temperature in Celcius"

SENSORS_OUTPUT=`curl http://$ARDUINO_IP:$ARDUINO_PORT/ | grep "$ARDUINO_GREP" | awk '{print $1}' `

# Get the number of counters
counter=0
for sensor in $SENSORS_OUTPUT
do
	let counter+=1
done

NBR_SENSORS=$counter

# Echo header
echo "{"
echo "  \"version\":\"1.0.0\","
echo "  \"datastreams\":["

# Echo lines
counter=0
for str in $SENSORS_OUTPUT
do
	let counter+=1
	#echo $counter
	if [ $counter -lt $NBR_SENSORS ];then
		echo "      {\"id\":\"sensor"$counter"\", \"current_value\":\"$str\"},"
	else
		echo "      {\"id\":\"sensor"$counter"\", \"current_value\":\"$str\"}"

	fi 
done

# Echo footer
echo "  ] "
echo "} "

