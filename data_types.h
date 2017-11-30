/* define required data types here for multiple inclusions */

typedef struct cstr_t {
	char *buf;
	int len;
} cstr_t;

typedef struct config_t {
	char *key;
	char *value;
} config_t;

typedef struct cmd_t {
	char *text;
} cmd_t;

typedef struct irc_inpt_t {
	char *prefix;
	char *username;
	char *command;
	char *argument;
} irc_inpt_t;

