#
# Copyright (C) 2001,2002,2003,2004,2005,2006,2007 Jorge Daza Garcia-Blanes
# Copyright (C) 2009,2010 Andreas Schroeder
# Copyright (C) 2010 Alistair Leslie-Hughes
#
# This file is part of DrQueue
#
# DrQueue is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# DrQueue is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
# USA
#

import sys
import glob
import os
import platform,re
import subprocess

Universal_Binaries_Short_Name = 'fat'

def get_architecture(env,escape=False,underscore=True):
    machine = platform.machine()
    if underscore:
        machine = re.sub('\s','_',machine)
    if escape:
        machine = re.escape(machine)
    if not machine:
        machine = 'unknown'
    elif platform.uname()[0] == 'Darwin':
        if env.get('universal_binary'):
            machine = Universal_Binaries_Short_Name
        elif platform.architecture()[0] == '64bit':
            machine = 'x86_64'
    return machine

def wrapper_complete_command (env,cmdlist):
    kernel = platform.system()
    arch = get_architecture(env)
    rlist = []
    for cmd in cmdlist:
        cmd = os.path.split(cmd)[1] # Removes any directory component
        rlist.append('.'.join([cmd,kernel,arch]))
    return rlist

def get_abspath_glob(path):
    pathlist=glob.glob(path)
    rlist=[]
    for file in pathlist:
        rlist.append(os.path.abspath(file))
    return rlist

def copy_with_clean(src_files,dest_files,dest_path,env):
    rlist = []
    for s,d in zip(src_files,dest_files):
        t = env.InstallAs(os.path.join(dest_path,d),s)
        rlist.append(t)
    return rlist

