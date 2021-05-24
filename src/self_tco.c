/*
 * @(#) self-Tail Call Optimization (TCO) code, generated from
 * @(#) _src/self_tco.c.carml_
*/

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <gc.h>
#include <carmlc.h>

#ifdef DEBUG 
#define dwalk(x, y) walk(x, y)
#else
#define dwalk(x, y)
#endif

extern char *hstrdup(const char *);

// NOTE: this code is generated from
// src/self_tco.c.carml, so if we really need to change this,
// we probably should fix that file first...
int
self_tco_p(const char * name, AST * src){
    int res = 0;
    int idx = 0;
    int tag = 0;

    if(src == NULL) {
        return 0;
    } else {
        tag = src->tag;
    }

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
                return 1;
            } else {
                idx = idx + 2;
            }

        }

        return 0;
    } else {
        return 0;
    }

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
    const int clen = (src->lenchildren - 1) * 2;
    const int ilen = impl->lenchildren;
    int idx = 0;
    int sidx = 1;
    int base = 0;
    AST * shadow = nil;
    AST * param = nil;
    AST * result = nil;
    ret->tag = TBEGIN;
    ret->lenchildren = clen;
    ret->children = hmalloc(clen * sizeof(AST * ));
    while(idx < ilen){
        param = get_parameter_ident(impl, idx);
        shadow = shadow_ident(param);
        result = src->children[sidx];
        ret->children[idx] = make_set_bang(shadow, result);
        idx = idx + 1;
        sidx = sidx + 1;
    }

    base = idx;
    idx = 0;
    while(idx < ilen){
        param = get_parameter_ident(impl, idx);
        shadow = shadow_ident(param);
        ret->children[idx + base] = make_set_bang(param, shadow);
        idx = idx + 1;
    }

    return ret;
}
char *
shadow_name(char * name){
    int len = 3 + strlen(name);
    char * ret = hmalloc(3 + (sizeof(char) * strlen(name)));
    stpcpy(ret, name);
    strcat(ret, "_sh");
    return ret;
}
char *
get_parameter_name(AST * src, int idx){
    const AST * ret = nil;
    if(idx < src->lenchildren) {
        ret = src->children[idx];
        ret = ret->children[0];
        return ret->value;
    }

    return "";
}
AST * 
get_parameter_ident(AST * src, int idx){
    const AST * ret = nil;
    if(idx < src->lenchildren) {
        ret = src->children[idx];
        return ret->children[0];
    }

    return nil;
}
AST * 
get_parameter_type(AST * src, int idx){
    const AST * ret = nil;
    if(idx < src->lenchildren) {
        ret = src->children[idx];
        return ret->children[1];
    }

    return nil;
}
AST * 
define_shadow_params(AST * src, AST * body){
    AST * ret = hmalloc(sizeof(AST * ));
    AST * tmp = nil;
    AST * * vbuf = (AST * *)hmalloc(sizeof(AST * ) * 64);
    int idx = 0;
    int cidx = 0;
    int capacity = 64;
    int length = 0;
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
        vbuf[idx] = tmp;
        idx = idx + 1;
    }

    length = idx;
    idx = 0;
    while(idx < body->lenchildren){
        tmp = body->children[idx];
        if((tmp->tag == TVAR) || (tmp->tag == TVAL)) {
            vbuf[length] = tmp;
            length = length + 1;
        }

        idx = idx + 1;
    }

    idx = 0;
    ret->lenchildren = length + 1;
    ret->children = hmalloc((length + 1) * sizeof(AST **));
    while(idx < length){
        ret->children[idx] = vbuf[idx];
        idx = idx + 1;
    }

    tmp = hmalloc(sizeof(AST));
    tmp->tag = TWHILE;
    tmp->lenchildren = 2;
    tmp->children = hmalloc(2 * sizeof(AST * ));
    ret->children[length] = tmp;

    return ret;
}
AST * 
make_ident(char * src){
    AST * ret = hmalloc(sizeof(AST));
    ret->lenchildren = 0;
    ret->tag = TIDENT;
    ret->value = hstrdup(src);
    return ret;
}
AST * 
make_boolean(int original_value){
    AST * ret = hmalloc(sizeof(AST));
    ret->lenchildren = 0;
    ret->children = nil;
    if(original_value == 0) {
        ret->tag = TFALSE;;
    } else {
        ret->tag = TTRUE;
    }

    return ret;
}
AST * 
copy_body(AST * src, AST * self, uint8_t finalp){
    AST * ret = hmalloc(sizeof(AST));
    AST * * buf = (AST * *)hmalloc(sizeof(AST * ) * 128);
    int sidx = 0;
    int bidx = 0;
    const int srccap = src->lenchildren;
    const int srctag = src->tag;
    ret->value = hstrdup(src->value);
    ret->tag = src->tag;
    if(srctag == TCALL) {
        if(finalp) {
            return shadow_params(src, self);
        } else {
            while(bidx < srccap){
                buf[bidx] = copy_body(src->children[sidx], self, 0);
                bidx = bidx + 1;
                sidx = sidx + 1;
            }
        }
    } else if(srctag == TIDENT) {
        if(finalp) {
            ret->tag = TCALL;
            ret->lenchildren = 2;
            ret->children = hmalloc(2 * sizeof(AST * ));
            buf[0] = make_ident("return");
            buf[1] = make_ident(src->value);
            bidx = 2;
        }
    } else if(srctag == TMATCH) {
        1;
    } else if(srctag == TIF) {
        buf[0] = copy_body(src->children[0], self, 0);
        buf[1] = copy_body(src->children[1], self, finalp);
        buf[2] = copy_body(src->children[2], self, finalp);
        bidx = 3;
    } else if(srctag == TWHEN) {
        buf[0] = copy_body(src->children[0], self, 0);
        buf[1] = copy_body(src->children[1], self, finalp);
        bidx = 2;
    } else if(srctag == TBEGIN) {
        while(bidx < srccap){
            if((finalp) && (bidx == srccap - 1)) {
                buf[bidx] = copy_body(src->children[sidx], self, finalp);
            } else {
                buf[bidx] = copy_body(src->children[sidx], self, 0);

            }

            bidx = bidx + 1;
            sidx = sidx + 1;
        }
    } else {
        while(bidx < srccap){
            buf[bidx] = copy_body(src->children[sidx], self, 0);
            bidx = bidx + 1;
            sidx = sidx + 1;
        }
    }

    sidx = 0;
    ret->lenchildren = bidx;
    ret->children = hmalloc(bidx * sizeof(AST * ));
    while(sidx < bidx){
        ret->children[sidx] = buf[sidx];
        sidx = sidx + 1;
    }

    return ret;
}
AST * 
rewrite_tco(AST * src){
    const AST * params = define_shadow_params(src->children[0], src->children[1]);
    AST * ret = hmalloc(sizeof(AST * ));
    AST * body = params->children[params->lenchildren - 1];
    ret->tag = TDEF;
    ret->lenchildren = 3;
    ret->children = hmalloc(3 * sizeof(AST * * ));
    ret->value = src->value;
    ret->children[0] = src->children[0];
    ret->children[2] = src->children[2];
    ret->children[1] = params;
    body->children[0] = make_boolean(1);
    body->children[1] = copy_body(src->children[1], src->children[0], 1);
    return ret;
}
