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

    /* umount hidden volume */
    if ((pid = fork()) < 0) {
        printf("Error while forking!\n");
        _exit(-1);
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/umount", "umount", 
                    "/dev/mapper/vg-hidden", (char*)0) < 0) {
            printf("Error while executing umout hidden!\n");
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

    /* umount outter volume */
    if ((pid = fork()) < 0) {
        printf("Error while forking!\n");
        _exit(-1);
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/umount", "umount", 
                    "/dev/mapper/vg-outter", (char*)0) < 0) {
            printf("Error while executing umount hidden!\n");
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

    /* close vg */
    if ((pid = fork()) < 0) {
        printf("Error while forking!\n");
        _exit(-1);
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/lvm", "lvm", "vgchange", "-a",
                    "n", (char*)0) < 0) {
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

    /* close vg */
    if ((pid = fork()) < 0) {
        printf("Error while forking!\n");
        _exit(-1);
    } else if (pid == 0) {      /* child process */
        if (execl("/system/bin/lvm", "lvm", "vgexport", "vg", (char*)0) < 0) {
            printf("Error while executing vgexport!\n");
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
