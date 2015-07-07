/**
* Licensed to the Apache Software Foundation (ASF) under one
* or more contributor license agreements. See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership. The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License. You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <regex.h>

#include <hbase/hbase.h>

/* Found under /libhbase/src/test/native/common */
#include <byte_buffer.h>
#include "hedis.h"

#line __LINE__ "main.c"

/*
 * Sample code to illustrate usage of libhbase APIs
 */

#ifdef __cplusplus
extern  "C" {
#endif

#define CHECK_API_ERROR(retCode, ...) \
    HBASE_LOG_MSG((retCode ? HBASE_LOG_LEVEL_ERROR : HBASE_LOG_LEVEL_INFO), \
        __VA_ARGS__, retCode);
// test REGEX: https://regex101.com/r/bB3mQ1/4
#define HEDIS_COMMAND_PATTERN "([a-zA-Z0-9_\\-]+)@([#:a-zA-Z0-9_\\\\\\-]+)(@([@#a-zA-Z0-9_\\\\\\-]+)(:([a-zA-Z0-9_\\-]+))?)?"
#define HEDIS_COMMAND_LENGTH 6
#define HEDIS_COMMAND_TABLE_INDEX 0
#define HEDIS_COMMAND_ROWKEY_INDEX 1
#define HEDIS_COMMAND_COLUMN_FAMILY_INDEX 3
#define HEDIS_COMMAND_COLUMN_QUALIFIER_INDEX 5
#define MAX_ERROR_MSG 0x1000

typedef struct cell_data_t_ {
    bytebuffer value;
    hb_cell_t  *hb_cell;
    struct cell_data_t_ *next_cell;
} cell_data_t;

cell_data_t*
new_cell_data() {
    cell_data_t *cell_data = (cell_data_t*) calloc(1, sizeof(cell_data_t));
    cell_data->next_cell = NULL;
    return cell_data;
}

typedef struct row_data_t_ {
    bytebuffer key;
    struct cell_data_t_ *first_cell;
} row_data_t;

void
release_row_data(row_data_t *row_data) {
    if (row_data != NULL) {
        cell_data_t *cell = row_data->first_cell;
        while (cell) {
            bytebuffer_free(cell->value);
            free(cell->hb_cell);
            cell_data_t *cur_cell = cell;
            cell = cell->next_cell;
            free(cur_cell);
        }
        bytebuffer_free(row_data->key);
        free(row_data);
    }
}

hedisConfigEntry **hedis_entries;
int hedis_entry_count;
regex_t *r = NULL;
char *value;
char *char_rowkey = NULL;
char *char_family = NULL;
char *char_qualifier = NULL;
bytebuffer byte_rowkey = NULL;
bytebuffer byte_family = NULL;
bytebuffer byte_qualifier = NULL;

char *convert(byte_t *a, size_t length) {
    char *result = malloc(sizeof(char) * length);

    sprintf(result, "%.*s", length, a);

    return result;
}

void printRow(const hb_result_t result) {
    const byte_t *key = NULL;
    size_t key_len = 0;
    hb_result_get_key(result, &key, &key_len);
    size_t cell_count = 0;
    hb_result_get_cell_count(result, &cell_count);
    HBASE_LOG_INFO("Row=\'%.*s\', cell count=%d", key_len, key, cell_count);
    const hb_cell_t **cells;
    hb_result_get_cells(result, &cells, &cell_count);
    for (size_t i = 0; i < cell_count; ++i) {
        HBASE_LOG_INFO(
            "Cell %d: family=\'%.*s\', qualifier=\'%.*s\', "
            "value=\'%.*s\', timestamp=%lld.", i,
            cells[i]->family_len, cells[i]->family,
            cells[i]->qualifier_len, cells[i]->qualifier,
            cells[i]->value_len, cells[i]->value, cells[i]->ts);
    }
}

char * to_json(const hb_result_t result) {
    const byte_t *key = NULL;
    size_t key_len = 0;

    char *json;

    json = malloc(sizeof(char) * 200);

    hb_result_get_key(result, &key, &key_len);

    if (key == NULL) {
        return NULL;
    }

    size_t cell_count = 0;

    hb_result_get_cell_count(result, &cell_count);

    const hb_cell_t **cells;

    hb_result_get_cells(result, &cells, &cell_count);

    // show JSON: {"rowkey":"thisisrowkey","columns":[{"name":"cf:gu","value":"M0000001"},{"name":"cf:nk","value":"kewang"}]}
    strcpy(json, "{\"rowkey\":\"");

    strcat(json, convert(key, key_len));
    strcat(json, "\",\"columns\":[");

    for (size_t i = 0; i < cell_count; i++) {
        strcat(json, "{\"name\":\"");
        strcat(json, convert(cells[i]->family, cells[i]->family_len));
        strcat(json, ":");
        strcat(json, convert(cells[i]->qualifier, cells[i]->qualifier_len));
        strcat(json, "\",\"value\":\"");
        strcat(json, convert(cells[i]->value, cells[i]->value_len));
        strcat(json, "\"},");
    }

    int json_length = strlen(json);

    memmove(&json[json_length - 1], &json[json_length], 1);

    strcat(json, "]}");

    return json;
}

/**
 * Get synchronizer and callback
 */
volatile bool get_done = false;
pthread_cond_t get_cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t get_mutex = PTHREAD_MUTEX_INITIALIZER;

void
get_callback(int32_t err, hb_client_t client,
             hb_get_t get, hb_result_t result, void *extra) {
    if (err == 0) {
        const char *table_name;
        size_t table_name_len;
        hb_result_get_table(result, &table_name, &table_name_len);
        HBASE_LOG_INFO("Received get callback for table=\'%.*s\'.",
                       table_name_len, table_name);

        printRow(result);

        value = to_json(result);

        printf("value: %s\n", value);

        hb_result_destroy(result);
    } else {
        HBASE_LOG_ERROR("Get failed with error code: %d.", err);
    }

    hb_get_destroy(get);

    pthread_mutex_lock(&get_mutex);
    get_done = true;
    pthread_cond_signal(&get_cv);
    pthread_mutex_unlock(&get_mutex);
}

void
wait_for_get() {
    HBASE_LOG_INFO("Waiting for get operation to complete.");
    pthread_mutex_lock(&get_mutex);
    while (!get_done) {
        pthread_cond_wait(&get_cv, &get_mutex);
    }
    pthread_mutex_unlock(&get_mutex);
    HBASE_LOG_INFO("Get operation completed.");
}

/**
 * Scan synchronizer and callbacks
 */
volatile bool scan_done = false;
pthread_cond_t scan_cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t scan_mutex = PTHREAD_MUTEX_INITIALIZER;

void scan_callback(int32_t err, hb_scanner_t scanner,
                   hb_result_t results[], size_t num_results, void *extra) {
    if (num_results) {
        const char *table_name;
        size_t table_name_len;
        hb_result_get_table(results[0], &table_name, &table_name_len);
        HBASE_LOG_INFO("Received scan_next callback for table=\'%.*s\', row count=%d.",
                       table_name_len, table_name, num_results);

        for (int i = 0; i < num_results; ++i) {
            printRow(results[i]);
            hb_result_destroy(results[i]);
        }
        hb_scanner_next(scanner, scan_callback, NULL);
    } else {
        hb_scanner_destroy(scanner, NULL, NULL);
        pthread_mutex_lock(&scan_mutex);
        scan_done = true;
        pthread_cond_signal(&scan_cv);
        pthread_mutex_unlock(&scan_mutex);
    }
}

void
wait_for_scan() {
    HBASE_LOG_INFO("Waiting for scan to complete.");
    pthread_mutex_lock(&scan_mutex);
    while (!scan_done) {
        pthread_cond_wait(&scan_cv, &scan_mutex);
    }
    pthread_mutex_unlock(&scan_mutex);
    HBASE_LOG_INFO("Scan completed.");
}

/**
 * Client destroy synchronizer and callbacks
 */
volatile bool client_destroyed = false;
pthread_cond_t client_destroyed_cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t client_destroyed_mutex = PTHREAD_MUTEX_INITIALIZER;

void
client_disconnection_callback(int32_t err,
                              hb_client_t client, void *extra) {
    HBASE_LOG_INFO("Received client disconnection callback.");
    pthread_mutex_lock(&client_destroyed_mutex);
    client_destroyed = true;
    pthread_cond_signal(&client_destroyed_cv);
    pthread_mutex_unlock(&client_destroyed_mutex);
}

void
wait_client_disconnection() {
    HBASE_LOG_INFO("Waiting for client to disconnect.");
    pthread_mutex_lock(&client_destroyed_mutex);
    while (!client_destroyed) {
        pthread_cond_wait(&client_destroyed_cv, &client_destroyed_mutex);
    }
    pthread_mutex_unlock(&client_destroyed_mutex);
    HBASE_LOG_INFO("Client disconnected.");
}

int
ensureTable(hb_connection_t connection, const char *table_name) {
    int32_t retCode = 0;
    hb_admin_t admin = NULL;

    if ((retCode = hb_admin_create(connection, &admin)) != 0) {
        HBASE_LOG_ERROR("Could not create HBase admin : errorCode = %d.", retCode);

        goto cleanup;
    }

    if ((retCode = hb_admin_table_exists(admin, NULL, table_name)) == 0) {
        HBASE_LOG_INFO("Table '%s' exists", table_name);
    } else if (retCode != ENOENT) {
        HBASE_LOG_ERROR("Error while checking if the table exists: errorCode = %d.", retCode);
    }

cleanup:
    if (admin) {
        hb_admin_destroy(admin, NULL, NULL);
    }

    return retCode;
}

int init(hedisConfigEntry **entries, int entry_count) {
    hedis_entries = entries;
    hedis_entry_count = entry_count;

    r = malloc(sizeof(regex_t));

    int status = regcomp(r, HEDIS_COMMAND_PATTERN, REG_EXTENDED | REG_NEWLINE);

    if (status != 0) {
        char error_message[MAX_ERROR_MSG];

        regerror(status, r, error_message, MAX_ERROR_MSG);

        printf("Regex error compiling '%s': %s\n", HEDIS_COMMAND_PATTERN, error_message);

        return -1;
    }

    return 0;
}

int parse_hedis_command(const char * to_match, char ** str) {
    /* "P" is a pointer into the string which points to the end of the
     *        previous match. */
    const char * p = to_match;
    /* "N_matches" is the maximum number of matches allowed. */
    const int n_matches = HEDIS_COMMAND_LENGTH + 1;
    /* "M" contains the matches found. */
    regmatch_t m[n_matches];

    int i = 0;
    int nomatch = regexec(r, p, n_matches, m, 0);

    if (nomatch) {
        printf("No more matches.\n");

        return -1;
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

    return i - 1;
}

char *get_value(const char *str) {
    char **commands = malloc(sizeof(char *) * HEDIS_COMMAND_LENGTH);

    int command_length = parse_hedis_command(str, commands);

    if (command_length == -1) {
        return NULL;
    }

    int32_t retCode = 0;
    FILE* logFile = NULL;
    hb_connection_t connection = NULL;
    hb_client_t client = NULL;

    // reinitialize pthread
    get_cv = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    get_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    client_destroyed_cv = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    client_destroyed_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

    const char *zookeeper;

    for (int i = 0; i < hedis_entry_count; i++) {
        if (!strcasecmp(hedis_entries[i]->key, "zookeeper")) {
            zookeeper = malloc(sizeof(char) * strlen(hedis_entries[i]->value));

            strcpy(zookeeper, hedis_entries[i]->value);

            break;
        }
    }

    char_rowkey = commands[HEDIS_COMMAND_ROWKEY_INDEX];

    if (command_length >= HEDIS_COMMAND_COLUMN_FAMILY_INDEX + 1) {
        char_family = commands[HEDIS_COMMAND_COLUMN_FAMILY_INDEX];
        byte_family = bytebuffer_strcpy(char_family);
    }

    if (command_length >= HEDIS_COMMAND_COLUMN_QUALIFIER_INDEX + 1) {
        char_qualifier = commands[HEDIS_COMMAND_COLUMN_QUALIFIER_INDEX];
        byte_qualifier = bytebuffer_strcpy(char_qualifier);
    }

    byte_rowkey = bytebuffer_strcpy(char_rowkey);

    const char *table_name = commands[HEDIS_COMMAND_TABLE_INDEX];
    const char *zk_root_znode = NULL;
    const size_t table_name_len = strlen(table_name);

    hb_log_set_level(HBASE_LOG_LEVEL_DEBUG); // defaults to INFO
    const char *logFilePath = getenv("HBASE_LOG_FILE");
    if (logFilePath != NULL) {
        FILE* logFile = fopen(logFilePath, "a");
        if (!logFile) {
            retCode = errno;
            fprintf(stderr, "Unable to open log file \"%s\"", logFilePath);
            perror(NULL);

            goto cleanup;
        }
        hb_log_set_stream(logFile); // defaults to stderr
    }

    if ((retCode = hb_connection_create(zookeeper,
                                        zk_root_znode,
                                        &connection)) != 0) {
        HBASE_LOG_ERROR("Could not create HBase connection : errorCode = %d.", retCode);

        goto cleanup;
    }

    HBASE_LOG_INFO("Connecting to HBase cluster using Zookeeper ensemble '%s'.",
                   zookeeper);
    if ((retCode = hb_client_create(connection, &client)) != 0) {
        HBASE_LOG_ERROR("Could not connect to HBase cluster : errorCode = %d.", retCode);

        goto cleanup;
    }

    // fetch a row with rowkey
    hb_get_t get = NULL;
    hb_get_create(byte_rowkey->buffer, byte_rowkey->length, &get);

    if (char_family != NULL) {
        if (char_qualifier == NULL) {
            hb_get_add_column(get, (byte_t *)char_family, strlen(char_family), NULL, 0);
        } else {
            hb_get_add_column(get, (byte_t *)char_family, strlen(char_family), (byte_t *)char_qualifier, strlen(char_qualifier));
        }
    }

    hb_get_set_table(get, table_name, table_name_len);
    hb_get_set_num_versions(get, 10); // up to ten versions of each column

    get_done = false;
    hb_get_send(client, get, get_callback, NULL);
    wait_for_get();

cleanup:
    if (client) {
        HBASE_LOG_INFO("Disconnecting client.");
        hb_client_destroy(client, client_disconnection_callback, NULL);
        wait_client_disconnection();
    }

    if (connection) {
        hb_connection_destroy(connection);
    }

    if (logFile) {
        fclose(logFile);
    }

    if (char_qualifier != NULL) {
        bytebuffer_free(byte_qualifier);
    }

    if (char_family != NULL) {
        bytebuffer_free(byte_family);
    }

    for (int i = 0; i < command_length; i++) {
        free(commands[i]);
    }

    free(commands);

    bytebuffer_free(byte_rowkey);

    pthread_cond_destroy(&get_cv);
    pthread_mutex_destroy(&get_mutex);

    pthread_cond_destroy(&client_destroyed_cv);
    pthread_mutex_destroy(&client_destroyed_mutex);

    HBASE_LOG_INFO("Return value: %s\n", value);

    return value;
}

#ifdef __cplusplus
}
#endif