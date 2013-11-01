

for channel in {0..31}
do

echo ""
echo " pulse on channel " $channel
echo "ready? [Y/n]"
read a
if [ "$a" != "Y" ] && [ "$a" != "y" ] && [ "$a" != "" ]
then
    exit
fi

sudo  env LD_LIBRARY_PATH=/home/nova/root/lib:/usr/lib/:/usr/local/lib/:/usr/lib/:/usr/local/lib/ ./myusblib
mv channels.txt channels_$channel.txt

done

