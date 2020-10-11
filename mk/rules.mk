ifeq ($(V),1)
SILENT=
else
SILENT=@
endif

ifeq ($(DEBUG),)
_DBGFLAG=
else
_DBGFLAG=-g
endif


ifeq ($(CC),)
CC=gcc
endif

UPFIND = $(shell while test $(1) $(2) && echo $$PWD/$(2) && exit 0; test $$PWD != /; do cd ..; done;)

CCOMP1	= $(CROSS_COMPILE)$(CC) $(CFLAGS) $(_DBGFLAG) -Wall -Werror -c $(1) -o $(2) && $(CC) -MM  $(CFLAGS) $(1) | sed -e 's!.*\.o:!$(2):!' > $(2:%.o=%.d)
LINK1	= $(CROSS_COMPILE)$(CC) $(_DBGFLAG) -o $(1) $(2) $(LDFLAGS) 
CCOMP	= echo "CC  $(2)"; $(CCOMP1)
LINK	= echo "LD  $(1)"; $(LINK1)

%.o:	%.c
	$(SILENT)$(call CCOMP$(V),$<,$@)

$(TARGETS): 
	$(SILENT)$(call LINK$(V),$@,$^)
