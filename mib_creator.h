#include <stdio.h>
#include <sys/syscall.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct LEAF_STRUCT{
    char *name;
    char *type;
    char *def_val;
    char *description;
    char *access;

    struct LEAF_STRUCT *next;
}LEAF;

typedef struct OBJECT_STRUCT{
    char *type; //scalar -table 
    char *table_name;
    char *table_no;
    char *table_type; //static - dynamic
    char *table_entry_count;

    LEAF *leaf_head;
    struct OBJECT_STRUCT *next;
}OBJECT;

typedef struct GROUP_STRUCT{
    char *name;
    char *no;

    OBJECT *object_head;
    struct GROUP_STRUCT *next;
}GROUP;


typedef struct MIB_STRUCT{
    char *mib_file_name ;   //Mib File Name
    char *mib_OID ;   //Mib OID number
    char *enterprise_name ;   //Enreprise Name
    char *enterprise_no ;  //Enreprise No
    char *device_type ;   //Device Type (switch/router/wifi/escda)
    char *project_name;    //Project Name

    GROUP *group_head;
}MIB;

#define KB_MEMORY 1024

char *BUFFER_READ = NULL;
char input_file[KB_MEMORY];
MIB *mib;

int is_equal_string(char *str1, char *str2, int case_sensitive);

//frees everything
void free_everything_and_terminate(){
    GROUP *group_iter, *temp_group;
    OBJECT *object_iter, *temp_object;
    LEAF *leaf_iter, *temp_leaf;

    if(mib != NULL){
        group_iter = mib->group_head;
        while (group_iter != NULL){
            object_iter = group_iter->object_head;
            while (object_iter != NULL){
                if(!is_equal_string(object_iter->type,"scalar",0)){
                    free(object_iter->table_name);
                    free(object_iter->table_no);
                    free(object_iter->table_type);
                    free(object_iter->table_entry_count);
                }
                free(object_iter->type); 

                leaf_iter = object_iter->leaf_head;
                while (leaf_iter != NULL){
                    free(leaf_iter->name); 
                    free(leaf_iter->def_val); 
                    free(leaf_iter->access); 
                    free(leaf_iter->description);
                    free(leaf_iter->type); 
                    temp_leaf = leaf_iter;
                    leaf_iter = leaf_iter->next;
                    free(temp_leaf);
                }
                temp_object = object_iter;
                object_iter = object_iter->next;
                free(temp_object);
            }
            if(object_iter != NULL) free(object_iter);
            temp_group = group_iter;
            group_iter = group_iter->next;
            free(temp_group);
        }
        free(mib->device_type);
        free(mib->enterprise_name);
        free(mib->enterprise_no);
        free(mib->mib_file_name);
        free(mib->mib_OID);
        free(mib->project_name);
        free(mib);
    }
    if(BUFFER_READ != NULL) free(BUFFER_READ);

    exit(EXIT_SUCCESS);
}

//file reader that uses file lock, it reads once with read() command
void read_file(const char *filePath){

    struct stat st; 
    int file_size = 0;
    if (stat(filePath, &st) == 0){      //taking file size
        file_size = st.st_size;
    }else{
        perror("File not found \n");
        free_everything_and_terminate();  
    }
    if(file_size == 0){
        perror("File is empty!! \n");
        free_everything_and_terminate();
    }

    BUFFER_READ = (char*)malloc((file_size+5)*file_size); //allocating buffer memory

    int fd = open( filePath , O_RDWR);

    if(fd == -1){
        perror("Error: File cannot be opened.\n");
        free_everything_and_terminate();
    }

    struct flock lock;
    //locking file
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    if(fcntl(fd,F_SETLKW,&lock) == -1){
        perror("Error: Filelock didn't locked\n");
        close (fd);
        free_everything_and_terminate();
    }

    //reading file
    if(read(fd, BUFFER_READ, file_size ) <=0){
        printf("READ ERROR\n");
    }

    //unlocking file
    lock.l_type = F_UNLCK;
    if(fcntl(fd,F_SETLKW,&lock) == -1){
        perror("Error: Filelock didn't unlocked\n");
        free_everything_and_terminate();
    }
    close (fd);

    
}

//commandline argumant parser
void get_opt_handler(int argc, char **argv){
    int opt = 0;
    int count = 0;

    if(argc < 2){
        perror("WARNING: Wrong argumant type given.\nYou should give argumants like: ./mib_creator -i inputfile.ods \n");
        free_everything_and_terminate();
    }
    while((opt = getopt(argc, argv, "i:")) != -1) { 
        switch(opt){ 
            case 'i': 
                if(PRINT_DETAILS)printf("input file: %s\n", optarg); 
                
                strcpy(input_file, optarg);
                count++;
                break; 
            case '?':
                perror("You should give argumants like: ./mib_creator -i inputfile.ods\n");
                fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
                free_everything_and_terminate();

            default:
                perror("You should give argumants like: ./mib_creator -i inputfile.ods\n");
                fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
                free_everything_and_terminate();
        } 
    } 
      
    if(count > 1){
        perror("WARNING: Wrong argumant type given.\nYou should give argumants like: ./mib_creator -i inputfile.ods \n");
        free_everything_and_terminate();
    }
} 

