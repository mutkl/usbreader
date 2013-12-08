#define intro
intro(){
echo "************************"
echo "*** MUON STAND DAQ *****"
echo "************************"
echo " "
return 1
}

# read n of events
read_n_events(){
 read user_nevents
 if [ "$user_nevents" != "" ]
     then
     if [ $user_nevents -eq $user_nevents 2> /dev/null ]; then
     # $user_nevents is a number
	 default_nevents=$user_nevents
     else
	 echo $user_nevents is not a number
     fi
 fi
}

# ask if continue with next command
ask_continue_with_next_command(){
    echo " "
    echo " ********************* I WILL DO " $command ": CONTINUE? [Y/n/s]"
}

# read output flag
ask_output_flag(){
 echo " output flag? defualt: " $output_flag " existing names: "
 ls -tr output/
 read VARNAME
 if [ "$VARNAME" != "" ] 
 then
     output_flag=$VARNAME
 fi
}



## initial parameters
default_nevents=1000
user=`whoami`
environ="sudo env LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu/root5.34/lib:/usr/lib/:/usr/local/lib/:/usr/lib/:/usr/local/lib/"
output_flag="$(date +%y%m%d%s)"


 #####################################
 ########## main code here ##########
 #####################################

 intro

 echo " how many events? (default= " $default_nevents ", -1 for all events)"

# get number of events
 read_n_events
 echo " will run on " $default_nevents " events "
 
 
# loop on commands to execute
 #for command in './setup_daq' './acquire.sh' './select'     'root -l convert_data_to_root.C'  'root -l viewer.C'   'root -l analyze_data.C' ; do
 for command in './setup_daq' './run.sh'; do


     # decide on next step
     ask_continue_with_next_command 
     read a

     # exit
     if [ "$a" != "Y" ] && [ "$a" != "y" ] && [ "$a" != "" ] && [ "$a" != "s" ]
     then
	 exit
     fi
    
    # skip    
     if [ "$a" = "s" ] 
     then
	 echo "skip"
	 
	 if [ "$mycommand" == "root -l clean_data.C" ]
	 then
	     cp data.root data_clean.root
	 fi
	 continue
     fi


     #execute command
     echo DOING $command

     #setup
     if [ "$command" == "./setup_daq" ]
     then
	 $environ $command 
      #receive
     elif [ "$command" == "./run.sh" ]
     then
	 #receive each board
	 boardslist=`grep dev data_params.txt | awk -F "  " '{print $1}'`
	 echo " [enter] to proceed starting all boards "
	 read user_nevents
	 $environ $command $default_nevents $boardslist
     fi
     
 done ## executed all commands


 ask_output_flag

 # create output directory
 dirname="output.$output_flag"
 if [ ! -d "$dirname" ]; then
     mkdir output/$dirname
 fi
 if [ ! -d "$dirname/data" ]; then
     mkdir output/$dirname/data
 fi


 # save output files
 for output in 'raw_data_??????_??????.txt' 'coincidence_data_??????.txt' 'coincidence_data_??????.root' ; do
     mv $output output/$dirname/data/
 done
 
 cp detector.inp output/$dirname/$output_flag-detector.inp

 for output in  'data.root'  'result.root' 'data_params.txt' 'detector.eps' ; do
     if [ -e "$output" ]; then
	 newname="$output_flag-$output"
	 mv $output output/$dirname/$newname
     fi
 done
 echo "done, output is in " output/$dirname

 exit 1
