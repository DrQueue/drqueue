#!/bin/sh
START=$PWD
cd $1/Controller
cp ServicesController.Form1.resources.src.gz ServicesController.Form1.resources.gz
gunzip ServicesController.Form1.resources.gz
$2/csc.exe /t:winexe /win32icon:../Resources/drqueue.ico /out:ServicesController.exe /resource:ServicesController.Form1.resources *.cs 
mv ServicesController.exe $1
cd $1/ServiceInstaller
gcc -mno-cygwin service.cpp -o service.exe
mv service.exe $1
cd $1/Services/Service-ipc
$2/csc.exe /out:service-ipc.exe *.cs
mv service-ipc.exe $1
cd $1/Services/Service-master
$2/csc.exe /out:service-master.exe *.cs
mv service-master.exe $1
cd $START
