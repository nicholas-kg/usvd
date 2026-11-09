#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"

/* Τα αρχεία βάσης δημιουργούνται στον κατάλογο DBs/ του project */
#define DB_FILE "DBs/myheap.db"

/*
 * O driver της ασκησης.
 *
 * Το εκτελέσιμο πρέπει να διαβάζει τις εγγραφές που αποθηκεύει από ένα αρχείο
 * κειμένου, το οποίο υποδεικνύεται με τη σημαία -i:
 *
 *     ./bin/myheap -i inputs/records.txt
 *
 * Κάθε γραμμή του αρχείου περιέχει μία εγγραφή, με τα πεδία χωρισμένα με κενά
 * και στη σειρά που έχουν στη δομή Record:
 *
 *     id firstname lastname postcode city
 *     151 Dimitra Stasinou 12376 Ioannina
 *
 * Ο driver θα πρέπει να δημιουργεί το αρχείο σωρού, να εισάγει σε αυτό τις
 * εγγραφές του αρχείου εισόδου, και να επιδεικνύει τη λειτουργία των
 * συναρτήσεων του API.
 */

 /*
  * Make sure that after each implementation you delete the produced file DBs/myheap.db
  * since subsequent calls of the driver will terminate with a failure since the file the
  * program tries to create already exists. (Or run this without deleting the file to test
  * if it returns failue as expected).
 */

int main(int argc, char *argv[]) {
    if(BF_Init(LRU) != BF_OK){
        fprintf(stderr, "Error initializing BF level\n");
        return EXIT_FAILURE;        
    }
    FILE *input_file = NULL;
    int ret = EXIT_SUCCESS;
    //Sanity checks for arguments' form
    if (argc != 3){
        fprintf(stderr, "Usage %s -i <input_file>\n", argv[0]);
        ret = EXIT_FAILURE;
        goto end;
    }
    if (strcmp(argv[1], "-i") != 0) {
        fprintf(stderr, "Usage %s -i <input_file,txt>\n", argv[0]);
        ret = EXIT_FAILURE;
        goto end;
    }
    input_file = fopen(argv[2], "r");
    if (input_file == NULL) {
        fprintf(stderr, "Error opening input file\n");
        return EXIT_FAILURE;
    }

    const char* hpfile = DB_FILE;
    if (HP_CreateFile(hpfile) != HP_OK){
        fprintf(stderr, "Unable to create heap file. File probably exists already\n");
        ret = EXIT_SUCCESS;
        goto end;
    }
    int fileDesc;
    if (HP_OpenFile(hpfile, &fileDesc) != HP_OK) {
        fprintf(stderr, "Couldn't open file\n");
        ret = EXIT_FAILURE;
        goto end;
    }
    Record record;
    //Read lines from the input file which should be properly formatted as explained above
    //The reads are limited to the specified size of fields in record.h
    while( fscanf(input_file, "%d %25s %20s %d %20s",&record.id,record.firstname,
        record.lastname, &record.postcode, record.city) == 5){
        if(HP_InsertEntry(fileDesc, record) != HP_OK){
            fprintf(stderr, "Error during insertion of record with id: %d\n", record.id);
            HP_CloseFile(fileDesc);
            ret = EXIT_FAILURE;
            goto end;
        }
    }
    printf("\nRecords Inserted successfully\nPrinting records inserted now\n\n");
    HP_PrintAllEntries(fileDesc, "id", NULL);
    Record fetched;
    printf("\n");
    if (HP_GetEntry(fileDesc, 1, 2, &fetched) == HP_OK){
        printf("Retrieved record: ");
        printRecord(fetched);
    } 
    else{
        printf("Couldn't retrieve block. Either it doesn't exist or something unexpected happened\n");
    }
    printf("\n");
    int target = 203;//Chosen since 3 records share this and is ideal to show the iterator at work.
    int record_count=1;
    while(HP_GetNextRecordWithID(fileDesc, &fetched,&target) == HP_OK){
        printf("Record #%d with id=%d is ", record_count,target);
        printRecord(fetched);
        printf("Call number %d of GetNextRecordWithId ended\n\n", record_count);
        record_count++;
    } 
    HP_CloseFile(fileDesc);
end:
    if(input_file != NULL) fclose(input_file);
    return ret;
}