//controlls if file exist or not
int is_file_exist(char *path){
    if( access( path, F_OK ) == -1)return 0;
    return 1;
}

//converts ods files to csv files (use popen) (control that if file exist adn name true)
void convert_ods_to_csv(char *path){ 
    char cwd[KB_MEMORY];
    char command[KB_MEMORY];

    if (getcwd(cwd, sizeof(cwd)) != NULL){
        if(PRINT_DETAILS) printf("Current working dir: %s\n", cwd);
    }else {
       perror("getcwd() error\n");
       free_everything_and_terminate();
    }

    if(!is_file_exist(input_file)){
        perror("Input file is not found, Please check your filepath in commandline .\nTerminating...");
        exit(EXIT_SUCCESS);
    }

    sprintf(command,"libreoffice --convert-to csv --outdir %s %s",cwd, path);
    system(command);  //popen                                                       //!!!!!!!!!DANGEROUS
}

//increments cursor to end of the given delimeter 
void increment_cursor_EODelimeter(int *cursor, char delimeter){
    while (BUFFER_READ[*cursor] != delimeter && BUFFER_READ[*cursor] != '\0')  ++(*cursor);
    ++(*cursor);
}

//increments cursor to end of the given delimeter and takes that string while incremented
char* take_string_EODelimeter(int *cursor, char delimeter){
    char *str = (char*)malloc(1024);
    int i = 0;
    while (BUFFER_READ[*cursor] != '\n' && BUFFER_READ[*cursor] != delimeter && BUFFER_READ[*cursor] != '\0') 
        str[i++] = BUFFER_READ[(*cursor)++];
    
    str[i] = '\0';
    (*cursor)++;
    return str;
}

//parses mib file requirements
void parse_mib_requirements(int *cursor, char delimeter){
    increment_cursor_EODelimeter(cursor,'\n');
    increment_cursor_EODelimeter(cursor,delimeter);
    mib->mib_file_name = take_string_EODelimeter(cursor, delimeter);
    increment_cursor_EODelimeter(cursor,'\n');
    increment_cursor_EODelimeter(cursor,delimeter);
    mib->mib_OID = take_string_EODelimeter(cursor, delimeter);
    increment_cursor_EODelimeter(cursor,'\n');
    increment_cursor_EODelimeter(cursor,delimeter);
    mib->enterprise_name = take_string_EODelimeter(cursor, delimeter);
    increment_cursor_EODelimeter(cursor,'\n');
    increment_cursor_EODelimeter(cursor,delimeter);
    mib->enterprise_no = take_string_EODelimeter(cursor, delimeter);
    increment_cursor_EODelimeter(cursor,'\n');
    increment_cursor_EODelimeter(cursor,delimeter);
    mib->device_type = take_string_EODelimeter(cursor, delimeter);
    increment_cursor_EODelimeter(cursor,'\n');
    increment_cursor_EODelimeter(cursor,delimeter);
    mib->project_name = take_string_EODelimeter(cursor, delimeter);
    increment_cursor_EODelimeter(cursor,'\n');

    if(strlen(mib->device_type) == 0 || strlen(mib->enterprise_name) == 0 || strlen(mib->enterprise_no) == 0 || strlen(mib->mib_file_name) == 0 || strlen(mib->project_name) == 0 ){
        perror("*********************************\nMIB requirements cannot be blank\n**********************************\n");
        free_everything_and_terminate();
    }

    if(PRINT_DETAILS){
        printf("REQUIREMENTS: \n");
        printf("Mib file name: %s \n", mib->mib_file_name);
        printf("Mib OID: %s \n", mib->mib_OID);
        printf("Mib enterprise name: %s \n", mib->enterprise_name);
        printf("Mib enterprise no: %s \n", mib->enterprise_no);
        printf("Mib device type: %s \n", mib->device_type);
        printf("Mib project name: %s \n", mib->project_name);
    }
}

//is end of file
int is_EOF(int *cursor){
    if(BUFFER_READ[*cursor] == '\0') return 1;
    return 0;
}

int is_equal_char(char ch1, char ch2, int case_sensitive){
    if(case_sensitive) return ch1 == ch2;
    return tolower(ch1) == tolower(ch2);
}

