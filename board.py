import subprocess, sys, os
import multiprocessing

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
def main():

    intro()
    events = read_n_events()
    continue_with_next_command("setup_daq")
    subprocess.call(['sudo','./setup_daq'])    
    boards = [] 
    continue_with_next_command("receive_one")
    #just do one board
    #subprocess.call(['sudo','./receive_one',str(boards[1][0:4]),str(events)])   
    
    commands = ['setup_daq','recieve_one']    
    
    for command in commands:
        continue_with_next_command(command)
        if command == 'setup_daq'  : 
            subprocess.call(['sudo','./%s' % command])
            boards = get_boards()
            
        if command == 'receive_one': 
            for board in boards:
                subprocess.call(['sudo','./%s' % command ,str(board[0:4]),str(events),])
        
        
    
    print 'End of %d events.' % events

if __name__ == '__main__' :
    main()
