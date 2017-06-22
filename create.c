#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int
main(int argc, char** argv)
{
    int ret, child_ret, status;    
    pid_t pid;

	if ((pid = fork()) < 0) {
        printf("Error while forking!\n");
        _exit(-1);
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/mount", "mount", "-o","remount,rw", "/",(char*)0) < 0) {
            printf("Error while remounting /!\n");
            _exit(-1);
        }
    } else {
        if (waitpid(pid, &status, 0) < 0) {
            printf("Error waiting child process!\n");
            _exit(-1);
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        }
    }

	if ((pid = fork()) < 0) {
        printf("Error while forking!\n");
        _exit(-1);
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/mount", "mount", "-o","remount,rw", "/system",(char*)0) < 0) {
            printf("Error while remounting /system!\n");
            _exit(-1);
        }
    } else {
        if (waitpid(pid, &status, 0) < 0) {
            printf("Error waiting child process!\n");
            _exit(-1);
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        }
    }

    if ((pid = fork()) < 0) {
        printf("Error while forking!\n");
        _exit(-1);
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/lvm", "lvm", "pvcreate","-y", "/dev/block/loop1",(char*)0) < 0) {
            printf("Error while executing lvm pvcreate!\n");
            _exit(-1);
        }
    } else {
        if (waitpid(pid, &status, 0) < 0) {
            printf("Error waiting child process!\n");
            _exit(-1);
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        }
    }

    if ((pid = fork()) < 0) {
        printf("Error while forking!\n");
        _exit(-1);
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/lvm", "lvm", "vgcreate","-y", "vg",  "/dev/block/loop1",(char*)0) < 0) {
            printf("Error while executing lvm vgcreate!\n");
            _exit(-1);
        }
    } else {
        if (waitpid(pid, &status, 0) < 0) {
            printf("Error waiting child process!\n");
            _exit(-1);
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        }
    }

    if ((pid = fork()) < 0) {
        printf("Error while forking!\n");
        _exit(-1);
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/lvm", "lvm", "lvcreate","-y", "--name",  
                    "metadata","--size", "100M", "vg", (char*)0) < 0) {
            printf("Error while executing lvm lvcreate!\n");
            _exit(-1);
        }
    } else {
        if (waitpid(pid, &status, 0) < 0) {
            printf("Error waiting child process!\n");
            _exit(-1);
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        }
    }

    if ((pid = fork()) < 0) {
        printf("Error while forking!\n");
        _exit(-1);
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/lvm", "lvm", "lvcreate","-y", "--name",  
                    "data","--size", "1.5G", "vg", (char*)0) < 0) {
            printf("Error while executing lvm lvcreate data!\n");
            _exit(-1);
        }
    } else {
        if (waitpid(pid, &status, 0) < 0) {
            printf("Error waiting child process!\n");
            _exit(-1);
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        }
    }

    /* Convert to thin volumes */
    if ((pid = fork()) < 0) {
        printf("Error while forking!\n");
        _exit(-1);
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/lvm", "lvm", "lvconvert", "-y", "--thinpool",  
                    "vg/data","--poolmetadata", "vg/metadata", (char*)0) < 0) {
            printf("Error while executing lvm lvconvert!\n");
            _exit(-1);
        }
    } else {
        if (waitpid(pid, &status, 0) < 0) {
            printf("Error waiting child process!\n");
            _exit(-1);
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        }
    }
    
    /* Creating virtual volume */
    if ((pid = fork()) < 0) {
        printf("Error while forking!\n");
        _exit(-1);
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/lvm", "lvm", "lvcreate","-y", "--name",  
                    "outter","--virtualsize", "1.5G", "--thinpool",
                    "vg/data", (char*)0) < 0) {
            printf("Error while executing lvm lvcreate virtual volume!\n");
            _exit(-1);
        }
    } else {
        if (waitpid(pid, &status, 0) < 0) {
            printf("Error waiting child process!\n");
            _exit(-1);
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        }
    }

    /* Creating virtual volume */
    if ((pid = fork()) < 0) {
        printf("Error while forking!\n");
        _exit(-1);
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/lvm", "lvm", "lvcreate","-y", "--name",  
                    "hidden","--virtualsize", "1.5G", "--thinpool",
                    "vg/data", (char*)0) < 0) {
            printf("Error while executing lvm lvcreate virtual volume!\n");
            _exit(-1);
        }
    } else {
        if (waitpid(pid, &status, 0) < 0) {
            printf("Error waiting child process!\n");
            _exit(-1);
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        }
    }

    /* Format the volume with make_ext4fs */
    if ((pid = fork()) < 0) {
        printf("Error while forking!\n");
        _exit(-1);
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/make_ext4fs", "make_ext4fs", 
                    "/dev/mapper/vg-outter", (char*)0) < 0) {
            printf("Error while formatting outter volume!\n");
            _exit(-1);
        }
    } else {
        if (waitpid(pid, &status, 0) < 0) {
            printf("Error waiting child process!\n");
            _exit(-1);
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        }
    }


     /* Format the hidden volume with make_ext4fs */
    if ((pid = fork()) < 0) {
        printf("Error while forking!\n");
        _exit(-1);
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/make_ext4fs", "make_ext4fs", 
                    "/dev/mapper/vg-hidden", (char*)0) < 0) {
            printf("Error while formatting hidden volume!\n");
            _exit(-1);
        }
    } else {
        if (waitpid(pid, &status, 0) < 0) {
            printf("Error waiting child process!\n");
            _exit(-1);
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        }
    }

	if ((pid = fork()) < 0) {
        printf("Error while forking!\n");
        _exit(-1);
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/mount", "mount", "-o","remount,ro", "/",(char*)0) < 0) {
            printf("Error while remounting / as read only!\n");
            _exit(-1);
        }
    } else {
        if (waitpid(pid, &status, 0) < 0) {
            printf("Error waiting child process!\n");
            _exit(-1);
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        }
    }

	if ((pid = fork()) < 0) {
        printf("Error while forking!\n");
        _exit(-1);
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/mount", "mount", "-o","remount,ro", "/system",(char*)0) < 0) {
            printf("Error while remounting /system as read only!\n");
            _exit(-1);
        }
    } else {
        if (waitpid(pid, &status, 0) < 0) {
            printf("Error waiting child process!\n");
            _exit(-1);
        }
        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        }
    }
   
    printf("done!\n");

    return 0;
}
