#!/bin/sh
START=$PWD
cd $1/Controller
$2/csc.exe /t:winexe /win32icon:App.ico /out:ServicesController.exe /resource:ServicesController.Form1.resources *.cs 
mv ServicesController.exe $1
cd $START
