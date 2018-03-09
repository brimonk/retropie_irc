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
#include "constants.h"
#include "irc.h"
#include "helpers.h"

/* these irc functions are for sending things across the wire */
int irc_set_nick(int sock, int arrlen, cstr_t **out)
{
    char nick_packet[512];
	int sendretval;

    sprintf(nick_packet, "NICK %s\r\n", (*out)[0].buf);
    sendretval = send(sock, nick_packet, strlen(nick_packet), 0);

	return (sendretval > 0) ? 1 : 0;
}

int irc_send_user(int sock, int arrlen, cstr_t **out)
{
    char user_packet[512];
	int sendretval;
    sprintf(user_packet, "USER %s 0 * :%s\r\n", (*out)[0].buf, (*out)[0].buf);
    sendretval = send(sock, user_packet, strlen(user_packet), 0);

	return (sendretval > 0) ? 1 : 0;
}

int irc_join_channel(int sock, int arrlen, cstr_t **out)
{
    char join_packet[512];
	int sendretval;
    sprintf(join_packet, "JOIN %s\r\n", (*out)[0].buf);
    sendretval = send(sock, join_packet, strlen(join_packet), 0);

	return (sendretval > 0) ? 1 : 0;
}

int irc_send_pong(int sock, int arrlen, cstr_t **out)
{
    char pong_packet[512];
	int sendretval;
    sprintf(pong_packet, "PONG :%s\r\n", (*out)[0].buf);
    sendretval = send(sock, pong_packet, strlen(pong_packet), 0);

	return (sendretval > 0) ? 1 : 0;
}

int irc_send_message(int sock, int arrlen, cstr_t **out)
{
    char message_packet[512];
	int sendretval;
	memset(message_packet, 0, BUFLEN);
    sprintf(message_packet, "PRIVMSG %s :%s\r\n", (*out)[0].buf, (*out)[1].buf);
    sendretval = send(sock, message_packet, strlen(message_packet), 0);

	return (sendretval > 0) ? 1 : 0;
}

int irc_send_quit(int sock, int arrlen, cstr_t **out)
{
	char message_packet[512];
	int sendretval;
	memset(message_packet, 0, BUFLEN);
    sprintf(message_packet, "QUIT :%s\r\n", (*out)[0].buf);
    sendretval = send(sock, message_packet, strlen(message_packet), 0);

	return (sendretval > 0) ? 1 : 0;

}

/* routines specific for the PRIVMSG command */
int irc_privmsg(int socket, char *input, list_t *ptr, cstr_t *str)
{
	int lib_return;

	lib_return = 0;

	if (get_argument(str[0].buf, str[0].len, input, 1)) {
		return IRC_PARSE_ERR;
	}

	if (get_username(str[1].buf, str[1].len, input)) {
		return IRC_PARSE_ERR;
	}

	if (get_one_arg(str[2].buf, str[2].len, input)) {
		return IRC_PARSE_ERR;
	}

	if (get_argument_arguments(str[3].buf, str[3].len, input)) {
		return IRC_PARSE_ERR;
	}

	if (get_prefix(str[32].buf, str[32].len, input)) {
		return IRC_PARSE_ERR;
	}

	// if (get_command(str[34].buf, str[34].len, input)) {
	// 	return IRC_PARSE_ERR;
	// }

	if (get_last_argument(str[35].buf, str[35].len, input)) {
		return IRC_PARSE_ERR;
	}

	/* spend some time setting up our cstr_t values */

	lib_return = irc_privmsg_namedfunc(&str[8], ptr, str);
	if (!lib_return) {
		lib_return = irc_privmsg_unnamedfunc(&str[8], ptr, str);
	}

	/* provide the properly formatted cstr_t and the return value */
	if (get_argument(str[0].buf, str[0].len, input, 1)) {
		return IRC_PARSE_ERR;
	}
	strncpy(str[1].buf, str[8].buf, BUFLEN - strlen(str[8].buf));
	strncat(str[1].buf, "\r\n", BUFLEN - strlen(str[8].buf));

	irc_privmsg_respond(socket, lib_return, str);

	return lib_return;
}

int irc_privmsg_namedfunc(cstr_t *buf, list_t *ptr, cstr_t *str)
{
	int val;
	char *tmp_text;

	list_t *h;
	val = 0;

	/* 
	 * iterate through the linked list we have, comparing the argument
	 * text with lib_tbl_t->text
	 */

	for (h = ptr; h != NULL; h = h->next) {
		tmp_text = ((lib_tbl_t *)h->data)->text;

		if (strcmp(str[2].buf, tmp_text) == 0) {
			val = ((lib_tbl_t *)h->data)->
				funcptr(buf[0].buf, buf[0].len, &str);
			break;
		}
	}

	return val;
}

