#!/bin/sh
START=$PWD
cd $1/ServiceInstaller
gcc -mno-cygwin service.cpp -o service.exe
mv service.exe $1
cd $1/Services/Service-ipc
$2/csc.exe /out:service-ipc.exe *.cs
mv service-ipc.exe $1
cd $1/Services/Service-master
$2/csc.exe /out:service-master.exe *.cs
mv service-master.exe $1
cd $1/Services/Service-slave
$2/csc.exe /out:service-slave.exe *.cs
mv service-slave.exe $1
cd $1/Services/ServicesController
$2/csc.exe /t:winexe /win32icon:App.ico /out:ServicesController.exe /resource:Form1.resources *.cs 
mv ServicesController.exe $1
cd $START
