%module drqueue
%{
#include "libdrqueue.h"
%}

%include "libdrqueue.h"
%include "computer.h"
%include "computer_info.h"
%include "computer_status.h"

typedef unsigned int time_t;
typedef unsigned short int uint16_t;
typedef unsigned long int uint32_t;

%pythoncode %{
def get_computer_list (who):
	computer_list = _drqueue.new_computerpp()
	ncomputers = _drqueue.request_computer_list (computer_list,who)
	result = []
	for i in range (ncomputers):
		result.append(_drqueue.get_computer_from_list(computer_list,i))
	_drqueue.free_computerpp(computer_list)
	return result
%}

%extend computer_limits {
	struct pool get_pool (int n)
	{
		struct pool pool;
		
		if (n >= self->npools) {
			return pool;
		} else if ( n < 0 ) {
			return pool;
		}

		if (self->npools) {
			if ((self->pool = (struct pool *) computer_pool_attach_shared_memory(self->poolshmid)) == (void*)-1) {
				perror ("Attaching");
				fprintf (stderr,"ERROR attaching memory %d shmid\n", self->poolshmid);
				return pool;
			}
		}
	
		strncpy(pool.name,self->pool[n].name,MAXNAMELEN-1);

		computer_pool_detach_shared_memory (self->pool);

		return pool;
	}
}

%inline %{
struct computer **new_computerpp () {
	struct computer **r = malloc (sizeof (struct computer **));
	return r;
}
void free_computerpp (struct computer **p) {
	free (p);
}
struct computer *get_computer_from_list (struct computer **computer,int n) {
	return computer[n];
}
%}

int request_computer_list (struct computer **computer, int who);
int request_comp_xfer (uint32_t icomp, struct computer *comp, int who);

