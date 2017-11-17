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

//#define DEBUG

struct img_node_s {
    char *filename;
    struct img_node_s *next;
};

struct config_s {
    char last_dir[PATH_MAX];
    char last_wp[NAME_MAX];
    long timeout;
};

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

void dbg( char *msg , ... ){
    #ifdef DEBUG
    va_list args;
    if ( strlen( msg ) != 0 ) {
        fprintf( stdout, "DEBUG - " );
        va_start( args, msg );
        vfprintf( stdout, msg, args );
        va_end ( args );
        fprintf( stdout, "\n" );
    }
    #endif
}

int main ( int argc, char *argv[] ){
    
    char config_file[PATH_MAX+NAME_MAX];
    char image_dir[PATH_MAX];
    char temp_dir[PATH_MAX];
    int timeout = 0;
    int option = 0;
    long timeout_secs = 0;
    struct config_s *config_struct = malloc ( sizeof ( struct config_s ) );
    
    if ( argc == 1 ){
        fprintf( stdout, "Type '%s -h' for help!\n", argv[0] );
        return EXIT_FAILURE;
    }
    
    strcpy( config_file , getenv( "HOME" ) );
    add_slash( config_file );
    strcat( config_file, ".config/bgchange.conf" );
    //strcpy( config_file, "C:\\Users\\MuellerM152\\bgchange.conf" );
    
    opterr = 0;
    while ( ( option = getopt ( argc, argv, "d:ht:" ) ) != -1 ){
        
        switch ( option ){
            case 'd':
            case 'D':
                strcpy ( image_dir, optarg );
                dbg( "current dir: %s", image_dir );
                break;
            
            case 'h':
            case 'H':
                show_help( argv[0] );
                return EXIT_FAILURE;
            
            case 't':
            case 'T':
                timeout = atoi( optarg );
                dbg( "set timeout to %d mins", timeout );
                break;
            
            case '?':
                if ( optopt == 'd' || optopt == 'D' ){
                    fprintf ( stderr, "Option -%c requires an argument.\n", optopt );
                }
                else if ( optopt == 't' || optopt == 'T' ){
                    fprintf ( stderr, "Option -%c requires an argument.\n", optopt );
                }
                else if ( isprint (optopt) ){
                    fprintf (stderr, "Unknown option '-%c'.\n", optopt);
                }
                else{
                    fprintf ( stderr, "Unknown option character '\\x%x'.\n", optopt );
                }
                return EXIT_FAILURE;
            default:
               ;
        }
        
    }
    
    load_config( config_file, config_struct );
    
    if ( is_dir( image_dir ) == 0 ){
        add_slash( image_dir );
        get_temp_dir( temp_dir );
        add_slash( temp_dir );
    }
    else {
        fprintf( stderr, "The directory '%s' did not exist.\n", image_dir );
        return EXIT_FAILURE;
    }
    
    if ( timeout <= 0 ){
        timeout = 10;
    }
    
    strcpy( config_struct->last_dir, image_dir );
    config_struct->timeout = timeout;
    
    #ifndef DEBUG
    timeout_secs = timeout * 60;
    #else
    timeout_secs = timeout;
    #endif

    struct img_node_s *lst_start = NULL;
    struct img_node_s *lst_cur = NULL;
    struct dirent *direntry = NULL;
    
    DIR *dirp = opendir( image_dir );
    while ( ( direntry = readdir( dirp ) ) != NULL) {
        
        if ( strcmp ( direntry->d_name, "." ) != 0 && strcmp ( direntry->d_name, ".." ) != 0 ){
            
            char *file_ext = get_file_ext( direntry->d_name );
            char fullpathandfile[PATH_MAX + NAME_MAX];
            
            strcpy( fullpathandfile , image_dir );
            if ( strcmp ( file_ext, "bmp" ) == 0 || strcmp ( file_ext, "BMP" ) == 0 ){
                strcat ( fullpathandfile , direntry->d_name );
                lst_append( &lst_start, fullpathandfile );
            }
            else if ( strcmp ( file_ext, "png" ) == 0 || strcmp ( file_ext, "PNG" ) == 0 ){
                strcat ( fullpathandfile , direntry->d_name );
                lst_append( &lst_start, fullpathandfile );
            }
            else if ( strcmp ( file_ext, "jpg" ) == 0 || strcmp ( file_ext, "jpeg" ) == 0 || strcmp ( file_ext, "JPG" ) == 0 || strcmp ( file_ext, "JPEG" ) == 0){
                strcat ( fullpathandfile , direntry->d_name );
                lst_append( &lst_start, fullpathandfile );
            }
            
        }
        
    }
    closedir ( dirp );
    
    if ( lst_count ( lst_start ) == 0 ){
        fprintf( stderr, "There are no compatible images in the directory.");
        return EXIT_FAILURE;
    }
    lst_cur = lst_start;
    
    #ifdef DEBUG
    lst_print( lst_cur );
    #endif
    
    if ( strcmp( config_struct->last_dir , image_dir ) == 0 ){
		while ( lst_cur->next != NULL ){
			if ( strcmp( get_file_name( lst_cur->filename ) , config_struct->last_wp ) == 0 ){
				dbg( "selecting last wallpaper: %s", lst_cur->filename );
				break;
			}
			lst_cur = lst_cur->next;
		}	
	}
    
    save_config( config_file, config_struct );
    
    char switch_command[PATH_MAX+NAME_MAX+25];    
    while ( 1 ){
        
        sprintf( switch_command, "pcmanfm --set-wallpaper=%s", lst_cur->filename );
        strcpy( config_struct->last_wp, get_file_name( lst_cur->filename ) );
        save_config( config_file, config_struct );
        
        dbg( "next command: %s", switch_command );
        
        #ifndef DEBUG
        if ( system( switch_command ) == -1 ){
			fprintf( stderr, "Error - cant't execute %s\n", switch_command );
		}
        #endif
        
        sleep ( (unsigned int)timeout_secs );
        
        if ( lst_cur->next == NULL ){
            lst_cur = lst_start;
        }
        else{
            lst_cur = lst_cur->next;
        } 
        
    }
    
    save_config( config_file, config_struct );
        
    return EXIT_SUCCESS;
}

