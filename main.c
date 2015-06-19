#include <stdio.h>
#include <regex.h>
#include "hedis.h"

#define HEDIS_COMMAND_PATTERN "(\\w+):(\\w+)"
#define MAX_ERROR_MSG 0x1000

hedisConfigEntry **hedis_entries;
int hedis_entry_count;

int init(hedisConfigEntry **entries, int entry_count){
	hedis_entries = entries;
	hedis_entry_count = entry_count;

	for(int i = 0; i < entry_count; i++){
		printf("%s: %s\n", entries[i]->key, entries[i]->value);
	}

	return 0;
}

char **parse_hedis_command(const char * to_match) {
    regex_t * r = malloc(sizeof(regex_t));

    int status = regcomp(r, HEDIS_COMMAND_PATTERN, REG_EXTENDED | REG_NEWLINE);

    if (status != 0) {
        char error_message[MAX_ERROR_MSG];

        regerror(status, r, error_message, MAX_ERROR_MSG);

        printf("Regex error compiling '%s': %s\n", HEDIS_COMMAND_PATTERN, error_message);

        return NULL;
    }

    char **str = malloc(sizeof(char *) * 2);

    /* "P" is a pointer into the string which points to the end of the
     *        previous match. */
    const char * p = to_match;
    /* "N_matches" is the maximum number of matches allowed. */
    const int n_matches = 10;
    /* "M" contains the matches found. */
    regmatch_t m[n_matches];

    int i = 0;
    int nomatch = regexec(r, p, n_matches, m, 0);

    if (nomatch) {
        printf("No more matches.\n");

        return NULL;
    }

    for (i = 0; i < n_matches; i++) {
        int start;
        int finish;

        if (m[i].rm_so == -1) {
            break;
        }

        start = m[i].rm_so + (p - to_match);
        finish = m[i].rm_eo + (p - to_match);

        if (i != 0) {
            int size = finish - start;

            str[i - 1] = malloc(sizeof(char) * size);

            sprintf(str[i - 1], "%.*s", size, to_match + start);
        }
    }

    p += m[0].rm_eo;

    return str;
}

char *get_value(const char *str){
	char **result = parse_hedis_command(str);

	char *value = malloc(sizeof(char) * 30);

	sprintf(value, "first: %s, second: %s", result[0], result[1]);

	return value;
}