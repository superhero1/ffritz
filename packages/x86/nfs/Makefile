## Template for integrating "3rd prty" tool
##
## Tool specific options for configuring, building and installing 
## are listed in ../../package.mk
##
## By default,
## - tarball/git repository is downloaded (../../external/url-mypackage or ../../external/git-mypackage must exist)
## - extracted/cloned below "build",
## - patched with all .patch files located here
## - configured according to CONFTYPE
## - built below build
## - installed (make install) according to INSTALL_ rules
##
## make clean deletes the build directory
##
## To integrate the build result into the application package, add
## this to ../ffritz/Makefile:
##
## INSTDIRS   += ../mypackage/install
##
## Missing libraries etc. should first be searched for in buildroot and
## added via "make userconfig" there.
##
############################################################################

# Find directory containing package.mk upwards
#
PKGDIR=$(shell while test -f package.mk && echo $$PWD && exit 0; [ $$PWD != / ]; do cd ..; done)

CONFTYPE            = autoreconf
PKG_CONFIGURE_FLAGS = --prefix=/ --exec-prefix=/
INSTALL_BIN         = 

include $(PKGDIR)/package.mk
