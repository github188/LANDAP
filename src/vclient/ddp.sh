#!/bin/sh

##################################
# ddp.sh
#   This script file is used to launch and stop ddp process.
#   It is allowed to be modified in order to integrate ddp to real environment.
##################################

# *****NOTICE *****
# appname is the executable name
# partner can change this filename at integration
appname=ddpd_vclient

# ***** NOTICE *****
# dir is the directory where executable is located in real environment (appname)
# partner can change this dir at integration
dir=$(dirname `readlink -f "$0"`)
apppath=$dir/$appname

# ***** WARNING *****
# pid is the process id found by function find_pid
# In device, there is ONLY ONE process allowed to run.
# Therefore, the existed process must be found and be killed before launching a new one.
pid=0

print_help() {
    echo "Usage: ddp.sh [start | stop | debug]"
    echo "  start : exec ddp client executable"
    echo "  stop  : stop a running ddp client program"
}

find_pid() {
    #pid=$(pidof $appname)
    pid=$(ps -ef | grep $appname | grep -v grep | awk '{print $2}')

    if [ "$pid" = "" ] || [ "$pid" = "0" ]; then
        echo "No $appname running"
    else
        echo "pid = $pid"
    fi
}

stop_pid() {
    echo "Stop"
    while [ "$pid" != "" -a "$pid" != "0" ]
    do
        kill -s USR1 $pid
        sleep 3
        find_pid
    done
}

start_pid() {
    stop_pid

    while [ "$pid" != "" -a "$pid" != "0" ]
    do
        sleep 1
        find_pid
    done

    echo "Start"
    [ -x "$apppath" ];  nohup $apppath > /dev/null 2>&1 &
}


# main
find_pid

if [ "$1" = "start" ]; then
    start_pid

elif [ "$1" = "stop" ]; then
    stop_pid

else
    print_help
fi
