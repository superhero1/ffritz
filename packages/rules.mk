ifeq ($(V),1)
SILENT=
else
SILENT=@
endif

ifeq ($(CC),)
CC=gcc
endif

CCOMP1	= $(CROSS_COMPILE)$(CC) $(CFLAGS) -Wall -Werror -c $(1) -o $(2) && $(CC) -MM  $(CFLAGS) $(1) | sed -e 's!.*\.o:!$(2):!' > $(2:%.o=%.d)
LINK1	= $(CROSS_COMPILE)$(CC) -o $(1) $(2) $(LDFLAGS) 
CCOMP	= echo "CC  $(2)"; $(CCOMP1)
LINK	= echo "LD  $(1)"; $(LINK1)

%.o:	%.c
	$(SILENT)$(call CCOMP$(V),$<,$@)

$(TARGETS): 
	$(SILENT)$(call LINK$(V),$@,$^)
