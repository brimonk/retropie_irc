/* We separately define each irc function to properly segment our code */

int irc_set_nick(int sock, int arrlen, cstr_t **out);
int irc_send_user(int sock, int arrlen, cstr_t **out);
int irc_join_channel(int sock, int arrlen, cstr_t **out);
int irc_send_pong(int sock, int arrlen, cstr_t **out);
int irc_send_message(int sock, int arrlen, cstr_t **out);
int irc_send_quit(int sock, int arrlen, cstr_t **out);

int get_prefix(char *dest, int size, char *line);
int get_username(char *dest, int size, char *line);
char *get_command(char line[]);
char *get_last_argument(char line[]);
char *get_argument(char line[], int argno);
char *get_argument_arguments(char *beginning);
char *get_one_arg(char *input);

/* irc_privmsg specifics */
int irc_privmsg(int socket, char *input, list_t *ptr, cstr_t *str);
int irc_privmsg_namedfunc(cstr_t *buf, list_t *ptr, cstr_t *str);
int irc_privmsg_unnamedfunc(cstr_t *buf, list_t *ptr, cstr_t *str);
void irc_privmsg_respond(int socket, int irc_returnval, cstr_t *buf);
