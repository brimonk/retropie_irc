#include <stdlib.h>
#include <string.h>
#include "cstr.h"
#include "data_types.h"

int cstr_init(cstr_t **ptr, int len, int buflen)
{
	int i, retval;

	retval = 0;
	*ptr = malloc(sizeof(cstr_t) * len);

	if (*ptr) {
		for (i = 0; i < len; i++) {
			(*ptr)[i].len = buflen;
			(*ptr)[i].buf = malloc(buflen);
			memset((*ptr)[i].buf, 0, len);

			if (!(*ptr)[i].buf) {
				retval = 1;
				break;
			}
		}
	}

	return retval;
}

void cstr_free(cstr_t **ptr, int num)
{
	int i;

	if (*ptr) {
		for (i = 0; i < num; i++) {
			memset(&((*ptr)[i]), 0, sizeof(cstr_t));
		}
	}
}
