/* 
 * while the file is called, 'config.h', we also have definitions for functions
 * that deal with logging.
 */

int read_line(int sock, char buffer[]);
void log_with_date(char line[]);
void log_to_file(char line[], FILE *logfile);