void increment_cursor_to_string(char *str, int *cursor, int case_sensitive){
    int counter = 0;
    while (counter != strlen(str) && BUFFER_READ[*cursor] != '\0')
        if(is_equal_char(BUFFER_READ[(*cursor)++], str[counter], case_sensitive)) counter++;
        else counter = 0;
}

int is_equal_string(char *str1, char *str2, int case_sensitive){
    if(str1 == NULL || str2 == NULL || strlen(str1) != strlen(str2)) return 0;
    int counter = 0;
    while (counter != strlen(str1))
        if(is_equal_char(str1[counter], str2[counter], case_sensitive)) counter++;
        else return 0;

    return 1;    
}

//checks the line if that line contains that spesific string without moving cursor
int has_line_string(int *cursor, char *str, int case_sensitive){
    int counter = 0;
    int i = *cursor;
    while (counter != strlen(str) && BUFFER_READ[i] != '\0' && BUFFER_READ[i] != '\n')
        if(is_equal_char(BUFFER_READ[i++], str[counter], case_sensitive)) counter++;
        else counter = 0;

    return counter == strlen(str);
}

//checks if line is blank or not
int is_line_blank(int *cursor){
    return has_line_string(cursor, ",,,,,,,,,,",0);
}

//stores the leaves if there are any
void store_leaves(int *cursor, OBJECT *object_iter, LEAF *leaf_iter, char delimeter, int max_count){
    int count = 0;
    while(is_line_blank(cursor)) increment_cursor_EODelimeter(cursor,'\n');

    while(has_line_string(cursor, "leaf",0)){
        if(object_iter->leaf_head == NULL){
            object_iter->leaf_head = malloc(sizeof(LEAF));
            leaf_iter = object_iter->leaf_head;
        }else{
            leaf_iter->next = malloc(sizeof(LEAF));
            leaf_iter = leaf_iter->next;
        }
        leaf_iter->next = NULL;

        increment_cursor_to_string("leaf", cursor, 0);
        increment_cursor_EODelimeter(cursor,delimeter);
        leaf_iter->name = take_string_EODelimeter(cursor, delimeter);
        increment_cursor_EODelimeter(cursor,delimeter);
        leaf_iter->type = take_string_EODelimeter(cursor, delimeter);
        increment_cursor_EODelimeter(cursor,delimeter);
        leaf_iter->def_val = take_string_EODelimeter(cursor, delimeter);
        increment_cursor_EODelimeter(cursor,delimeter);
        leaf_iter->access = take_string_EODelimeter(cursor, delimeter);
        increment_cursor_EODelimeter(cursor,delimeter);
        leaf_iter->description = take_string_EODelimeter(cursor, delimeter);
        count++;
    }

    if(count != max_count){
        perror("You should write right amount of table entry count");
        free_everything_and_terminate();
    }

}   

