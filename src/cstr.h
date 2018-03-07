#ifndef _CSTRT_
#define _CSTRT_

typedef struct cstr_t {
	char *buf;
	int len;
} cstr_t;

int cstr_init(cstr_t **ptr, int len, int buflen);
void cstr_free(cstr_t *ptr, int num);
void cstr_clear(cstr_t *ptr, int num, int len);

#endif