char *add_slash ( char *dir ){
    size_t len = strlen( dir );
    if ( dir[len-1] != '/' ) {
        strcat( dir, "/" );
    }
    return dir;
}

char *get_temp_dir ( char *dir ){
    if ( getenv( "TMPDIR" ) != NULL ){
        strcpy( dir, getenv( "TMPDIR" ) );
    } else if ( getenv( "TEMPDIR" ) != NULL ){
        strcpy( dir, getenv( "TEMPDIR" ) );
    } else if ( getenv( "TMP" ) != NULL ){
        strcpy( dir, getenv( "TMP" ) );
    } else if ( getenv( "TEMP" ) != NULL ){
        strcpy( dir, getenv( "TEMP" ) );
    } else {
        strcpy( dir, "/tmp" );
    }
    add_slash( dir );
    dbg( "found temp dir at %s", dir );
    return dir;
}

int is_dir ( char *dir ){
    struct stat dir_s;
    if ( stat( dir, &dir_s ) == 0 && S_ISDIR( dir_s.st_mode ) )
    {
        return 0;
    }
    return -1;
}

int is_file ( char *file ){
    struct stat file_s;
    if ( stat( file, &file_s ) == 0 && S_ISREG( file_s.st_mode ) )
    {
        return 0;
    }
    return -1;
}

char* get_file_ext ( char *file ){
    char *ext = strrchr( file, '.');
    if( !ext ){
        return "";
    }
    else{
        return ext + 1;
    }
}

char* get_file_name ( char *file ){
    char *filename = strrchr( file, '/' );
    if( !filename ){
        return "";
    }
    else{
        return filename + 1;
    }
}

void show_help ( char *prgname ) {
    fprintf( stdout, "Wallpaperchanger - switch desktop background in a Lubuntu environment.\n\n" );
    fprintf( stdout, "Usage:\n%s [OPTIONS]\n\n", prgname );
    fprintf( stdout, "Options:\n");
    fprintf( stdout, "-d\tdirectory name of the background images\n" );
    fprintf( stdout, "-h\tshow this help text\n" );
    fprintf( stdout, "-t\ttimeout between images in minutes (default is 10 mins)\n" );
}

