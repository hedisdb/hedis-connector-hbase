#include <stdio.h>
#include "hedis.h"

int init(hedisConfigEntry *entry){
	printf("%s: %s\n", entry->key, entry->value);
	printf("hello world connector\n");

	return 0;
}