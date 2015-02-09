#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <glib.h>
#include <sys/stat.h>

#include "user_dir.h"


#define USER_DIR_ROOT G_DIR_SEPARATOR_S ".testfarm-vu"


char *user_dir(char *subdir, char *filename)
{
	static char *home = NULL;
	static int home_len = 0;
	int size, len;
	char *path;

	/* Retrieve home directopry name */
	if (home == NULL) {
		home = getenv("HOME");
		if ((home == NULL) || (home[0] == '\0'))
			home = "/tmp";

		home_len = strlen(home);
		if ((home_len > 0) && (home[home_len-1] == G_DIR_SEPARATOR))
			home[home_len--] = '\0';
	}

	size = home_len + strlen(USER_DIR_ROOT) + 1;
	if (subdir != NULL)
		size += strlen(subdir) + 1;
	if (filename != NULL)
		size += strlen(filename) + 1;
	path = (char *) malloc(size);

	len = snprintf(path, size, "%s" USER_DIR_ROOT, home);
	if ( mkdir(path, 0777) == -1 ) {
		if ( errno != EEXIST ) {
			fprintf(stderr, "mkdir(%s): %s\n", path, strerror(errno));
			goto failed;
		}
	}

	if (subdir != NULL) {
		len += snprintf(path+len, size-len, G_DIR_SEPARATOR_S "%s", subdir);
		if ( mkdir(path, 0777) == -1 ) {
			if ( errno != EEXIST ) {
				fprintf(stderr, "mkdir(%s): %s\n", path, strerror(errno));
				goto failed;
			}
		}
	}

	if (filename != NULL) {
		len += snprintf(path+len, size-len, G_DIR_SEPARATOR_S "%s", filename);
	}

	return path;

failed:
	free(path);
	return NULL;
}
