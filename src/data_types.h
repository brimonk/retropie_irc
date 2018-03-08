/* define required data types here for multiple inclusions */

#include <limits.h>
#include "cstr.h"

#ifndef _DATATYPES_
#define _DATATYPES_

typedef struct str_dict_t {
	char *key;
	char *value;
} str_dict_t;

/* 
 * every irc function takes in an input * and output * structure, while
 * returning only a status integer
 */

/* 
 * to easily iterate through every irc function, store them in a list
 *
 * we might eventually add an indicator to show if a user needs to be an admin
 * to run the command
 */
typedef struct irc_cmd_t {
	char *text;
	int (*funcptr)(int, int, cstr_t **);
} irc_cmd_t;


typedef struct lib_cmd_t {
	char *text;
	int (*funcptr)(char *, int, cstr_t **);
	int type; /* where type is any enum of the IRC_TYPE_* */
} lib_cmd_t;

typedef struct lib_tbl_t {
	char *text;
	void *objhandle;
	int (*funcptr)(char *, int, cstr_t **);
	int type; /* where type is any enum of the IRC_TYPE_* */
} lib_tbl_t;

enum { /* type of IRC command */
	IRC_TYPE_CALLRESPONSE = 1,
	IRC_TYPE_RESPONSE = 2
};

enum { /* return value to return to caller */
	IRC_RETURN_OK = 1,
	IRC_RETURN_NOOUT = 2,
	IRC_RETURN_NOMEM = 3,
	IRC_RETURN_BADPERM = 4,
	IRC_RETURN_CMDNOSUPPORT = 5,

	IRC_PARSE_ERR = INT_MAX - 1,
	IRC_RETURN_QUIT             = INT_MAX
};

typedef struct list_t {
	struct list_t *next;
	struct list_t *prev;
	void *data;
} list_t;

#endif

