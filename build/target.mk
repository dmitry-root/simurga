# Makefile that generates needed rules for specified target
# Variables to be set:
#   OUTPUT        -- build output to generate
#   OUTDIR        -- directory where to install output, w/o trailing slash
#   C_SOURCES     -- list of C source files to build
#   CXX_SOURCES   -- list of C++ source files to build
#   SOURCES       -- list of source files to build written in a current language
#   USE_C         -- set to "yes" in order to build target using C compiler instead of C++
#   TYPE          -- set to one of exe, lib or dll. The default is exe


.PHONY: all install

# == Calculate the relative directory where the project resides in src ==
TARGETPATH := $(abspath $(CURDIR))
TARGETPATH := $(subst $(SRCROOT)/,, $(TARGETPATH))
TARGETPATH := $(strip $(TARGETPATH))
TMPDIR ?= $(TMPROOT)/$(TARGETPATH)

ifeq (x$(OUTDIR), x)
ifeq ($(TYPE), exe)
    INSTDIR := $(DESTDIR)$(PREFIX)/bin
else
    INSTDIR := $(DESTDIR)$(PREFIX)/lib
endif
else
    INSTDIR := $(DESTDIR)$(PREFIX)/$(OUTDIR)
endif

# == Calculate desired compiler ==
ifeq ($(USE_C), yes)
    TLINK = $(CC)
else
    TLINK = $(CXX)
endif
ifeq ($(TYPE), dll)
    LDFLAGS += -shared
endif

# == Prepare strip command ==
ifeq ($(DEBUG), yes)
    TSTRIP =
else
    TSTRIP = $(STRIP) $@
endif

# == Calculate list of object files ==
C_SOURCES := $(filter %.c,$(SOURCES))
CXX_SOURCES := $(filter %.cpp,$(SOURCES))
C_DEPFILES := $(C_SOURCES:%.c=%.d)
CXX_DEPFILES := $(CXX_SOURCES:%.cpp=%.d)
C_OBJECTS := $(C_SOURCES:%.c=%.o)
CXX_OBJECTS := $(CXX_SOURCES:%.cpp=%.o)
OBJECTS := $(addprefix $(TMPDIR)/,$(C_OBJECTS)) $(addprefix $(TMPDIR)/,$(CXX_OBJECTS))

# == Calculate full path to output file ==
OUTPUT_FN := $(TMPDIR)/$(OUTPUT)

# == Common targets ==
all: $(TMPDIR) $(OUTPUT_FN)

ifneq ($(TYPE), lib)
install: $(OUTPUT_FN)
	install -d $(INSTDIR)
	install $(OUTPUT_FN) $(INSTDIR)/$(OUTPUT)
else
install:
endif

$(OUTPUT_FN): $(OBJECTS) $(LIBS) $(DEPS)
ifeq ($(TYPE), lib)
	$(AR) cru $@ $^
else
	$(TLINK) $(LDFLAGS) $(LIBDIRS) -o $@ $(OBJECTS) $(LIBS) $(LIBADD)
	$(TSTRIP)
endif

$(TMPDIR)/%.d: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDES) $(DEFINES) $(DEPOPTS) -MM -MP -MT $(<:%.c=$(TMPDIR)/%.o) -MF $@ $<
	echo "$(<:%.c=$(TMPDIR)/%.o): Makefile $(MAKE_TARGET) $(BUILDROOT)/Makefile" >> $@

$(TMPDIR)/%.d: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) $(DEFINES) $(DEPOPTS) -MM -MP -MT $(<:%.cpp=$(TMPDIR)/%.o) -MF $@ $<
	echo "$(<:%.cpp=$(TMPDIR)/%.o): Makefile $(MAKE_TARGET) $(BUILDROOT)/Makefile" >> $@

$(TMPDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(INCLUDES) $(DEFINES) -o $@ $<

$(TMPDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) $(DEFINES) -o $@ $<

-include $(addprefix $(TMPDIR)/,$(C_DEPFILES) $(CXX_DEPFILES))

