#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>
#include <err.h>

// Figure out where our runtime files are located.
static const char *get_lotus_runtimefile(const char *file)
{
    static char procpath[PATH_MAX];
    static char filepath[PATH_MAX];
    static char *lotusdir;

    // Cache this path so it only has to be looked up once.
    if (lotusdir == NULL) {
        if (readlink("/proc/self/exe", procpath, PATH_MAX) == -1) {
            err(EXIT_FAILURE, "Failed to determine the lotus root directory");
        }
        // Figure out the containing directory from the exe path.
        lotusdir = dirname(procpath);
    }

    // Append the requested filename, obviously this is not reentrant, but 123
    // does not use threads.
    snprintf(filepath, PATH_MAX, "%s/%s", lotusdir, file);
    return filepath;
}

// This routine examines a path requested by Lotus, and will
// map it to something more suitable for use on Linux.
const char * map_unix_pathname(const char *unixpath)
{
    const char *filename = strrchr(unixpath, '/');

    if (*unixpath == '\0')
        return unixpath;

    // Check if we just need to substitute in the root path.
    if (strncmp(unixpath + 1, "{LOTUSROOT}", strlen("{LOTUSROOT}")) == 0) {
        return get_lotus_runtimefile(unixpath + 1 + strlen("{LOTUSROOT}"));
    }

    if (filename == NULL)
        return unixpath;

    if (strcmp(filename, "/.l123set") == 0) {
        // This is the 123 configuration file, if a user has not created one
        // we can map it to the default configuration file instead, which
        // is the directory where 123 is located.
        if (access(unixpath, F_OK) != 0) {
            return get_lotus_runtimefile("l123set.cf");
        }
    }

    // No mapping required
    return unixpath;
}
