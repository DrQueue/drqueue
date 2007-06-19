AM_CPPFLAGS = -I$(top_srcdir)/libdrqueue -D_GNU_SOURCE -D_NO_COMPUTER_POOL_SEMAPHORES -D_NO_COMPUTER_SEMAPHORES @my_sys@
AM_CFLAGS = @GTK2_CFLAGS@
AM_CXXFLAGS = @GTK2_CFLAGS@
bin_PROGRAMS = drqman
drqman_SOURCES = drqm_autorefresh.c drqm_common.c drqm_computers.c \
    drqm_globals.c drqm_info.c drqm_jobs_3delight.c \
	drqm_jobs_aftereffects.c drqm_jobs_aqsis.c drqm_jobs_blender.c \
	drqm_jobs_bmrt.c drqm_jobs.c drqm_jobs_jdd.c drqm_jobs_lightwave.c \
	drqm_jobs_mantra.c drqm_jobs_maya.c drqm_jobs_mentalray.c \
	drqm_jobs_nuke.c drqm_jobs_pixie.c drqm_jobs_shake.c \
	drqm_jobs_terragen.c drqm_jobs_turtle.c drqm_jobs_xsi.c \
	drqm_request.c main.c notebook.c drqm_autorefresh.h drqm_common.h \
	drqm_computers.h drqm_cygwin.h drqm_globals.h drqm_info.h \
	drqm_jobs.h drqm_jobs_3delight.h drqm_jobs_aftereffects.h \
	drqm_jobs_aqsis.h drqm_jobs_blender.h drqm_jobs_bmrt.h \
	drqm_jobs_common.h drqm_jobs_jdd.h drqm_jobs_lightwave.h \
	drqm_jobs_mantra.h drqm_jobs_maya.h drqm_jobs_mentalray.h \
	drqm_jobs_nuke.h drqm_jobs_pixie.h drqm_jobs_shake.h \
	drqm_jobs_terragen.h drqm_jobs_turtle.h drqm_jobs_xsi.h \
	drqm_request.h drqman.h info_icon.h job_icon.h notebook.h slave_icon.h
drqman_LDFLAGS = @GTK2_LIBS@
drqman_LDADD = $(top_srcdir)/libdrqueue/libdrqueue.la
