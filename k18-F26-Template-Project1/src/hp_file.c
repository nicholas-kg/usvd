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
    info->Last_block = block;
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
	BF_Block* first;
	CALL_BF(BF_OpenFile(fileName, fileDesc));
	CALL_BF(BF_GetBlock(fileDesc, 0,first));
	char* mdata = CALL_BF(BF_Block_GetData(first));
	memcpy(metadata, mdata, sizeof(HP_info));
	if (metadata->filetype != "HP_file"){
		CALL_BF(BF_CloseFile(fileDesc));
		return HP_ERROR;
	}
	openCount++;
  	return HP_OK;
}

HP_ErrorCode HP_CloseFile(int fileDesc) {
  /* TODO: γράψτε στον δίσκο ό,τι εκκρεμεί και κλείστε το αρχείο */
  return HP_ERROR;
}

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
  return HP_ERROR;
}

HP_ErrorCode HP_GetNextRecordWithID(int fileDesc, Record *record, void *value) {
  /* TODO: υλοποιήστε τον iterator. Η κατάσταση του (πού σταμάτησε η
     προηγούμενη κλήση) πρέπει να επιβιώνει ανάμεσα στις κλήσεις. */
  return HP_ERROR;
}
