#ifndef _BGCHANGER_H_
#define _BGCHANGER_H_

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>

/* comment or uncomment this macro for DEBUG mode */
//#define DEBUG

struct img_node_s {
    char *filename;
    struct img_node_s *next;
};

struct config_s {
    char last_dir[PATH_MAX];
    char last_wp[NAME_MAX];
    long timeout;
    int recursive;
};

void dbg( char *msg , ... );

char* add_slash     ( char * );
char* get_temp_dir  ( char * );
char* get_file_ext  ( char * );
char* get_file_name ( char * );
int is_dir      ( char * );
int is_file     ( char * );
void show_help  ( char * );
void lst_append ( struct img_node_s ** , char * );
void lst_delete ( struct img_node_s ** );
int  lst_count  ( struct img_node_s * );
void lst_print  ( struct img_node_s * );
int load_config ( char * , struct config_s * );
int save_config ( char * , struct config_s * );
int check_dir( char * , struct img_node_s ** , int );


#endif

