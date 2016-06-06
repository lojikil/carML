#include <stdio.h>
#include <stdint.h>
#include <strings.h>
#include <gc.h>

#define hmalloc GC_MALLOC
#define debugln printf("dying here on line %d?\n", __LINE__);

#define nil NULL
#define nul '\0'

typedef enum {
    LSTART, LDEF0, LDEF1, LDEF2, LE0, LELSE1,
    LELSE2, LELSE3, LVAR0, LVAR1, LVAR2, LT0,
    LTHEN0, LTHEN1, LTHEN2, LTYPE0, LTYPE1,
    LTYPE2, LBEGIN0, LBEGIN1, LBEGIN2, LBEGIN3,
    LBEGIN4, LEQ0, LNUM0, LIDENT0, LEND0, LEND1,
    LEND2, LMATCH0, LCOMMENT, LMCOMMENT, LPOLY0,
    LPOLY1, LPOLY2, LPOLY3, LRECORD0,
    LRECORD1, LRECORD2, LRECORD3, LRECORD4, LRECORD5,
    LMATCH1, LMATCH2, LMATCH3, LMATCH4, LMATCH5, LEOF
} LexStates;

typedef enum {
    TDEF, TBEGIN, TEND, TEQUAL, TCOREFORM,
    TIDENT, TCALL, TOPAREN, TCPAREN, TMATCH,
    TIF, TELSE, TTHEN, TTYPE, TPOLY, TVAR,
    TARRAY, TRECORD, TINT, TFLOAT, TSTRING,
    TCHAR, TBOOL, TEQ, TSEMI, TEOF, TPARAMLIST,
    TTDECL
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
    char *value;
    uint32_t lenvalue;
    uint32_t lenchildren;
    struct _AST **children;
};

typedef struct _AST AST;

typedef enum _ASTEITHERTAG { ASTLEFT, ASTRIGHT } ASTEitherTag;

typedef struct _ASTEither {
    ASTEitherTag tag;
    struct {
        int line;
        int error;
        char *message;
    } left;
    AST *right;
} ASTEither;

char *hstrdup(const char *);
int next(FILE *, char *, int);
ASTEither *read(FILE *);
ASTEither *ASTLeft(int, int, char *);
ASTEither *ASTRight(AST *);
void walk(AST *, int);
int compile(FILE *, FILE *);
int iswhite(int);
int isident(int);
int isbrace(int);

int
main(int ac, char **al) {
    ASTEither *ret = nil;
    AST *tmp = nil;
    GC_INIT();
    do {
        printf(">>> ");
        ret = read(stdin);
        //ret = next(stdin, &buf[0], 512);
        //printf("%s %d\n", buf, ret);
        if(ret->tag == ASTLEFT) {
            printf("parse error: %s", ret->left.message);
        } else {
            tmp = ret->right;

            if(tmp->tag == TEOF) {
                break;
            } else if(tmp->tag == TIDENT && !strncmp(tmp->value, "quit", 4)) {
                break;
            }
            walk(tmp, 0);
            printf("\n");
        }
    } while(1);
    return 0;
}

ASTEither *
ASTLeft(int line, int error, char *message) {
    /* CADT would be pretty easy here, but there's little
     * point in doing what this compiler will do anyway eventually
     * So I toil away, in the dark, writing out things I know how
     * to automate, so that a brighter future may be created from
     * those dark times when we languished in the C.
     */
    ASTEither *head = (ASTEither *)hmalloc(sizeof(ASTEither));
    head->tag = ASTLEFT;
    head->left.line = line;
    head->left.error = error;
    head->left.message = hstrdup(message);
    return head;
}

ASTEither *
ASTRight(AST *head) {
    ASTEither *ret = (ASTEither *)hmalloc(sizeof(ASTEither));
    ret->tag = ASTRIGHT;
    ret->right = head;
    return ret;
}

int
iswhite(int c){
    return (c == ' ' || c == '\r' || c == '\n' || c == '\v' || c == '\t');
}

int
isbrace(int c) {
    return (c == '{' || c == '}' || c == '(' || c == ')' || c == '[' || c == ']');
}

