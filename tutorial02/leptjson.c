#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <errno.h> 
#include <stdio.h> 
#include <math.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch)>='0' && (ch)<='9')
#define ISDIGIT1TO9(ch)     ((ch)>='1' && (ch)<='9')
typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v) {
    switch (*c->json)
    {
    case 'n':
        if (c->json[1] != 'u' || c->json[2] != 'l' || c->json[3] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
        c->json += 4;
        v->type = LEPT_NULL;
        return LEPT_PARSE_OK;
    case 't':
        if (c->json[1] != 'r' || c->json[2] != 'u' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
        c->json += 4;
        v->type = LEPT_TRUE;
        return LEPT_PARSE_OK;
    case 'f':
        if (c->json[1] != 'a' || c->json[2] != 'l' || c->json[3] != 's' || c->json[4] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
        c->json += 5;
        v->type = LEPT_FALSE;
        return LEPT_PARSE_OK;
    default:
        return LEPT_PARSE_INVALID_VALUE;
    }

}

static int lept_parse_number(lept_context* c, lept_value* v) {
    const char* p = c->json;
    if(*p == '-') p++;
    if(*p == '0') p++;
    else{
        if(!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
        for(p++;ISDIGIT1TO9(*p);p++);
    }
    if(*p=='.'){
        p++;
        if(!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for(p++;ISDIGIT(*p);p++);
    }
    if(*p == 'e' || *p == 'E'){
        p++;
        if(*p =='+' || *p =='-') p++;
        if(!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for(p++;ISDIGIT(*p);p++);
    }
    /* \TODO validate number */
    errno = 0;
    v->n= strtod(c->json,NULL);
    if(errno== ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL || v->n == HUGE_VALF || v->n == HUGE_VALL)){
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    v->type = LEPT_NUMBER;
    c->json = p;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v);
        case 'f':  return lept_parse_literal(c, v);
        case 'n':  return lept_parse_literal(c, v);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
