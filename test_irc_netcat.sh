#!/bin/bash

pkill -f logcaster-cpp
sleep 1

./log-caster-cpp-reconstructed/build/logcaster-cpp -i &
sleep 1

LOG_FILE=/tmp/irc_test.log

# Start sender in the background
(
sleep 1
echo "NICK sender"
echo "USER sender 0 * :Sender"
echo "JOIN #logs-all"
echo "PRIVMSG #logs-all :Hello from netcat"
) | nc localhost 6667 &

# Start listener in the foreground
(
echo "NICK listener"
echo "USER listener 0 * :Listener"
echo "JOIN #logs-all"
sleep 3
) | nc localhost 6667 > $LOG_FILE

pkill -f logcaster-cpp
