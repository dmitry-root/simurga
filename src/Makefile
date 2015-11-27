SOURCES := $(wildcard *.cpp)
OUTPUT := simurga
TYPE := exe
LIBS := $(TMPROOT)/fastcgipp/libfastcgipp.a
LIBADD += -lmysqlclient_r -lboost_system -lboost_thread -lpthread -lm -ldl
EXE_LDFLAGS := $(LDFLAGS)

include $(MAKE_TARGET)
