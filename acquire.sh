environ="sudo env LD_LIBRARY_PATH=/home/nova/root/lib:/usr/lib/:/usr/local/lib/:/usr/lib/:/usr/local/lib/"

echo starting board $1 for $2 events

$environ ./receive_one $1 $2 &



