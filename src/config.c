#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "constants.h"

void log_with_date(char line[])
{
    char date[50];
    struct tm *current_time;

    time_t now = time(0);
    current_time = gmtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", current_time);

    printf("[%s] %s\n", date, line);
}

void log_to_file(char line[], FILE *logfile)
{
    char date[50];
    struct tm *current_time;

    time_t now = time(0);
    current_time = gmtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", current_time);

    fprintf(logfile, "[%s] %s\n", date, line);
    fflush(logfile);
}

char *get_config(str_dict_t *ptr, int len, char *key)
{
	/* 
	 * retrieves config value pointer for input key
	 * DO NOT FREE
	 */

	int i, j;

	j = strlen(key);
	for (i = 0; i < len; i++) {
		if (strncmp(ptr[i].key, key, j) == 0) {
			return ptr[i].value;
		}
	}

	return NULL;
}

int config_load(str_dict_t **ptr, int len, char *file_name)
{
	/* 
	 * generalized function to load only len items (lines) from a config file
	 * returns non-zero on error
	 *
	 * WARNING
	 *	if the config file has blank lines, this breaks. Some more logic needs
	 *	to be added to check if the line actually has what we're 
	 *	trying to parse
	 */

	int i;
	char buf[BUFLEN];

	*ptr = malloc(sizeof(str_dict_t) * len);

	if (!*ptr) {
		return -1;
	}
 
	memset(&buf[0], 0, BUFLEN);

    FILE *fptr = fopen("config.txt", "r");

    if (fptr) {
		for (i = 0; i < len && fgets(buf, BUFLEN, fptr) == &buf[0]; i++) {
			/* get space for key and value (wastefully) */
			(*ptr)[i].key   = malloc(BUFLEN);
			(*ptr)[i].value = malloc(BUFLEN);

			if (!((*ptr)[i].key && (*ptr)[i].value)) {
				return -1;
			}

			/* parse the values from buffer into (*ptr)[i].key && value */
			sscanf(buf, "%511s = %511s ",
					(*ptr)[i].key,
					(*ptr)[i].value);
        }

        fclose(fptr);
    } else {
		return -1;
	}

	return i;
}