int lst_init( struct img_node_s **lst, char *txt )
{
  if ( *lst == NULL )
  {
    *lst = malloc( sizeof( struct img_node_s ) );
    (*lst)->filename = malloc( sizeof( char ) * (unsigned int)strlen( txt ) );
    strcpy ( (*lst)->filename, txt );
    (*lst)->next = NULL;
    return 1;
  }
  else
  {
    return 0;
  }
}

void lst_append( struct img_node_s **lst, char *txt )
{
  if ( (lst_init( &*lst, txt )) == 0 )
  {
    struct img_node_s *cur = *lst;
    while ( cur->next != NULL )
    {
      cur = cur->next;
    }
    cur->next = malloc( sizeof( struct img_node_s ) );
    cur->next->filename = malloc( sizeof( char ) * (unsigned int)strlen( txt ) );
    strcpy ( cur->next->filename, txt );
    cur->next->next = NULL;
  }
}

void lst_delete( struct img_node_s **lst ){
    if ( *lst != NULL ){
        struct img_node_s *nxt = NULL;
        while ( (*lst)->next != NULL ){
            nxt = (*lst)->next;
            free ( *lst );
            *lst = nxt;
        }
        free( *lst );
        *lst = NULL;
    }
}

int lst_count( struct img_node_s *lst )
{
  if ( lst != NULL )
  {
    int i = 0;
    struct img_node_s *cur = lst;
    while ( cur->next != NULL )
    {
      ++i;
      cur = cur->next;
    }
    ++i;
    return i;
  }
  else
  {
    return 0;
  }
}

void lst_print( struct img_node_s *lst )
{
  if ( lst != NULL )
  {
    int i = 0;
    struct img_node_s *cur = lst;
    fprintf(stdout, " #\tText\n");
    fprintf(stdout, "--\t--------------------\n");
    while ( cur->next != NULL )
    {
      fprintf(stdout, "%2d\t%s\n", ++i, cur->filename);
      cur = cur->next;
    }
    fprintf(stdout, "%2d\t%s\n", ++i, cur->filename);
    fprintf(stdout, "the list (%p) contains %d entries.\n", lst, i);
  }
  else
  {
    fprintf(stdout, "the list (%p) is empty.\n", lst);
  }
}

int load_config ( char *config_file, struct config_s *myconf ){
    dbg( "loading config file: %s", config_file );
    FILE *file_ptr;
    file_ptr = fopen( config_file , "r" );
    if ( file_ptr == NULL ){
        //fprintf( stderr, "Error - can't open config file '%s'.\n", config_file );
        dbg( "can't open config file '%s'", config_file );
        return -1;
    }
    else{
        char line[PATH_MAX];
        while ( fgets( line, PATH_MAX, file_ptr ) != NULL ){
            sscanf( line, "LastImagePath=%s", myconf->last_dir);
            sscanf( line, "LastWallpaper=%s", myconf->last_wp);
            sscanf( line, "LastTimeout=%ld", &myconf->timeout);
        }
        fclose( file_ptr );
    }
    dbg("current settings: LastImagePath=%s | LastWallpaper=%s | LastTimeout=%ld", myconf->last_dir, myconf->last_wp, myconf->timeout);
    return 0;
}

int save_config ( char *config_file, struct config_s *myconf ){
    FILE *file_p;
    file_p = fopen( config_file , "w+" );
    if ( file_p == NULL ){
        fprintf( stderr, "Error - can't create config file '%s'.\n", config_file );
        return -1;
    }
    fprintf( file_p, "LastImagePath=%s\n", myconf->last_dir );
    fprintf( file_p, "LastWallpaper=%s\n", myconf->last_wp );
    fprintf( file_p, "LastTimeout=%ld\n", myconf->timeout );
    fclose( file_p );
    dbg( "saved config successfully" );
    return 0;
}

