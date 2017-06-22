#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/mount.h>
#include <errno.h>
#include <stdlib.h>
#include <selinux/selinux.h>

#include "lvmcreate.h"

#define LOG_TAG "LVM"
#include "cutils/log.h"


int
create_lvm(unsigned int encrypt_dev_no)
{
   int ret, status;
   char dev_path[128];               /* Used for storing the encrypted dev file */
    pid_t pid;

    int error[2];
    int out[2];

    char buf[4096];
	
	SLOGI("Current userid: %d\n",getuid());
    SLOGI("enforcement value %d\n",security_getenforce());


    if (pipe(error) == -1) {
        SLOGE("error creating pipe error\n");
    }

    if (pipe(out) == -1) {
        SLOGE("error creating pipe out\n");
    }

    if (mount("/","/",NULL, MS_REMOUNT, NULL) == -1) {
        SLOGE("remount / error: %s", strerror(errno));
    }

    if (mount("/system","/system",NULL, MS_REMOUNT, NULL) == -1) {
        SLOGE("remount /system error: %s", strerror(errno));
    }

    /* Creating pv */
    if ((pid = fork()) < 0) {
        SLOGE("Error while forking!\n");
        return -1;
    } else if (pid == 0) {      /* child process */

        memset(dev_path,0,sizeof(dev_path));
        snprintf(dev_path, 128, "/dev/block/dm-%u", encrypt_dev_no);
        SLOGI("Device path: %s", dev_path);

        if (execl("/system/bin/lvm", "lvm", "pvcreate","-y", dev_path,(char*)0) < 0) {
            SLOGE("Error while executing lvm pvcreate!\n");
            return -1;
        }
    } else {

        if (waitpid(pid, &status, 0) < 0 ) {
            SLOGE("Error waiting child process!, Error while executing lvm pvcreate!\n");
            return -1;
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
            SLOGI("pvreate / return code %d\n", ret);
        }
    }

    /* Creating vg */
    if ((pid = fork()) < 0) {
        SLOGE("Error while forking!\n");
        return -1;
    } else if (pid == 0) {      /* child process */
        close(error[0]);
        dup2(error[1],STDERR_FILENO);

        close(out[0]);
        dup2(out[1], STDOUT_FILENO);

        memset(dev_path,0,sizeof(dev_path));
        snprintf(dev_path, 128, "/dev/block/dm-%u", encrypt_dev_no);
        SLOGI("Device path: %s", dev_path);

        if (execl("/system/bin/lvm", "lvm", "vgcreate", "-y", "vg", dev_path, (char*)0) < 0) {
            SLOGE("Error while executing lvm vgcreate!\n");
            return -1;
        }
    } else {
        int n;
        close(error[1]);
        close(out[1]);
        memset(buf, 0, sizeof(buf));
        n = read(error[0], buf, 4096);
        SLOGI("childprocess errMSG:\n");
        SLOGI("%s\n",buf);

        memset(buf, 0, sizeof(buf));
        n = read(out[0], buf, 4096);
        SLOGI("childprocess outMSG:\n");
        SLOGI("%s\n",buf);

        if (waitpid(pid, &status, 0) < 0 ) {
            SLOGE("Error waiting child process!, Error while executing lvm vgcreate!\n");
            return -1;
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
            SLOGI("vgcreate / return code %d\n", ret);
        }
    }

    /* Creating lv metadata */
    if ((pid = fork()) < 0) {
        SLOGE("Error while forking!\n");
        return -1;
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/lvm", "lvm", "lvcreate","-y",  "--name",  
                    "metadata","--size", "2M", "vg", (char*)0) < 0) {
            SLOGE("Error while executing lvm lvcreate!\n");
            return -1;
        }
    } else {
        if (waitpid(pid, &status, 0) < 0 ) {
            SLOGE("Error waiting child process!,Error while executing lvm lvcreate!\n");
            return -1;
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
            SLOGI("lvcreate / return code %d\n", ret);
        }
    }


    /* Creating lv data */
    if ((pid = fork()) < 0) {
        SLOGE("Error while forking!\n");
        return -1;
    } else if (pid == 0) {      /* child process */

        if (execl("/system/bin/lvm", "lvm", "lvcreate","-y", "--name",  
                    "data","--size", "25G", "vg", (char*)0) < 0) {
            SLOGE("Error while executing lvm lvcreate data!\n");
            return -1;
        }
    } else {

        if (waitpid(pid, &status, 0) < 0 ) {
            SLOGE("Error waiting child process!, Error while executing lvm lvcreate data!\n");
            return -1;
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        }
    }

    /* Convert to thin volumes */
    if ((pid = fork()) < 0) {
        SLOGE("Error while forking!\n");
        return -1;
    } else if (pid == 0) {      /* child process */
        
        if (execl("/system/bin/lvm", "lvm", "lvconvert", "-y", "--thinpool",  
                    "vg/data","--poolmetadata", "vg/metadata", (char*)0) < 0) {
            SLOGE("Error while executing lvm lvconvert!\n");
            return -1;
        }
    } else {
        
        if (waitpid(pid, &status, 0) < 0 ) {
            SLOGE("Error waiting child process!, Error while executing lvm lvconvert!\n");
            return -1;
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
            SLOGI("lvconvert / return code %d\n", ret);
        }
    }
    
    /* Creating virtual volume outter */
    if ((pid = fork()) < 0) {
        SLOGE("Error while forking!\n");
        return -1;
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/lvm", "lvm", "lvcreate","-y", "--name",  
                    "outter","--virtualsize", "25G", "--thinpool",
                    "vg/data", (char*)0) < 0) {
            SLOGE("Error while executing lvm lvcreate virtual volume!\n");
            return -1;
        }
    } else {
        if (waitpid(pid, &status, 0) < 0 ) {
            SLOGE("Error waiting child process!, Error while executing lvm lvcreate virtual volume!\n");
            return -1;
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
            SLOGI("virtual lvcreate / return code %d\n", ret);
        }
    }

    /* Creating virtual volume hidden */
    if ((pid = fork()) < 0) {
        SLOGE("Error while forking!\n");
        return -1;
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/lvm", "lvm", "lvcreate","-y", "--name",  
                    "hidden","--virtualsize", "25G", "--thinpool",
                    "vg/data", (char*)0) < 0) {
            SLOGE("Error while executing lvm lvcreate virtual volume!\n");
            return -1;
        }
    } else {
        if (waitpid(pid, &status, 0) < 0 ) {
            SLOGE("Error waiting child process!, Error while executing lvm lvcreate virtual volume!\n");
            return -1;
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        }
    }

    /* Format the volume with make_ext4fs */
    if ((pid = fork()) < 0) {
        SLOGE("Error while forking!\n");
        return -1;
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/make_ext4fs", "make_ext4fs", "-f",
                    "/dev/mapper/vg-outter", (char*)0) < 0) {
            SLOGE("Error while formatting outter volume!\n");
            return -1;
        }
    } else {
        if (waitpid(pid, &status, 0) < 0 ) {
            SLOGE("Error waiting child process!, Error while formatting outter volume!\n");
            return -1;
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
            SLOGI("format outter return code %d\n", ret);
        }
    }


     /* Format the hidden volume with make_ext4fs */
    if ((pid = fork()) < 0) {
        SLOGE("Error while forking!\n");
        return -1;
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/make_ext4fs", "make_ext4fs", "-f",
                    "/dev/mapper/vg-hidden", (char*)0) < 0) {
            SLOGE("Error while formatting hidden volume!\n");
            return -1;
        }
    } else {
        if (waitpid(pid, &status, 0) < 0 ) {
            SLOGE("Error waiting child process! Error while formatting hidden volume!\n");
            return -1;
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        }
    }

    /* Remount / and /system as read-only */
    if (mount("/","/",NULL, MS_REMOUNT | MS_RDONLY, NULL)== -1) {
        SLOGE("remount / as rdonly error: %s\n", strerror(errno));
    }

    if (mount("/system","/system",NULL, MS_REMOUNT | MS_RDONLY, NULL)== -1) {
        SLOGE("remount /system as rdonly error: %s\n", strerror(errno));
    }

    SLOGI("done!\n");

    return ret;
}

