/* 
 * while the file is called, 'config.h', we also have definitions for functions
 * that deal with logging.
 */

void log_with_date(char line[]);
void log_to_file(char line[], FILE *logfile);
char *get_config(char name[]);

