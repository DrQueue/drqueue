//
// Copyright (C) 2001,2002,2003,2004 Jorge Daza Garcia-Blanes
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
// 
//
// $Id$
// 
// To set up a maya job from a file and send it to the master
//

#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

#if defined (__IRIX)
#include <sys/types.h>
#else
#include <stdint.h>
#endif

#include "sendjob.h"
#include "libdrqueue.h"

int main (int argc,char *argv[])
{
  int opt;
  int toj = 0;

  presentation();

  while ((opt = getopt (argc,argv,"hvt:")) != -1) {
    switch (opt) {
    case 'v':
      //      show_version (argv);
      exit (0);
		case 't':
			toj = str2toj (optarg);
			break;
    case '?':
    case 'h':
      usage();
      exit (1);
    }
  }

  if ((argc < 3) || (toj == TOJ_NONE)) {
    usage ();
    exit (1);
  }

	set_default_env();

  if (!common_environment_check()) {
    std::cerr << "Error checking the environment: " << drerrno_str() << std::endl;
    exit (1);
  }

  std::ifstream infile(argv[argc-1]);
	switch (toj) {
		case TOJ_MAYA:
  		if (RegisterMayaJobFromFile (infile)) {
				std::cerr << "Error registering MAYA job from file: " << argv[argc-1] << std::endl;
    		exit (1);
  		}
			break;
		case TOJ_MENTALRAY:
  		if (RegisterMentalrayJobFromFile (infile)) {
				std::cerr << "Error registering MENTALRAY job from file: " << argv[argc-1] << std::endl;
    		exit (1);
  		}
			break;
		case TOJ_BLENDER:
  		if (RegisterBlenderJobFromFile (infile)) {
				std::cerr << "Error registering BLENDER job from file: " << argv[argc-1] << std::endl;
    		exit (1);
  		}
			break;
		case TOJ_BMRT:
  		if (RegisterBmrtJobFromFile (infile)) {
				std::cerr << "Error registering BMRT job from file: " << argv[argc-1] << std::endl;
    		exit (1);
  		}
			break;
		case TOJ_THREEDELIGHT:
  		if (RegisterThreedelightJobFromFile (infile)) {
				std::cerr << "Error registering 3Delight job from file: " << argv[argc-1] << std::endl;
    		exit (1);
  		}
			break;
		case TOJ_PIXIE:
  		if (RegisterPixieJobFromFile (infile)) {
				std::cerr << "Error registering PIXIE job from file: " << argv[argc-1] << std::endl;
    		exit (1);
  		}
			break;
		case TOJ_LIGHTWAVE:
  		if (RegisterLightwaveJobFromFile (infile)) {
				std::cerr << "Error registering Lightwave job from file: " << argv[argc-1] << std::endl;
    		exit (1);
  		}
			break;
					case TOJ_AFTEREFFECTS:
  		if (RegisterAftereffectsJobFromFile (infile)) {
				std::cerr << "Error registering After Effects job from file: " << argv[argc-1] << std::endl;
    		exit (1);
  		}
			break;
		case TOJ_SHAKE:
  		if (RegisterShakeJobFromFile (infile)) {
				std::cerr << "Error registering Shake job from file: " << argv[argc-1] << std::endl;
    		exit (1);
  		}
			break;
		case TOJ_AQSIS:
  		if (RegisterAqsisJobFromFile (infile)) {
				std::cerr << "Error registering Aqsis job from file: " << argv[argc-1] << std::endl;
    		exit (1);
  		}
			break;
		case TOJ_TERRAGEN:
  		if (RegisterTerragenJobFromFile (infile)) {
				std::cerr << "Error registering Terragen job from file: " << argv[argc-1] << std::endl;
    		exit (1);
  		}
			break;
		case TOJ_NUKE:
  		if (RegisterNukeJobFromFile (infile)) {
				std::cerr << "Error registering Nuke job from file: " << argv[argc-1] << std::endl;
    		exit (1);
  		}
			break;
	}

  std::cerr << "Job sent successfuly to the queue\n";

  exit (0);
}

void presentation (void)
{
  std::cout << "DrQueue - by Jorge Daza García Blanes\n\n";
}

