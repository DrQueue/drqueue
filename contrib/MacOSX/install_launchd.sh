#!/bin/bash

# Launchd installer for OSX
# Andreas Schroeder, 2009

echo "\nThis will prepare your system for automatic startup of DrQueue daemons.\n\
You need admin priviledges for this. Please type in your password if asked.\n\
This has only been tested on Mac OSX 10.5 Leopard so far.\n"

setup_master () {
 echo " * Installing Launchd script for master daemon."
 echo "sudo cp org.drqueue.Master.plist /Library/LaunchDaemons/"
 sudo cp org.drqueue.Master.plist /Library/LaunchDaemons/
 echo " "
}

setup_slave () {
 echo " * Installing Launchd script for slave daemon."
 echo "sudo cp org.drqueue.Slave.plist /Library/LaunchDaemons/"
 sudo cp org.drqueue.Slave.plist /Library/LaunchDaemons/
 echo " "
}

setup_shm_sys () {
 # set kernel variables for shared memory for every startup
 echo " * Setting up kernel variables for next boot in /etc/sysctl.conf."
 echo sudo su root -c "echo kern.sysv.shmmax=536870912 >>/etc/sysctl.conf"
 sudo su root -c "echo kern.sysv.shmmax=536870912 >>/etc/sysctl.conf"
 echo sudo su root -c "echo kern.sysv.shmmin=1 >>/etc/sysctl.conf"
 sudo su root -c "echo kern.sysv.shmmin=1 >>/etc/sysctl.conf"
 echo sudo su root -c "echo kern.sysv.shmmni=128 >>/etc/sysctl.conf"
 sudo su root -c "echo kern.sysv.shmmni=128 >>/etc/sysctl.conf"
 echo sudo su root -c "echo kern.sysv.shmseg=32 >>/etc/sysctl.conf"
 sudo su root -c "echo kern.sysv.shmseg=32 >>/etc/sysctl.conf"
 echo sudo su root -c "echo kern.sysv.shmall=131072 >>/etc/sysctl.conf"
 sudo su root -c "echo kern.sysv.shmall=131072 >>/etc/sysctl.conf"
 echo " "
}

setup_shm_now () {
 # set kernel variables for shared memory for now
 echo " * Setting up kernel variables for this session."
 sudo sysctl -w kern.sysv.shmmax=536870912
 sudo sysctl -w kern.sysv.shmmin=1
 sudo sysctl -w kern.sysv.shmmni=128
 sudo sysctl -w kern.sysv.shmseg=32
 sudo sysctl -w kern.sysv.shmall=131072
 echo " "
}

setup_user () {
 # add a system user for daemons
 echo sudo dscl . -create /Users/drqueue
 sudo dscl . -create /Users/drqueue
 echo sudo dscl . -create /Users/drqueue UserShell /bin/bash
 sudo dscl . -create /Users/drqueue UserShell /bin/bash
 echo sudo dscl . -create /Users/drqueue RealName "DrQueue daemons"
 sudo dscl . -create /Users/drqueue RealName "DrQueue daemons"
 echo sudo dscl . -create /Users/drqueue PrimaryGroupID 20
 sudo dscl . -create /Users/drqueue PrimaryGroupID 20
 echo sudo dscl . -create /Users/drqueue NFSHomeDirectory /Users/drqueue
 sudo dscl . -create /Users/drqueue NFSHomeDirectory /Users/drqueue
 echo sudo dscl . -create /Users/drqueue UniqueID 555
 sudo dscl . -create /Users/drqueue UniqueID 555
 echo sudo mkdir /Users/drqueue
 sudo mkdir /Users/drqueue
 echo sudo chown drqueue /Users/drqueue
 sudo chown drqueue /Users/drqueue
 echo " "
}
  
start_master () {
 # start Launchd script for master
 echo " * Starting master daemon."
 echo sudo launchctl load /Library/LaunchDaemons/org.drqueue.Master.plist
 sudo launchctl load /Library/LaunchDaemons/org.drqueue.Master.plist
 echo " "
}

start_slave () {
 # start Launchd script for slave
 echo " * Starting slave daemon."
 echo sudo launchctl load /Library/LaunchDaemons/org.drqueue.Slave.plist
 sudo launchctl load /Library/LaunchDaemons/org.drqueue.Slave.plist
 echo " "
}

menu_daemon () {
 echo " * What should be started automatically on this machine? Press M for master, S for slave or B for both: \c"
 read CHAR

 if [ "$CHAR" == "M" ]; then
	echo "\nMaster only selected.\n"
	setup_master
	setup_shm_sys
	setup_shm_now
	start_master
	break
 elif [ "$CHAR" == "S" ]; then
	echo "\nSlave only selected.\n"
	setup_slave
	setup_shm_sys
	setup_shm_now
	start_slave
	break
 elif [ "$CHAR" == "B" ]; then
        echo "\nBoth master and slave selected.\n"
	setup_master
	setup_slave
	setup_shm_sys
	setup_shm_now
	start_master
	start_slave
	break
 else
	menu_daemon
 fi
}

menu_user () {
 echo " * Should a daemon user 'drqueue' be created? Running daemons as root user can lead to security problems. Press Y for yes, or N for no: \c"
 read CHAR

 if [ "$CHAR" == "Y" ]; then
	echo "\n * Creating a daemon user 'drqueue'."
	setup_user
	break
 elif [ "$CHAR" == "N" ]; then
	echo "\nNot creating a daemon user.\n "
	break
 else
	menu_daemon
 fi
}

# program start

menu_user

menu_daemon

echo "Finished. Try to run Drqman to see if the daemons are working."

