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
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include <arpa/inet.h>
#include <dirent.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>

#include "data_types.h"
#include "constants.h"
#include "irc.h"
#include "config.h"

#define LIBRARY_DIR "./src/lib"

int mk_socket(str_dict_t *config_ptr, int config_items);
int cstr_init(cstr_t **ptr, int len, int buflen);
void cstr_clean(cstr_t **ptr, int num);
int load_irc_lib(char *lib_dir, lib_cmd_t **ptr);
int load_lib(lib_cmd_t **ptr, int n, char *filename);

void error_and_exit(char *s);

int read_line(int sock, int n, char *buffer);
int read_line_nonblock(int sock, int n, char *buffer);
int parse_irc_lines(char ***dest, char *buf, int n);
char *get_prefix(char line[]);
char *get_username(char line[]);
char *get_command(char line[]);
char *get_last_argument(char line[]);
char *get_argument(char line[], int argno);


/* define our list of built in irc functions */
irc_cmd_t irc_cmd_list[] = {
	{ "NICK", irc_set_nick },
	{ "USER", irc_send_user },
	{ "JOIN", irc_join_channel },
	{ "PONG", irc_send_pong },
	{ "PRIVMSG", irc_send_message }
};

/* nice signal handler for ctrl-c */
int *socket_ptr;
static void sock_close(int signo)
{
	close(*socket_ptr);
}

int main(int argc, char **argv) {

	/* define our array of cstrs */
	int config_items, tmp, socket_desc, lib_funcs;
	char *config_name = "config.txt";
	char logline[BUFLEN], filename[BUFLEN], line[BUFLEN];
	lib_cmd_t *lib_ptr;
	str_dict_t *config_ptr;
	cstr_t *str_ptr;

	if (signal(SIGTERM, sock_close) == SIG_ERR) {
		error_and_exit("Error occurred while setting signal handler");
	}

	if ((config_items = config_load(&config_ptr, 64, config_name)) < 0) {
		/* if config_load returns -1, we had some issues */
		error_and_exit("Config loading couldn't complete properly");
	}

	if (cstr_init(&str_ptr, 64, BUFLEN)) {
		error_and_exit("Not enough memory");
	}

	/* before an irc connection, we load up our shared object functionality */
	lib_funcs = load_irc_lib(LIBRARY_DIR, &lib_ptr);

	socket_desc = mk_socket(config_ptr, config_items);

    char *nick = get_config(config_ptr, config_items, "nick");
    char *channels = get_config(config_ptr, config_items, "channels");

	strncpy(str_ptr[0].buf, nick, BUFLEN);

	/* just initializing the irc connection */
    irc_set_nick(socket_desc, 5, &str_ptr);
    irc_send_user(socket_desc, 5, &str_ptr);

	strncpy(str_ptr[0].buf, channels, BUFLEN);
    irc_join_channel(socket_desc, 5, &str_ptr);

    FILE *logfile = fopen("log/bot.log.txt", "a+");

	if (!logfile) {
		fprintf(stderr, "Can't read file\n");
		return 1;
	}

    for (;;) {
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

		fprintf(logfile, "%s\n", line);
        
        char *prefix = get_prefix(line);
        char *username = get_username(line);
        char *command = get_command(line);
        char *argument = get_last_argument(line);

        if (strcmp(command, "PING") == 0){
			strncpy(str_ptr[0].buf, argument, 512);
            irc_send_pong(socket_desc, 5, &str_ptr);
            log_with_date("Got ping. Replying with pong...");
        } else if (strcmp(command, "PRIVMSG") == 0){
            char *channel = get_argument(line, 1);
			char *buf = malloc(BUFLEN);

			if (!buf) {
				fprintf(stderr, "memory error\n");
				return 1;
			}

            sprintf(logline, "%s/%s: %s", channel, username, argument);
            log_with_date(logline);

			/* 
			 * check if there's a message to respond to by iterating over
			 * the list of text->func_ptrs.
			 */

			strncpy(str_ptr[0].buf, channel, BUFLEN);
			strncpy(str_ptr[1].buf, "Hello", BUFLEN);
			strncat(str_ptr[1].buf, " ", BUFLEN - strlen(str_ptr[1].buf));
			strncat(str_ptr[1].buf, username, BUFLEN - strlen(str_ptr[1].buf));
			strncat(str_ptr[1].buf, "\r\n", BUFLEN - strlen(str_ptr[1].buf));

			if (strncmp(argument, "!hello", 6) == 0) {
				irc_send_message(socket_desc, 2, &str_ptr);
			}

            sprintf(filename, "%s.log.txt", channel);
            freopen(filename, "a+", logfile);
            log_to_file(logline, logfile);
            free(channel);
        } else if (strcmp(command, "JOIN") == 0){
            char *channel = get_argument(line, 1);
            sprintf(logline, "%s joined %s.", username, channel);
            log_with_date(logline);
            
            sprintf(filename, "%s.log.txt", channel);
            freopen(filename, "a+", logfile);
            log_to_file(logline, logfile);
            free(channel);
        } else if (strcmp(command, "PART") == 0){
            char *channel = get_argument(line, 1);
            sprintf(logline, "%s left %s.", username, channel);
            log_with_date(logline);
            
            sprintf(filename, "%s.log.txt", channel);
            freopen(filename, "a+", logfile);
            log_to_file(logline, logfile);
            free(channel);
        }

		memset(logline, 0, BUFLEN);

		if (prefix) { free(prefix); }
        if (username) { free(username); }
        if (command) { free(command); }
        if (argument) { free(argument); }
    }

	fclose(logfile);
}

