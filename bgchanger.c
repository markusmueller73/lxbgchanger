#include "bgchanger.h"

int main ( int argc, char *argv[] ){
    
    char config_file[PATH_MAX+NAME_MAX];
    struct config_s *config = malloc ( sizeof ( struct config_s ) );

    strcpy( config_file , getenv( "XDG_CONFIG_HOME" ) );
    add_slash( config_file );
    #ifdef DEBUG
    strcat( config_file, "bgchange_dbg.conf" );
    #else
    strcat( config_file, APP_CONF );
    #endif
    
    config->timeout     = 0;
    config->recursive   = 0;
    
    if ( argc == 1 ){
        if ( load_config( config_file, config ) != 0 ){
            fprintf( stdout, "%s - %s.\n",APP_NAME, APP_DESC );
            fprintf( stdout, "Type '%s -h' for help!\n", argv[0] );
            return EXIT_FAILURE;
        }
    }
    else{
        int option = 0;
        opterr = 0;
        
        load_config( config_file, config );
        
        while ( ( option = getopt ( argc, argv, "d:hm:rt:v" ) ) != -1 ){
        
            switch ( option ){
                case 'd':
                case 'D':
                    strcpy ( config->last_dir, optarg );
                    dbg( "current dir: %s", config->last_dir );
                    break;
                
                case 'h':
                case 'H':
                    show_help( argv[0] );
                    return EXIT_FAILURE;
                case 'm':
                case 'M':
                    strcpy ( config->wp_mode, optarg );
                    lower_case( config->wp_mode );
                    dbg( "current wallpaper mode: %s", config->wp_mode );
                    break;
                case 'r':
                case 'R':
                    config->recursive = 1;
                    dbg( "scan dirs recursive" );
                    break;
                case 't':
                case 'T':
                    config->timeout = atoi( optarg );
                    dbg( "set timeout to %d mins", config->timeout );
                    break;
                case 'v':
                case 'V':
                    fprintf( stdout, "Version %d.%d\n\n", MAJOR_VERSION, MINOR_VERSION );
                    return EXIT_FAILURE;
                case '?':
                    if ( optopt == 'd' || optopt == 'D' || optopt == 'm' || optopt == 'M' || optopt == 't' || optopt == 'T' ){
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
    }
       
    if ( is_dir( config->last_dir ) == 0 ){
        add_slash( config->last_dir );
    }
    else {
        fprintf( stderr, "The directory '%s' did not exist.\n", config->last_dir );
        return EXIT_FAILURE;
    }
    
    if ( config->timeout <= 0 ){
        config->timeout = 10;
    }
    
    if ( strlen( config->wp_mode ) == 0 ){
        strcpy( config->wp_mode , "stretch" );
    }
    
    struct img_node_s *lst_start = NULL;
    struct img_node_s *lst_cur = NULL;
    
    if ( check_dir ( config->last_dir , &lst_start , config->recursive ) == 0 ){
        fprintf( stderr, "There are no compatible images in the directory.");
        return EXIT_FAILURE;
    }
    lst_cur = lst_start;
    
    #ifdef DEBUG
    lst_print( lst_cur );
    #endif
    
    while ( lst_cur->next != NULL ) {
        if ( strcmp( config->last_wp, get_file_name( lst_cur->filename ) ) == 0 ){
            dbg( "select last image: %s", config->last_wp );
            break;
        }
        lst_cur = lst_cur->next;
    }
    
    char switch_command[PATH_MAX+NAME_MAX+25];    
    while ( 1 ){
        
        sprintf( switch_command, "pcmanfm --set-wallpaper=\"%s\" --wallpaper-mode=%s", lst_cur->filename, config->wp_mode );
        strcpy( config->last_wp, get_file_name( lst_cur->filename ) );
        save_config( config_file, config );
        
        dbg( "next command: %s", switch_command );
        
        #ifndef DEBUG
        if ( system( switch_command ) == -1 ){
            fprintf( stderr, "Execution error for command '%s'\n", switch_command );
        }
        #endif
        
        #ifdef DEBUG
        sleep ( (unsigned int)config->timeout );
        #else
        sleep ( ( (unsigned int)config->timeout ) * 60 );
        #endif
        
        if ( lst_cur->next == NULL ){
            lst_cur = lst_start;
        }
        else{
            lst_cur = lst_cur->next;
        } 
        
    }
    
    save_config( config_file, config );
        
    return EXIT_SUCCESS;
}

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

char *lower_case ( char *str ){
    int i = 0, len = strlen( str );
    for ( i = 0 ; i <= len ; i++ ){
        if ( str[i] >= 'A' && str[i] <= 'Z' )
            str[i] += ( 'a' - 'A' );
    }
    return str;
}

char *upper_case ( char *str ){
    int i = 0, len = strlen( str );
    for ( i = 0 ; i <= len ; i++ ){
        if ( str[i] >= 'a' && str[i] <= 'z' )
            str[i] -= ( 'a' - 'A' );
    }
    return str;
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
    fprintf( stdout, "%s - %s.\n\n", APP_NAME, APP_DESC );
    fprintf( stdout, "Usage:\n%s [OPTIONS]\n\n", prgname );
    fprintf( stdout, "Options:\n");
    fprintf( stdout, "-d <PATH>\tdirectory name of the background images\n" );
    fprintf( stdout, "-h\t\tshow this help text\n" );
    fprintf( stdout, "-m <MODE>\tset wallpaper mode (see available modes at bottom)\n" );
    fprintf( stdout, "-r\t\tcheck the directory recursive\n" );
    fprintf( stdout, "-t <MINUTES>\ttimeout between images in minutes (default is 10 mins)\n" );
    fprintf( stdout, "-v\t\tshow the version and quit\n\n" );
    fprintf( stdout, "Available wallpaper modes:\n" );
    fprintf( stdout, " center\t\tplace on center of monitor\n" );
    fprintf( stdout, " crop\t\tstretch and crop to fill monitor\n" );
    fprintf( stdout, " fit\t\tstretch to fit monitor size\n" );
    fprintf( stdout, " screen\t\tstretch to fill entire screen\n" );
    fprintf( stdout, " stretch\tstretch to fill entire monitor (default)\n" );
    fprintf( stdout, " tile\t\ttile to fill entire monitor\n\n" );
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
    //cur->next->filename = malloc( sizeof( char ) * (unsigned int)strlen( txt ) );
    cur->next->filename = calloc( (unsigned int)strlen( txt ) + 1 , sizeof( char ) );
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
            sscanf( line, "LastWallpaperMode=%s", myconf->wp_mode);
            sscanf( line, "LastTimeout=%ld", &myconf->timeout);
            sscanf( line, "CheckRecursive=%d", &myconf->recursive);
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
    fprintf( file_p, "LastWallpaperMode=%s\n", myconf->wp_mode );
    fprintf( file_p, "LastTimeout=%ld\n", myconf->timeout );
    fprintf( file_p, "CheckRecursive=%d\n", myconf->recursive );
    fclose( file_p );
    dbg( "saved config successfully" );
    return 0;
}

int check_dir( char *image_dir , struct img_node_s **lst , int recursive ){

    int images = 0;
    struct dirent *direntry = NULL;     
    DIR *dirp = opendir( image_dir );
    
    add_slash( image_dir );
    
    while ( ( direntry = readdir( dirp ) ) != NULL) {
        
        if ( strcmp ( direntry->d_name, "." ) != 0 && strcmp ( direntry->d_name, ".." ) != 0 ){
            char fullpathandfile[PATH_MAX + NAME_MAX];
            
            strcpy( fullpathandfile , image_dir );
            strcat ( fullpathandfile , direntry->d_name );
            dbg( "found file: %s", direntry->d_name );
            
            if ( is_dir( fullpathandfile ) == 0 ){
                if ( recursive == 1 ){
                    images += check_dir( fullpathandfile , &*lst , recursive );
                }
            }
            else{
                char *file_ext = lower_case( get_file_ext( direntry->d_name ) );
                if ( strcmp ( file_ext, "bmp" ) == 0 || strcmp ( file_ext, "png" ) == 0 || strcmp ( file_ext, "jpeg" ) == 0 || strcmp ( file_ext, "jpg" ) == 0 ){
                    lst_append( &*lst, fullpathandfile );
                    images++;
                }
            }
        }
        
    }
    closedir ( dirp );
    
    return images;
}