int 
open_lvm(void)
{
	int ret, status;    
    pid_t pid;

	SLOGI("Start open!\n");

    /* Remounting / and /system as rw permission */
    if (mount("/","/",NULL, MS_REMOUNT, NULL) == -1) {
        SLOGE("remount / error: %s", strerror(errno));
    }

    if (mount("/system","/system",NULL, MS_REMOUNT, NULL) == -1) {
        SLOGE("remount /system error: %s", strerror(errno));
    }

    /* import vg */
    if ((pid = fork()) < 0) {
        SLOGE("Error while forking!\n");
        return -1;
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/lvm", "lvm", "vgimport", "vg", (char*)0) < 0) {
            SLOGE("Error while executing vgimport!\n");
            return -1;
        }
    } else {
        if (waitpid(pid, &status, 0) < 0 ) {
            SLOGE("Error waiting child process! Error while executing vgimport!\n");
            return -1;
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
            SLOGI("vgimport return code %d\n", ret);
        }
    }

    /* active all lv's in vg */
    if ((pid = fork()) < 0) {
        SLOGE("Error while forking!\n");
        return -1;
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/lvm", "lvm", "vgchange", "-a",
                    "y", "vg", (char*)0) < 0) {
            SLOGE("Error while executing lvm vgchange!\n");
            return -1;
        }
    } else {
        if (waitpid(pid, &status, 0) < 0 ) {
            SLOGE("Error waiting child process! Error while executing lvm vgchange!\n");
            return -1;
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
            SLOGI("vgchange return code %d\n", ret);
        }
    }

	/* Remount / and /system as read-only */
    if (mount("/","/",NULL, MS_REMOUNT | MS_RDONLY, NULL)== -1) {
        SLOGE("remount / as rdonly error: %s\n", strerror(errno));
    }

    if (mount("/system","/system",NULL, MS_REMOUNT | MS_RDONLY, NULL)== -1) {
        SLOGE("remount /system as rdonly error: %s\n", strerror(errno));
    }
      
    SLOGI("done!\n");

    return ret;
}

