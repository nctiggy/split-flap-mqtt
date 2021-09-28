# split-flap-mqtt

This is a rework of Dave19171's splitflap project I found over at prusaprinters.org

https://github.com/Dave19171/split-flap
https://www.prusaprinters.org/prints/69464-split-flap-display

No changes to the Arduino unit code.
ESP code now has OTA firmware updates and MQTT subscription!

The ESP is now subscribed to a MQTT topic. Send a string to the topic and watch the splitflap do its thing!

I use Node-red to send data to the queue. Currently I just send the time, but will be adding functionality as time permits.

You can import the basic flow into node-red if you are interested
