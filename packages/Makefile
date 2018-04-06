
all:
	@mkdir -p dl
	@make -C x86
	@make -C arm

base:
	@make -C x86 base
	@make -C arm base
	
base-install:
	@make -C x86 base-install
	@make -C arm base-install
