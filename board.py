import subprocess, sys, os
import multiprocessing,time

def intro():
    print '      ************************'
    print '      *** MUON STAND DAQ *****'
    print '      ************************'
    
def read_n_events():
    default = 1000
    try :
        n_evts = int(raw_input("How many events? (default = 100, -1 for continuous): "))
        return n_evts

    except ValueError :
        return default
        
def continue_with_next_command(command) :
    print ' '
    choice = raw_input('**** I will do: %s. CONTINUE? [Y/n] ****: ' % command)
    if choice in ('Y','y','') :
        print 'Executing ' + command
        return
    else:
        print 'Exiting'
        sys.exit(0)

def get_boards():
    f = open('data_params.txt')
    boards = []
    for line in f:
        boards.append(line)
        
    return boards

def execute_receive_one(board,events):
    #subprocess.call(['sudo','./receive_one&',str(board),str(events),])
    #things aren't looking good im sry
    subprocess.call("sudo ./receive_one %s %d &" % (str(board),int(events)), shell=True)

def main():

    intro()
    events = read_n_events()
    boards = []
    jobs=[]
    commands = ['setup_daq','receive_one','wait']    
    
    for command in commands:
        continue_with_next_command(command)
        if command == 'setup_daq'  : 
            subprocess.call(['sudo','./%s' % command])
            boards = get_boards()
              
        if command == 'receive_one': 
            for board in boards:
                execute_receive_one(board[0:4],events)
                #seriously what is wrong with multiprocessing???
                #p = multiprocessing.Process(target=execute_receive_one, args=(board[0:4],events))
                #jobs.append(p)
            #for i in jobs:
            #    i.start()

    
    print 'End of %d events.' % events

if __name__ == '__main__' :
    main()
