/*
 * Brian Chrzanowski
 * Tue Nov 28, 2017 00:03
 *
 * Major modifications from sirlogsalot on github.
 *
 * Main features:
 *
 *	- Retropie wiki loading features.
 *	      !wiki  - searches the wiki for keys given as the second word
 *	      !alias - searches the alias table, then returns through the value.
 *	               if there is a wiki alias, the wiki search is performed,
 *	               then completed.
 *
 *  - Administrators
 *        !admin - presents a list of admins to the users
 *
 *  - BadConnection Kicker
 *	      Automatically happens. 
 *
 *	- Pluggable Modules. Each irc command is loaded through a shared object.
 *	  The Makefile compiles each module 
 *
 * TODO::
 *
 *		1. map of irc commands and their related functions. The 'main' loop
 *		   becomes elegant with this and not a chain of if-else statements.
 *		2. Implement regular retropie things.
 *		3. Retry connecting if we weren't given a command to quit.
 *		4. Implement irc modules. Should take in:
 *		      parsed irc command structure
 *		      irc message structure
 *		   and return
 *		      cstr_t - to send over the socket
 *		5. Change the config formula to become a sorted map, to allow for
 *		   
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

#include "data_types.h"
#include "irc.h"
#include "config.h"

int read_line(int sock, int n, char *buffer);
char *get_prefix(char line[]);
char *get_username(char line[]);
char *get_command(char line[]);
char *get_last_argument(char line[]);
char *get_argument(char line[], int argno);
int cstr_init(cstr_t **ptr, int len, int buflen);

#define BUFLEN 512

/* have a global config pointer */
config_t *config_ptr;

/* define our list of built in irc functions */
cmd_t irc_cmd_list[] = {
	{ "NICK", irc_set_nick },
	{ "USER", irc_send_user },
	{ "JOIN", irc_join_channel },
	{ "PONG", irc_send_pong },
	{ "PRIVMSG", irc_set_nick }
};

int main(int argc, char **argv) {

	/* define our array of cstrs */
	int tmp;
	char ***linearr;
	char logline[BUFLEN], filename[BUFLEN], line[BUFLEN];
	cstr_t *str_ptr;

	if (cstr_init(&str_ptr, 64, BUFLEN)) {
		fprintf(stderr, "Not enough memory\n");
		return 1;
	}

    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1){
       perror("Could not create socket");
       exit(1);
    }
    
    char *ip = get_config("server");
    char *port = get_config("port");

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));

    free(ip);
    free(port);

    if (connect(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror("Could not connect");
        exit(1);
    }
    
    char *nick = get_config("nick");
    char *channels = get_config("channels");

	strncpy(str_ptr[0].buf, nick, 512);

	/* just initializing the irc connection */
    irc_set_nick(socket_desc, 5, &str_ptr);
    irc_send_user(socket_desc, 5, &str_ptr);

	strncpy(str_ptr[0].buf, channels, 512);
    irc_join_channel(socket_desc, 5, &str_ptr);

    free(nick);
    free(channels);

    FILE *logfile = fopen("log/bot.log.txt", "a+");

	if (!logfile) {
		fprintf(stderr, "Can't read file\n");
		return 1;
	}

    while (1) {
		memset(line, 0, BUFLEN);
        tmp = read_line(socket_desc, BUFLEN, line);

		if (!tmp) {
			continue;
		}

		/*
		 * if we have bytes to read
		 *   parse each line into a "message"
		 *   iterate over the array of lines
		 *
		 *   parse each one
		 *   log each line
		 *   execute commands for each line where applicable
		 */

		tmp = parse_irc_lines(linearr, line, BUFLEN);
		if (tmp) {
		}

		fprintf(logfile, "%s\n", line);
        
        char *prefix = get_prefix(line);
        char *username = get_username(line);
        char *command = get_command(line);
        char *argument = get_last_argument(line);

		printf("%s\n", line);

		fprintf(logfile, "\n\n");
		fprintf(logfile, "prefix : %s\n", prefix);
		fprintf(logfile, "username : %s\n", username);
		fprintf(logfile, "command : %s\n", command);
		fprintf(logfile, "argument : %s\n", argument);

        // if (strcmp(command, "PING") == 0){
		// 	strncpy(str_ptr[0].buf, argument, 512);
        //     irc_send_pong(socket_desc, 5, &str_ptr);
        //     log_with_date("Got ping. Replying with pong...");
        // } else if (strcmp(command, "PRIVMSG") == 0){
        //     char *channel = get_argument(line, 1);

        //     sprintf(logline, "%s/%s: %s", channel, username, argument);
        //     log_with_date(logline);

        //     sprintf(filename, "%s.log.txt", channel);
        //     freopen(filename, "a+", logfile);
        //     log_to_file(logline, logfile);
        //     free(channel);
        // } else if (strcmp(command, "JOIN") == 0){
        //     char *channel = get_argument(line, 1);
        //     sprintf(logline, "%s joined %s.", username, channel);
        //     log_with_date(logline);
        //     
        //     sprintf(filename, "%s.log.txt", channel);
        //     freopen(filename, "a+", logfile);
        //     log_to_file(logline, logfile);
        //     free(channel);
        // } else if (strcmp(command, "PART") == 0){
        //     char *channel = get_argument(line, 1);
        //     sprintf(logline, "%s left %s.", username, channel);
        //     log_with_date(logline);
        //     
        //     sprintf(filename, "%s.log.txt", channel);
        //     freopen(filename, "a+", logfile);
        //     log_to_file(logline, logfile);
        //     free(channel);
        // }

		// memset(logline, 0, BUFLEN);

        // free(prefix);
        // free(username);
        // free(command);
        // free(argument);
    }

	fclose(logfile);
}


int read_line(int sock, int n, char *buffer)
{
	/* 
	 * read at most n bytes from sock and store into buffer 
	 * returns the number of bytes processed, or -1 on socket error
	 */

	int val, pollval;
	struct pollfd poll_event = {0};

	poll_event.fd = sock;
	poll_event.events = POLLIN;
	val = pollval = 0;

	pollval = poll(&poll_event, 1, 15);

	if (pollval) {
		memset(buffer, 0, n);
		val = recv(sock, buffer, n, 0);

		if (val < 0) {
			return val;
		}

		if (2 <= val && buffer[val - 2] == '\r' && buffer[val - 1] == '\n') {
			buffer[val - 2] = '\0';
		}

		return val;
	}

	return val;
}

int parse_irc_lines(char ***dest, char *buf, int n)
{
	/* parse bytes provided by an irc server into an array of lines */
	int i, newln_total;

	newln_total = return_val = 0;

	for (i = 0; i < n; i++) {
		if (buf[i] == '\n') { /* irc provides \r\n, only count one of them */
			newln_total++;
			buf[i] = '\0';
		}
	}

	return newln_total;
}

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

int cstr_init(cstr_t **ptr, int len, int buflen)
{
	int i, retval;

	retval = 0;
	*ptr = malloc(sizeof(cstr_t) * len);

	if (*ptr) {
		for (i = 0; i < len; i++) {
			(*ptr)[i].len = buflen;
			(*ptr)[i].buf = malloc(buflen);

			if (!(*ptr)[i].buf) {
				retval = 1;
				break;
			}
		}
	}

	return retval;
}