int irc_privmsg_unnamedfunc(cstr_t *buf, list_t *ptr, cstr_t *str)
{
	int val;
	list_t *h;

	val = 0;
	/* execute all of the things that have "" as their text */
	for (h = ptr; h != NULL; h = h->next) {
		lib_tbl_t *tmp = (lib_tbl_t *)h->data;

		if (strcmp(tmp->text, "") == 0) {
			val = ((lib_tbl_t *)h->data)->
				funcptr(buf[0].buf, buf[0].len, &str);
			break;
		}
	}

	return val;
}

void irc_privmsg_respond(int socket, int irc_returnval, cstr_t *buf)
{
	/* 
	 * this wrapper remains in the case that we need special cases 
	 * for the various IRC_RETURN functions
	 */

	switch (irc_returnval) {
	case IRC_RETURN_OK:
	default:
		irc_send_message(socket, 2, &buf);
		break;
	}
}

/* 
 * TODO :: change these parsing routines to not have the following:
 *
 * 1. remove calls to malloc - have destination buffers with an associated size
 */
int get_prefix(char *dest, int size, char *line)
{
	char *splitted;
    char clone[512];

	if (512 < size) {
		return 1;
	}

    strncpy(clone, line, strlen(line)+1);

    if (line[0] == ':') {
        splitted = strtok(clone, " ");

        if (splitted != NULL){
            strncpy(dest, splitted + 1, strlen(splitted)+1);
        } else {
            dest[0] = '\0';
        }
    } else {
        dest[0] = '\0';
    }

	return 0;
}

int get_username(char *dest, int size, char *line)
{
	char *splitted;
    char clone[512];

	if (512 < size) {
		return 1;
	}

    strncpy(clone, line, strlen(line)+1);

    if (strchr(clone, '!') != NULL) {
        splitted = strtok(clone, "!");
        if (splitted != NULL) {
            strncpy(dest, splitted + 1, strlen(splitted) + 1);
        } else {
            dest[0] = '\0';
        }
    } else {
        dest[0] = '\0';
    }

    return 0;
}

int get_command(char *dest, int size, char *line)
{
	char *splitted;
    char clone[512];
    strncpy(clone, line, strlen(line)+1);
    splitted = strtok(clone, " ");

	if (512 < size) {
		return 1;
	}

    if (splitted != NULL){
        if (splitted[0] == ':'){
            splitted = strtok(NULL, " ");
        }
        if (splitted != NULL){
            strncpy(dest, splitted, strlen(splitted)+1);
        }else{
            dest[0] = '\0';
        }
    }else{
        dest[0] = '\0';
    }

    return 0;
}

int get_last_argument(char *dest, int size, char *line)
{
	char *splitted;
    char clone[512];

	if (512 < size) {
		return 1;
	}

    strncpy(clone, line, strlen(line) + 1);

    splitted = strstr(clone, " :");

    if (splitted != NULL) {
        strncpy(dest, splitted + 2, strlen(splitted) + 1);
    } else {
        dest[0] = '\0';
    }

	return 0;
}

int get_argument(char *dest, int size, char *line, int argno)
{
    char clone[512];

	if (512 < size) {
		return 1;
	}

    strncpy(clone, line, strlen(line)+1);
    
    int current_arg = 0;
    char *splitted = strtok(clone, " ");

    while (splitted != NULL) {
        if (splitted[0] != ':') {
            current_arg++;
        }

        if (current_arg == argno+1) {
            strncpy(dest, splitted, strlen(splitted)+1);
			return 0;
        }

        splitted = strtok(NULL, " ");
    }
    
    if (current_arg != argno){
        dest[0] = '\0';
    }

    return 0;
}

int get_argument_arguments(char *dest, int size, char *input)
{
	/* returns a pointer to the next word after line */
	char *curr;

	curr = strstr(input, " :");
	if (curr) {
		curr += 2;
		strncpy(dest, curr, strlen(curr) + 1);
	}

	return 0;
}

int get_one_arg(char *dest, int size, char *input)
{
	char *splitted;
	char clone[512];

	if (512 < size) {
		return 1;
	}

    splitted = strstr(input, " :");
	splitted += 2;

    if (splitted != NULL){
		char *tmp = strchr(splitted, ' ');

		if (tmp != NULL) {
			strncpy(clone, splitted, tmp - splitted);
			*(clone + (tmp - splitted)) = 0;
		} else { /* one argument command */
			strncpy(clone, splitted, strlen(splitted));
			clone[strlen(splitted)] = 0;
		}

    } else {
		dest = NULL;
    }

	if (strlen(clone) > 0) {
		strncpy(dest, clone, strlen(clone));
	}

    return 0;
}
