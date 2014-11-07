#
# The contents of this file are subject to the Mozilla Public License
# Version 1.0 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.
#
# The Original Code is Mobile Application Link.
#
# The Initial Developer of the Original Code is AvantGo, Inc.
# Portions created by AvantGo, Inc. are Copyright (C) 1997-1999
# AvantGo, Inc. All Rights Reserved.
#
# Contributor(s):
#

# PENDING(linus) add debug vs. release stuff
# This makefile requires gmake
BINDIR=$(AGINSTALLDIR)/bin
CC = gcc
LD = ld
CP = /bin/cp

ifeq ($(AGPLAT),Linux)
PLAT_LDFLAGS = -Bshareable -warn-common
endif

ifeq ($(AGPLAT),SunOS)
PLAT_LDFLAGS = -B dynamic -G
endif

ifeq ($(AGPLAT),FreeBSD)
PLAT_LDFLAGS = -Bshareable
endif

CFLAGS = $(DEBUG_CFLAGS) $(OPT_CFLAGS) $(EXTRA_CFLAGS) $(PLAT_CFLAGS) $(IFLAGS)
LDFLAGS = $(DEBUG_LDFLAGS) $(OPT_LDFLAGS) $(PLAT_LDFLAGS) $(EXTRA_LDFLAGS)
LIBS = $(EXTRA_LIBS) $(DEBUG_LIBS) $(OPT_LIBS) $(PLAT_LIBS) -lc

ifeq ($(AGPLAT),SunOS)
LIBS +=-lsocket -lnsl -ldl
endif

# This is really fast, so just do it every time.
trash := $(shell mkdir -p $(OBJDIR))
trash := $(shell perl $(TREETOP)/mal/make/dsptomake.pl $(DSPFILE) $(OBJDIR)/build.mk $(OBJDIR)/ofiles.mk)

# Bring in the list of ofiles generated from the ms dsp file.
include $(OBJDIR)/ofiles.mk

$(OBJDIR)/$(LIBNAME).so: $(OFILES)
	@echo ""
	$(LD)  -o $@ $(LDFLAGS) $(OFILES) $(LIBS)
	@if [ -d "${BINDIR}" ]; then  \
		echo ""; \
		echo $(CP) $@ ${BINDIR}; \
		$(CP) $@ ${BINDIR}; \
	fi

clean:
	/bin/rm -rf $(OBJDIR)

# Bring in the list of build rules for each source file.
include $(OBJDIR)/build.mk
