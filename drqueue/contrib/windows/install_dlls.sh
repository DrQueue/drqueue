#!/bin/sh

echo Installing linked windows dll..

cp -v --reply=no "`which cygwin1.dll`" $1

#copy exe linked dll
DLLS=`objdump.exe -p $1/*.exe |grep "DLL Name" |cut -d \  -f 3 |sort -u`
for file in $DLLS;
do
 FILEPATH=`which $file|grep -i /usr/bin`
 if test "$FILEPATH"; then  
  cp -v --reply=no "`which $file`" $1;
 fi
done

#copy exe linked dll
DLLS=`objdump.exe -p $1/*.dll |grep "DLL Name" |cut -d \  -f 3 |sort -u`
for file in $DLLS;
do
 FILEPATH=`which $file|grep -i /usr/bin`
 if test "$FILEPATH"; then  
  cp -v --reply=no "`which $file`" $1;
 fi
done

