# Determine arch
CONFIG_WIN32 = $(shell uname -s|grep -i cygwin>/dev/null&&echo y)
CONFIG_LINUX = $(shell uname -s|grep -i linux>/dev/null&&echo y)

# Config for Linux
CC-$(CONFIG_LINUX):=$(CC)
CFLAGS-$(CONFIG_LINUX)=`pkg-config gtk+-2.0 --cflags`
LDFLAGS-$(CONFIG_LINUX)=`pkg-config gtk+-2.0 --libs`

# Config for Win32
GTK_WIN32_PATH-$(CONFIG_WIN32) = $(shell cygpath -m /opt/gtk-dev-win32)

MINGWDIR := /opt/mingw/bin
CC-$(CONFIG_WIN32) := $(MINGWDIR)/gcc

GTK_WIN32_PATH := $(GTK_WIN32_PATH-y)
GTK_WIN32_SUBPATH = include/atk-1.0 include/cairo include/glib-2.0 include/gtk-2.0 include/pango-1.0 lib/gtk-2.0/include lib/glib-2.0/include include
CFLAGS-$(CONFIG_WIN32)=$(foreach i,$(GTK_WIN32_SUBPATH),-I$(GTK_WIN32_PATH)/$i) -mms-bitfields
LDFLAGS-$(CONFIG_WIN32)=-L$(GTK_WIN32_PATH)/lib -lgtk-win32-2.0 -lglib-2.0 -lgdk-win32-2.0 -lgdk_pixbuf-2.0 -lgmodule-2.0 -lgobject-2.0 -lpangowin32-1.0 -lpangoft2-1.0 -lpango-1.0 -lgthread-2.0 -lintl -mwindows


#####
CFLAGS = $(CFLAGS-y) -DFOR_MY_OWN_USE -DTELEKEYPAD_IS_MODAL
LDFLAGS = $(LDFLAGS-y)
CC := $(CC-y)

SRC=$(wildcard *.h)
EXEC=$(subst .h,,$(SRC))


all: $(EXEC)

$(EXEC): $(EXEC).o main.o
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -f *.o $(EXEC) $(EXEC).exe
