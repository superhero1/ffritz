BR_VERSION_OVERRIDE = -2020.08

ARCHDIR := $(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/arch.mk
include $(REPODIR)/mk/buildroot.mk

