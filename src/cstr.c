#include <stdlib.h>
#include <string.h>
#include "cstr.h"
#include "data_types.h"

int cstr_init(cstr_t **ptr, int len, int buflen)
{
	int i, retval;
	cstr_t *tmp;

	retval = 0;
	*ptr = malloc(sizeof(cstr_t) * len);

	if (*ptr) {

		tmp = *ptr;
		for (i = 0; i < len; i++) {
			tmp[i].len = buflen;
			tmp[i].buf = malloc(buflen);

			if (tmp[i].buf) {
				memset(tmp[i].buf, 0, len);
			} else {
				retval = 1;
				break;
			}
		}
	}

	return retval;
}

void cstr_free(cstr_t *ptr, int num)
{
	int i;

	for (i = 0; i < num; i++) {
		free(ptr[i].buf);
	}

	free(ptr);
}

void cstr_clear(cstr_t *ptr, int num, int len)
{
	int i;

	for (i = 0; i < num; i++) {
		memset(ptr[i].buf, 0, len);
	}
}
