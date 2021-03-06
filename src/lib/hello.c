/* 
 * Just responds "hello" to a specific user
 *
 * Because this is the first, "testing" module, I'm setting up specific
 * conventions here.
 *
 * There are also two included functions, which when someone uses a !hello
 * command, the bot will say hello back to them: hello(), and another command
 * someone can use as an example of an "in context reply", which will allow
 * for a line's state to be checked. IE, a line is entirely upper case,
 * yell at them for writing in all upper case.
 *
 *
 *
 * each shared object must contain an "entry_dict"
 *
 * the input for command - response 
 *
 *		dest		- destination where reponse string is stored
 *		dest_size	- max length of buffer you can write in. YOUR FAULT IF
 *					  YOU FUCK UP
 *		**in		- this is where it gets interesting
 *
 *					  (*in)[0] - channel you're writing in
 *					  (*in)[1] - user you're writing to
 *					  (*in)[2] - a copy of the command we _think_
 *								 you represent
 *					  (*in)[3] - the rest of their input text
 *
 *					  IE
 *						channel = (*in)[0].buf;
 * 						user    = (*in)[1].buf;
 * 						command = (*in)[2].buf;
 * 						input   = (*in)[3].buf;
 *
 * return values, defined in ../data_types.h
 *		IRC_RETURN_OK		- everything is a-ok
 *		IRC_RETURN_NOOUT	- the command executed, but didn't have any output
 *		IRC_RETURN_NOMEM	- not enough memory (malloc fail)
 *		IRC_RETURN_BADPERM	- the passed in user doesn't have permission for
 *							  this function
 *		IRC_RETURN_CMDNOSUPPORT
 *							- this library doesn't support this command
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "../data_types.h"

int hello(char *dest, int dest_size, cstr_t **in);
int no_all_caps(char *dest, int dest_size, cstr_t **in);

lib_cmd_t entry_dict[] = {
	{"!hello", hello, IRC_TYPE_CALLRESPONSE},
	{"", no_all_caps, IRC_TYPE_RESPONSE},
	{NULL, NULL, 0} /* signifies no more entries */
};

/* therefore, a nice entry function might be formatted like this */
int hello(char *dest, int dest_size, cstr_t **in)
{
	int returnval;
	char *user;

	user = (*in)[1].buf;
	returnval = IRC_RETURN_OK;

	if (!dest) {
		returnval = IRC_RETURN_NOMEM;
	} else {
		strncpy(dest, "Hello, ", dest_size - strlen(dest));
		strncat(dest, user, dest_size - strlen(dest));
	}

	return returnval;
}

int no_all_caps(char *dest, int dest_size, cstr_t **in)
{
	/* just a function to check if each byte in the message is uppercase */

	/* boilerplate */
	int returnval, i, upperchars;
	char *user, *input;
	char *message = "%s, please don't write in all caps. IT'S REALLY ANNOYING.";

	user  = (*in)[1].buf;
	input = (*in)[3].buf;
	returnval = IRC_RETURN_OK;

	for (i = 0, upperchars = 0; i < strlen(input); i++) {
		if (isupper(input[i])) {
			upperchars++;
		}
	}

	/* if 90% of your message is uppercase */
	if ((double)upperchars / (double)strlen(input) > .8) {
		/* check if we have enough space */
		if (dest_size < strlen(message) - 2 + strlen(user)) {
			returnval = IRC_RETURN_NOMEM;
		} else {
			sprintf(dest, message, user);
		}
	}

	return returnval;
}

