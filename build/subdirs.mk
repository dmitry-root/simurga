# Generate needed targets by propagating compilation of $(SUBDIRS) directories

.PHONY: all install $(SUBDIRS)

all: TARGET = all
all: $(SUBDIRS)

install: TARGET = install
install: $(SUBDIRS)

$(SUBDIRS): %:
	@$(MAKE) -C $@ $(TARGET)

