CC = gcc
TARGET = malsync

PLAT := $(shell uname)
	
ifeq ($(PLAT),SunOS)
LINKLIBS = -ldl -lnsl -lsocket
endif

ifeq ($(PLAT),Linux)
LINKLIBS = -ldl -lm
endif

ifeq ($(PLAT),Darwin)
LINKLIBS = -lm
endif

TREETOP = ../../..
PILOT_LINK_DIR = $(TREETOP)/vendor/pilot/pilot-link.0.9.5

CFLAGS = -Wall -g -DMALSYNC -I$(TREETOP)/mal/common \
  -I$(TREETOP)/mal/client/common -I$(PILOT_LINK_DIR)/include -I.

ifeq ($(PLAT),Darwin)
CFLAGS += --traditional-cpp -DDARWIN -D__unix__ -D__FreeBSD__ -D_HPUX_SOURCE
endif

SRC = malsync.c  MAL31ServerConfig.c MAL31DBConfig.c MAL31UserConfig.c \
  $(TREETOP)/mal/common/AGNet.c \
  $(TREETOP)/mal/common/AGBase64.c \
  $(TREETOP)/mal/common/AGProxy.c \
  $(TREETOP)/mal/common/AGArray.c \
  $(TREETOP)/mal/common/AGDBConfig.c \
  $(TREETOP)/mal/common/AGProtectedMem.c \
  $(TREETOP)/mal/common/AGLocationConfig.c \
  $(TREETOP)/mal/common/AGBufferedNet.c \
  $(TREETOP)/mal/common/AGUtilUnix.c \
  $(TREETOP)/mal/common/AGDeviceInfo.c \
  $(TREETOP)/mal/common/AGProtocol.c \
  $(TREETOP)/mal/common/AGPalmProtocol.c \
  $(TREETOP)/mal/common/AGWriter.c \
  $(TREETOP)/mal/common/AGBufferReader.c \
  $(TREETOP)/mal/common/AGHashTable.c \
  $(TREETOP)/mal/common/AGReader.c \
  $(TREETOP)/mal/common/AGBufferWriter.c \
  $(TREETOP)/mal/common/AGMsg.c \
  $(TREETOP)/mal/common/AGRecord.c \
  $(TREETOP)/mal/common/AGCollection.c \
  $(TREETOP)/mal/common/AGDigest.c \
  $(TREETOP)/mal/common/AGMD5.c \
  $(TREETOP)/mal/common/md5.c \
  $(TREETOP)/mal/common/AGSynchronize.c \
  $(TREETOP)/mal/common/AGUserConfig.c \
  $(TREETOP)/mal/client/common/AGClientProcessor.c \
  $(TREETOP)/mal/client/common/AGCommandProcessor.c \
  $(TREETOP)/mal/client/common/AGSyncProcessor.c \
  $(TREETOP)/mal/common/AGServerConfig.c

ifeq ($(PLAT),SunOS)
SRC += ap_snprintf.c
endif

OBJS = $(SRC:.c=.o)

ifeq ($(PLAT),Darwin)
DYNLINK =
else
DYNLINK = -rdynamic
endif

$(TARGET): $(OBJS)
	$(CC) $(DYNLINK) $(CFLAGS) -o $@ $(OBJS) \
          -L${PILOT_LINK_DIR}/libsock -lpisock $(LINKLIBS)
clean:
	rm -f $(OBJS) $(TARGET)

.c.o:
	gcc $(CFLAGS) -c -o $@ $<





