/* define required data types here for multiple inclusions */

typedef struct cstr_t {
	char *buf;
	int len;
} cstr_t;

typedef struct config_t {
	char *key;
	char *value;
} config_t;

enum {
	IRC_JOIN_CHANNEL,
	IRC_SEND_MESSAGE,
	IRC_SEND_PONG,
	IRC_SEND_USER_PACKET,
	IRC_SET_NICK
};

/* 
 * every irc function takes in an input * and output * structure, while
 * returning only a status integer
 */

/* to easily iterate through every irc function, store them in a list */
typedef struct cmd_t {
	char *text;
	int enumval;
	int (*funcptr)(int, int, cstr_t **);
} cmd_t;

