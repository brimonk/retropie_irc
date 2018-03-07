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

int irc_privmsg(int socket, char *input, list_t *ptr, cstr_t *str)
{
	int lib_return;
	cstr_t *resp_buf;

	lib_return = 0;
	if (cstr_init(&resp_buf, 8, BUFLEN)) {
		error_and_exit("Not enough memory for irc_privmsg\n");
	}

	char *prefix = get_prefix(input);
	char *username = get_username(input);
	char *command = get_command(input);
	char *argument = get_last_argument(input);
	char *channel = get_argument(input, 1);

	/* spend some time setting up our cstr_t values */
	strncpy(str[0].buf, channel, BUFLEN);
	strncpy(str[1].buf, username, BUFLEN);
	strncpy(str[2].buf, argument, BUFLEN);
	strncpy(str[3].buf, argument, BUFLEN);

	lib_return = irc_privmsg_namedfunc(resp_buf, ptr, str);
	if (!lib_return) {
		lib_return = irc_privmsg_unnamedfunc(resp_buf, ptr, str);
	}

	/* provide the properly formatted cstr_t and the return value */
	strncpy(resp_buf[1].buf, resp_buf[0].buf,
			BUFLEN - strlen(resp_buf[1].buf));
	strncat(resp_buf[1].buf, "\r\n", BUFLEN - strlen(resp_buf[1].buf));
	strncpy(resp_buf[0].buf, channel, BUFLEN);

	resp_buf[1].len = strlen(resp_buf[1].buf);
	resp_buf[0].len = strlen(resp_buf[0].buf);

	irc_privmsg_respond(socket, lib_return, resp_buf);

	free(prefix);
	free(username);
	free(command);
	free(argument);

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
	 * text with lib_cmd_t->text
	 */

	for (h = ptr; h != NULL; h = h->next) {
		tmp_text = ((lib_cmd_t *)h->data)->text;

		if (strcmp(str[2].buf, tmp_text) == 0) {
			val = ((lib_cmd_t *)h->data)->
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
		lib_cmd_t *tmp = (lib_cmd_t *)h->data;

		if (strcmp(tmp->text, "") == 0) {
			val = ((lib_cmd_t *)h->data)->
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

/* WARNING :: these parsing methods aren't clean TODO :: fix these */
char *get_prefix(char line[])
{
    char *prefix = malloc(512);
    char clone[512];
    strncpy(clone, line, strlen(line)+1);
    if (line[0] == ':'){
        char *splitted = strtok(clone, " ");
        if (splitted != NULL){
            strncpy(prefix, splitted+1, strlen(splitted)+1);
        }else{
            prefix[0] = '\0';
        }
    }else{
        prefix[0] = '\0';
    }
    return prefix;
}

char *get_username(char line[])
{
    char *username = malloc(512);
    char clone[512];
    strncpy(clone, line, strlen(line)+1);
    if (strchr(clone, '!') != NULL){
        char *splitted = strtok(clone, "!");
        if (splitted != NULL){
            strncpy(username, splitted+1, strlen(splitted)+1);
        }else{
            username[0] = '\0';
        }
    }else{
        username[0] = '\0';
    }
    return username;
}

char *get_command(char line[])
{
    char *command = malloc(512);
    char clone[512];
    strncpy(clone, line, strlen(line)+1);
    char *splitted = strtok(clone, " ");

	if (!command) {
		return NULL;
	}

    if (splitted != NULL){
        if (splitted[0] == ':'){
            splitted = strtok(NULL, " ");
        }
        if (splitted != NULL){
            strncpy(command, splitted, strlen(splitted)+1);
        }else{
            command[0] = '\0';
        }
    }else{
        command[0] = '\0';
    }

    return command;
}

char *get_last_argument(char line[])
{
    char *argument = malloc(512);
    char clone[512];
    strncpy(clone, line, strlen(line)+1);
    char *splitted = strstr(clone, " :");
    if (splitted != NULL){
        strncpy(argument, splitted+2, strlen(splitted)+1);
    }else{
        argument[0] = '\0';
    }
    return argument;
}

char *get_argument(char line[], int argno)
{
    char *argument = malloc(512);
    char clone[512];
    strncpy(clone, line, strlen(line)+1);
    
    int current_arg = 0;
    char *splitted = strtok(clone, " ");
    while (splitted != NULL){
        if (splitted[0] != ':'){
            current_arg++;
        }
        if (current_arg == argno+1){
            strncpy(argument, splitted, strlen(splitted)+1);
            return argument;
        }
        splitted = strtok(NULL, " ");
    }
    
    if (current_arg != argno){
        argument[0] = '\0';
    }
    return argument;
}

