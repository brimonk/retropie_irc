/* define required data types here for multiple inclusions */

typedef struct cstr_t {
	char *buf;
	int len;
} cstr_t;

typedef struct config_t {
	char *key;
	char *value;
} config_t;

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


enum { IRC_TYPE_CALLRESPONSE, IRC_TYPE_RESPONSE };
typedef struct lib_cmd_t {
	char *text;
	int (*funcptr)(char *, int, cstr_t **);
	int type; /* where type is any enum of the IRC_TYPE_* */
} lib_cmd_t;

enum { IRC_RETURN_OK,
	   IRC_RETURN_NOMEM,
	   IRC_RETURN_BADPERM,
	   IRC_RETURN_CMDNOSUPPORT
};