def get_git_commit():
    try:
        gitlog = subprocess.Popen(["git", "log", "--oneline", "-1"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    except OSError:
        commit_string = ""
    else:
        output = gitlog.communicate()[0]
        if gitlog.returncode > 0:
            commit_string = ""
            print("Not a Git repository. Can't fetch commit id.")
        else:
            commit_string = output.split(" ")[0]
            print("Current Git commit id is: "+commit_string)
    return commit_string

def write_git_rev(commit_id):
    os.chdir("libdrqueue")
    rev_file = open("git_rev.h", "w")
    rev_file.write("#define REVISION \""+commit_id+"\"\n")
    os.chdir("..")
    rev_file.close()

# Default to mingw on windows for now.
if sys.platform == "win32":
    env_lib = Environment (ENV=os.environ, TOOLS = ['mingw'])
else:
    env_lib = Environment (ENV=os.environ)

# Default install path for unix systems
pathprefix = '/usr/local'

# Use the specific directory for non-unix systems.
if sys.platform == "win32":
    pathprefix = 'C:\\Program Files\\drqueue'

# Configuration options
opts = Variables('scons.conf')
opts.AddVariables(PathVariable('DESTDIR','Alternate root directory','',[]),
                  PathVariable('PREFIX','Directory to install under',pathprefix, PathVariable.PathAccept),
                  BoolVariable('universal_binary', 'Whether to build as an Universal Binary (MacOS X >= 10.3.9 only)', 0),
                  BoolVariable('build_linux32', 'Whether to explicitly build 32 bit binaries (Linux only),', 0),
                  BoolVariable('build_linux64', 'Whether to explicitly build 64 bit binaries (Linux only),', 0),
                  BoolVariable('build_drqman', 'Build drqman', 1),
                  BoolVariable('build_ppc', 'When used with universal_binary, whether to build for the PPC architecture', 0),
                  BoolVariable('enable_warnings','Enable warnings for compiling',0),
                  BoolVariable('enable_debug','Enable debugging information for compiling',0))
opts.Update(env_lib)
opts.Save('scons.conf',env_lib)

Help(opts.GenerateHelpText(env_lib))

# fetch Git commit id and write into git_rev.h
write_git_rev(get_git_commit())

if not env_lib.GetOption('clean'):
    conf = Configure(env_lib)
    if conf.CheckCHeader('unistd.h'):
        conf.env.Append(CPPDEFINES = Split ('HAVE_UNISTD_H'))
    if conf.CheckCHeader('stdint.h'):
        conf.env.Append(CPPDEFINES = Split('HAVE_STDINT_H'))
    if conf.CheckCHeader('getopt.h'):
        conf.env.Append(CPPDEFINES = Split('HAVE_GETOPT_H'))
    if conf.CheckCHeader('inttypes.h'):
        conf.env.Append(CPPDEFINES = Split ('HAVE_INTTYPES_H'))
    if conf.CheckCHeader('pwd.h'):
        conf.env.Append(CPPDEFINES = Split ('HAVE_PWD_H'))
    #if not conf.CheckType('PATH_MAX'):
    #    conf.env.Append(CPPDEFINES = Split ('PATH_MAX=512'))
    if not conf.CheckFunc('snprintf'):
        conf.env.Append(CPPDEFINES = Split ('snprintf=_snprintf'))
    env_lib = conf.Finish()

# Installation paths
idir_prefix = os.path.normpath(os.path.join('${DESTDIR}','${PREFIX}','drqueue'))
idir_bin    = os.path.join(idir_prefix,'bin')
idir_bin_viewcmd = os.path.join(idir_bin,'viewcmd')
idir_etc    = os.path.join(idir_prefix,'etc')
idir_db     = os.path.join(idir_prefix,'db')
idir_doc    = os.path.join(idir_prefix,'doc')
idir_logs   = os.path.join(idir_prefix,'logs')
idir_tmp    = os.path.join(idir_prefix,'tmp')
Export('env_lib idir_prefix idir_bin idir_bin_viewcmd idir_etc idir_db idir_doc idir_logs idir_tmp')

env_lib.Append (CPPPATH=['.','libdrqueue'])

# mingw doesnt need the -fPIC flag.
if env_lib['CC'] == "gcc" and not sys.platform == "win32":
    env_lib.Append (CCFLAGS=Split('-fPIC'))

env_lib.Append (CPPDEFINES = Split ('COMM_REPORT _GNU_SOURCE ' \
                + '_NO_COMPUTER_POOL_SEMAPHORES _NO_COMPUTER_SEMAPHORES'),
                CFLAGS = Split(os.environ.get('CFLAGS', '')),
                CXXFLAGS = [Split(os.environ.get('CXXFLAGS', ''))])

print "Platform is: ",sys.platform
if sys.platform == "linux2":
    env_lib.Append (CPPDEFINES = Split ('-D__LINUX'))
    if env_lib.get('build_linux32'):
        env_lib.Append (CCFLAGS = Split('-m32'))
        env_lib.Append (LINKFLAGS = Split('-m32'))
    elif env_lib.get('build_linux64'):
        env_lib.Append (CCFLAGS = Split('-m64'))
        env_lib.Append (LINKFLAGS = Split('-m64'))
elif sys.platform == "darwin":
    env_lib.Append (CPPDEFINES = Split ('-D__OSX'))
    if env_lib.get('universal_binary'):
        print "Building as an MacOS X Universal Binary"
        universal_archs = "-arch i386"
        if platform.architecture()[0] == '64bit':
            universal_archs += " -arch x86_64"            
        if env_lib.get('build_ppc'):
            universal_archs += " -arch ppc"
            env_lib.Append (CCFLAGS = Split('-isysroot /Developer/SDKs/MacOSX10.4u.sdk '+universal_archs+' -mmacosx-version-min=10.4'))
            env_lib.Append (LINKFLAGS = Split('-isysroot /Developer/SDKs/MacOSX10.4u.sdk '+universal_archs+' -mmacosx-version-min=10.4'))
        else:
            env_lib.Append (CCFLAGS = Split(universal_archs+' -mmacosx-version-min=10.5'))
            env_lib.Append (LINKFLAGS = Split(universal_archs+' -mmacosx-version-min=10.5'))
        env_lib['CC'] = '/usr/bin/gcc-4.0'
        env_lib['CXX'] = '/usr/bin/g++-4.0'
elif sys.platform == "irix6":
    env_lib.Append (CPPDEFINES = Split ('-D__IRIX'))
    env_lib['CC'] = 'c99'
elif sys.platform == "cygwin":
    env_lib.Append (CPPDEFINES = Split ('-D__CYGWIN'))
    os.environ['PKG_CONFIG_PATH'] = 'C:\GTK\lib\pkgconfig'
elif sys.platform == "win32":
    # Ignore MSVC security warnings by default.
    env_lib.Append (CPPDEFINES = Split ('_WIN32 _CRT_SECURE_NO_WARNINGS'))
    env_lib.Append (LIBS = Split ('kernel32 ws2_32 advapi32'))
    if env_lib['CC'] == "cl":
        env_lib.Append (LIBS = Split ('msvcrt OLDNAMES'))
        env_lib.Append (LINKFLAGS = Split('/DEFAULTLIB:MSVCRT /NODEFAULTLIB'))
elif (sys.platform == "freebsd7") or (sys.platform == "freebsd8"):
    env_lib.Append (CPPDEFINES = Split ('-D__FREEBSD'))
else:
    print "Unknown platform: %s"%(sys.platform,)
    exit (1)

# add additional warnings if requested
if env_lib.get('enable_warnings'):
    if env_lib['CC'] == "cl":
        env_lib.Append (CCFLAGS = Split('/W4'))
    else: 
        env_lib.Append (CCFLAGS = Split('-Wall -Wextra'))

# add additional debug output if requested
if env_lib.get('enable_debug'):
    env_lib.Append (CPPDEFINES = Split ('_DEBUG'))
    if env_lib['CC'] == "cl":
        env_lib.Append (CCFLAGS = Split('/Od'))
        env_lib.Append (LINKFLAGS = Split('/Debug /SUBSYSTEM:Console'))
    else: 
        env_lib.Append (CCFLAGS = Split('-g'))

# Base construction environment that links with the library
env = env_lib.Clone()
env.Append (LIBS = ['drqueue'],LIBPATH = ['libdrqueue'])

if sys.platform == "win32":
    # Ignore MSVC security warnings by default.
    env.Append (CPPDEFINES = Split ('_WIN32 _CRT_SECURE_NO_WARNINGS'))
    env.Append (LIBS = Split ('kernel32 ws2_32 advapi32'))

conscripts = []

#
# libdrqueue.a
#
conscripts.append('libdrqueue/SConscript')

#
# Main
#
master = env.Program ('master.c')
slave = env.Program ('slave.c')
if sys.platform == 'cygwin'  or sys.platform == 'win32':
    main_list = [ 'master.exe', 'slave.exe' ]
else:
    main_list = [ 'master', 'slave' ]
Default (master)
Default (slave)

#
# drqman
#
if env_lib['build_drqman']:
    print "Building drqman"
    conscripts.append('drqman/SConscript')
else:
    print "Not building drqman"

# Build all the files.
SConscript(conscripts, exports=['env_lib','main_list'])

## Build python modules
#Export ('env','env_lib','libdrqueue','libdrqueue_src')
#SConscript(['python/SConscript'])

#
# Tools
#
#if sys.platform == 'cygwin':
#  cmdline_tools = [ 'jobfinfo.exe','jobinfo.exe','compinfo.exe','requeue.exe','cfgreader.exe',
#                  'cjob.exe','blockhost.exe','sendjob.exe' ]
#  cpp_tools = [ 'sendjob.exe' ]
#else:
cmdline_tools = [ 'jobfinfo','jobinfo','compinfo','requeue','cfgreader',
                  'cjob','blockhost','sendjob' ]
cpp_tools = [ 'sendjob' ]

ctools = {}
for tool in cmdline_tools:
    if tool not in cpp_tools:
        ctools[tool] = env.Program (tool+'.c')
    else:
        ctools[tool ]= env.Program (tool+'.cpp')
    Default(ctools[tool])

install_base = idir_prefix

if sys.platform == 'cygwin' or sys.platform == 'win32':
    cmdline_tools_tmp = []
    for tool in cmdline_tools:
        cmdline_tools_tmp.append(tool + '.exe')
    cmdline_tools = cmdline_tools_tmp

bin_list = main_list + cmdline_tools
wrapped_bin_list = wrapper_complete_command (env,bin_list)
#wrapped_bin_copies = copy_with_clean(bin_list,wrapped_bin_list,'bin',env)
wrapped_bin_copies = copy_with_clean(bin_list,wrapped_bin_list,idir_bin,env)

# install etc files
etc_files = glob.glob(os.path.join('etc','*'))
copy_with_clean(etc_files,etc_files,idir_prefix,env)

# install bin files
bin_files = glob.glob(os.path.join('bin','*'))
# remove viewcmd because it's a directory, not a file
try:
    bin_files.remove('bin/viewcmd')
except ValueError:
    print("bin/viewcmd not found.")

# remove drqman wrapper if drqman wasn't built
if not env_lib['build_drqman']:
    bin_files.remove('bin/drqman')
    bin_files.remove('bin/drqman_win.sh')
copy_with_clean(bin_files,bin_files,idir_prefix,env)

# install viewcmd files
bin_viewcmd_files = glob.glob(os.path.join('bin/viewcmd','*'))
copy_with_clean(bin_viewcmd_files,bin_viewcmd_files,idir_prefix,env)

# install documentation files
doc_files = [ 'AUTHORS', 'COPYING', 'ChangeLog', 'INSTALL', 'NEWS', 'README', 'README.darwin', 'README.mentalray', 'README.python', 'README.shell_variables', 'README.windows', 'setenv' ]
n = len(doc_files)
for i in range(0, n):
    env.Install(idir_doc, doc_files[i])

env.Command (idir_prefix,[],[Mkdir("$TARGET"),Chmod("$TARGET",0777)])
perm_logs = env.Command (idir_logs,[],[Mkdir("$TARGET"),Chmod("$TARGET",0777)])
env.Clean(perm_logs,idir_logs)
perm_tmp = env.Command (idir_tmp,[],[Mkdir("$TARGET"),Chmod("$TARGET",0777)])
env.Clean(perm_tmp,idir_tmp)
perm_db = env.Command (idir_db,[],[Mkdir("$TARGET"),Chmod("$TARGET",0777)])
env.Clean(perm_db,idir_db)

#env.Depends (install_base,perm_logs)
#env.Depends (install_base,perm_tmp)
#env.Depends (install_base,perm_db)
env.Alias('install-wrapped-bin',wrapped_bin_copies)
env.Alias('install-bin',idir_bin)
env.Alias('install-bin_viewcmd',idir_bin_viewcmd)
env.Alias('install-etc',idir_etc)
env.Alias('install-tmp',perm_tmp)
env.Alias('install-db',perm_db)
env.Alias('install-doc',idir_doc)
env.Alias('install-logs',perm_logs)
env.Alias('install',['install-wrapped-bin','install-bin_viewcmd','install-bin','install-etc','install-db','install-doc','install-logs','install-tmp'])
