BR_VERSION_OVERRIDE = -2019.05-p6

ARCHDIR := $(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/arch.mk
include $(REPODIR)/mk/buildroot.mk
