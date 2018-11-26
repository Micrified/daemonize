#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>

/*
 *******************************************************************************
 *                             Symbolic Constants                              *
 *******************************************************************************
*/


#define PANIC_FMT	"%s:%d Panic: \"%s\", errno: \"%s\""

#define PATH_FDS	"/proc/self/fd"

#define ERR			-1


/*
 *******************************************************************************
 *                            Function Declarations                            *
 *******************************************************************************
*/


/*
 *******************************************************************************
 *                            Function Definitions                             *
 *******************************************************************************
*/


// Terminates program with message if boolean condition nonzero.
void panicIf (unsigned short b, const char *msg) {
	if (b == 0) return;
	fprintf(stderr, PANIC_FMT, __FILE__, __LINE__, msg, strerror(errno));
	exit(EXIT_FAILURE);
}

// Converts given string to integer. Returns NULL pointer on error.
int *strToInt (const char *s) {
	static int i;
	long l;
	char *ep;

	l = strtol(s, &ep, 10);

	if (ep == s || errno == ERANGE || l < INT_MIN || l > INT_MAX) {
		return NULL;
	}

	i = (int)l;
	return &i;
}

// Returns value one larger than largest file-descriptor process can open.
int getFDLimit (void) {
	struct rlimit lim;
	panicIf((getrlimit(RLIMIT_NOFILE, &lim) == ERR), "getrlimit call failed!");
	return lim.rlim_cur;
}

// Closes all open file-descriptors in the process.
void closeAllFDS (void) {
	int dfd, *fdp;
	DIR *dp;
	struct dirent *entry;

	// Attempt to open directory.
	if ((dp = opendir(PATH_FDS)) == NULL || (dfd = dirfd(dp)) == ERR) {
		goto fallback;
	}

	// Read entries, close anything other than stdin/stdout/stderr and dir-fd.
	while ((entry = readdir(dp)) != NULL) {
		if ((fdp = strToInt(entry->d_name)) != NULL && *fdp != dfd && fdp > 2) {
			close(*fdp);
		}
	}

	// Close directory.
	panicIf(closedir(dp) == ERR, "Couldn't close " PATH_FDS);

	// Return.
	return;

	fallback:
	for (int i = 3; i < getFDLimit(); i++) {
		close(i); // No exception handled.
	}
}

// Resets all signal-handlers.



/*
 *******************************************************************************
 *                                Main Program                                 *
 *******************************************************************************
*/


int main (int argc, const char *argv[]) {
	printf("My limit for file-desciptors is %d\n", getFDLimit());
	closeAllFDS();
	return EXIT_SUCCESS;
}