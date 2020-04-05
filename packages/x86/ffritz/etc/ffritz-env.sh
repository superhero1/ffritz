# FFINSTDIR is either in  /var/media/ftp/ (if atom fs is unmodified) or
# /usr/local (if package is intergrated into atom squashfs)
#
export FFINSTDIR=/usr/local
PATH=$PATH:/sbin:$FFINSTDIR/bin
NVRAM=`readlink -f /tmp/ffnvram`
FFLOG=/var/log/ffritz.log
BR_DIR=/tmp/br

export FFRITZ_HOME=/var/media/ftp/ffritz
export FFRITZ_DATA=/var/media/ftp/ffritz
