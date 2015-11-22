SOURCES := $(wildcard *.cpp)
OUTPUT := simurga
TYPE := exe
LIBADD += $(TMPROOT)/fastcgipp/libfastcgipp.a -lmysqlclient_r -lboost_system -lboost_thread -lpthread -lm -ldl
EXE_LDFLAGS := $(LDFLAGS)

include $(MAKE_TARGET)
