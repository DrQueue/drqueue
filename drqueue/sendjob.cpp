//
// $Id: sendjob.cpp,v 1.2 2002/12/02 22:24:08 jorge Exp $
// 
// To set up a maya job from a file and send it to the master
//

#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

#ifdef __LINUX
#include <stdint.h>
#else
# ifdef __IRIX
#include <sys/types.h>
# else
#  error You need to define the OS, or OS defined not supported
# endif
#endif

#include "sendjob.h"
#include "libdrqueue.h"

int main (int argc,char *argv[])
{
  int opt;
    
  presentation();

  while ((opt = getopt (argc,argv,"hv")) != -1) {
    switch (opt) {
    case 'v':
      //      show_version (argv);
      exit (0);
    case '?':
    case 'h':
      usage();
      exit (1);
    }
  }

  if (argc < 2) {
    usage ();
    exit (1);
  }

  
  ifstream infile(argv[1]);
  if (RegisterJobFromFile (infile)) {
    cerr << "Error register job from file: " << argv[1] << endl;
    exit (1);
  }

  cerr << "Job sent successfuly to the queue\n";

  //  register_job (&job);

  exit (0);
}

void presentation (void)
{
  cout << "DrQueue - by Triple-e VFX\n\n";
}

void cleanup (int signum)
{
  exit(0);
}

void usage (void)
{
  cerr << "Usage: sendjob [-vh] <job_file>\n";
  cerr << "Valid options:\n"
       << "\t-v version information\n"
       << "\t-h prints this help\n";
}

int RegisterJobFromFile (ifstream &infile)
{
  // Job variables for the script generator
  struct job job;
  struct mayasgi mayaSgi;

  string owner;
  string jobName;
  string camera;
  int frameStart,frameEnd,frameStep;
  int resX,resY;
  string scenePath;
  string renderDir;
  string fileFormat;
  string image;
  char *pathToScript;

  getline(infile,owner);
  getline(infile,jobName);
  getline(infile,camera);
  infile >> frameStart;
  infile >> frameEnd;
  infile >> frameStep;
  infile >> resX;
  infile >> resY;
  getline(infile,scenePath);	// 
  getline(infile,scenePath);	// Get two times because '>>' leaves the pointer before \n 
  getline(infile,renderDir);
  getline(infile,fileFormat);
  getline(infile,image);

  strncpy(mayaSgi.file_owner,owner.c_str(),BUFFERLEN-1);
  strncpy(mayaSgi.camera,camera.c_str(),BUFFERLEN-1);
  mayaSgi.res_x = resX;
  mayaSgi.res_y = resY;
  strncpy(mayaSgi.scene,scenePath.c_str(),BUFFERLEN-1);
  strncpy(mayaSgi.renderdir,renderDir.c_str(),BUFFERLEN-1);
  strncpy(mayaSgi.format,fileFormat.c_str(),BUFFERLEN-1);
  snprintf(mayaSgi.scriptdir,BUFFERLEN,"%s/tmp/",getenv("DRQUEUE_ROOT"));
  strncpy(mayaSgi.image,image.c_str(),BUFFERLEN-1);

  if (!(pathToScript = mayasg_create(&mayaSgi))) {
    cerr << "Error creating script file\n";
    return 1;
  }

  strncpy (job.name,jobName.c_str(),MAXNAMELEN-1);
  strncpy (job.cmd,pathToScript,MAXCMDLEN-1);
  strncpy (job.owner,owner.c_str(),MAXNAMELEN-1);
  strncpy (job.email,owner.c_str(),MAXNAMELEN-1);
  job.frame_start = frameStart;
  job.frame_end = frameEnd;
  job.frame_step = frameStep;
  job.priority = 500;

  job.koj = KOJ_MAYA;
  strncpy (job.koji.maya.scene,scenePath.c_str(),BUFFERLEN-1);
  strncpy (job.koji.maya.renderdir,renderDir.c_str(),BUFFERLEN-1);
  strncpy (job.koji.maya.image,"",BUFFERLEN-1);
  strncpy (job.koji.maya.viewcmd,"",BUFFERLEN-1);

  job.limits.os_flags = OSF_LINUX;
  job.limits.nmaxcpus = -1;
  job.limits.nmaxcpuscomputer = -1;

  if (!register_job(&job)) {
    cerr << "Error sending job to the queue\n";
    return 1;
  }

  return 0;
}






