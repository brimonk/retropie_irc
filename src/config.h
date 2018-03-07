/* 
 * while the file is called, 'config.h', we also have definitions for functions
 * that deal with logging.
 */

#include "data_types.h"

void log_to_file(char line[], FILE *logfile);
char *get_config(str_dict_t *ptr, int len, char *key);
int config_load(str_dict_t **ptr, int len, char *file_name);
void config_free(str_dict_t **ptr, int size);
