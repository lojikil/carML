#include <stdio.h>
#include <stdint.h>
#include <strings.h>

typedef enum {
    LSTART, LDEF0, LDEF1, LDEF2, LE0, LELSE1,
    LELSE2, LELSE3, LVAR0, LVAR1, LVAR2, LT0,
    LTHEN0, LTHEN1, LTHEN2, LTYPE0, LTYPE1,
    LTYPE2, LBEGIN0, LBEGIN1, LBEGIN2, LBEGIN3,
    LBEGIN4, LEQ0, LNUM0, LIDENT0, LEND0, LEND1,
    LEND2, LMATCH0, LCOMMENT, LMCOMMENT, LPOLY0
} LexStates;

typedef enum {
    TDEF, TBEGIN, TEND, TEQUAL, TCOREFORM,
    TIDENT, TCALL, TOPAREN, TCPAREN, TMATCH,
    TIF, TELSE, TTHEN, TTYPE, TPOLY, TVAR,
    TARRAY, TRECORD, TINT, TFLOAT, TSTRING,
    TCHAR, TBOOL, TEQ, TSEMI
} TypeTag;

struct _AST {
    TypeTag tag;
    /* there's going to have to be some
     * interpretation here, as we're not
     * breaking down the ASTs into discrete
     * objects, but rather just grouping them
     * into a generic a piece as possible.
     * so, for exapmple, and IF block would
     * have a lenchildren == 3, no matter what
     * whereas a BEGIN block lenchildren == N,
     * where N >= 0. In the real thing, would
     * probably be best to make this a poly with
     * each member broken down, or an SRFI-57-style
     * struct.
     * in fact... if you have row-polymorphism, there's
     * no need for SRFI-57 inhereitence, save for to
     * create convience methods... hmm... :thinking_face:
     */
    uint32_t lenchildren;
    struct _AST **children;
};

typedef struct _AST AST;

int next(FILE *, char *, int);
AST *read(FILE *);
int compile(FILE *, FILE *);
int iswhite(int);

int
main(int ac, char **al) {
    int ret = 0;
    char buf[512];
    printf("here: \n");
    do {
        printf(">>> \n");
        ret = next(stdin, &buf[0], 512);
        printf("%s %d", buf, ret);
    } while(!strncmp(buf, "quit", 512));
    return 0;
}

int
iswhite(int c){
    return (c == ' ' || c == '\r' || c == '\n' || c == '\v' || c == '\t');
}