void cleanup (int signum)
{
  exit(0);
}

void usage (void)
{
  std::cerr << "Usage: sendjob [-vh] -t <type> <job_file>\n"
						<< "Valid options:\n"
						<< "\t-v version information\n"
						<< "\t-h prints this help\n"
						<< "\t-t [maya|blender|mentalray|bmrt|aqsis|3delight|pixie|lightwave|terragen|nuke|aftereffects|shake] type of job\n";
}

int RegisterMayaJobFromFile (std::ifstream &infile)
{
  // Job variables for the script generator
  struct job job;
  struct mayasgi mayaSgi;

  std::string owner;
  std::string jobName;
  std::string camera;
  int frameStart,frameEnd,frameStep;
  int resX,resY;
  std::string scenePath;
  std::string renderDir;
	std::string projectDir;
  std::string fileFormat;
  std::string image;
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
	getline(infile,projectDir);
  getline(infile,fileFormat);
  getline(infile,image);

  strncpy(mayaSgi.file_owner,owner.c_str(),BUFFERLEN-1);
  strncpy(mayaSgi.camera,camera.c_str(),BUFFERLEN-1);
  mayaSgi.res_x = resX;
  mayaSgi.res_y = resY;
  strncpy(mayaSgi.scene,scenePath.c_str(),BUFFERLEN-1);
  strncpy(mayaSgi.renderdir,renderDir.c_str(),BUFFERLEN-1);
	strncpy(mayaSgi.projectdir,projectDir.c_str(),BUFFERLEN-1);
	strncpy(mayaSgi.format,fileFormat.c_str(),BUFFERLEN-1);
  snprintf(mayaSgi.scriptdir,BUFFERLEN,"%s/tmp/",getenv("DRQUEUE_ROOT"));
  strncpy(mayaSgi.image,image.c_str(),BUFFERLEN-1);

  if (!(pathToScript = mayasg_create(&mayaSgi))) {
    std::cerr << "Error creating script file\n";
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
	strncpy (job.koji.maya.projectdir,projectDir.c_str(),BUFFERLEN-1);
  strncpy (job.koji.maya.image,"",BUFFERLEN-1);
  strncpy (job.koji.maya.viewcmd,"",BUFFERLEN-1);

  job.limits.os_flags = OSF_LINUX;
  job.limits.nmaxcpus = (uint16_t)-1;
  job.limits.nmaxcpuscomputer = (uint16_t)-1;
	job.limits.memory = 0;
	strncpy (job.limits.pool,"Default",MAXNAMELEN-1);

  if (!register_job(&job)) {
    std::cerr << "Error sending job to the queue\n";
    return 1;
  }

  return 0;
}

int RegisterMentalrayJobFromFile (std::ifstream &infile)
{
  // Job variables for the script generator
  struct job job;
  struct mentalraysgi mentalraySgi;

  std::string owner;
  std::string jobName;
  std::string camera;
  int frameStart,frameEnd,frameStep;
  int resX,resY;
  std::string scenePath;
  std::string renderDir;
  std::string fileFormat;
  std::string image;
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

  strncpy(mentalraySgi.file_owner,owner.c_str(),BUFFERLEN-1);
  strncpy(mentalraySgi.camera,camera.c_str(),BUFFERLEN-1);
  mentalraySgi.res_x = resX;
  mentalraySgi.res_y = resY;
  strncpy(mentalraySgi.scene,scenePath.c_str(),BUFFERLEN-1);
  strncpy(mentalraySgi.renderdir,renderDir.c_str(),BUFFERLEN-1);
  strncpy(mentalraySgi.format,fileFormat.c_str(),BUFFERLEN-1);
  snprintf(mentalraySgi.scriptdir,BUFFERLEN,"%s/tmp/",getenv("DRQUEUE_ROOT"));
  strncpy(mentalraySgi.image,image.c_str(),BUFFERLEN-1);

  if (!(pathToScript = mentalraysg_create(&mentalraySgi))) {
    std::cerr << "Error creating script file\n";
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

  job.koj = KOJ_MENTALRAY;
  strncpy (job.koji.mentalray.scene,scenePath.c_str(),BUFFERLEN-1);
  strncpy (job.koji.mentalray.renderdir,renderDir.c_str(),BUFFERLEN-1);
  strncpy (job.koji.mentalray.image,"",BUFFERLEN-1);
  strncpy (job.koji.mentalray.viewcmd,"",BUFFERLEN-1);

  job.limits.os_flags = OSF_LINUX;
  job.limits.nmaxcpus = (uint16_t)-1;
  job.limits.nmaxcpuscomputer = (uint16_t)-1;
	job.limits.memory = 0;
	strncpy (job.limits.pool,"Default",MAXNAMELEN-1);

  if (!register_job(&job)) {
    std::cerr << "Error sending job to the queue\n";
    return 1;
  }

  return 0;
}

int RegisterBlenderJobFromFile (std::ifstream &infile)
{
  // Job variables for the script generator
  struct job job;
  struct blendersgi blenderSgi;

  std::string jobName;
  int frameStart,frameEnd,frameStep;
  std::string scenePath;
  char *pathToScript;

  getline(infile,jobName);
  infile >> frameStart;
  infile >> frameEnd;
  infile >> frameStep;
  getline(infile,scenePath);	//
  getline(infile,scenePath);	// Get two times because '>>' leaves the pointer before \n 

  strncpy(blenderSgi.scene,scenePath.c_str(),BUFFERLEN-1);
  snprintf(blenderSgi.scriptdir,BUFFERLEN,"%s/tmp/",getenv("DRQUEUE_ROOT"));

  if (!(pathToScript = blendersg_create(&blenderSgi))) {
		std::cerr << "Error creating script file\n";
    return 1;
  }

  strncpy (job.name,jobName.c_str(),MAXNAMELEN-1);
  strncpy (job.cmd,pathToScript,MAXCMDLEN-1);
  strncpy (job.owner,getenv("USER"),MAXNAMELEN-1);
  strncpy (job.email,getenv("USER"),MAXNAMELEN-1);
  job.frame_start = frameStart;
  job.frame_end = frameEnd;
  job.frame_step = frameStep;
  job.block_size = frameStep;
  job.priority = 500;

  job.koj = KOJ_BLENDER;
  strncpy (job.koji.blender.scene,scenePath.c_str(),BUFFERLEN-1);
  strncpy (job.koji.blender.viewcmd,"",BUFFERLEN-1);

  job.limits.os_flags = OSF_LINUX;
  job.limits.nmaxcpus = (uint16_t)-1;
  job.limits.nmaxcpuscomputer = (uint16_t)-1;
	job.limits.memory = 0;
	strncpy (job.limits.pool,"Default",MAXNAMELEN-1);

  if (!register_job(&job)) {
		std::cerr << "Error sending job to the queue\n";
    return 1;
  }

  return 0;
}

int RegisterLightwaveJobFromFile (std::ifstream &infile)
{
  // Job variables for the script generator
  struct job job;
  struct lightwavesgi lightwaveSgi;

  std::string owner;
  std::string jobName;
  int frameStart,frameEnd,frameStep;
  std::string scenePath;
  std::string configDir;
	std::string projectDir;
  char *pathToScript;

  getline(infile,owner);
  getline(infile,jobName);
  infile >> frameStart;
  infile >> frameEnd;
  infile >> frameStep;
  getline(infile,scenePath);	//
  getline(infile,scenePath);	// Get two times because '>>' leaves the pointer before \n 
  getline(infile,configDir);
	getline(infile,projectDir);

//  strncpy(lightwaveSgi.file_owner,owner.c_str(),BUFFERLEN-1);
  strncpy(lightwaveSgi.scene,scenePath.c_str(),BUFFERLEN-1);
  strncpy(lightwaveSgi.configdir,configDir.c_str(),BUFFERLEN-1);
	strncpy(lightwaveSgi.projectdir,projectDir.c_str(),BUFFERLEN-1);
  snprintf(lightwaveSgi.scriptdir,BUFFERLEN,"%s/tmp/",getenv("DRQUEUE_ROOT"));

  if (!(pathToScript = lightwavesg_create(&lightwaveSgi))) {
    std::cerr << "Error creating script file\n";
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

  job.koj = KOJ_LIGHTWAVE;
  strncpy (job.koji.lightwave.scene,scenePath.c_str(),BUFFERLEN-1);
  strncpy (job.koji.lightwave.configdir,configDir.c_str(),BUFFERLEN-1);
	strncpy (job.koji.lightwave.projectdir,projectDir.c_str(),BUFFERLEN-1);
  strncpy (job.koji.lightwave.viewcmd,"",BUFFERLEN-1);

  job.limits.os_flags = OSF_LINUX;
  job.limits.nmaxcpus = (uint16_t)-1;
  job.limits.nmaxcpuscomputer = (uint16_t)-1;
	job.limits.memory = 0;
	strncpy (job.limits.pool,"Default",MAXNAMELEN-1);

  if (!register_job(&job)) {
    std::cerr << "Error sending job to the queue\n";
    return 1;
  }

  return 0;
}

int RegisterTerragenJobFromFile (std::ifstream &infile)
{
  // Job variables for the script generator
  struct job job;
  struct terragensgi terragenSgi;

  std::string owner;
  std::string jobName;
  int frameStart,frameEnd,frameStep;
  std::string scenePath;
  std::string worldfile;
	std::string terrainfile;
  char *pathToScript;

  getline(infile,owner);
  getline(infile,jobName);
  infile >> frameStart;
  infile >> frameEnd;
  infile >> frameStep;
  getline(infile,scenePath);	//
  getline(infile,scenePath);	// Get two times because '>>' leaves the pointer before \n 
  getline(infile,worldfile);
	getline(infile,terrainfile);

  strncpy(terragenSgi.file_owner,owner.c_str(),BUFFERLEN-1);
  strncpy(terragenSgi.scriptfile,scenePath.c_str(),BUFFERLEN-1);
  strncpy(terragenSgi.worldfile,worldfile.c_str(),BUFFERLEN-1);
	strncpy(terragenSgi.terrainfile,terrainfile.c_str(),BUFFERLEN-1);
  snprintf(terragenSgi.scriptdir,BUFFERLEN,"%s/tmp/",getenv("DRQUEUE_ROOT"));

  if (!(pathToScript = terragensg_create(&terragenSgi))) {
    std::cerr << "Error creating script file\n";
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

  job.koj = KOJ_TERRAGEN;
  strncpy (job.koji.terragen.scriptfile,scenePath.c_str(),BUFFERLEN-1);
  strncpy (job.koji.terragen.worldfile,worldfile.c_str(),BUFFERLEN-1);
	strncpy (job.koji.terragen.terrainfile,terrainfile.c_str(),BUFFERLEN-1);
  strncpy (job.koji.terragen.viewcmd,"",BUFFERLEN-1);

  job.limits.os_flags = OSF_LINUX;
  job.limits.nmaxcpus = (uint16_t)-1;
  job.limits.nmaxcpuscomputer = (uint16_t)-1;
	job.limits.memory = 0;
	strncpy (job.limits.pool,"Default",MAXNAMELEN-1);

  if (!register_job(&job)) {
    std::cerr << "Error sending job to the queue\n";
    return 1;
  }

  return 0;
}

int RegisterShakeJobFromFile (std::ifstream &infile)
{
  // Job variables for the script generator
  struct job job;
  struct shakesgi shakeSgi;

  std::string owner;
  std::string jobName;
  int frameStart,frameEnd,frameStep;
  std::string scenePath;
  char *pathToScript;

  getline(infile,owner);
  getline(infile,jobName);
  infile >> frameStart;
  infile >> frameEnd;
  infile >> frameStep;
  getline(infile,scenePath);	//
  getline(infile,scenePath);	// Get two times because '>>' leaves the pointer before \n 

//  strncpy(shakeSgi.file_owner,owner.c_str(),BUFFERLEN-1);
  strncpy(shakeSgi.script,scenePath.c_str(),BUFFERLEN-1);
  snprintf(shakeSgi.scriptdir,BUFFERLEN,"%s/tmp/",getenv("DRQUEUE_ROOT"));

  if (!(pathToScript = shakesg_create(&shakeSgi))) {
    std::cerr << "Error creating script file\n";
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

  job.koj = KOJ_SHAKE;
  strncpy (job.koji.shake.script,scenePath.c_str(),BUFFERLEN-1);
  strncpy (job.koji.shake.viewcmd,"",BUFFERLEN-1);

  job.limits.os_flags = OSF_LINUX;
  job.limits.nmaxcpus = (uint16_t)-1;
  job.limits.nmaxcpuscomputer = (uint16_t)-1;
	job.limits.memory = 0;
	strncpy (job.limits.pool,"Default",MAXNAMELEN-1);

  if (!register_job(&job)) {
    std::cerr << "Error sending job to the queue\n";
    return 1;
  }

  return 0;
}

int RegisterNukeJobFromFile (std::ifstream &infile)
{
  // Job variables for the script generator
  struct job job;
  struct nukesgi nukeSgi;

  std::string owner;
  std::string jobName;
  int frameStart,frameEnd,frameStep;
  std::string scenePath;
  char *pathToScript;

  getline(infile,owner);
  getline(infile,jobName);
  infile >> frameStart;
  infile >> frameEnd;
  infile >> frameStep;
  getline(infile,scenePath);	//
  getline(infile,scenePath);	// Get two times because '>>' leaves the pointer before \n 

//  strncpy(nukeSgi.file_owner,owner.c_str(),BUFFERLEN-1);
  strncpy(nukeSgi.scene,scenePath.c_str(),BUFFERLEN-1);
  snprintf(nukeSgi.scriptdir,BUFFERLEN,"%s/tmp/",getenv("DRQUEUE_ROOT"));

  if (!(pathToScript = nukesg_create(&nukeSgi))) {
    std::cerr << "Error creating script file\n";
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

  job.koj = KOJ_NUKE;
  strncpy (job.koji.nuke.scene,scenePath.c_str(),BUFFERLEN-1);
  strncpy (job.koji.nuke.viewcmd,"",BUFFERLEN-1);

  job.limits.os_flags = OSF_LINUX;
  job.limits.nmaxcpus = (uint16_t)-1;
  job.limits.nmaxcpuscomputer = (uint16_t)-1;
	job.limits.memory = 0;
	strncpy (job.limits.pool,"Default",MAXNAMELEN-1);

  if (!register_job(&job)) {
    std::cerr << "Error sending job to the queue\n";
    return 1;
  }

  return 0;
}

int RegisterThreedelightJobFromFile (std::ifstream &infile)
{
  // Job variables for the script generator
  struct job job;
  struct threedelightsgi threedelightSgi;

  std::string owner;
  std::string jobName;
  int frameStart,frameEnd,frameStep;
  std::string scenePath;
  char *pathToScript;

  getline(infile,owner);
  getline(infile,jobName);
  infile >> frameStart;
  infile >> frameEnd;
  infile >> frameStep;
  getline(infile,scenePath);	//
  getline(infile,scenePath);	// Get two times because '>>' leaves the pointer before \n 

  strncpy(threedelightSgi.file_owner,owner.c_str(),BUFFERLEN-1);
  strncpy(threedelightSgi.scene,scenePath.c_str(),BUFFERLEN-1);
  snprintf(threedelightSgi.scriptdir,BUFFERLEN,"%s/tmp/",getenv("DRQUEUE_ROOT"));

  if (!(pathToScript = threedelightsg_create(&threedelightSgi))) {
    std::cerr << "Error creating script file\n";
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

  job.koj = KOJ_3DELIGHT;
  strncpy (job.koji.threedelight.scene,scenePath.c_str(),BUFFERLEN-1);
  strncpy (job.koji.threedelight.viewcmd,"",BUFFERLEN-1);

  job.limits.os_flags = OSF_LINUX;
  job.limits.nmaxcpus = (uint16_t)-1;
  job.limits.nmaxcpuscomputer = (uint16_t)-1;
	job.limits.memory = 0;
	strncpy (job.limits.pool,"Default",MAXNAMELEN-1);

  if (!register_job(&job)) {
    std::cerr << "Error sending job to the queue\n";
    return 1;
  }

  return 0;
}

int RegisterPixieJobFromFile (std::ifstream &infile)
{
  // Job variables for the script generator
  struct job job;
  struct pixiesgi pixieSgi;

  std::string owner;
  std::string jobName;
  int frameStart,frameEnd,frameStep;
  std::string scenePath;
  char *pathToScript;

  getline(infile,owner);
  getline(infile,jobName);
  infile >> frameStart;
  infile >> frameEnd;
  infile >> frameStep;
  getline(infile,scenePath);	//
  getline(infile,scenePath);	// Get two times because '>>' leaves the pointer before \n 

//  strncpy(pixieSgi.file_owner,owner.c_str(),BUFFERLEN-1);
  strncpy(pixieSgi.scene,scenePath.c_str(),BUFFERLEN-1);
  snprintf(pixieSgi.scriptdir,BUFFERLEN,"%s/tmp/",getenv("DRQUEUE_ROOT"));

  if (!(pathToScript = pixiesg_create(&pixieSgi))) {
    std::cerr << "Error creating script file\n";
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

  job.koj = KOJ_PIXIE;
  strncpy (job.koji.pixie.scene,scenePath.c_str(),BUFFERLEN-1);
  strncpy (job.koji.pixie.viewcmd,"",BUFFERLEN-1);

  job.limits.os_flags = OSF_LINUX;
  job.limits.nmaxcpus = (uint16_t)-1;
  job.limits.nmaxcpuscomputer = (uint16_t)-1;
	job.limits.memory = 0;
	strncpy (job.limits.pool,"Default",MAXNAMELEN-1);

  if (!register_job(&job)) {
    std::cerr << "Error sending job to the queue\n";
    return 1;
  }

  return 0;
}

int RegisterAftereffectsJobFromFile (std::ifstream &infile)
{
  // Job variables for the script generator
  struct job job;
  struct aftereffectssgi aftereffectsSgi;

  std::string owner;
  std::string jobName;
  int frameStart,frameEnd,frameStep;
  std::string scenePath;
  std::string comp;
  char *pathToScript;

  getline(infile,owner);
  getline(infile,jobName);
  infile >> frameStart;
  infile >> frameEnd;
  infile >> frameStep;
  getline(infile,scenePath);	//
  getline(infile,scenePath);	// Get two times because '>>' leaves the pointer before \n 
  getline(infile,comp);

//  strncpy(aftereffectsSgi.file_owner,owner.c_str(),BUFFERLEN-1);
  strncpy(aftereffectsSgi.project,scenePath.c_str(),BUFFERLEN-1);
  strncpy(aftereffectsSgi.comp,comp.c_str(),BUFFERLEN-1);
  snprintf(aftereffectsSgi.scriptdir,BUFFERLEN,"%s/tmp/",getenv("DRQUEUE_ROOT"));

  if (!(pathToScript = aftereffectssg_create(&aftereffectsSgi))) {
    std::cerr << "Error creating script file\n";
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

  job.koj = KOJ_AFTEREFFECTS;
  strncpy (job.koji.aftereffects.project,scenePath.c_str(),BUFFERLEN-1);
  strncpy (job.koji.aftereffects.comp,comp.c_str(),BUFFERLEN-1);
  strncpy (job.koji.aftereffects.viewcmd,"",BUFFERLEN-1);

  job.limits.os_flags = OSF_LINUX;
  job.limits.nmaxcpus = (uint16_t)-1;
  job.limits.nmaxcpuscomputer = (uint16_t)-1;
	job.limits.memory = 0;
	strncpy (job.limits.pool,"Default",MAXNAMELEN-1);

  if (!register_job(&job)) {
    std::cerr << "Error sending job to the queue\n";
    return 1;
  }

  return 0;
}

int RegisterAqsisJobFromFile (std::ifstream &infile)
{
  // Job variables for the script generator
  struct job job;
  struct aqsissgi aqsisSgi;

  std::string owner;
  std::string jobName;
  int frameStart,frameEnd,frameStep;
  std::string scenePath;
  char *pathToScript;

  getline(infile,owner);
  getline(infile,jobName);
  infile >> frameStart;
  infile >> frameEnd;
  infile >> frameStep;
  getline(infile,scenePath);	//
  getline(infile,scenePath);	// Get two times because '>>' leaves the pointer before \n 

//  strncpy(aqsisSgi.file_owner,owner.c_str(),BUFFERLEN-1);
  strncpy(aqsisSgi.scene,scenePath.c_str(),BUFFERLEN-1);
  snprintf(aqsisSgi.scriptdir,BUFFERLEN,"%s/tmp/",getenv("DRQUEUE_ROOT"));

  if (!(pathToScript = aqsissg_create(&aqsisSgi))) {
    std::cerr << "Error creating script file\n";
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

  job.koj = KOJ_AQSIS;
  strncpy (job.koji.aqsis.scene,scenePath.c_str(),BUFFERLEN-1);
  strncpy (job.koji.aqsis.viewcmd,"",BUFFERLEN-1);

  job.limits.os_flags = OSF_LINUX;
  job.limits.nmaxcpus = (uint16_t)-1;
  job.limits.nmaxcpuscomputer = (uint16_t)-1;
	job.limits.memory = 0;
	strncpy (job.limits.pool,"Default",MAXNAMELEN-1);

  if (!register_job(&job)) {
    std::cerr << "Error sending job to the queue\n";
    return 1;
  }

  return 0;
}

int RegisterBmrtJobFromFile (std::ifstream &infile)
{
  // Job variables for the script generator
  struct job job;
  struct bmrtsgi bmrtSgi;

  std::string owner;
  std::string jobName;
  int frameStart,frameEnd,frameStep;
  std::string scenePath;
  char *pathToScript;

  getline(infile,owner);
  getline(infile,jobName);
  infile >> frameStart;
  infile >> frameEnd;
  infile >> frameStep;
  getline(infile,scenePath);	//
  getline(infile,scenePath);	// Get two times because '>>' leaves the pointer before \n 

//  strncpy(bmrtSgi.file_owner,owner.c_str(),BUFFERLEN-1);
  strncpy(bmrtSgi.scene,scenePath.c_str(),BUFFERLEN-1);
  snprintf(bmrtSgi.scriptdir,BUFFERLEN,"%s/tmp/",getenv("DRQUEUE_ROOT"));

  if (!(pathToScript = bmrtsg_create(&bmrtSgi))) {
    std::cerr << "Error creating script file\n";
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

  job.koj = KOJ_BMRT;
  strncpy (job.koji.bmrt.scene,scenePath.c_str(),BUFFERLEN-1);
  strncpy (job.koji.bmrt.viewcmd,"",BUFFERLEN-1);

  job.limits.os_flags = OSF_LINUX;
  job.limits.nmaxcpus = (uint16_t)-1;
  job.limits.nmaxcpuscomputer = (uint16_t)-1;
	job.limits.memory = 0;
	strncpy (job.limits.pool,"Default",MAXNAMELEN-1);

  if (!register_job(&job)) {
    std::cerr << "Error sending job to the queue\n";
    return 1;
  }

  return 0;
}

int str2toj (char *str)
{
	int toj = TOJ_NONE;

	if (strstr(str,"maya") != NULL) {
		toj = TOJ_MAYA;
	} else if (strstr(str,"mentalray") != NULL) {
		toj = TOJ_MENTALRAY;
	} else if (strstr(str,"blender") != NULL) {
		toj = TOJ_BLENDER;
	} else if (strstr(str,"bmrt") != NULL) {
		toj = TOJ_BMRT;
	} else if (strstr(str,"3delight") != NULL) {
		toj = TOJ_THREEDELIGHT;
	} else if (strstr(str,"aqsis") != NULL) {
		toj = TOJ_AQSIS;
	} else if (strstr(str,"lightwave") != NULL) {
		toj = TOJ_LIGHTWAVE;
	} else if (strstr(str,"pixie") != NULL) {
		toj = TOJ_PIXIE;
	} else if (strstr(str,"aftereffects") != NULL) {
		toj = TOJ_AFTEREFFECTS;
	} else if (strstr(str,"shake") != NULL) {
		toj = TOJ_SHAKE;
	} else if (strstr(str,"nuke") != NULL) {
		toj = TOJ_NUKE;
	} else if (strstr(str,"terragen") != NULL) {
		toj = TOJ_TERRAGEN;		
	}

	return toj;
}

