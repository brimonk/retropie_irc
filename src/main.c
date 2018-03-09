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
#include <time.h>
#include <string.h>

#include <arpa/inet.h>
#include <dirent.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>

#include "config.h"
#include "cstr.h"
#include "constants.h"
#include "data_types.h"
#include "irc.h"
#include "list.h"

#define LIBRARY_DIR "./src/lib"

int mk_socket(str_dict_t *config_ptr, int config_items);
int load_irc_lib(char *lib_dir, list_t **ptr);
int load_lib(list_t **ptr, char *filename);

void error_and_exit(char *s);

int read_line(int sock, int n, char *buffer);
int read_line_nonblock(int sock, int n, char *buffer);
int parse_irc_lines(char ***dest, char *buf, int n);
int get_mem(void **ptr, int n);
void unload_lib(list_t *ptr);


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

int main(int argc, char **argv)
{
	/* define our array of cstrs */
	int config_items, tmp, socket_desc, lib_num, irc_status;
	char *config_name = "config.txt";
	char logline[BUFLEN], filename[BUFLEN], line[BUFLEN];
	list_t *list_ptr = 0;
	str_dict_t *config_ptr = 0;
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
	lib_num = load_irc_lib(LIBRARY_DIR, &list_ptr);
	printf("Library Number: %d\n", lib_num);

	socket_desc = mk_socket(config_ptr, config_items);
	socket_ptr = &socket_desc;

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

    for (irc_status = IRC_RETURN_OK; irc_status != IRC_RETURN_QUIT;) {
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
        
		if (get_prefix(str_ptr[16].buf, str_ptr[16].len, line)) {
			fprintf(stderr, "error parsing\n");
			continue;
		}

		if (get_username(str_ptr[17].buf, str_ptr[17].len, line)) {
			fprintf(stderr, "error parsing username\n");
			continue;
		}

		if (get_command(str_ptr[18].buf, str_ptr[18].len, line)) {
			fprintf(stderr, "error parsing command\n");
			continue;
		}

		if (get_last_argument(str_ptr[19].buf, str_ptr[19].len, line)) {
			fprintf(stderr, "error parsing argument\n");
			continue;
		}

        char *username = str_ptr[17].buf;
        char *command = str_ptr[18].buf;
        char *argument = str_ptr[19].buf;

        if (strcmp(command, "PING") == 0) {
			strncpy(str_ptr[0].buf, argument, 512);
            irc_send_pong(socket_desc, 5, &str_ptr);
			sprintf(logline, "Got ping. Replying with pong...");
			log_to_file(logline, logfile);
        } else {
			if (get_argument(str_ptr[20].buf, str_ptr[20].len, line, 1)) {
				fprintf(stderr, "error parsing channel\n");
				continue;
			}

			char *channel = str_ptr[20].buf;

			if (strcmp(command, "PRIVMSG") == 0) {

				sprintf(logline, "%s/%s: %s", channel, username, argument);
				log_to_file(logline, logfile);

				/* meat and potatoes */
				irc_status = irc_privmsg(socket_desc, line, list_ptr, str_ptr); 

				sprintf(filename, "%s.log.txt", channel);
				freopen(filename, "a+", logfile);
				log_to_file(logline, logfile);
			} else if (strcmp(command, "JOIN") == 0) {
				sprintf(logline, "%s joined %s.", username, channel);
				log_to_file(logline, logfile);
				
				sprintf(filename, "%s.log.txt", channel);
				freopen(filename, "a+", logfile);
				log_to_file(logline, logfile);
			} else if (strcmp(command, "PART") == 0) {
				sprintf(logline, "%s left %s.", username, channel);
				log_to_file(logline, logfile);
				
				sprintf(filename, "%s.log.txt", channel);
				freopen(filename, "a+", logfile);
				log_to_file(logline, logfile);
			}
		}

		cstr_clear(str_ptr, 64, BUFLEN);
		memset(logline, 0, BUFLEN);
    }

	/* quit irc */
	strncpy(str_ptr[0].buf, "I was told to...", str_ptr[0].len);
	irc_send_quit(socket_desc, 1, &str_ptr);

	fclose(logfile);
	shutdown(socket_desc, SHUT_RDWR);

	/* unload our libraries and free some mem */
	unload_lib(list_ptr);
	cstr_free(str_ptr, 64);
	config_free(&config_ptr, 64);

	return 0;
}

int read_line(int sock, int n, char *buffer)
{
    int length = 0;

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

int load_irc_lib(char *lib_dir, list_t **list_ptr)
{
	/* 
	 * spin through lib_dir, loading each *.so into the ptr list 
	 * returns number of lib_tbl_t we fully loaded
	 */

	DIR *dirptr;
	struct dirent *dir_struct;
	int returnval;
	char *dot_ptr;
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

				returnval += load_lib(list_ptr, buf);
			}
		}

		closedir(dirptr);
	} else {
		printf("Couldn't open '%s'\n", lib_dir);
	}

	return returnval;
}

int load_lib(list_t **ptr, char *filename)
{
	void *objhandle;
	lib_cmd_t *dictptr;
	lib_tbl_t *copy;
	list_t *data;
	int i = 0, val = 0;
	char *error;

	/* begin by loading objhandle with the shared object handle */
	objhandle = dlopen(filename, RTLD_NOW);

	if (!objhandle) {
		error_and_exit(dlerror());
	}

	/* now load the table each shared object must have */
	*(void **) (&dictptr) = dlsym(objhandle, "entry_dict");
	error = dlerror();
	if (error != NULL) {
		error_and_exit(error);
	}

	/* iterate through the table, loading each function into ptr */
	for (i = 0; dictptr[i].text != NULL && dictptr[i].funcptr && dictptr[i].type; i++) {
		/* dump the data into another dynamic structure */
		copy = malloc(sizeof(lib_tbl_t));
		if (copy) {
			if (get_mem((void **)&copy->text, BUFLEN)) {
				error_and_exit("Couldn't get memory in load_lib, text\n");
			}

			strcpy(copy->text, dictptr[i].text);
			copy->objhandle = objhandle;
			copy->funcptr = dictptr[i].funcptr;
			copy->type = dictptr[i].type;

			data = list_create(copy); /* make a new node */
			*ptr = list_add(*ptr, data);
			val++;
		} else {
			error_and_exit("Couldn't get memory in load_lib\n");
		}
	}

	return val;
}

void unload_lib(list_t *ptr)
{
	/* recursively walk through the list, freeing all of the objects  */
	list_t *curr;
	lib_tbl_t *a;

	while (ptr != NULL) {
		curr = ptr;
		ptr = ptr->next;

		a = (lib_tbl_t *)curr->data;

		if (a) {
			free(a->text);

			dlclose(a->objhandle);
			dlerror();

			free(a);
		}

		free(curr);
	}
}

int get_mem(void **ptr, int n)
{
	int i;
	*ptr = malloc(n);

	i = 0;
	if (!(*ptr)) {
		i = 1;
	}

	return i;
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