int
close_lvm(void)
{
	int ret, status;    
    pid_t pid;

	SLOGI("Start closing!\n");

	if (mount("/","/",NULL, MS_REMOUNT, NULL) == -1) {
        SLOGE("remount / error: %s", strerror(errno));
    }

    if (mount("/system","/system",NULL, MS_REMOUNT, NULL) == -1) {
        SLOGE("remount /system error: %s", strerror(errno));
    }

    if (umount("/dev/mapper/vg-hidden") == -1) {
        SLOGE("unable to umount hidden volume! or it doesn't mounted.\n");
    }

    if (umount("/dev/mapper/vg-outter") == -1) {
        SLOGE("unable to umount outter volume! or it doesn't mounted.\n");
    }

    /* close vg */
    if ((pid = fork()) < 0) {
        SLOGE("Error while forking!\n");
        return -1;
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/lvm", "lvm", "vgchange", "-a",
                    "n", "vg", (char*)0) < 0) {
            SLOGE("Error while executing lvm vgchange!\n");
            return -1;
        }
    } else {
        if (waitpid(pid, &status, 0) < 0 ) {
            SLOGE("Error waiting child process! Error while executing lvm vgchange!\n");
            return -1;
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        }
    }

    /* close vg */
    if ((pid = fork()) < 0) {
        SLOGE("Error while forking!\n");
        return -1;
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/lvm", "lvm", "vgexport", "vg", (char*)0) < 0) {
            SLOGE("Error while executing vgexport!\n");
            return -1;
        }
    } else {
        if (waitpid(pid, &status, 0) < 0 ) {
            SLOGE("Error waiting child process!Error while executing vgexport!\n");
            return -1;
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        }
    }

    /* Remount / and /system as read-only */
    if (mount("/","/",NULL, MS_REMOUNT | MS_RDONLY, NULL)== -1) {
        SLOGE("remount / as rdonly error: %s\n", strerror(errno));
    }

    if (mount("/system","/system",NULL, MS_REMOUNT | MS_RDONLY, NULL)== -1) {
        SLOGE("remount /system as rdonly error: %s\n", strerror(errno));
    }
      
    SLOGI("done!\n");

    return ret;
}
