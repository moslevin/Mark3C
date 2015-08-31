# Include common prelude make file
include $(ROOT_DIR)base.mak

# If we're building a library, set IS_LIB and LIBNAME
# If we're building a driver, set IS_DRV and DRVNAME
# If we're building an app, set IS_APP and APPNAME
IS_LIB=1
LIBNAME=mark3c

#this is the list of the objects required to build the kernel
C_SOURCE= \
	atomic.c \
	blocking.c \
	driver.c \
    eventflag.c \
	ll.c \
	mailbox.c \
	message.c \
	mutex.c \
	notify.c \
	profile.c \
	quantum.c \
	scheduler.c \
	ksemaphore.c \
	thread.c \
	threadlist.c \
	kernel.c \
	timer.c \
	timerlist.c \
	timerscheduler.c \
	tracebuffer.c \
    writebuf16.c \
    kernelaware.c

# These files are built from source files in their respective 
# subdirectories under ./cpu/arch/variant/toolchain
PORT_C_SOURCE= \
	threadport.c \
	kernelswi.c \
	kerneltimer.c \
	kernelprofile.c


# Include the rest of the script that is actually used for building the 
# outputs
include $(ROOT_DIR)build.mak