int
next(FILE *fdin, char *buf, int buflen) {
    /* fetch the _next_ token from input, and tie it
     * to a token type. fill buffer with the current
     * item, so that we can return identifiers or
     * numbers.
     */
    int state = 0, rc = 0, cur = 0, idx = 0;
    while(1) {
        if(idx >= buflen) {
            break;
        }
        /* NOTE: this would work _really_ well as a state transition table;
         * most of this code would melt away into simple table lookups, based
         * on the current state and the current input. I might eventually rewrite
         * this as a simple state-transition table. Would also be fun to write a 
         * recursive descent parser generator based on a simple specification and
         * generate said table, for LL(k).
         */
        cur = fgetc(fdin);
        buf[idx++] = cur;
        switch(state) {
            case 0:
                while(cur == ' ' || cur == '\r' || cur == '\t' || cur == '\n') {
                    cur = fgetc(fdin);
                }
                /* probably _should_ collapse this into a
                 * a HSM instead of the current flat SM...
                 * that would definitely shrink the code 
                 * quite a bit too...
                 */
                switch(cur) {
                    case 'd': // definition
                        state = LDEF0;
                        break;
                    case 'e': // else or end
                        state = LE0;
                        break;
                    case 'v': // var
                        state = LVAR0;
                        break;
                    case 't': // then or type
                        state = LT0;
                        break;
                    case 'b': // begin
                        state = LBEGIN0;
                        break;
                    case '=': // equal
                        state = LEQ0;
                        break;
                    case ';': // semi-colon is a statement-breaker...
                        return TSEMI;
                    case 'p': // poly
                        state = LPOLY0;
                        break;
                    case '#': // line comment
                        state = LCOMMENT;
                        break;
                    case '{': // multi-line comment
                        state = LMCOMMENT;
                        break;
                    case 'm': // match
                        state = LMATCH0;
                        break;
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    case '.': // I dislike the idea of '.NNN' floats, but...
                        state = LNUM0;
                        break;
                    default:
                        state = LIDENT0;
                        break;
                }
                break;
            case LDEF0:
                if(cur == 'e') {
                    state = LDEF1;
                } else {
                    state = LIDENT0;
                }
                break;
            case LDEF1:
                if(cur == 'f') {
                    state = LDEF2;
                } else {
                    state = LIDENT0;
                }
                break;
            case LDEF2:
                if(iswhite(cur)) {
                    return TDEF;
                } else if (cur == ';') {
                    ungetc(cur, fdin);
                    return TDEF;
                } else {
                    state = LIDENT0;
                }
                break;
            case LE0:
                if(cur == 'l') {
                    state = LELSE1;
                } else if(cur == 'n') {
                    state = LEND1;
                } else {
                    state = LIDENT0;
                }
                break;
            case LELSE1:
                if(cur == 's') {
                    state = LELSE2;
                } else {
                    state = LIDENT0;
                }
                break;
            case LELSE2:
                if(cur == 'e') {
                    state = LELSE3;
                } else {
                    state = LIDENT0;
                }
                break;
            case LELSE3:
                if(iswhite(cur)) {
                    return TELSE;
                } else if(cur == ';') {
                    ungetc(cur, fdin);
                    return TELSE;
                } else {
                    state = LIDENT0;
                }
                break;
            case LVAR0:
                if(cur == 'a') {
                    state = LVAR1;
                } else {
                    state = LIDENT0;
                }
                break;
            case LVAR1:
                if(cur == 'r') {
                    state = LVAR2;
                } else {
                    state = LIDENT0;
                }
                break;
            case LVAR2:
                if(iswhite(cur)) {
                    return TVAR;
                } else if(cur == ';') {
                    ungetc(cur, fdin);
                    return TVAR;
                } else {
                    state = LIDENT0;
                }
                break;
            case LT0:
                if(cur == 'h') {
                    state = LTHEN0;
                } else if(cur == 'y') {
                    state = LTYPE0;
                } else {
                    state = LIDENT0;
                }
                break;
            case LTHEN0:
                if(cur == 'e') {
                    state = LTHEN1;
                } else {
                    state = LIDENT0;
                }
                break;
            case LTHEN1:
                if(cur == 'n') {
                    state = LTHEN2;
                } else {
                    state = LIDENT0;
                }
                break;
            case LTHEN2:
                if(iswhite(cur)) {
                    return TTHEN;
                } else if(cur == ';'){
                    ungetc(cur, fdin);
                    return TTHEN;
                } else {
                    state = LIDENT0;
                }
                break;
            case LTYPE0:
                if(cur == 'p'){
                    state = LTYPE1;
                } else {
                    state = LIDENT0;
                }
                break;
            case LTYPE1:
                if(cur == 'e') {
                    state = LTYPE2;
                } else {
                    state = LIDENT0;
                }
                break;
            case LTYPE2:
                if(iswhite(cur)) {
                    return TTYPE;
                } else if(cur == ';') {
                    ungetc(cur, fdin);
                    return TTYPE;
                } else {
                    state = LIDENT0;
                }
            case LBEGIN0:
                if(cur == 'e'){
                    state = LBEGIN1;
                } else {
                    state = LIDENT0;
                }
                break;
            case LBEGIN1:
                if(cur == 'g'){
                    state = LBEGIN2;
                } else {
                    state = LIDENT0;
                }
                break;
            case LBEGIN2:
                if(cur == 'i') {
                    state = LBEGIN3;
                } else {
                    state = LIDENT0;
                }
                break;
            case LBEGIN3:
                if(cur == 'n') {
                    state = LBEGIN4;
                } else {
                    state = LIDENT0;
                }
                break;
            case LBEGIN4:
                if(iswhite(cur)) {
                    return TBEGIN;
                } else if(cur == ';') {
                    ungetc(cur, fdin);
                    return TBEGIN;
                } else {
                    state = LIDENT0;
                }
                break;
            case LEQ0:
                if(iswhite(cur)) {
                    return TEQ;
                } else if(cur == ';') {
                    ungetc(cur, fdin);
                    return TEQ;
                } else {
                    state = LIDENT0;
                }
                break;
            case LNUM0:
                cur = fgetc(fdin);
                while(cur >= '0' && cur <= '9') {
                    buf[idx++] = cur;
                }

                if(iswhite(cur)) {
                    return TINT;
                } else if(cur == ';') {
                    ungetc(cur, fdin);
                    return TINT;
                } else {
                    state = LIDENT0;
                }
                break;
            case LIDENT0:
                break;
        }
    }
    return rc;
}

AST *
read(FILE *fdin) {
    /* _read_ from `fdin` until a single AST is constructed, or EOF
     * is reached.
     */
    return NULL;
}

int
compile(FILE *fdin, FILE *fdout) {
    /* _compile_ a file from `fdin`, using _read_, and generate some
     * decent C code from it, which is written to `fdout`.
     */
    return -1;
}