int read_line(int sock, int n, char *buffer) {
    size_t length = 0;

    while (1) {
        char data;
        int result = recv(sock, &data, 1, 0);

        if ((result <= 0) || (data == EOF)){
            perror("Connection closed");
            exit(1);
        }

        buffer[length] = data;
        length++;
        
        if (length >= 2 && buffer[length-2] == '\r' && buffer[length-1] == '\n') {
            buffer[length-2] = '\0';
            return length;
        }
    }
}

int read_line_nonblock(int sock, int n, char *buffer)
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
	int i, ln_idx, tmp, newln_total;

	newln_total = ln_idx = 0;

	for (i = 0; i < n; i++) {
		if (buf[i] == '\n') { /* irc provides \r\n, only count one of them */
			newln_total++;
			buf[i] = '\0';
		}
	}

	for (i = 0; i < newln_total; i++) {
		tmp = strlen(&buf[ln_idx]);

		if (n < ln_idx + tmp) {
			break;
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

int load_irc_lib(char *lib_dir, lib_cmd_t **ptr)
{
	/* 
	 * spin through lib_dir, loading each *.so into the ptr list 
	 * returns number of lib_cmd_ts we fully loaded
	 */

	DIR *dirptr;
	struct dirent *dir_struct;
	char *dot_ptr;
	int returnval;
	char buf[BUFLEN];

	dirptr = opendir(lib_dir);
	returnval = 0;

	if (dirptr) {
		while ((dir_struct = readdir(dirptr)) != NULL) {
			dot_ptr = strrchr(dir_struct->d_name, '.');

			if (strncmp(dot_ptr, ".so", 3) == 0) {
				/* load into our *ptr, providing a full path */
				memset(buf, 0, BUFLEN);

				strcat(buf, lib_dir); /* prepare for dlopen */
				strcat(buf, "/./");
				strcat(buf, dir_struct->d_name);

				printf("%s\n", buf);

				returnval += load_lib(ptr, returnval, buf);
			}
		}

		closedir(dirptr);
	} else {
		printf("Couldn't open '%s'\n", lib_dir);
		returnval = -1;
	}

	return returnval;
}

int load_lib(lib_cmd_t **ptr, int n, char *filename)
{
	void *objhandle;
	lib_cmd_t *dicthandle;
	int i = 0, val = 0;
	char *error, *text;

	/* begin by loading objhandle with the shared object handle */
	objhandle = dlopen(filename, RTLD_NOW);

	if (!objhandle) {
		error_and_exit(dlerror());
	}

	/* now load the table each shared object must have */
	*(void **) (&dicthandle) = dlsym(objhandle, "entry_dict");
	error = dlerror();
	if (error != NULL) {
		error_and_exit(error);
	}

	/* iterate through the table, loading each function into ptr */
	for (i = 0; (text = dicthandle[i].text) != NULL; i++) {
		printf("%s\n", text);
	}

	return val;
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

void cstr_clean(cstr_t **ptr, int num)
{
	int i;

	if (*ptr) {
		for (i = 0; i < num; i++) {
			memset(&((*ptr)[i]), 0, sizeof(cstr_t));
		}
	}
}


int mk_socket(str_dict_t *config_ptr, int config_items)
{
    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1){
       perror("Could not create socket");
       exit(1);
    }

	/* initialize the specific things we need from the config */
    char *ip = get_config(config_ptr, config_items, "server");
    char *port = get_config(config_ptr, config_items, "port");

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));

    if (connect(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror("Could not connect");
        exit(1);
    }

	return socket_desc;
}

void error_and_exit(char *s)
{
	fprintf(stderr, "%s\n", s);
	exit(EXIT_FAILURE);
}
