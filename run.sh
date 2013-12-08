environ="sudo env LD_LIBRARY_PATH=/home/nova/root/lib:/usr/lib/:/usr/local/lib/:/usr/lib/:/usr/local/lib/"

echo starting boards $2 $3 $4 $5 $6 for $1 events

$environ ./receive_all $1 $2 $3 $4 $5 $6 &



