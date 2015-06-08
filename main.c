#include <stdio.h>
#include "hedis.h"

hedisConfigEntry **hedis_entries;

int init(hedisConfigEntry **entries, int entry_count){
	printf("hello world connector\n");

	hedis_entries = entries;

	for(int i = 0; i < entry_count; i++){
		printf("%s: %s\n", entries[i]->key, entries[i]->value);
	}

	return 0;
}

char *get_value(){
	return hedis_entries[0]->key;
}