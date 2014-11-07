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

CC = gcc

CFLAGS =$(DEBUG_CFLAGS) $(OPT_CFLAGS) $(EXTRA_CFLAGS) $(PLAT_CFLAGS) $(IFLAGS)

# This is really fast, so just do it every time.
trash := $(shell mkdir -p $(OBJDIR))
trash := $(shell perl $(TREETOP)/mal/make/dsptomake.pl $(DSPFILE) $(OBJDIR)/build.mk $(OBJDIR)/ofiles.mk)

# Bring in the list of ofiles generated from the ms dsp file.
include $(OBJDIR)/ofiles.mk

$(OBJDIR)/$(LIBNAME).a: $(OFILES)
	@echo ""
	rm -f $(OBJDIR)/$(LIBNAME).a
	ar cr $(OBJDIR)/$(LIBNAME).a $(OFILES)
	ranlib $(OBJDIR)/$(LIBNAME).a

clean:
	/bin/rm -rf $(OBJDIR)

# Bring in the list of build rules for each source file.
include $(OBJDIR)/build.mk

