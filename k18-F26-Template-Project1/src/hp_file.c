#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "block_records.h"
#include "hp_file.h"
#include "record.h"

/* Βοηθητικό macro: καλεί μια συνάρτηση του επιπέδου BF και, σε περίπτωση
   σφάλματος, εκτυπώνει την περιγραφή του και επιστρέφει HP_ERROR */
#define CALL_BF(call)         \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      return HP_ERROR;        \
    }                         \
  }

/*Assumptions Made
The first block of each file (Block 0) stores only metadata (HP_info)
Each block that stores records has a counter of said records. Its' adress
is at block_start+RECORDS_PER_BLOCK * sizeof(Record) since the block has 56 free bytes at the end.
This choice is justified by looking at the blockGetRecord (didn't want to change it).
Remember to update metadata (check hp_info) and openCount and do the respective sanity checks.*/

//MAKE SURE TO ALWAYS CHECK WHETHER THIS IS < MAX_OPEN_FILES
static int openCount = 0;

HP_ErrorCode HP_CreateFile(const char *fileName) {
    int fileDesc;
    BF_Block*  block;
    HP_info* info= {0};
	/*We don't add this to the openfiles array since
	it will be closed in the end of this function*/
    CALL_BF(BF_CreateFile(fileName));
    CALL_BF(BF_OpenFile(fileName, &fileDesc));
    BF_Block_init(&block);
    CALL_BF(BF_AllocateBlock(fileDesc, block));

    char* data = BF_Block_GetData(block);
    info->filetype = "HP_file";
    info->record_count = 0;
    info->block_count = 1;
    info->Last_block = block;
    info->iterator_pos = 1;//Block 0 is for metadata only.
    info->iterator_slot = 0;
    memcpy(data, info, sizeof(HP_info));
  
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(Block));
    BF_BlockDestroy(block);
    CALL_BF(BF_CloseFile(fileDesc));
    return HP_OK;
}

HP_ErrorCode HP_OpenFile(const char *fileName, int *fileDesc) {
  /* TODO: ανοίξτε το αρχείο, διαβάστε τα μεταδεδομένα από το πρώτο μπλοκ και
    ελέγξτε ότι πρόκειται όντως για αρχείο σωρού */
	if(openCount >= BF_MAX_OPEN_FILES) return HP_ERROR;
	HP_info* metadata;
	BF_Block* first = NULL;
  	BF_Block_init(&first);

	CALL_BF(BF_OpenFile(fileName, fileDesc));
	if(BF_GetBlock(fileDesc, 0,first)!=BF_OK){
    BF_BlockDestroy(&first);
    BF_CloseFile(fileDesc);
    return HP_ERROR;
  	}
	
  	char* mdata = BF_Block_GetData(first);
	memcpy(metadata, mdata, sizeof(HP_info));
	
  	if (strcmp(metadata->filetype, "HP_file") != 0){
		BF_CloseFile(fileDesc);
		return HP_ERROR;
	}
	openCount++;
  return HP_OK;
}

HP_ErrorCode HP_CloseFile(int fileDesc) {
  /* TODO: γράψτε στον δίσκο ό,τι εκκρεμεί και κλείστε το αρχείο */
  return HP_ERROR;
}

//Idea: Reserve 4 bytes of every block to keep a counter
//of records in this block. Bytes at the end of the file used
//--> 4 from BF_BLOCK_SIZE - RECORDS_PER_BLOCK * sizeof(Record)
HP_ErrorCode HP_InsertEntry(int fileDesc, Record record) {
  /* TODO: βρείτε ή δεσμεύστε το κατάλληλο μπλοκ και αντιγράψτε εκεί την
     εγγραφή με την blockSetRecord. Μην ξεχάσετε να ενημερώσετε τα
     μεταδεδομένα του αρχείου. */
  return HP_ERROR;
}

HP_ErrorCode HP_PrintAllEntries(int fileDesc, char *fieldName, void *value) {
  /* TODO: διατρέξτε τα μπλοκ του αρχείου και εκτυπώστε τις εγγραφές που στο
     πεδίο fieldName έχουν τιμή value. Αν value == NULL, εκτυπώστε τις όλες. */
  return HP_ERROR;
}

HP_ErrorCode HP_GetEntry(int fileDesc, int blockid, int position,
                         Record *record) {
  	/* TODO: φέρτε το μπλοκ blockid και διαβάστε την εγγραφή της θέσης position
    με την blockGetRecord */
	BF_Block* target;
	BF_Block_init(&target);
	CALL_BF(BF_GetBlock(fileDesc, blockid, target));
	if(blockGetRecord(target, position, record) != 0){
		CALL_BF(BF_UnpinBlock(target));
		BF_BlockDestroy(target);
		return HP_ERROR;	
	}
	//GetBlock pins the block now we need to unpin it and delete the local
	//instance of the block
	CALL_BF(BF_UnpinBlock(target));
	BF_BlockDestroy(target);
  	return HP_OK;
}

HP_ErrorCode HP_GetNextRecordWithID(int fileDesc, Record *record, void *value) {
    HP_info* metadata = {0};
    BF_Block* first;
    int *target = (int*)value;
    BF_Block_init(&first);
    BF_GetBlock(fileDesc, 0, first);
    char* data = BF_Block_GetData(first);
    memcpy(metadata, data, sizeof(HP_info));
    //Sanity checks
    if(strcmp(metadata->filetype, "HP_file") != 0) return HP_ERROR;
    if(metadata->record_count == 0) return HP_EOF;
    while(metadata->iterator_pos < metadata->block_count){
        BF_Block* block;
        BF_Block_init(block);
        //read number of records to not read null data (or leftover values in mem)
        int recordsInBlock = 0;
        CALL_BF(BF_GetBlock(fileDesc,metadata->iterator_pos,block));
        memcpy(&recordsInBlock, block+sizeof(Record)*RECORDS_PER_BLOCK, sizeof(int));
        while(info->iterator_slot < recordsInBlock){
            Record* cur;
            blockGetRecord(block,metadata->iterator_slot,cur);
            metadata->iterator_slot++;
            if(cur->id == target){
              //set result and cleanup
                record = cur;
                BF_UnpinBlock(block);
                memcpy(data,metadata,sizeof(HP_info));
                BF_UnpinBlock(first);
                BF_BlockDestroy(block);
                BF_BlockDestroy(first);
                return HP_OK;
            }
        }
        metadata->iterator_pos++;
        metadata->iterator_slot=0;
    }
    //we reached EOF so we reset the iterator.
    metadata->iterator_pos = 1;
    metadata->iterator_slot = 0;
    //Cleanup and metadata update
    CALL_BF(BF_UnpinBlock(block));
    memcpy(data,metadata,sizeof(HP_info));
    BF_Block_SetDirty(first);
    CALL_BF(BF_UnpinBlock(first));
    BF_BlockDestroy(block);
    BF_BlockDestroy(first);
    return HP_EOF;
}
