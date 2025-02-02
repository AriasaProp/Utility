/*
 * Copyright (c) 2009, 2010 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include "jansson/strbuffer.h"
#include "jansson/util.h"

#define STRBUFFER_MIN_SIZE  16
#define STRBUFFER_FACTOR    2

int strbuffer_init(strbuffer_t *strbuff)
{
    strbuff->size = STRBUFFER_MIN_SIZE;
    strbuff->length = 0;

    strbuff->value = malloc(strbuff->size);
    if(!strbuff->value)
        return -1;

    /* initialize to empty */
    strbuff->value[0] = '\0';
    return 0;
}

void strbuffer_close(strbuffer_t *strbuff)
{
    free(strbuff->value);
    strbuff->size = 0;
    strbuff->length = 0;
    strbuff->value = NULL;
}

void strbuffer_clear(strbuffer_t *strbuff)
{
    strbuff->length = 0;
    strbuff->value[0] = '\0';
}

const char *strbuffer_value(const strbuffer_t *strbuff)
{
    return strbuff->value;
}

char *strbuffer_steal_value(strbuffer_t *strbuff)
{
    char *result = strbuff->value;
    strbuffer_init(strbuff);
    return result;
}

int strbuffer_append(strbuffer_t *strbuff, const char *string)
{
    return strbuffer_append_bytes(strbuff, string, strlen(string));
}

int strbuffer_append_byte(strbuffer_t *strbuff, char byte)
{
    return strbuffer_append_bytes(strbuff, &byte, 1);
}

int strbuffer_append_bytes(strbuffer_t *strbuff, const char *data, int size)
{
    if(strbuff->length + size >= strbuff->size)
    {
        strbuff->size = max(strbuff->size * STRBUFFER_FACTOR,
                            strbuff->length + size + 1);

        strbuff->value = realloc(strbuff->value, strbuff->size);
        if(!strbuff->value)
            return -1;
    }

    memcpy(strbuff->value + strbuff->length, data, size);
    strbuff->length += size;
    strbuff->value[strbuff->length] = '\0';

    return 0;
}

char strbuffer_pop(strbuffer_t *strbuff)
{
    if(strbuff->length > 0) {
        char c = strbuff->value[--strbuff->length];
        strbuff->value[strbuff->length] = '\0';
        return c;
    }
    else
        return '\0';
}
