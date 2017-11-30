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
typedef struct cmd_t {
	char *text;
	int (*funcptr)(int, int, cstr_t **);
} cmd_t;

