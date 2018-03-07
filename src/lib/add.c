/*
 * Brian Chrzanowski
 * Wed Mar 07, 2018 02:47
 *
 * adding things through irc
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "../data_types.h"

int add(char *dest, int dest_size, cstr_t **in);

lib_cmd_t entry_dict[] = {
	{"!add", add, IRC_TYPE_CALLRESPONSE},
	{NULL, NULL, 0} /* signifies no more entries */
};

int add(char *dest, int dest_size, cstr_t **in)
{
	int returnval;
	char *message = "%s : %d";
	char *user;
	char *command;
	char *first_op, *second_op;

	/* boilerplate */
	user = (*in)[1].buf;
	command = (*in)[3].buf;
	returnval = IRC_RETURN_OK;

	first_op = command;
	second_op = strchr(first_op, ' ');
	*second_op = '\0';
	second_op++; /* we now point at the second operand*/

	int tmp = atoi(first_op) + atoi(second_op);

	/* check the return size */
	if (strlen(message) - 4 + 16 < dest_size) { /* biggest int is 16 chars?? */
		sprintf(dest, message, user, tmp);
	} else {
		returnval = IRC_RETURN_NOMEM;
	}

	return returnval;
}