int
isident(int c){
    return (!iswhite(c) && c != ';' && c != '"' && c != '\'' && !isbrace(c));
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
        /* XXX: this currently introduces a bug.
         * because the states for the various
         * keywords are broken out, when they hit
         * something that _isn't_ part of the
         * keyword, they end up adding it, even
         * tho it should break the ident. So, 
         * for example, "t e" should be two
         * identifiers, but because it's part
         * of the "then or type" case, it ends
         * up being *one*. could add a hack to
         * the LIDENT case to check this, or
         * reorganize the states below to be
         * better (such as via a translation
         * table). Probably will add the hack
         * to the C-version, and fix it
         * properly in the 29 version.
         */
        cur = fgetc(fdin);
        if(feof(fdin)) {
            return TEOF;
        }
        buf[idx++] = cur;
        //printf("%d %d\n", idx, state);
        switch(state) {
            case LSTART:
                if(iswhite(cur)) {
                    idx--;
                    while(cur == ' ' || cur == '\r' || cur == '\t' || cur == '\n') {
                        cur = fgetc(fdin);
                    }
                    buf[idx++] = cur;
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
                    case 'r':
                        state = LRECORD0;
                        break;
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
            case LEND0:
                if(cur == 'n') {
                    state = LEND1;
                } else {
                    state = LIDENT0;
                }
                break;
            case LEND1:
                if(cur == 'd') {
                    state = LEND2;
                } else {
                    state = LIDENT0;
                }
                break;
            case LEND2:
                if(iswhite(cur)) {
                    return TEND;
                } else if(cur == ';') {
                    ungetc(cur, fdin);
                    return TEND;
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
            case LPOLY0: 
                if(cur == 'o') {
                    state = LPOLY1;
                } else {
                    state = LIDENT0;
                }
                break;
            case LPOLY1: 
                if(cur == 'l') {
                    state = LPOLY2;
                } else {
                    state = LIDENT0;
                }
                break;
            case LPOLY2: 
                if(cur == 'y') {
                    state = LPOLY3;
                } else {
                    state = LIDENT0;
                }
                break;
            case LPOLY3:
                if(iswhite(cur)) {
                    return TPOLY;
                } else if(cur == ';') {
                    ungetc(cur, fdin);
                    return TPOLY;
                } else {
                    state = LIDENT0;
                }
                break;
            case LRECORD0: 
                if(cur == 'e') {
                    state = LRECORD1;
                } else {
                    state = LIDENT0;
                }
                break;
            case LRECORD1: 
                if(cur == 'c') {
                    state = LRECORD2;
                } else {
                    state = LIDENT0;
                }
                break;
            case LRECORD2: 
                if(cur == 'o') {
                    state = LRECORD3;
                } else {
                    state = LIDENT0;
                }
                break;
            case LRECORD3: 
                if(cur == 'r') {
                    state = LRECORD4;
                } else {
                    state = LIDENT0;
                }
                break;
            case LRECORD4: 
                if(cur == 'd') {
                    state = LRECORD5;
                } else {
                    state = LIDENT0;
                }
                break;
            case LRECORD5:
                if(iswhite(cur)) {
                    return TRECORD;
                } else if(cur == ';') {
                    ungetc(cur, fdin);
                    return TRECORD;
                } else {
                    state = LIDENT0;
                }
                break;
            case LMATCH0: 
                if(cur == 'a') {
                    state = LMATCH1;
                } else {
                    state = LIDENT0;
                }
                break;
            case LMATCH1: 
                if(cur == 't') {
                    state = LMATCH2;
                } else {
                    state = LIDENT0;
                }
                break;
            case LMATCH2: 
                if(cur == 'c') {
                    state = LMATCH3;
                } else {
                    state = LIDENT0;
                }
                break;
            case LMATCH3: 
                if(cur == 'h') {
                    state = LMATCH4;
                } else {
                    state = LIDENT0;
                }
                break;
            case LMATCH4:
                if(iswhite(cur)) {
                    return TMATCH;
                } else if(cur == ';') {
                    ungetc(cur, fdin);
                    return TMATCH;
                } else {
                    state = LIDENT0;
                }
                break;
            case LNUM0:
                cur = fgetc(fdin);
                while(cur >= '0' && cur <= '9') {
                    buf[idx++] = cur;
                    cur = fgetc(fdin);
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
                /* XXX: hairy code away */
                while(isident(cur)) {
                    cur = fgetc(fdin);
                    buf[idx++] = cur;
                }
                buf[idx - 1] = nul;
                ungetc(cur, fdin);
                return TIDENT;
                break;
        }
    }
    return rc;
}

ASTEither *
read(FILE *fdin) {
    /* _read_ from `fdin` until a single AST is constructed, or EOF
     * is reached.
     */
    AST *head = nil, *tmp = nil, *vectmp[128];
    ASTEither *sometmp = nil;
    int ltype = 0, ltmp = 0, idx = 0, flag = 0;
    char buffer[512] = {0};

    ltype = next(fdin, &buffer[0], 512);
    switch(ltype) {
        case TEOF:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TEOF;
            return ASTRight(head);
        case TDEF:
            ltmp = next(fdin, &buffer[0], 512);
            if(ltmp != TIDENT) {
                return ASTLeft(0, 0, "parser error");
            }

            head = (AST *) hmalloc(sizeof(AST));

            head->value = hstrdup(buffer);
            head->lenvalue = strnlen(buffer, 512);

            /* the specific form we're looking for here is 
             * TDEF TIDENT (TIDENT *) TEQUAL TEXPRESSION
             * that (TIDENT *) is the parameter list to non-nullary
             * functions. I wonder if there should be a specific 
             * syntax for side-effecting functions...
             */

            sometmp = read(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else {
                tmp = sometmp->right;
            }

            while(tmp->tag == TIDENT) {
                vectmp[idx++] = tmp;
                sometmp = read(fdin);

                if(sometmp->tag == ASTLEFT) {
                    return sometmp;
                } else {
                    tmp = sometmp->right;
                }

            }

            if(tmp->tag != TEQ) {
                return ASTLeft(0, 0, "parser error: a `DEF` parameter list *must* be followed by `=`");
            }

            /* convert `vectmp` into a TPARAMLIST AST node.
             */

            AST *params = nil;
            if(idx > 0) {    
                //debugln;
                params = (AST *) hmalloc(sizeof(AST));
                params->children = (AST **) hmalloc(sizeof(AST *) * idx);
                for(int i = 0; i < idx; i++) {
                    params->children[i] = vectmp[i];
                }
                //debugln;

                params->tag = TPARAMLIST;
                params->lenchildren = idx;
                //debugln;
            }

            /* ok, now that we have the parameter list and the syntactic `=` captured,
             * we read a single expression, which is the body of the procedure we're
             * defining.
             */
            sometmp = read(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else {
                tmp = sometmp->right;
            }

            /* Now, we have to build our actual TDEF AST.
             * a TDEF is technically a list:
             * - TIDENT that is our name.
             * - TPARAMLIST that holds our parameter names
             * - and some TExpression that represents our body
             */
            head->children = (AST **)hmalloc(sizeof(AST *) * 2);
            head->children[0] = params;
            head->children[1] = tmp;
            return ASTRight(head);
        case TBEGIN:
            while(1) {
                //debugln;
                sometmp = read(fdin);
                //debugln;
                if(sometmp->tag == ASTLEFT) {
                    return sometmp;
                } 
                //debugln; 
                tmp = sometmp->right;
                //debugln;
                if(tmp->tag == TEND) {
                    //debugln;
                    break;
                } else if(tmp->tag == TIDENT) {
                    /* NOTE: general strategy for emitting a CALL
                     * read values into vectmp, but mark the start
                     * of the TCALL (flag == 0), and when we hit
                     * a TSEMI (';') or TNEWL ('\n'), collapse
                     * them into a single TCALL AST node. This works
                     * in a similar vein for "()" code, but there
                     * we just read until a closing parenthesis is
                     * met (TSEMI should be a parse error there?
                     */
                } else if(tmp->tag == TSEMI || tmp->tag == TNEWL) {

                }
                //debugln;
                vectmp[idx++] = tmp;
                //printf("tmp == nil? %s\n", tmp == nil ? "yes" : "no");
            }
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TBEGIN;
            head->children = (AST **)hmalloc(sizeof(AST *) * idx);
            head->lenchildren = idx;
            for(int i = 0; i < idx; i++){ 
                //printf("vectmp[i] == nil? %s\n", vectmp[i] == nil ? "yes" : "no");
                head->children[i] = vectmp[i];
            }
            return ASTRight(head);
        case TEND:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TEND;
            return ASTRight(head);
        case TEQUAL:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TEQUAL;
            return ASTRight(head);
        case TCOREFORM:
            break;
        case TIDENT:
            head = (AST *)hmalloc(sizeof(AST));
            head->value = hstrdup(buffer);
            head->tag = TIDENT;
            return ASTRight(head);
        case TCALL:
            break;
        case TOPAREN:
            break;
        case TCPAREN:
            break;
        case TMATCH:
            break;
        case TIF:
            break;
        case TTHEN:
            break;
        case TELSE:
            break;
        case TWHEN:
            break;
        case TTYPE:
            break;
        case TPOLY:
            break;
        case TVAR:
            break;
        case TARRAY:
            break;
        case TRECORD:
            break;
        case TINT:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TINT;
            head->value = hstrdup(buffer);
            return ASTRight(head);
        case TFLOAT:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TFLOAT;
            head->value = hstrdup(buffer);
            return ASTRight(head);
        case TSTRING:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TSTRING;
            head->value = hstrdup(buffer);
            return ASTRight(head);
        case TCHAR:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TCHAR;
            head->value = hstrdup(buffer);
            return ASTRight(head);
        case TBOOL:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TBOOL;
            head->value = hstrdup(buffer);
            return ASTRight(head);
        case TEQ:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TEQ;
            return ASTRight(head);
        case TNEWL:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TNEWL;
            return ASTRight(head);
        case TSEMI:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TSEMI;
            return ASTRight(head);
    }
    return nil; 
}

void
walk(AST *head, int level) {
    int idx = 0;

    for(; idx < level; idx++) {
        printf("    ");
    }
    if(head == nil) {
        printf("(nil)\n");
        return;
    }
    switch(head->tag) {
        case TDEF:
            printf("(define %s ", head->value);
            if(head->children[0] != nil) {
                walk(head->children[0], level);
            }
            printf("\n");
            walk(head->children[1], level + 1);
            printf(")\n");
            break;
        case TPARAMLIST:
            printf("(parameter-list ");
            for(;idx < head->lenchildren; idx++) {
                walk(head->children[idx], 0); 
            }
            printf(") ");
            break;
        case TIDENT:
            printf("(identifier %s)", head->value);
            break;
        case TFLOAT:
            printf("(float %s)", head->value);
            break;
        case TINT:
            printf("(integer %s)", head->value);
            break;
        case TSTRING:
            printf("(string \"%s\")", head->value);
            break;
        case TBEGIN:
            printf("(begin %d\n", head->lenchildren);
            for(idx = 0; idx < head->lenchildren; idx++){
                walk(head->children[idx], level + 1);
                printf("\n");
            }
            printf(")\n");
            break;
        case TEND:
            break;
        default:
            printf("(tag %d)\n", head->tag);
            break;
    }
    return;
}

int
compile(FILE *fdin, FILE *fdout) {
    /* _compile_ a file from `fdin`, using _read_, and generate some
     * decent C code from it, which is written to `fdout`.
     */
    return -1;
}

char *
hstrdup(const char *s)
{
        char *ret = nil; 
        int l = 0, i = 0; 
        if(s == nil) 
            return nil; 
        l = strlen(s);
        if((ret = (char *)hmalloc(sizeof(char) * l + 1)) == nil) 
            return nil; 
        for(;i < l;i++)
            ret[i] = s[i];
        ret[i] = nul; 
        return ret; 
}
