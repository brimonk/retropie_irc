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

#include "data_types.h"
#include "irc.h"
#include "config.h"

int read_line(int sock, char buffer[]);
char *get_prefix(char line[]);
char *get_username(char line[]);
char *get_command(char line[]);
char *get_last_argument(char line[]);
char *get_argument(char line[], int argno);

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
	int i;
	cstr_t *str_ptr, str_list[5];
	str_ptr = &str_list[0];

	for (i = 0; i < 5; i++) {
		str_list[i].buf = malloc(512);
		if (str_list[i].buf) {
			str_list[i].len = 512;
			memset(str_list[i].buf, 0, 512);
		}
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

	strncpy(str_list[0].buf, nick, 512);

	/* just initializing the irc connection */
    irc_set_nick(socket_desc, 5, &str_ptr);
    irc_send_user(socket_desc, 5, &str_ptr);

	strncpy(str_list[0].buf, channels, 512);
    irc_join_channel(socket_desc, 5, &str_ptr);

    free(nick);
    free(channels);

    FILE *logfile = fopen("log/bot.log.txt", "a+");

    while (1){
        char line[512];
        read_line(socket_desc, line);
        
        char *prefix = get_prefix(line);
        char *username = get_username(line);
        char *command = get_command(line);
        char *argument = get_last_argument(line);

        if (strcmp(command, "PING") == 0){
			strncpy(str_list[0].buf, argument, 512);
            irc_send_pong(socket_desc, 5, &str_ptr);
            log_with_date("Got ping. Replying with pong...");
        }else if (strcmp(command, "PRIVMSG") == 0){
            char logline[512];
            char *channel = get_argument(line, 1);

            sprintf(logline, "%s/%s: %s", channel, username, argument);
            log_with_date(logline);

            char filename[500];
            sprintf(filename, "%s.log.txt", channel);
            freopen(filename, "a+", logfile);
            log_to_file(logline, logfile);
            free(channel);
        }else if (strcmp(command, "JOIN") == 0){
            char logline[512];
            char *channel = get_argument(line, 1);
            sprintf(logline, "%s joined %s.", username, channel);
            log_with_date(logline);
            
            char filename[500];
            sprintf(filename, "%s.log.txt", channel);
            freopen(filename, "a+", logfile);
            log_to_file(logline, logfile);
            free(channel);
        }else if (strcmp(command, "PART") == 0){
            char logline[512];
            char *channel = get_argument(line, 1);
            sprintf(logline, "%s left %s.", username, channel);
            log_with_date(logline);
            
            char filename[500];
            sprintf(filename, "%s.log.txt", channel);
            freopen(filename, "a+", logfile);
            log_to_file(logline, logfile);
            free(channel);
        }

        free(prefix);
        free(username);
        free(command);
        free(argument);
    }
}


int read_line(int sock, char buffer[])
{
    int length = 0;
    while (1){
        char data;
        int result = recv(sock, &data, 1, 0);
        if ((result <= 0) || (data == EOF)){
            perror("Connection closed");
            exit(1);
        }
        buffer[length] = data;
        length++;
        if (length >= 2 && buffer[length-2] == '\r' && buffer[length-1] == '\n'){
            buffer[length-2] = '\0';
            return length;
        }
    }
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
