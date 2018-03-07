/* define required data types here for multiple inclusions */

typedef struct cstr_t {
	char *buf;
	int len;
} cstr_t;

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

enum { /* type of IRC command */
	IRC_TYPE_CALLRESPONSE = 1,
	IRC_TYPE_RESPONSE = 2
};

enum { /* return value to return to caller */
	IRC_RETURN_OK				= 0x00,
	IRC_RETURN_NOOUT			= 0x01,
	IRC_RETURN_NOMEM			= 0x02,
	IRC_RETURN_BADPERM			= 0x04,
	IRC_RETURN_CMDNOSUPPORT		= 0x08,

	IRC_RETURN_QUIT             = 0xFFFF
};

typedef struct list_t {
	struct list_t *next;
	struct list_t *prev;
	void *data;
} list_t;
