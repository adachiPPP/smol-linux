#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define REPO "https://adachippp.github.io/smolrepo"

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

static char *resolve_alias(const char *pkg) {
    char url[512];
    snprintf(url, sizeof(url), "%s/packages.txt", REPO);

    char cmd[600];
    snprintf(cmd, sizeof(cmd), "curl -sL %s", url);

    FILE *fp = popen(cmd, "r");
    if (!fp) return NULL;

    static char filename[256];
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char name[256], file[256];
        if (sscanf(line, "%255s %255s", name, file) == 2) {
            if (strcmp(name, pkg) == 0) {
                strncpy(filename, file, sizeof(filename) - 1);
                pclose(fp);
                return filename;
            }
        }
    }
    pclose(fp);
    return NULL;
}

static int install_local(const char *path) {
    printf("installing local package %s...\n", path);
    char *argv[] = {"tar", "-xzf", (char *)path, "-C", "/", NULL};
    if (run(argv) == 0) {
        printf("package installed successfully uwu\n");
        return 0;
    }
    return 1;
}

static int install_remote(const char *pkg) {
    printf("resolving alias for %s...\n", pkg);
    char *filename = resolve_alias(pkg);
    char fallback[300];
    if (!filename) {
        printf("alias not found trying fallback name...\n");
        snprintf(fallback, sizeof(fallback), "%s.tar.gz", pkg);
        filename = fallback;
    }

    printf("fetching and installing %s from smolrepo...\n", filename);
    char url[600];
    snprintf(url, sizeof(url), "%s/%s", REPO, filename);

    char cmd[700];
    snprintf(cmd, sizeof(cmd), "curl -sL %s | tar -xzf - -C /", url);
    int rc = system(cmd);
    if (rc == 0) {
        printf("package installed successfully uwu\n");
        return 0;
    }
    return 1;
}

static int search(const char *term) {
    printf("searching smolrepo for '%s'...\n", term);
    char cmd[600];
    snprintf(cmd, sizeof(cmd),
             "curl -sL %s/packages.txt | grep -i '%s'", REPO, term);
    system(cmd);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: smol [-L local_file] [-G package_alias] [-Gs search_term] [-R package_alias]\n");
        return 1;
    }

    if (strcmp(argv[1], "-L") == 0) {
        if (argc < 3) { fprintf(stderr, "error: specify a local file path\n"); return 1; }
        return install_local(argv[2]);
    } else if (strcmp(argv[1], "-G") == 0) {
        if (argc < 3) { fprintf(stderr, "error: specify a package name or alias\n"); return 1; }
        return install_remote(argv[2]);
    } else if (strcmp(argv[1], "-Gs") == 0) {
        if (argc < 3) { fprintf(stderr, "error: specify a search term\n"); return 1; }
        return search(argv[2]);
    } else {
        fprintf(stderr, "usage: smol [-L local_file] [-G package_alias] [-Gs search_term] [-R package_alias]\n");
        return 1;
    }
}
