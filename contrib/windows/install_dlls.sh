#!/bin/sh

#copy exe linked dll
DLLS=`objdump.exe -p $1/*.exe |grep "DLL Name" |grep -E -e "(lib|cyg)" |cut -d \  -f 3 |sort -u`
for file in $DLLS;
do
 cp -v --reply=no "`which $file`" $1;
done

#copy dll linked dll
DLLS=`objdump.exe -p $1/* |grep "DLL Name" |grep -E -e "(lib|cyg)" |cut -d \  -f 3 |sort -u`
for file in $DLLS;
do
 cp -v --reply=no "`which $file`" $1;
done

#copy linked dll linked dll
DLLS=`objdump.exe -p $1/* |grep "DLL Name" |grep -E -e "(lib|cyg)" |cut -d \  -f 3 |sort -u`
for file in $DLLS;
 do cp -v --reply=no "`which $file`" $1;
done

#copy linked dll linked dll
DLLS=`objdump.exe -p $1/* |grep "DLL Name" |grep -E -e "(lib|cyg)" |cut -d \  -f 3 |sort -u`
for file in $DLLS;
 do cp -v --reply=no "`which $file`" $1;
done
