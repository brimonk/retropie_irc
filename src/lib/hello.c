/* 
 * Just responds "hello" to a specific user
 *
 * Because this is the first, "testing" module, I'm setting up specific
 * conventions here.
 *
 * input to the entry() function
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
 * return values
 *		-1			- memory error of some sort
 *		0			- no issues
 *		1			- permissions issue
 *		2			- innappropriate command
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *entry_command = "!hello";

/* therefore, a nice entry function might be formatted like this */
int entry(char *dest, int dest_size, cstr_t **in)
{
	int returnval;
	char *channel, *user, *command, *input;
	channel = (*out)[0].buf;
	user    = (*out)[1].buf;
	command = (*out)[2].buf;
	input   = (*out)[3].buf;
	returnval = 0;

	printf("%s\n", channel);
	printf("%s\n", user);
	printf("%s\n", command);
	printf("%s\n", input);

	if (!dest) {
		return -1;
	}

	if (strncmp(command, entry_command, strlen(entry_command)) != 0) {
		return 2;
	}

	strncpy(dest, "Hello, ", dest_size - strlen(dest));
	strncpy(dest, user, dest_size - strlen(dest));

	return returnval;
}
