# this makefile is to be used with gmake

# Global variables
include $(XSISDK_ROOT)/mkfiles/vars.gnu

# Sources to be compiled
SOURCES := \
   Splash.cpp \
   $(END_OF_LIST)

# Target
# Note: Depending where the source code
# is located you may want to adjust this line to 
# output directly into your /Application/Plugins directory
# example SOTARGETS  := ../Application/Plugins/Splash.so
SOTARGETS  := ./Splash.so

# Global rules
include $(XSISDK_ROOT)/mkfiles/rules.gnu

# Specific dependencies
$(SOTARGETS): $(ALLOBJECTS) $(XSISDKLIBS)
$(CXXOBJECTS): $(OBJDIR)/%.o: %.cpp
$(COBJECTS): $(OBJDIR)/%.o: %.c

