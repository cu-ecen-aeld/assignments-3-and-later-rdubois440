#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

# Working command
#make ARCH=arm64 CROSS_COMPILE=/opt/arm/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-

set -e
set -u

#PATH=$PATH:/opt/arm/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin
#PATH=$PATH:/opt/arm/arm-gnu-toolchain-14.2.rel1-x86_64-aarch64-none-linux-gnu/bin
#OUTDIR=/tmp/aeld
OUTDIR=/home/build/coursera/outdir/
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
#CROSS_COMPILE=aarch64-none-linux-gnu-
CROSS_COMPILE=/opt/arm/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
#CROSS_COMPILE=aarch64-none-linux-gnu-
#CROSS_COMPILE=/opt/arm/aarch64--musl--stable-2018.11-1/bin/aarch64-linux-

DEV_DIR=`pwd`


if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}

else
	echo "${OUTDIR}/linux-stable already exist" 
fi



if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
	echo Current dir is `pwd`
	echo $CROSS_COMPILE
	${CROSS_COMPILE}gcc -v
	make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE defconfig
	#make ARM=$ARCH CROSS_COMPILE=$CROSS_COMPILE menuconfig
	echo "Using command make ARM=$ARCH CROSS_COMPILE=$CROSS_COMPILE all"
	make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE all
	make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE modules
	make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE dtbs

fi

echo "Adding the Image in outdir"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories

	mkdir ${OUTDIR}/rootfs
	mkdir ${OUTDIR}/rootfs/home
	mkdir ${OUTDIR}/rootfs/bin
	mkdir ${OUTDIR}/rootfs/sbin
	mkdir ${OUTDIR}/rootfs/dev
	mkdir ${OUTDIR}/rootfs/etc
	mkdir ${OUTDIR}/rootfs/lib
	mkdir ${OUTDIR}/rootfs/lib64
	mkdir ${OUTDIR}/rootfs/proc
	mkdir ${OUTDIR}/rootfs/sys
	mkdir ${OUTDIR}/rootfs/tmp
	mkdir ${OUTDIR}/rootfs/var
	mkdir ${OUTDIR}/rootfs/var/log
	mkdir ${OUTDIR}/rootfs/usr
	mkdir ${OUTDIR}/rootfs/usr/bin
	mkdir ${OUTDIR}/rootfs/usr/sbin
	mkdir ${OUTDIR}/rootfs/usr/lib



cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}

	exit
    # TODO:  Configure busybox
else
    cd busybox
fi

# TODO: Make and install busybox

	make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE distclean
	make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE defconfig
	make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE 
	make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE install


cd "$OUTDIR"/rootfs



echo "Library dependencies"
pwd
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs

sysroot=`${CROSS_COMPILE}gcc --print-sysroot`
echo "sysroot is ${sysroot}"
cp ${sysroot}/lib/ld-linux-aarch64.so.1 ${OUTDIR}/rootfs/lib
cp ${sysroot}/lib/ld-linux-aarch64.so.1 ${OUTDIR}/rootfs/lib64


cp ${sysroot}/lib64/libm.so.6 ${OUTDIR}/rootfs/lib64
cp ${sysroot}/lib64/libresolv.so.2 ${OUTDIR}/rootfs/lib64
cp ${sysroot}/lib64/libc.so.6   ${OUTDIR}/rootfs/lib64


# TODO: Make device nodes
	sudo mknod -m 666 ${OUTDIR}/rootfs/dev/null c 1 3
	sudo mknod -m 666 ${OUTDIR}/rootfs/dev/console c 5 1


# TODO: Clean and build the writer utility

echo "Dev Dir is ${DEV_DIR}"
cd $DEV_DIR

make clean
make CROSS_COMPILE=$CROSS_COMPILE 



# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs

cp writer "${OUTDIR}/rootfs/home"
cp finder.sh "${OUTDIR}/rootfs/home"
cp finder-test.sh "${OUTDIR}/rootfs/home"
cp autorun-qemu.sh "${OUTDIR}/rootfs/home"
mkdir "${OUTDIR}/rootfs/home/conf"
cp conf/* "${OUTDIR}/rootfs/home/conf/"

cd "$OUTDIR"/rootfs


# TODO: Chown the root directory
	sudo chown root:root "${OUTDIR}/rootfs"

# TODO: Create initramfs.cpio.gz

	cd "${OUTDIR}/rootfs"
	find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
	gzip  -f ${OUTDIR}/initramfs.cpio

	cp ${OUTDIR}/linux-stable/arch/arm64/boot/Image ${OUTDIR}


