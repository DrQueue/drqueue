/*
      Service Installer for NSIS script
      
      */

#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <malloc.h>

int main(int argc, char *argv[])
{
   if(argc<6)
   {
      printf("Insufficient arguments: Service u/i ServiceName ServicePath DisplayName m/a [DependOnServiceName]\n");
      return 1;
   }
   
        SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
        SC_HANDLE hService;

        if(hSCM == NULL)
        {
                printf("Could not open Service Control Manager. Aborting.\n");
                return 1;
        }
  char *depend = NULL;
  if (argc >= 7)
  {
	int slen = strlen(argv[6]);
	int len = slen + 10;
	depend = (char *) malloc(len);
	memset(depend, 0, len); 
	strcpy(depend, argv[6]);
	printf("Service depend on %s\n", depend); 
   }
   if(*argv[1]!='u' && *argv[1]!='U')
   {
		if (argv[5][0] == 'm')
		{
              		printf("installing on demand start service\n"); 
		 	hService = CreateService(hSCM, argv[2],
	                _T(argv[4]),
                	SERVICE_ALL_ACCESS,
                	SERVICE_WIN32_OWN_PROCESS,
                	SERVICE_DEMAND_START,
                	SERVICE_ERROR_IGNORE,
                	argv[3],
                	NULL,NULL, depend,
			NULL,
			NULL );
		}
		else
		{
              		printf("installing autostart service\n"); 
                	hService = CreateService(hSCM, argv[2],
	                _T(argv[4]),
                	SERVICE_ALL_ACCESS,
                	SERVICE_WIN32_OWN_PROCESS,
                	SERVICE_AUTO_START,
                	SERVICE_ERROR_IGNORE,
                	argv[3],
                	NULL,NULL, depend,
			NULL,
			NULL );
		}
                if (hService == NULL) 
                {
                    printf("Create Service failed (%d)\n", GetLastError() );
                    CloseServiceHandle(hSCM);
                }
   }
   else
   {
       hService = OpenService( hSCM, argv[2], DELETE);
       if(hService!=NULL)
           DeleteService( hService );
   }
   
        CloseServiceHandle(hService);


        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);

        return 0;
}
