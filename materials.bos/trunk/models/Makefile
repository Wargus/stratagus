SUBDIRS = camera radar

.PHONY: clean subdirs $(SUBDIRS)

subdirs: $(SUBDIRS) Makefile_models

$(SUBDIRS):
	cp Makefile_models $@/Makefile
	$(MAKE) -C $@
clean:
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir clean; done
