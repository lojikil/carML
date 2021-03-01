#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <gc.h>
#include <carmlc.h>

// NOTE: this code is generated from
// src/self_tco.c.carml, so if we really need to change this,
// we probably should fix that file first...
int
self_tco_p(const char *name, AST *src){
    int idx = 0;
    const int tag = src->tag;
    if(tag == TDEF) {
        return self_tco_p(name, src->children[1]);
    } else if(tag == TFN) {
        return self_tco_p(name, src->children[1]);
    } else if(tag == TCALL) {
        return !strcmp(name, src->children[0]->value);
    } else if(tag == TBEGIN) {
        idx = src->lenchildren - 1;
        return self_tco_p(name, src->children[idx]);
    } else if(tag == TWHEN) {
        return self_tco_p(name, src->children[1]);
    } else if(tag == TIF) {
        return (self_tco_p(name, src->children[1])) || (self_tco_p(name, src->children[2]));
    } else if(tag == TMATCH) {
        idx = 1;
        while(idx < src->lenchildren){
            if(self_tco_p(name, src->children[idx])) {
                return YES;
            } else {
                idx = idx + 2;
            }
        }
        return NO;
    } else {
        return NO;
    }
}

char *
shadow_name(char * name){
    char * ret = hmalloc(3 + sizeof(char) * strlen(name));
    stpcpy(ret, name);
    strcat(ret, "_sh");
    return ret;
}

AST *
shadow_ident(AST * src){
    AST * ret = hmalloc(sizeof(AST * ));
    ret->tag = TIDENT;
    ret->lenchildren = 0;
    ret->value = shadow_name(src->value);
    return ret;
}

AST *
make_set_bang(AST * ident, AST * value){
    AST * ret = hmalloc(sizeof(AST * ));
    AST * setident = hmalloc(sizeof(AST * ));
    setident->tag = TIDENT;
    setident->value = hmalloc(5 * sizeof(char));
    stpncpy(setident->value, "set!", 5);
    ret->tag = TCALL;
    ret->lenchildren = 3;
    ret->children = hmalloc(3 * sizeof(AST * ));
    ret->children[0] = setident;
    ret->children[1] = ident;
    ret->children[2] = value;
    return ret;
}

AST *
shadow_params(AST * src, AST * impl){
    AST * ret = hmalloc(sizeof(AST * ));
    const int clen = src->lenchildren * 2;
    int idx = 0;
    int sidx = 0;
    AST * shadow = nil;
    AST * param = nil;
    AST * result = nil;
    ret->tag = TBEGIN;
    ret->lenchildren = clen;
    ret->children = hmalloc(clen * sizeof(AST * ));
    while(idx < clen){
        param = get_parameter_ident(impl, sidx);
        shadow = shadow_ident(param);
        result = src->children[sidx];
        ret->children[idx] = make_set_bang(shadow, result);
        ret->children[idx + 1] = make_set_bang(param, shadow);
        idx = idx + 2;
        sidx = sidx + 1;
    }
    return ret;
}

char *
get_parameter_name(AST * src, int idx){
    const AST * ret = nil;
    if(idx < src->lenchildren){
        ret = src->children[idx];
        ret = ret->children[0];
        return ret->value;
    }

    return "";
}

AST *
get_parameter_ident(AST * src, int idx){
    const AST * ret = nil;
    if(idx < src->lenchildren){
        ret = src->children[idx];
        return ret->children[0];
    }

    return nil;
}

AST *
get_parameter_type(AST * src, int idx){
    const AST * ret = nil;
    if(idx < src->lenchildren){
        ret = src->children[idx];
        return ret->children[1];
    }

    return nil;
}

AST *
define_shadow_params(AST * src){
    AST * ret = hmalloc(sizeof(AST * ));
    AST * tmp = nil;
    int idx = 0;
    ret->tag = TBEGIN;
    ret->lenchildren = src->lenchildren;
    ret->children = hmalloc(src->lenchildren * sizeof(AST * * ));
    while(idx < src->lenchildren){
        tmp = hmalloc(sizeof(AST * ));
        tmp->tag = TVAR;
        tmp->lenchildren = 2;
        tmp->value = shadow_name(get_parameter_name(src, idx));
        tmp->children = hmalloc(2 * sizeof(AST * ));
        tmp->children[0] = get_parameter_ident(src, idx);
        tmp->children[1] = get_parameter_type(src, idx);
        ret->children[idx] = tmp;
        idx = idx + 1;
    }

    return ret;
}

AST *
rewrite_tco(AST * src){
    const AST * params = define_shadow_params(src->children[0]);
    AST * ret = hmalloc(sizeof(AST * ));
    ret->tag = TDEF;
    ret->lenchildren = 3;
    ret->value = src->value;
    ret->children = hmalloc(3 * sizeof(AST **));
    ret->children[0] = src->children[0];
    ret->children[2] = src->children[2];
    ret->children[1] = params;
    return ret;
}
