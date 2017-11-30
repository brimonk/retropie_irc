/* 
 * here are all of the irc functions
 *
 * to allow main to be slick, I decided that each function should be
 * provided an array of cstr_ts on input, so irc functions like
 * irc_send_message would have the same function header.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "data_types.h"

int irc_set_nick(int sock, int arrlen, cstr_t **out)
{
    char nick_packet[512];
	int sendretval;

    sprintf(nick_packet, "NICK %s\r\n", out[0]->buf);
    sendretval = send(sock, nick_packet, strlen(nick_packet), 0);

	return (sendretval > 0) ? 1 : 0;
}

int irc_send_user(int sock, int arrlen, cstr_t **out)
{
    char user_packet[512];
	int sendretval;
    sprintf(user_packet, "USER %s 0 * :%s\r\n", out[0]->buf, out[0]->buf);
    sendretval = send(sock, user_packet, strlen(user_packet), 0);

	return (sendretval > 0) ? 1 : 0;
}

int irc_join_channel(int sock, int arrlen, cstr_t **out)
{
    char join_packet[512];
	int sendretval;
    sprintf(join_packet, "JOIN %s\r\n", out[0]->buf);
    sendretval = send(sock, join_packet, strlen(join_packet), 0);

	return (sendretval > 0) ? 1 : 0;
}

int irc_send_pong(int sock, int arrlen, cstr_t **out)
{
    char pong_packet[512];
	int sendretval;
    sprintf(pong_packet, "PONG :%s\r\n", out[0]->buf);
    sendretval = send(sock, pong_packet, strlen(pong_packet), 0);

	return (sendretval > 0) ? 1 : 0;
}

int irc_send_message(int sock, int arrlen, cstr_t **out)
{
    char message_packet[512];
	int sendretval;
    sprintf(message_packet, "PRIVMSG %s :%s\r\n", out[0]->buf, out[1]->buf);
    sendretval = send(sock, message_packet, strlen(message_packet), 0);

	return (sendretval > 0) ? 1 : 0;
}