//makes main parsing job
void csv_parser(char delimeter){
    int *cursor = malloc(sizeof(int));
    int count = 0;
    *cursor = 0;
    char *tmp;
    GROUP *group_iter = NULL;
    OBJECT *object_iter = NULL;
    LEAF *leaf_iter = NULL;

    //taking and controlling mib file requirements.
    parse_mib_requirements(cursor,delimeter);

    increment_cursor_to_string("group", cursor, 0);
    increment_cursor_EODelimeter(cursor,delimeter);

    if(is_EOF(cursor)){
        perror("There are no group found, Terminating....\n");
        free_everything_and_terminate();
    }

    //creating group
    mib->group_head = malloc(sizeof(GROUP));
    group_iter = mib->group_head;
    group_iter->next = NULL;
    group_iter->object_head = NULL;     
    
    group_iter->name = take_string_EODelimeter(cursor, delimeter);
    increment_cursor_EODelimeter(cursor,delimeter);
    group_iter->no = take_string_EODelimeter(cursor, delimeter);
    increment_cursor_EODelimeter(cursor,'\n');

    while(!is_EOF(cursor)){

        //state 0: controll line for group
        while(is_line_blank(cursor)) increment_cursor_EODelimeter(cursor,'\n');
        if(has_line_string(cursor, "group",0)){
            //allocating group
            group_iter->next = malloc(sizeof(GROUP));
            group_iter = group_iter->next;
            group_iter->next = NULL;
            group_iter->object_head = NULL;  
            
            increment_cursor_to_string("group", cursor, 0);
            increment_cursor_EODelimeter(cursor,delimeter);
            group_iter->name = take_string_EODelimeter(cursor, delimeter);
            increment_cursor_EODelimeter(cursor,delimeter);
            group_iter->no = take_string_EODelimeter(cursor, delimeter);
            increment_cursor_EODelimeter(cursor,'\n');
        }
        
        //eliminating blank lines
        while(is_line_blank(cursor)) increment_cursor_EODelimeter(cursor,'\n');

        //state 1: controll line for Object (if there is just a leaf just itself, than EXIT)
        increment_cursor_to_string("object", cursor, 0);
        increment_cursor_EODelimeter(cursor,delimeter);
        tmp = take_string_EODelimeter(cursor, delimeter);
        if(is_equal_string(tmp,"Table",0)){
            //creates table
            if(group_iter->object_head == NULL){ //if not created then create
                group_iter->object_head = malloc(sizeof(OBJECT));
                object_iter = group_iter->object_head;
            }else{
                object_iter->next = malloc(sizeof(OBJECT));
                object_iter = object_iter->next;
            }
            object_iter->next = NULL;
            object_iter->leaf_head = NULL;
            object_iter->type = malloc(KB_MEMORY);
            strcpy(object_iter->type,"Table");

            increment_cursor_EODelimeter(cursor,delimeter);
            object_iter->table_name = take_string_EODelimeter(cursor, delimeter);
            increment_cursor_EODelimeter(cursor,delimeter);
            object_iter->table_no = take_string_EODelimeter(cursor, delimeter);
            increment_cursor_EODelimeter(cursor,delimeter);
            object_iter->table_type = take_string_EODelimeter(cursor, delimeter);
            increment_cursor_EODelimeter(cursor,delimeter);
            object_iter->table_entry_count = take_string_EODelimeter(cursor, delimeter);

            count = atoi(object_iter->table_entry_count);
            store_leaves(cursor,object_iter,leaf_iter,delimeter,count);
            
        }else if(is_equal_string(tmp,"scalar",0)){
            //creates scalar
            if(group_iter->object_head == NULL){ //if not created then create
                group_iter->object_head = malloc(sizeof(OBJECT));
                object_iter = group_iter->object_head;
            }else{
                object_iter->next = malloc(sizeof(OBJECT));
                object_iter = object_iter->next;
            }

            object_iter->type = malloc(KB_MEMORY);
            strcpy(object_iter->type,"Table");
            increment_cursor_EODelimeter(cursor,'\n');

            store_leaves(cursor,object_iter,leaf_iter,delimeter,1);
        }
        free(tmp);
    }
    

    if(PRINT_DETAILS){
        group_iter = mib->group_head;
        while (group_iter != NULL){
            printf("Group; name: %s, no: %s\n", group_iter->name, group_iter->no);
            object_iter = group_iter->object_head;
            while (object_iter != NULL){
                if(!is_equal_string(object_iter->type,"scalar",0))
                    printf("\tObject; type: %s, name: %s, no: %s, table_type: %s, entry_count: %s\n", object_iter->type, object_iter->table_name, object_iter->table_no, object_iter->table_type, object_iter->table_entry_count);
                else
                    printf("\tObject; type: %s\n", object_iter->type);
                leaf_iter = object_iter->leaf_head;
                while (leaf_iter != NULL){
                   printf("\t\tLeaf; name: %s, type: %s, def value: %s, max access: %s, description: %s\n", leaf_iter->name, leaf_iter->name, leaf_iter->def_val, leaf_iter->access, leaf_iter->description);
                   leaf_iter = leaf_iter->next;
                }
                object_iter = object_iter->next;
            }
            group_iter = group_iter->next;
        }
    }
    free(cursor);
}



//**************************************  MEZAR *************************************
//buraya ilerde gerekli olabilecek kodlari ekliyoruz
//yanlis anlasilmasin burasi cöp degil, burası calisan kod yeri, hic calismamis olan kod mezara girmek icin terminate de olmamistir

/* 

void get_opt_handler(int argc, char **argv){
    int opt;
    int count = 0;

    if(argc < 3){
        printf("WARNING: Wrong argumant type given.\nYou should give argumants like: ./mib_creator -i inputfile.ods\n");
        free_everything_and_terminate();
    }
    while((opt = getopt(argc, argv, "i:o:")) != -1) { 
        switch(opt){ 
            case 'i': 
                printf("input: %s\n", optarg); 
                count++;
                break; 
            case 'o': 
                printf("output: %s\n", optarg); 
                count++;
                break;
            case '?':
                printf("You should give argumants like: ./mib_creator -i inputfile.ods\n");
                fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
                free_everything_and_terminate();

            default:
                printf("You should give argumants like: ./mib_creator -i inputfile.ods\n");
                fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
                free_everything_and_terminate();
        } 
    } 
      
    if(count > 2){
        printf("WARNING: Wrong argumant type given.\nYou should give argumants like: ./mib_creator -i inputfile.ods\n");
        free_everything_and_terminate();
    }
} 

*/