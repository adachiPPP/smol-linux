#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mount.h>
#include <sys/wait.h>

static int run(char *const argv[]) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(argv[0], argv);
        perror("execvp");
        _exit(127);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    }
    perror("fork");
    return -1;
}

static void do_mount(const char *src, const char *target, const char *fstype) {
    if (mount(src, target, fstype, 0, NULL) != 0) {
        fprintf(stderr, "warning: failed to mount %s on %s (%s)\n",
                fstype, target, strerror(errno));
    }
}

static void write_resolv_conf(void) {
    FILE *f = fopen("/etc/resolv.conf", "w");
    if (!f) {
        perror("fopen /etc/resolv.conf");
        return;
    }
    fprintf(f, "nameserver 8.8.8.8\n");
    fprintf(f, "nameserver 1.1.1.1\n");
    fclose(f);
}

int main(void) {
    setenv("PATH", "/bin:/sbin:/usr/bin:/usr/sbin", 1);

    do_mount("proc", "/proc", "proc");
    do_mount("sysfs", "/sys", "sysfs");
    do_mount("devtmpfs", "/dev", "devtmpfs");

    {
        char *argv[] = {"ip", "link", "set", "lo", "up", NULL};
        run(argv);
    }
    {
        char *argv[] = {"ip", "link", "set", "eth0", "up", NULL};
        run(argv);
    }

    {
        char *argv[] = {"udhcpc", "-i", "eth0", "-n", "-q", NULL};
        if (run(argv) != 0) {
            char *addr_argv[] = {"ip", "addr", "add", "10.0.2.15/24", "dev", "eth0", NULL};
            run(addr_argv);
            char *route_argv[] = {"ip", "route", "add", "default", "via", "10.0.2.2", "dev", "eth0", NULL};
            run(route_argv);
            write_resolv_conf();
        }
    }

    printf("welcome to smol linux :3\n");
    fflush(stdout);

    execl("/bin/sh", "/bin/sh", NULL);
    perror("execl /bin/sh");
    return 1;
}
