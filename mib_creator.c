#define CSV_DELIMETER_TYPE ','  //delimeter type of csv file
#define PRINT_DETAILS 1

#include "mib_creator.h"

/**
 * @brief you need to create this function. It creates .mib file via using MIB_STRUCTURE
 * 
 */
void mib_file_create(){

}

int main(int argc, char *argv[]){
    //make parameter control ( -i mib.csv) !!!!!ASK before code
    get_opt_handler(argc, argv);

    //converts ods file to csv
    convert_ods_to_csv(input_file);
    
    read_file("mibTalepFormu.csv"); //reads the file and stores into BUFFER_READ
    if(PRINT_DETAILS) printf("\n***********************************************\nREADED FILE \n%s*************************************************\n",BUFFER_READ);

    mib = malloc(sizeof(MIB));  //allocating mib structure
    //make parsing and storing into structures by giving the delimeter charachter
    csv_parser(CSV_DELIMETER_TYPE);

    //create mib file
    mib_file_create();

    //free everything                              //free inside of mib
    free_everything_and_terminate();
    printf("DONE ^-^\n");
}


/*
WHAT WE NEED ?

-> GUI
-> Button on GUI that checks if mib settings proper or not (basically what submit button does)

-> Loading created mib file into device 
*/