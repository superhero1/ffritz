# FFINSTDIR is either in  /var/media/ftp/ (if atom fs is unmodified) or
# /usr/local (if package is intergrated into atom squashfs)
#
export FFINSTDIR=/usr/local
export FFRITZ_HOME=/var/media/ftp/ffritz
PATH=$PATH:$FFINSTDIR/bin
NVRAM=/tmp/ffnvram
FFLOG=/var/log/ffritz.log
BR_DIR=/tmp/br
