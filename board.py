import subprocess, sys, os

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
    choice = raw_input('**** I will do: %s. CONTINUE? [Y/n] **** ' % command)
    if choice in ('Y','y','') :
        print 'Executing ' + command
        return
    else:
        print 'Exiting'
        sys.exit(0)
    
        
        
    
    
def main():

    intro()
    events = read_n_events()
    
    commands = ['setup_daq','recieve_all']    
    
    for command in commands:
        continue_with_next_command(command)
        if command == 'setup_daq': subprocess.call(['sudo','./%s' % command])
    
    
    
    
    print 'End of %d events.' % events

if __name__ == '__main__' :
    main()
