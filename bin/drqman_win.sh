[ -z $DRQUEUE_ROOT ]   && export DRQUEUE_ROOT=/usr/local/drqueue
[ -z $DRQUEUE_MASTER ] && export DRQUEUE_MASTER=127.0.0.1
export CYGWIN=server
export PATH=$PATH:$DRQUEUE_ROOT/bin

startxwin.bat &
sleep 5
export DISPLAY=:0
drqman.exe.CYGWIN_NT-5.1.i686 -o