/*
 * Brian Chrzanowski
 * Wed Mar 07, 2018 02:47
 *
 * Filling out a !quit command
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "../data_types.h"

int quit(char *dest, int dest_size, cstr_t **in);

lib_cmd_t entry_dict[] = {
	{"!quit", quit, IRC_TYPE_CALLRESPONSE},
	{NULL, NULL, 0} /* signifies no more entries */
};

int quit(char *dest, int dest_size, cstr_t **in)
{
	int returnval;
	char *message = "User %s requested we quit";
	char *user;

	/* boilerplate */
	user  = (*in)[1].buf;
	returnval = IRC_RETURN_QUIT;

	/* check the return size */
	if (strlen(message) - 2 + strlen(user) < dest_size) {
		sprintf(dest, message, user);
	} else {
		returnval = IRC_RETURN_NOMEM;
	}

	return returnval;
}
