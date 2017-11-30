/* We separately define each irc function to properly segment our code */

void irc_set_nick(int sock, char nick[]);
void irc_send_user_packet(int sock, char nick[]);
void irc_join_channel(int sock, char channel[]);
void irc_send_pong(int sock, char argument[]);
void irc_send_message(int sock, char to[], char message[]);

