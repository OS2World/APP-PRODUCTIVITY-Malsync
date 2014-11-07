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

# All makefiles must define TREETOP before including global.mk.  This
# is an absolute path to the top of the tree.

FULLTOP := $(shell (cd $(TREETOP); pwd))

ifndef AGINSTALLDIR
AGINSTALLDIR = $(HOME)/mal
endif

# Get some standard strings for the platform and machine we are
# building on.  We use this for the dir where the ofiles go.

ifndef AGPLAT
AGPLAT := $(shell uname)
endif

ifndef AGMACH
AGMACH := $(shell uname -m)
endif

OBJDIR = $(AGPLAT)-$(AGMACH)

# It simplifies the rest of the makefiles to use a _DEBUG env var instead
# of having multiple targets for everything.

ifdef _DEBUG
DEBUG_CFLAGS = -g -D_DEBUG
OPT_CFLAGS =
else
DEBUG_CFLAGS =
OPT_CFLAGS = -g -O2
endif # _DEBUG

