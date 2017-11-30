/* We separately define each irc function to properly segment our code */

int irc_set_nick(int sock, int arrlen, cstr_t **out);
int irc_send_user_packet(int sock, int arrlen, cstr_t **out);
int irc_join_channel(int sock, int arrlen, cstr_t **out);
int irc_send_pong(int sock, int arrlen, cstr_t **out);
int irc_send_message(int sock, int arrlen, cstr_t **out);

