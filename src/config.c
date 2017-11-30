#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void log_with_date(char line[])
{
    char date[50];
    struct tm *current_time;

    time_t now = time(0);
    current_time = gmtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", current_time);

    printf("[%s] %s\n", date, line);
}

void log_to_file(char line[], FILE *logfile)
{
    char date[50];
    struct tm *current_time;

    time_t now = time(0);
    current_time = gmtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", current_time);

    fprintf(logfile, "[%s] %s\n", date, line);
    fflush(logfile);
}

char *get_config(char name[])
{
    char *value = malloc(1024);
    FILE *configfile = fopen("config.txt", "r");
    value[0] = '\0';
    if (configfile != NULL){
        while (1){
            char configname[1024];
            char tempvalue[1024];
            int status = fscanf(configfile, " %1023[^= ] = %s ", configname, tempvalue); //Parse key=value
            if (status == EOF){
                break;
            }
            if (strcmp(configname, name) == 0){
                strncpy(value, tempvalue, strlen(tempvalue)+1);
                break;
            }
        }
        fclose(configfile);
    }
    return value;
}
