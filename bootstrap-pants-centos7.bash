yum -y update

yum -y install \
    libcurl-devel expat-devel fuse fuse-devel

export VCFS_INSTALL_PREFIX='/h/workspace/vcfs-pants-base/tmp'

PATH="/h/workspace/source-git:${PATH}"

export LD_LIBRARY_PATH="${VCFS_INSTALL_PREFIX}/lib"
