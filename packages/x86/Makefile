# Default make for atom
#
# Build ffritz package, which builds other packages on demand
#
.PHONY:	base

ARCHDIR := $(shell while test -f arch.mk && echo $$PWD && exit 0; test $$PWD != /; do cd ..; done)
include $(ARCHDIR)/arch.mk

all:
	@make -C ffritz

base:
	@make -C base

sdk:
	@make -C $(BRBUILDDIR) sdk-build

base-install:
	@make -C base base-install

clean:
	make -C ffritz clean

cleanall:
	make -C ffritz cleanall
