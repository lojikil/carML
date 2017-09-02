#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <gc.h>

#define hmalloc GC_MALLOC
#define debugln printf("dying here on line %d?\n", __LINE__);

#define nil NULL
#define nul '\0'

/* Lexical analysis states.
 * basically, the tokenizer is a 
 * statemachine, and this enum represents
 * all the state machine states. Because
 * we use a decision tree (encoded in an SM)
 * for parsing out keywords, there are lots
 * of state productions here.
 */
typedef enum {
    LSTART, LDEF0, LDEF1, LDEF2, LE0, LELSE1,
    LELSE2, LELSE3, LVAL0, LVAL1, LVAL2, LT0,
    LTHEN0, LTHEN1, LTHEN2, LTYPE0, LTYPE1,
    LTYPE2, LBEGIN0, LBEGIN1, LBEGIN2, LBEGIN3,
    LBEGIN4, LEQ0, LNUM0, LIDENT0, LEND0, LEND1,
    LEND2, LMATCH0, LCOMMENT, LMCOMMENT, LPOLY0,
    LPOLY1, LPOLY2, LPOLY3, LRECORD0, LIF0, LIF1,
    LRECORD1, LRECORD2, LRECORD3, LRECORD4, LRECORD5,
    LMATCH1, LMATCH2, LMATCH3, LMATCH4, LMATCH5, LEOF,
    LWHEN0, LWHEN1, LWHEN2, LWHEN3, LNEWL, LDO0, LDO1,
    LD0, LTRUE0, LTRUE1, LTRUE3, LTRUE4, LFALSE0,
    LFALSE1, LFALSE2, LFALSE3, LFALSE4, LFALSE5,
    LFN0, LFN1, LCASE0, LCASE1, LCASE2, LCASE3, LCASE4,
    LLET0, LLET1, LLET2, LLETREC0, LLETREC1, LLETREC2,
    LLETREC3, LLETREC4, LLETREC5, LLETREC6, LL0, LCHAR0,
    LCHAR1, LCHAR2, LCHAR3, LSTRT0, LSTRT1, LSTRT2, LSTRT3,
    LSTRT4, LSTRT5, LSTRT6, LINTT0, LINTT1, LINTT2, LFLOATT0,
    LFLOATT1, LFLOATT2, LFLOATT3, LFLOATT4, LFLOATT5, LINTT3,
    LARRAY0, LARRAY1, LARRAY2, LARRAY3, LARRAY4, LARRAY5,
    LB0, LI0, LUSE0, LF0, LC0, LR0, LW0, LOF0, LOF1,
    LDEQ0, LDEQ1, LDEQ2, LDEQ3, LDEC0, LDEC1, LDEC2, LDEC3,
    LDE0, LBOOL0, LBOOL1, LBOOL2, LREF0, LELSE0, LRE0, LTRUE2,
    LWITH0, LWITH1, LWITH2, LDEC4, LUSE1, LUSE2, LVAR2, LTAG0,
    LTAGIDENT
} LexStates;

/* AST tag enum.
 * basically, this is all the AST types, and is
 * used both for determining the type of AST
 * that is represented within the tree, as well as
 * returned from the tokenizer to say what type
 * of object it thinks is in buffer
 */
typedef enum {
    TDEF, TBEGIN, TEND, TEQUAL, TCOREFORM, // 4
    TIDENT, TCALL, TOPAREN, TCPAREN, TMATCH, // 9
    TIF, TELSE, TTHEN, TTYPE, TPOLY, TVAL, // 15
    TARRAY, TRECORD, TINT, TFLOAT, TSTRING, // 20
    TCHAR, TBOOL, TEQ, TSEMI, TEOF, TPARAMLIST, // 25
    TTDECL, TWHEN, TNEWL, TDO, TUNIT, TERROR,  // 31
    TLETREC, TLET, TFN, TCASE, TSTRT, TCHART, // 37
    TINTT, TFLOATT, TCOMMENT, TREF, TDEQUET, // 41
    TBOOLT, TWITH, TOF, TDECLARE, TFALSE, // 47
    TTRUE, TUSE, TIN, TCOLON, TRECDEF, // 52
    TCOMPLEXTYPE, TCOMMA, TOARR, TCARR, // 56
    TARRAYLITERAL, TBIN, TOCT, THEX, // 60
    TARROW, TFATARROW, TCUT, TDOLLAR, // 64
    TPIPEARROW, TUSERT, TVAR, TTAG, // 68
    TPARAMDEF, // 69
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

/* Represent the return type of Readers as
 * `data EitherAST = Right (AST) | Left Int Int String`.
 * this allows us to return Either an error in the form
 * of a line number, error number, and message, *or* an
 * actual AST form.
 */
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

/* A simple type to hold an ASTEither and an
 * offset into the stream as to where we saw
 * said ASTEither. Originally this was just a
 * wrapper, but I decided to linearize it, looking
 * forward to what I'm toying wrt SRFI-57-style
 * records
 */

typedef enum _ASTOFFSETTAG { ASTOFFSETLEFT, ASTOFFSETRIGHT } ASTOffsetTag;

typedef struct _ASTOFFSET {
    int offset;
    ASTOffsetTag tag;
    struct {
        int line;
        int error;
        char *message;
    } left;
    AST *right;
} ASTOffset;

char *hstrdup(const char *);
int next(FILE *, char *, int);
AST *mung_declare(const char **, const int **, int, int);
ASTOffset *mung_single_type(const char **, const int **, int, int, int);
ASTEither *readexpression(FILE *);
ASTEither *llreadexpression(FILE *, uint8_t);
ASTEither *ASTLeft(int, int, char *);
ASTEither *ASTRight(AST *);
ASTOffset *ASTOffsetLeft(int, int, char *, int);
ASTOffset *ASTOffsetRight(AST *, int);
void walk(AST *, int);
int compile(FILE *, FILE *);
int iswhite(int);
int isident(int);
int isbrace(int);
int istypeast(int);
int issimpletypeast(int);
int iscomplextypeast(int);

int
main(int ac, char **al) {
    ASTEither *ret = nil;
    AST *tmp = nil;
    FILE *fdin = nil;
    GC_INIT();

    if(ac > 1) {
        if((fdin = fopen(al[1], "r")) == nil) {
            printf("cannot open file \"%s\"\n", al[1]);
            return 1;
        }
        do {
            ret = readexpression(fdin);
            if(ret->tag == ASTLEFT) {
                printf("parse error: %s\n", ret->left.message);
                break;
            }

            tmp = ret->right;
            if(tmp->tag != TNEWL && tmp->tag != TEOF) {
                walk(tmp, 0);
                printf("\n");
            }
        } while(tmp->tag != TEOF);
        fclose(fdin);
    } else {
        printf("\
               ___  ___ _     \n\
               |  \\/  || |    \n\
  ___ __ _ _ __| .  . || |    \n\
 / __/ _` | '__| |\\/| || |    \n\
| (_| (_| | |  | |  | || |____\n\
 \\___\\__,_|_|  \\_|  |_/\\_____/\n");
        printf("\t\tcarML/C 2016.3\n");
        printf("(c) 2016 lojikil, released under ISC License.\n\n");
        do {
            printf(">>> ");
            ret = readexpression(stdin);
            //ret = next(stdin, &buf[0], 512);
            //printf("%s %d\n", buf, ret);
            if(ret->tag == ASTLEFT) {
                printf("parse error: %s\n", ret->left.message);
            } else {
                tmp = ret->right;

                if(tmp->tag == TEOF) {
                    break;
                } else if(tmp->tag == TIDENT && !strncmp(tmp->value, "quit", 4)) {
                    break;
                } else if(tmp->tag != TNEWL) {
                    walk(tmp, 0);
                }
                printf("\n");
            }
        } while(1);
    }
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

ASTOffset *
ASTOffsetLeft(int line, int error, char *message, int offset) {
    /* CADT would be pretty easy here, but there's little
     * point in doing what this compiler will do anyway eventually
     * So I toil away, in the dark, writing out things I know how
     * to automate, so that a brighter future may be created from
     * those dark times when we languished in the C.
     */
    ASTOffset *head = (ASTOffset *)hmalloc(sizeof(ASTOffset));
    head->tag = ASTOFFSETLEFT;
    head->left.line = line;
    head->left.error = error;
    head->left.message = hstrdup(message);
    head->offset = offset;
    return head;
}

ASTOffset *
ASTOffsetRight(AST *head, int offset) {
    ASTOffset *ret = (ASTOffset *)hmalloc(sizeof(ASTOffset));
    ret->tag = ASTOFFSETRIGHT;
    ret->right = head;
    ret->offset = offset;
    return ret;
}

int
iswhite(int c){
    /* newline (\n) is *not* whitespace per se, but a token.
     * this is so that we can inform the parser of when we
     * have hit a new line in things like a begin form.
     */
    return (c == ' ' || c == '\r' || c == '\v' || c == '\t');
}

int
isbrace(int c) {
    return (c == '{' || c == '}' || c == '(' || c == ')' || c == '[' || c == ']' || c == ';' || c == ',');
}

int
isident(int c){
    return (!iswhite(c) && c != '\n' &&  c != ';' && c != '"' && c != '\'' && !isbrace(c));
}

int
istypeast(int tag) {
    /* have to have some method
     * of checking user types here...
     * perhaps we should just use 
     * idents?
     */
    switch(tag) {
        case TARRAY:
        case TINTT:
        case TCHART:
        case TDEQUET:
        case TFLOATT:
        case TSTRT:
        case TTAG: // user types 
        case TBOOLT:
            return 1;
        default:
            return 0;
    }
}

int
issimpletypeast(int tag) {
    switch(tag) {
        case TINTT:
        case TCHART:
        case TFLOATT:
        case TSTRT:
        case TBOOLT:
            return 1;
        default:
            return 0;
    }
}

int
iscomplextypeast(int tag) {
    switch(tag) {
        case TARRAY:
        case TDEQUET:
        case TTAG: // user types :|
            return 1;
        default:
            return 0;
    }
}

int
next(FILE *fdin, char *buf, int buflen) {
    /* fetch the _next_ token from input, and tie it
     * to a token type. fill buffer with the current
     * item, so that we can return identifiers or
     * numbers.
     */
    int state = 0, rc = 0, cur = 0, idx = 0, substate = 0, tagorident = TIDENT;
    int defstate = 0;
    cur = fgetc(fdin);

    /* we don't really care about whitespace other than
     * #\n, so ignore anything that is whitespace here.
     */
    if(iswhite(cur)) {
        while(iswhite(cur)) {
            cur = fgetc(fdin);
        }
    }
    /* honestly, I'm just doing this because
     * it's less ugly than a label + goto's 
     * below. Could just use calculated goto's
     * but that would tie me into gcc entirely...
     * well, clang too BUT STILL.
     */
    while(1) {
        switch(cur) {
            case '\n':
                return TNEWL;
            case '(':
                return TOPAREN;
            case ')':
                return TCPAREN;
            case '{':
                return TBEGIN;
            case '}':
                return TEND;
            case '[':
                return TOARR;
            case ']':
                return TCARR;
            case ',':
                return TCOMMA;
            case '|':
                cur = fgetc(fdin);
                if(iswhite(cur) || isbrace(cur)) {
                    buf[idx++] = '|';
                    buf[idx] = nul;
                    ungetc(cur, fdin);
                    return TIDENT;
                } else if(cur == '>') {
                    return TPIPEARROW;
                } else {
                    ungetc(cur, fdin);
                    buf[idx++] = '|';
                }
                break;
            case '=':
                cur = fgetc(fdin);
                if(iswhite(cur) || isbrace(cur)) {
                    return TEQ;
                } else if(cur == '>') {
                    return TFATARROW;
                } else {
                    /* this technically introduces a bug:
                     * because we're not tracking a substate
                     * in here, we can't see that we've already
                     * seen a '=' here. There's a few hacks:
                     * - add a state tracker.
                     * - handle the ident cases.
                     * - cry (e.g. just allow the bug).
                     *
                     * I *think* what I'll do is add a state
                     * tracker... but that will require some machinery
                     * to make sure we get down to the ident case.
                     * for now, we can just cry if the user wants to
                     * use "==" as an ident.
                     */
                    ungetc(cur, fdin);
                    buf[idx++] = '=';
                }
                break;
            case '$':
                cur = fgetc(fdin);

                if(cur == '(') {
                    return TCUT;
                } else if(iswhite(cur) || isbrace(cur)) {
                    ungetc(cur, fdin);
                    return TDOLLAR;
                } else {
                    /* same jazz here as above with '='. */
                    ungetc(cur, fdin);
                    buf[idx++] = '$';
                }
                break;
            case ';':
                return TSEMI;
            case ':':
                return TCOLON;
            case '@':
                return TDECLARE;
            case '#':
                cur = fgetc(fdin);
                while(cur != '\n' && idx < 512) {
                    buf[idx++] = cur;
                    cur = fgetc(fdin);
                }
                return TCOMMENT;
            case '"':
                cur = fgetc(fdin);
                while(cur != '"') {
                    if(cur == '\\') {
                        cur = fgetc(fdin);
                        switch(cur) {
                            case 'n':
                                buf[idx++] = '\n';
                                break;
                            case 'r':
                                buf[idx++] = '\r';
                                break;
                            case 't':
                                buf[idx++] = '\t';
                                break;
                            case 'v':
                                buf[idx++] = '\v';
                                break;
                            case '0':
                                buf[idx++] = '\0';
                                break;
                            case '"':
                                buf[idx++] = '"';
                                break;
                            default:
                                buf[idx++] = cur;
                                break;
                        }
                    } else {
                        buf[idx++] = cur;
                    }
                    cur = fgetc(fdin);
                }
                buf[idx] = '\0';
                return TSTRING;
            case '\'':
                cur = fgetc(fdin);
                if(cur == '\\') {
                    cur = fgetc(fdin);
                    switch(cur) {
                        case 'n':
                            buf[idx++] = '\n';
                            break;
                        case 'r':
                            buf[idx++] = '\r';
                            break;
                        case 't':
                            buf[idx++] = '\t';
                            break;
                        case 'v':
                            buf[idx++] = '\v';
                            break;
                        case '0':
                            buf[idx++] = '\0';
                            break;
                        case '"':
                            buf[idx++] = '"';
                            break;
                        default:
                            buf[idx++] = cur;
                            break;
                    }
                } else {
                    buf[idx++] = cur;
                }
                cur = fgetc(fdin);
                if(cur != '\'') {
                    strncpy(buf, "missing character terminator", 30);
                    return TERROR;
                }
                buf[idx] = '\0';
                return TCHAR;
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
                substate = TINT;
                while(1) {
                    switch(substate) {
                        case TINT:
                            if((cur >= '0' && cur <= '9')) {
                                buf[idx++] = cur;
                            } else if(cur == '.') {
                                buf[idx++] = cur;
                                substate = TFLOAT;
                            } else if(cur == 'x' || cur == 'X') {
                                idx--;
                                substate = THEX;
                            } else if(cur == 'b' || cur == 'B') {
                                idx--;
                                substate = TBIN;
                            } else if(cur == 'o' || cur == 'O') {
                                idx--;
                                substate = TOCT;
                            } else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                buf[idx] = '\0';
                                return TINT;
                            } else { 
                                strncpy(buf, "incorrectly formatted integer", 512);
                                return TERROR;
                            }
                            break;
                        case TFLOAT:
                            if(cur >= '0' && cur <= '9') {
                                buf[idx++] = cur;
                            } else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                buf[idx] = '\0';
                                return TFLOAT;
                            } else { 
                                strncpy(buf, "incorrectly formatted floating point numeral", 512);
                                return TERROR;
                            }
                            break;
                        case THEX:
                            if((cur >= '0' && cur <= '9') || (cur >= 'a' && cur <= 'f') || (cur >= 'A' && cur <= 'F')) {
                                buf[idx++] = cur;
                            } else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                buf[idx] = '\0';
                                return THEX;
                            } else { 
                                strncpy(buf, "incorrectly formatted hex literal", 512);
                                return TERROR;
                            }
                            break;
                        case TOCT:
                            if(cur >= '0' && cur <= '7') {
                                buf[idx++] = cur;
                            } else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                buf[idx] = '\0';
                                return TOCT;
                            } else { 
                                strncpy(buf, "incorrectly formatted octal literal", 512);
                                return TERROR;
                            }
                            break;
                        case TBIN:
                            if(cur == '0' || cur == '1') {
                                buf[idx++] = cur;
                            } else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                buf[idx] = '\0';
                                return TBIN;
                            } else { 
                                strncpy(buf, "incorrectly formatted binary literal", 512);
                                return TERROR;
                            }
                            break;
                        default:
                            strncpy(buf, "incorrectly formatted numeral", 29);
                            return TERROR;
                    }
                    cur = fgetc(fdin);
                }
            /* should be identifiers down here... */
            default:
                while(1) {
                    if(feof(fdin)) {
                        return TEOF;
                    }

                    buf[idx++] = cur;

                    // so close to being what we want, but...
                    /*if(substate == LIDENT0 && (iswhite(cur) || cur == '\n' || isbrace(cur))) {
                        debugln;
                        ungetc(cur, fdin);
                        buf[idx - 1] = '\0';
                        return TIDENT;
                    }*/

                    /* the vast majority of the code below is
                     * generated... that still doesn't mean it's
                     * very good. There is a _ton_ of reproduced
                     * code in between different states, and it
                     * would be easy to generate a state machine
                     * table that handles the transition in a more
                     * compact way. This will be the next rev of
                     * this subsystem here.
                     */
                    switch(substate) {
                        case LSTART:
                            switch(cur) {
                                case 'a':
                                    substate = LARRAY0;
                                    break;
                                case 'b':
                                    substate = LB0;
                                    break;
                                case 'c':
                                    substate = LC0;
                                    break;
                                case 'd':
                                    substate = LD0;
                                    break;
                                case 'e':
                                    substate = LE0;
                                    break;
                                case 'f':
                                    substate = LF0;
                                    break;
                                case 'i':
                                    substate = LI0;
                                    break;
                                case 'l':
                                    substate = LL0;
                                    break;
                                case 'm':
                                    substate = LMATCH0;
                                case 'o':
                                    substate = LOF0;
                                    break;
                                case 'p':
                                    substate = LPOLY0;
                                    break;
                                case 'r':
                                    substate = LR0;
                                    break;
                                case 's':
                                    substate = LSTRT0;
                                    break;
                                case 't':
                                    substate = LT0;
                                    break;
                                case 'u':
                                    substate = LUSE0;
                                    break;
                                case 'v':
                                    substate = LVAL0;
                                    break;
                                case 'w':
                                    substate = LW0;
                                    break;
                                default:
                                    if(cur >= 'A' && cur <= 'Z') {
                                        substate = LTAG0;
                                    } else {
                                        substate = LIDENT0;
                                    }
                                    break;
                            }
                            break;
                        case LB0:
                            if(cur == 'e') {
                                substate = LBEGIN0;
                            } else if(cur == 'o') {
                                substate = LBOOL0;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LBEGIN0: 
                            if(cur == 'g') {
                                substate = LBEGIN1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LBEGIN1: 
                            if(cur == 'i') {
                                substate = LBEGIN2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LBEGIN2: 
                            if(cur == 'n') {
                                substate = LBEGIN3;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LBEGIN3:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TBEGIN;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LBOOL0: 
                            if(cur == 'o') {
                                substate = LBOOL1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LBOOL1: 
                            if(cur == 'l') {
                                substate = LBOOL2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LBOOL2:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TBOOLT;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LC0:
                            if(cur == 'h') {
                                substate = LCHAR0;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else if(cur == 'a') {
                                substate = LCASE0;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LD0:
                            if(cur == 'o') {
                                substate = LDO0;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else if(cur == 'e') {
                                substate = LDE0;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LDO0:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TDO;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LDE0:
                            if(cur == 'f') {
                                substate = LDEF0;
                            } else if(cur == 'q') {
                                substate = LDEQ0;
                            } else if(cur == 'c') {
                                substate = LDEC0;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LDEQ0:
                            if(cur == 'u') {
                                substate = LDEQ1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LDEQ1:
                            if(cur == 'e') {
                                substate = LDEQ2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LDEQ2:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                buf[idx] = '\0';
                                return TDEQUET;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LDEC0: 
                            if(cur == 'l') {
                                substate = LDEC1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LDEC1: 
                            if(cur == 'a') {
                                substate = LDEC2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LDEC2: 
                            if(cur == 'r') {
                                substate = LDEC3;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LDEC3: 
                            if(cur == 'e') {
                                substate = LDEC4;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LDEC4:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TDECLARE;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LDEF0:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            } else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                buf[idx] = '\0';
                                return TDEF;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LE0:
                            if(cur == 'l') {
                               substate = LELSE0;
                            } else if(cur == 'n') {
                               substate = LEND0;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else substate = LIDENT0;
                            break;
                        case LELSE0: 
                            if(cur == 's') {
                                substate = LELSE1;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LELSE1: 
                            if(cur == 'e') {
                                substate = LELSE2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LELSE2:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TELSE;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LEND0: 
                            if(cur == 'd') {
                                substate = LEND1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LEND1:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TEND;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LF0:
                            if(cur == 'n') {
                                substate = LFN0;
                            } else if (cur == 'l') {
                                substate = LFLOATT0;
                            } else if (cur == 'a') {
                                substate = LFALSE0;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LFALSE0: 
                            if(cur == 'l') {
                                substate = LFALSE1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LFALSE1: 
                            if(cur == 's') {
                                substate = LFALSE2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LFALSE2: 
                            if(cur == 'e') {
                                substate = LFALSE3;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LFALSE3:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TFALSE;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LFLOATT0: 
                            if(cur == 'o') {
                                substate = LFLOATT1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LFLOATT1: 
                            if(cur == 'a') {
                                substate = LFLOATT2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LFLOATT2: 
                            if(cur == 't') {
                                substate = LFLOATT3;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LFLOATT3:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TFLOATT;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LFN0:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TFN;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LARRAY0:
                            if(cur == 'r') {
                                substate = LARRAY1;
                            } else {
                                substate = LIDENT0;
                            } 
                            break;
                        case LI0:
                            if(cur == 'f') {
                                substate = LIF0;
                            } else if(cur == 'n') {
                                substate = LINTT0;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LINTT0: 
                            if(cur == 't') {
                                substate = LINTT1;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TIN;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LINTT1:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TINTT;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LIF0:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TIF;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LL0:
                            if(cur == 'e') {
                                substate = LLET1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LLET0: 
                            if(cur == 'e') {
                                substate = LLET1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LLET1: 
                            if(cur == 't') {
                                substate = LLET2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LLET2:
                            if(cur == 'r') {
                                substate = LLETREC0;
                            } else if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TLET;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LLETREC0: 
                            if(cur == 'e') {
                                substate = LLETREC1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LLETREC1: 
                            if(cur == 'c') {
                                substate = LLETREC2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LLETREC2:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TLETREC;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LR0:
                            if(cur == 'e') {
                                substate = LRE0;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LRE0:
                            if(cur == 'c') {
                                substate = LRECORD0;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else if(cur == 'f') {
                                substate = LREF0;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LRECORD0: 
                            if(cur == 'o') {
                                substate = LRECORD1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LRECORD1: 
                            if(cur == 'r') {
                                substate = LRECORD2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LRECORD2: 
                            if(cur == 'd') {
                                substate = LRECORD3;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LRECORD3:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TRECORD;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LREF0:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TREF;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LT0:
                            if(cur == 'h') {
                                //debugln;
                                substate = LTHEN0;
                            } else if(cur == 'y') {
                                substate = LTYPE0;
                            } else if(cur == 'r') {
                                substate = LTRUE0;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LTHEN0: 
                            if(cur == 'e') {
                                //debugln;
                                substate = LTHEN1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LTHEN1: 
                            if(cur == 'n') {
                                //debugln;
                                substate = LTHEN2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LTHEN2:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                //debugln;
                                ungetc(cur, fdin);
                                return TTHEN;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LTYPE0: 
                            if(cur == 'p') {
                                substate = LTYPE1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LTYPE1: 
                            if(cur == 'e') {
                                substate = LTYPE2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LTYPE2:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TTYPE;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LTRUE0: 
                            if(cur == 'u') {
                                substate = LTRUE1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LTRUE1: 
                            if(cur == 'e') {
                                substate = LTRUE2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LTRUE2:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TTRUE;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LARRAY1:
                            if(cur == 'r') {
                                substate = LARRAY2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate= LIDENT0;
                            }
                            break;
                        case LARRAY2:
                            if(cur == 'a') {
                                substate = LARRAY3;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LARRAY3:
                            if(cur == 'y') {
                                substate = LARRAY4;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LARRAY4:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            } else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                buf[idx] = '\0';
                                return TARRAY;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LW0:
                            if(cur == 'h') {
                                substate = LWHEN0;
                            } else if(cur == 'i') {
                                substate = LWITH0;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LWHEN0: 
                            if(cur == 'e') {
                                substate = LWHEN1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LWHEN1: 
                            if(cur == 'n') {
                                substate = LWHEN2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LWHEN2:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TWHEN;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LWITH0: 
                            if(cur == 't') {
                                substate = LWITH1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LWITH1: 
                            if(cur == 'h') {
                                substate = LWITH2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LWITH2:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TWITH;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LOF0: 
                            if(cur == 'f') {
                                substate = LOF1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LOF1:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TOF;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LPOLY0: 
                            if(cur == 'o') {
                                substate = LPOLY1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LPOLY1: 
                            if(cur == 'l') {
                                substate = LPOLY2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LPOLY2: 
                            if(cur == 'y') {
                                substate = LPOLY3;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LPOLY3:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TPOLY;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LUSE0: 
                            if(cur == 's') {
                                substate = LUSE1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LUSE1: 
                            if(cur == 'e') {
                                substate = LUSE2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LUSE2:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TUSE;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LVAL0: 
                            //debugln;
                            if(cur == 'a') {
                                substate = LVAL1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LVAL1: 
                            if(cur == 'l') {
                                substate = LVAL2;
                            } else if(cur == 'r') {
                                substate = LVAR2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LVAL2:
                            if(isident(cur)) {
                                //debugln;
                                substate = LIDENT0;
                            } else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                //debugln;
                                ungetc(cur, fdin);
                                return TVAL;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LVAR2:
                            if(isident(cur)) {
                                //debugln;
                                substate = LIDENT0;
                            } else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                //debugln;
                                ungetc(cur, fdin);
                                return TVAR;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LSTRT0: 
                            if(cur == 't') {
                                substate = LSTRT1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LSTRT1: 
                            if(cur == 'r') {
                                substate = LSTRT2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LSTRT2: 
                            if(cur == 'i') {
                                substate = LSTRT3;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LSTRT3: 
                            if(cur == 'n') {
                                substate = LSTRT4;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LSTRT4: 
                            if(cur == 'g') {
                                substate = LSTRT5;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LSTRT5:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TSTRT;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LMATCH0: 
                            if(cur == 'a') {
                                substate = LMATCH1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LMATCH1: 
                            if(cur == 't') {
                                substate = LMATCH2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LMATCH2: 
                            if(cur == 'c') {
                                substate = LMATCH3;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LMATCH3: 
                            if(cur == 'h') {
                                substate = LMATCH4;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LMATCH4:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TMATCH;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LTAG0:
                            tagorident = TTAG;
                            substate = LTAGIDENT;
                            break;
                        case LIDENT0:
                            tagorident = TIDENT;
                            substate = LTAGIDENT;
                            break;
                        case LTAGIDENT:
                            //printf("cur == %c\n", cur);
                            if(idx > 0 && (iswhite(buf[idx - 1]) || buf[idx - 1] == '\n' || isbrace(buf[idx - 1]))) {
                                //debugln;
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                //printf("idx: %d, buffer: %s\n", idx - 1, buf);
                                return tagorident;
                            } else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return tagorident;
                            } else if(!isident(cur)) {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                    }
                    cur = fgetc(fdin);
                }
        }
    }
}

/* currently, if there's a parse error mid-stream,
 * the REPL keeps reading. I think a better way of
 * handling that would be to lex tokens until we 
 * get a TEOF, and then use that _stream_ of 
 * tokens as input to a higher-level expressions
 * builder. A bit more work, but it would make the
 * REPL experience nicer.
 */
ASTEither *
readexpression(FILE *fdin) {
    return llreadexpression(fdin, 0);
}

ASTEither *
llreadexpression(FILE *fdin, uint8_t nltreatment) {
    /* _read_ from `fdin` until a single AST is constructed, or EOF
     * is reached.
     */
    AST *head = nil, *tmp = nil, *vectmp[128];
    ASTEither *sometmp = nil;
    int ltype = 0, ltmp = 0, idx = 0, flag = -1, typestate = 0;
    char buffer[512] = {0};
    char name[8] = {0};
    char errbuf[512] = {0};

    ltype = next(fdin, &buffer[0], 512);
    switch(ltype) {
        case TDOLLAR:
            tmp = (AST *)hmalloc(sizeof(AST));
            tmp->tag = TDOLLAR;
            return ASTRight(tmp);
        case TCOMMENT:
            /* do we want return this, so that we can
             * return documentation, &c.?
             */
            sometmp = readexpression(fdin);
            return sometmp;
        case TERROR:
            return ASTLeft(0, 0, hstrdup(&buffer[0]));
        case TEOF:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TEOF;
            return ASTRight(head);
        case TLET:
        case TLETREC:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = ltype;

            /* setup:
             * - head->value = name binding
             * - head->children[0] = value
             * - head->children[1] = body
             * - head->children[2] = type (optional)
             */


            if(ltype == TLET) {
                strncpy(name, "let", 3);
            } else {
                strncpy(name, "letrec", 6);
            }

            sometmp = readexpression(fdin);
            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else {
                tmp = sometmp->right;
            }

            if(tmp->tag != TIDENT) {
                snprintf(&errbuf[0],
                         512,
                         "%s's binding *must* be an IDENT: `%s IDENT = EXPRESION in EXPRESSION`",
                         name, name);
                return ASTLeft(0, 0, hstrdup(&errbuf[0]));
            }

            head->value = hstrdup(tmp->value);

            // add types here

            sometmp = readexpression(fdin);
            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else if(sometmp->right->tag != TEQ && sometmp->right->tag != TCOLON) {
                snprintf(&errbuf[0], 512,
                         "%s's IDENT must be followed by an `=`: `%s IDENT = EXPRESSION in EXPRESSION`",
                         name, name);
                return ASTLeft(0, 0, hstrdup(&errbuf[0]));
            } else if(sometmp->right->tag == TCOLON) {
                /* ok, the user is specifying a type here
                 * we have to consume the type, and then
                 * store it.
                 */
                head->lenchildren = 3;
                head->children = (AST **)hmalloc(sizeof(AST *) * 3);
                
                sometmp = readexpression(fdin);

                if(sometmp->tag == ASTLEFT) {
                    return sometmp;
                } else if(!istypeast(sometmp->right->tag)) {
                    return ASTLeft(0, 0, "a `:` form *must* be followed by a type definition...");
                } else if(issimpletypeast(sometmp->right->tag)) {
                    head->children[2] = sometmp->right;
                } else {
                    /* complex type...
                     */
                    flag = idx;
                    /* we hit a complex type,
                     * now we're looking for 
                     * either `of` or `=`.
                     */
                    vectmp[idx++] = sometmp->right;
                    typestate = 1; 
                    while(sometmp->right->tag != TEQ) {
                        sometmp = readexpression(fdin);

                        if(sometmp->right->tag == ASTLEFT) {
                            return sometmp;
                        }

                        switch(typestate) {
                            case 0: // awaiting a type
                                if(!istypeast(sometmp->right->tag)) {
                                    return ASTLeft(0, 0, "expected type in `:` form");
                                } else if(issimpletypeast(sometmp->right->tag)) {
                                    typestate = 2;
                                } else {
                                    typestate = 1;
                                }
                                vectmp[idx++] = sometmp->right;
                                break;
                            case 1: // awaiting either TOF or an end
                                if(sometmp->right->tag == TOF) {
                                    typestate = 0;
                                } else if(sometmp->right->tag == TEQ) {
                                    typestate = 3;
                                } else {
                                    return ASTLeft(0, 0, "expected either an `of` or a `=`");
                                }
                                break;
                            case 2:
                            case 3:
                                break;
                        }
                        if(typestate == 2 || typestate == 3) {
                            break;
                        }
                    }
                    /* collapse the above type states here... */
                    tmp = (AST *) hmalloc(sizeof(AST));
                    tmp->tag = TCOMPLEXTYPE;
                    tmp->lenchildren = idx - flag;
                    tmp->children = (AST **) hmalloc(sizeof(AST *) * tmp->lenchildren);
                    for(int cidx = 0, tidx = flag, tlen = tmp->lenchildren; cidx < tlen; cidx++, tidx++) {
                        tmp->children[cidx] = vectmp[tidx];
                    }
                    vectmp[flag] = tmp;
                    idx = flag;
                    flag = 0;
                    head->children[2] = tmp;
                }
               
                if(typestate != 3) {

                    sometmp = readexpression(fdin);

                    if(sometmp->tag == ASTLEFT) {
                        return sometmp;
                    } else if(sometmp->right->tag != TEQ) {
                        return ASTLeft(0, 0, "a `let` type definition *must* be followed by an `=`...");
                    }
                }

            } else { 
                /* if we hit a TEQ, then we don't need any extra allocation,
                 * just two slots.
                 */
                head->lenchildren = 2;
                head->children = (AST **)hmalloc(sizeof(AST *) * 2);
            }

            sometmp = readexpression(fdin);
            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else {
                head->children[0] = sometmp->right;
            }

            sometmp = readexpression(fdin);
            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else {
                tmp = sometmp->right;
            }

            /* really should check if this is an
             * `and`, such that we can have multiple 
             * bindings...
             */
            if(tmp->tag != TIN) {
                snprintf(&errbuf[0], 512,
                         "%s's EXPR  must be followed by an `in`: `%s IDENT = EXPRESSION in EXPRESSION`",
                         name, name);
                return ASTLeft(0, 0, hstrdup(&errbuf[0]));
            }

            sometmp = readexpression(fdin);
            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else {
                head->children[1] = sometmp->right;
            }

            return ASTRight(head);
        case TDECLARE:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TDECLARE;
            head->lenchildren = 2;
            head->children = (AST **)hmalloc(sizeof(AST *) * 2);
            int substate = 0, curoffset = 0;
            char *decls[32] = {0};
            int lexemes[32] = {0};

            /* the structure of a declaration:
             * head->value: ident name being declared
             * head->children[0] = parameter-list params
             * head->children[1] = return type;
             */

            ltmp = next(fdin, &buffer[0], 512);
            if(ltmp != TIDENT) {
                return ASTLeft(0, 0, "parser error");
            }

            head->value = hstrdup(buffer);

            /* ok, so first we collate the terms of the declaration
             * into a pair of stacks, one side is the string representation
             * the other the lexeme type.
             */
            do {
                ltmp = next(fdin, &buffer[0], 512);
                if(ltmp == TEOF || ltmp == TERROR || ltmp == TNEWL) {
                    break;
                }

                decls[curoffset] = hstrdup(buffer);
                lexemes[curoffset] = ltmp;
                curoffset++;
            } while(ltmp != TNEWL);

            /* next, we iterate o'er the list of collected terms
             * and parse them as a collection of type declarations.
             */

            tmp = mung_declare((const char **)decls, (const int **)lexemes, curoffset, TNEWL);

            head->children[0] = tmp;

            return ASTRight(head);
        case TFN:
        case TDEF:
            head = (AST *) hmalloc(sizeof(AST));
            head->tag = ltype;
            int loopflag = 1;
            AST *params = nil;

            if(ltype == TDEF){
                ltmp = next(fdin, &buffer[0], 512);
                if(ltmp != TIDENT) {
                    return ASTLeft(0, 0, "parser error");
                }
                head->value = hstrdup(buffer);
                head->lenvalue = strnlen(buffer, 512);
            }


            /* the specific form we're looking for here is 
             * TDEF TIDENT (TIDENT *) TEQUAL TEXPRESSION
             * that (TIDENT *) is the parameter list to non-nullary
             * functions. I wonder if there should be a specific 
             * syntax for side-effecting functions...
             */
            
            #ifdef NEVERDEF
            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else {
                tmp = sometmp->right;
            }
            #endif


            /*
             * so, we have to change this a bit. Instead of reading
             * in just a list of idents, we need to accept that there
             * can be : and => in here as well. Basically, we want to
             * be able to parse this:
             * 
             * def foo x : int bar : array of int => int = {
             *     # ...
             * }
             * 
             * this same code could then be used to make @ work, even
             * if the current stream idea in @ is nicer
             * actually, should just steal the code from records for 
             * here... besides, the same code could then be used for
             * declare... 
             */

            while(loopflag) {

                switch(typestate) {
                    case 0:
                        sometmp = readexpression(fdin);
                        debugln;
                        if(sometmp->tag == ASTLEFT) {
                            return sometmp;
                        } else if(sometmp->right->tag == TIDENT) {
                            // name
                            debugln;
                            typestate = 1;
                        } else if(sometmp->right->tag == TFATARROW) {
                            // return type
                            debugln;
                            typestate = 2;
                        } else if(sometmp->right->tag == TEQ) {
                            // body
                            debugln;
                            typestate = 3;
                        } else {
                            debugln;
                            printf("tag == %d\n", sometmp->right->tag);
                            return ASTLeft(0, 0, "`def` must have either a parameter list, a fat-arrow, or an equals sign.");
                        } 
                        debugln;
                        printf("typestate == %d\n", typestate);
                        break;
                    case 1: // TIDENT
                        debugln;
                        vectmp[idx] = sometmp->right;
                        flag = idx;
                        idx++;
                        sometmp = readexpression(fdin);
                        debugln;
                        if(sometmp->tag == ASTLEFT) {
                            debugln;
                            return sometmp;
                        } else if(sometmp->right->tag == TIDENT) {
                            debugln;
                            typestate = 1;
                        } else if(sometmp->right->tag == TFATARROW) {
                            debugln;
                            typestate = 2;
                        } else if(sometmp->right->tag == TEQ) {
                            debugln;
                            typestate = 3;
                        } else if(sometmp->right->tag == TCOLON) {
                            debugln;
                            typestate = 4;
                        } else {
                            debugln;
                            printf("tag == %d\n", sometmp->right->tag);
                            return ASTLeft(0, 0, "`def` identifiers *must* be followed by `:`, `=>`, or `=`");
                        }
                        break;
                    case 3: // TEQ, start function
                        /* mark the parameter list loop as closed,
                         * and begin to process the elements on the
                         * stack as a parameter list.
                         */
                        debugln;
                        loopflag = 0;
                        if(idx > 0) {
                            debugln;
                            params = (AST *) hmalloc(sizeof(AST));
                            params->children = (AST **) hmalloc(sizeof(AST *) * idx);
                            for(int i = 0; i < idx; i++) {
                                params->children[i] = vectmp[i];
                            }
                            debugln;

                            params->tag = TPARAMLIST;
                            params->lenchildren = idx;
                            debugln;
                        }
                        break;
                    case 2: // TFATARROW, return
                    case 4: // type

                        sometmp = readexpression(fdin, 1);

                        if(sometmp->tag == ASTLEFT) {
                            return sometmp;
                        } else if(!istypeast(sometmp->right->tag)) {
                            return ASTLeft(0, 0, "a `:` form *must* be followed by a type definition...");
                        } else if(issimpletypeast(sometmp->right->tag)) { // simple type
                            tmp = (AST *)hmalloc(sizeof(AST));
                            tmp->tag = TPARAMDEF;
                            tmp->lenchildren = 2;
                            tmp->children = (AST **)hmalloc(sizeof(AST *) * 2);
                            tmp->children[0] = vectmp[idx - 1];
                            tmp->children[1] = sometmp->right;
                            if(typestate == 2) {
                                vectmp[idx - 1] = tmp;
                            } else {
                                returntype = tmp;
                            }
                        } else { // complex type
                            vectmp[idx] = sometmp->right;
                            typestate = 5;
                        }
                        break;
                }
            }

                #ifdef NEVERDEF
                vectmp[idx++] = tmp;

                sometmp = llreadexpression(fdin, 1);
                if(sometmp->tag == ASTLEFT) {
                    return sometmp;
                } else if(sometmp->right->tag == TCOLON) {
                    sometmp = llreadexpression(fdin, 1);
                    if(sometmp->tag == ASTLEFT) {
                        return sometmp;
                    } if(!istypeast(sometmp->right->tag)) {
                        return ASTLeft(0, 0, "a `:` form *must* be followed by a type definition...");
                    } else if(issimpletypeast(sometmp->right->tag)) {
                        vectmp[idx] = sometmp->right;
                        sometmp = llreadexpression(fdin, 1);
                        if(sometmp->tag == ASTLEFT) {
                            return sometmp;
                        } else if(sometmp->right->tag != TCOMMA && sometmp->right->tag != TFATARROW && sometmp->right->tag != TEQ) {
                            return ASTLeft(0, 0, "a simple type *must* be followed by a comma, a fat-arrow or an equals sign...");
                        }
                        /* we have determined a `val <name> : <simple-type>` form
                         * here, so store them in the record's definition.
                         */
                        tmp = (AST *)hmalloc(sizeof(AST));
                        tmp->tag = TPARAMDEF;
                        tmp->lenchildren = 2;
                        tmp->children = (AST **)hmalloc(sizeof(AST *) * 2);
                        tmp->children[0] = vectmp[idx - 1];
                        tmp->children[1] = vectmp[idx];
                        vectmp[idx - 1] = tmp;
                    } else {
                        /* complex type...
                         h*/
                        flag = idx;
                        /* we hit a complex type,
                         * now we're looking for 
                         * either `of` or `=`.
                         */
                        vectmp[idx++] = sometmp->right;
                        typestate = 1; 
                        while(sometmp->right->tag != TEQ) {

                            sometmp = llreadexpression(fdin, 1);
                            if(sometmp->right->tag == ASTLEFT) {
                                return sometmp;
                            }

                            switch(typestate) {
                                case 0: // awaiting a type
                                    if(!istypeast(sometmp->right->tag)) {
                                        return ASTLeft(0, 0, "expected type in `:` form");
                                    } else if(issimpletypeast(sometmp->right->tag)) {
                                        typestate = 2;
                                    } else {
                                        typestate = 1;
                                    }
                                    vectmp[idx++] = sometmp->right;
                                    break;
                                case 1: // awaiting either TOF or an end
                                    if(sometmp->right->tag == TOF) {
                                        typestate = 0;
                                    } else if(sometmp->right->tag == TCOMMA) {
                                        typestate = 3;
                                    } else {
                                        return ASTLeft(0, 0, "expected either a comma or an `of`");
                                    }
                                    break;
                                case 2:
                                case 3:
                                    break;
                            }
                            if(typestate == 2 || typestate == 3) {
                                break;
                            }
                        }

                        /* collapse the above type states here... */
                        AST *ctmp = (AST *) hmalloc(sizeof(AST));
                        ctmp->tag = TCOMPLEXTYPE;
                        ctmp->lenchildren = idx - flag;
                        ctmp->children = (AST **) hmalloc(sizeof(AST *) * ctmp->lenchildren);
                        for(int cidx = 0, tidx = flag, tlen = ctmp->lenchildren; cidx < tlen; cidx++, tidx++) {
                            ctmp->children[cidx] = vectmp[tidx];
                        }

                        /* create the record field defition holder */
                        tmp = (AST *)hmalloc(sizeof(AST));
                        tmp->tag = TPARAMDEF;
                        tmp->lenchildren = 2;
                        tmp->children = (AST **)hmalloc(sizeof(AST *) * 2);
                        tmp->children[0] = vectmp[flag - 1];
                        tmp->children[1] = ctmp;
                        vectmp[flag - 1] = tmp;
                        idx = flag;
                        flag = 0;
                        if(typestate != 3) {

                            sometmp = llreadexpression(fdin, 1);

                            if(sometmp->tag == ASTLEFT) {
                                return sometmp;
                            } else if(sometmp->right->tag != TCOMMA && sometmp->right->tag != TFATARROW && sometmp->right->tag != TEQ) {
                                return ASTLeft(0, 0, "a `parameter` type definition *must* be followed by a comma, a fat-arrow or an equal");
                            }
                        }
                    }
                } else if(sometmp->right->tag == TCOMMA) {
                    tmp = (AST *)hmalloc(sizeof(AST));
                    tmp->tag = TPARAMDEF;
                    tmp->lenchildren = 1;
                    tmp->children = (AST **)hmalloc(sizeof(AST *));
                    tmp->children[0] = vectmp[idx - 1];
                    vectmp[idx - 1] = tmp;
                } else {
                    /* we didn't see a `:` or a #\n, so that's
                     * an error.
                     */
                    return ASTLeft(0, 0, "malformed parameter list");
                }

            }
            /* so, we've constructed a list of parameter members.
             * now, we just need to actually _make_ the parameter
             * structure now.
             */
            head->lenchildren = idx;
            head->children = (AST **)hmalloc(sizeof(AST *) * idx);
            for(int i = 0; i < idx; i++){
                head->children[i] = vectmp[i];
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

            #endif

            /* ok, now that we have the parameter list and the syntactic `=` captured,
             * we read a single expression, which is the body of the procedure we're
             * defining.
             */
            sometmp = readexpression(fdin);

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
            head->lenchildren = 2;
            head->children[0] = params;
            head->children[1] = tmp;
            return ASTRight(head);
        case TIF:
            /* this code is surprisingly gross.
             * look to clean this up a bit.
             */

            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TIF;
            head->children = (AST **)hmalloc(sizeof(AST *) * 3);
            head->lenchildren = 3;

            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            }

            head->children[0] = sometmp->right;

            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            }

            tmp = sometmp->right;

            if(tmp->tag != TTHEN) {
                return ASTLeft(0, 0, "missing THEN keyword after IF conditional: if conditional then expression else expression");
            }

            sometmp = readexpression(fdin);
            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            }

            head->children[1] = sometmp->right;

            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            }

            tmp = sometmp->right;

            if(tmp->tag != TELSE) {
                return ASTLeft(0, 0, "missing ELSE keyword after THEN value: if conditional then expression else expression");
            }

            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            }

            head->children[2] = sometmp->right;

            return ASTRight(head);
        case TBEGIN:
            while(1) {
                //debugln;
                sometmp = llreadexpression(fdin, 1); // probably should make a const for WANTNL
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
                } else if(tmp->tag == TIDENT || tmp->tag == TTAG) {
                    /* NOTE: general strategy for emitting a CALL
                     * read values into vectmp, but mark the start
                     * of the TCALL (flag == 0), and when we hit
                     * a TSEMI (';') or TNEWL ('\n'), collapse
                     * them into a single TCALL AST node. This works
                     * in a similar vein for "()" code, but there
                     * we just read until a closing parenthesis is
                     * met (TSEMI should be a parse error there?
                     */
                    if(flag == -1) {
                        /* set the TCALL flag, for collapsing later */ 
                        flag = idx;
                    }
                } else if(flag != -1 && (tmp->tag == TSEMI || tmp->tag == TNEWL || tmp->tag == TEND)) {
                    /* collapse the call into a TCALL
                     * this has some _slight_ problems, since it 
                     * uses the same stack as the TBEGIN itself,
                     * so as we add items to the body, there will
                     * be less left over for intermediate calls...
                     * could add another stack, or could just do it
                     * this way to see how much of a problem it actually
                     * is...
                     */
                    AST *tcall = (AST *)hmalloc(sizeof(AST));
                    tcall->tag = TCALL;
                    tcall->lenchildren = idx - flag;
                    //printf("idx == %d, flag == %d\n", idx, flag);
                    tcall->children = (AST **)hmalloc(sizeof(AST *) * tcall->lenchildren);
                    //printf("len == %d\n", tcall->lenchildren);
                    for(int i = 0; i < tcall->lenchildren; i++) {
                        //printf("i == %d\n", i);
                        AST* ttmp = vectmp[flag + i];
                        /*walk(ttmp, 0);
                        printf("\n");*/
                        tcall->children[i] = vectmp[flag + i];
                        /*walk(tcall->children[i], 0);
                        printf("\n");*/
                    }
                    idx = flag;
                    flag = -1;
                    tmp = tcall;
                } else if(tmp->tag == TNEWL) {
                    continue;
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
        case TIN:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TIN;
            return ASTRight(head);
        case TEQUAL:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TEQUAL;
            return ASTRight(head);
        case TCOREFORM:
            break;
        case TTAG:
        case TIDENT:
            head = (AST *)hmalloc(sizeof(AST));
            head->value = hstrdup(buffer);
            head->tag = ltype;
            return ASTRight(head);
        case TCALL:
            break;
        case TCUT:
        case TOPAREN:
            sometmp = readexpression(fdin);
            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            }

            tmp = sometmp->right;

            if(tmp->tag == TCPAREN) {
                if(ltype == TCUT){
                    return ASTLeft(0, 0, "illegal $() form");
                }
                tmp = (AST *)hmalloc(sizeof(AST));
                tmp->tag = TUNIT;
                return ASTRight(tmp);
            } else if(tmp->tag != TIDENT && tmp->tag != TTAG) {
                return ASTLeft(0, 0, "cannot call non-identifier object");
            }

            vectmp[idx++] = tmp;

            while(1) {
                sometmp = readexpression(fdin);
                if(sometmp->tag == ASTLEFT) {
                    return sometmp;
                }

                tmp = sometmp->right;
                if(tmp->tag == TCPAREN){
                    break;
                } else if(tmp->tag == TSEMI) {
                    return ASTLeft(0, 0, "illegal semicolon within parenthetical call");
                } else if(tmp->tag == TNEWL) {
                    continue;
                } else {
                    vectmp[idx++] = tmp;        
                }
            }

            head = (AST *)hmalloc(sizeof(AST));
            if(ltype == TCUT) {
                head->tag = TCUT;
            } else {
                head->tag = TCALL;
            }
            head->lenchildren = idx;
            head->children = (AST **)hmalloc(sizeof(AST *) * idx);
            for(int i = 0; i < idx; i++) {
                head->children[i] = vectmp[i];
            }
            return ASTRight(head);
        case TCPAREN:
            head = (AST *)hmalloc(sizeof(AST));
            head->value = hstrdup(buffer);
            head->tag = TCPAREN;
            return ASTRight(head);
        case TMATCH:
            break;
        case TTHEN:
            head = (AST *)hmalloc(sizeof(AST));
            head->value = hstrdup(buffer);
            head->tag = TTHEN;
            return ASTRight(head);
        case TELSE:
            head = (AST *)hmalloc(sizeof(AST));
            head->value = hstrdup(buffer);
            head->tag = TELSE;
            return ASTRight(head);
        case TWHEN:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TWHEN;
            head->children = (AST **)hmalloc(sizeof(AST *) * 2);
            head->lenchildren = 2;

            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else {
                head->children[0] = sometmp->right;
            }

            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else {
                tmp = sometmp->right;
            }

            if(tmp->tag != TDO) {
                return ASTLeft(0, 0, "missing `do` statement from `when`: when CONDITION do EXPRESSION");
            }

            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else {
                head->children[1] = sometmp->right;
            }

            return ASTRight(head);
        case TDO:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TDO;
            return ASTRight(head);
        case TOF:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TOF;
            return ASTRight(head);
        case TTYPE:
            break;
        case TPOLY:
            break;
        case TVAL:
        case TVAR:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = ltype;
            
            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else if(sometmp->right->tag != TIDENT) {
                return ASTLeft(0, 0, "val's name *must* be an identifier: `val IDENTIFIER = EXPRESSION`");
            } else {
                head->value = sometmp->right->value;
            }

            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else if(sometmp->right->tag != TEQ && sometmp->right->tag != TCOLON){
                printf("error: %d\n", sometmp->right->tag);
                return ASTLeft(0, 0, "val's identifiers *must* be followed by an `=`: `val IDENTIFIER = EXPRESSION`");
            } else if(sometmp->right->tag == TCOLON) {
                /* ok, the user is specifying a type here
                 * we have to consume the type, and then
                 * store it.
                 */
                head->lenchildren = 2;
                head->children = (AST **)hmalloc(sizeof(AST *) * 2);
                
                sometmp = readexpression(fdin);

                if(sometmp->tag == ASTLEFT) {
                    return sometmp;
                } else if(!istypeast(sometmp->right->tag)) {
                    return ASTLeft(0, 0, "a `:` form *must* be followed by a type definition...");
                } else if(issimpletypeast(sometmp->right->tag)) {
                    head->children[1] = sometmp->right;
                } else {
                    /* complex type...
                     */
                    flag = idx;
                    /* we hit a complex type,
                     * now we're looking for 
                     * either `of` or `=`.
                     */
                    vectmp[idx++] = sometmp->right;
                    typestate = 1; 
                    while(sometmp->right->tag != TEQ) {
                        sometmp = readexpression(fdin);

                        if(sometmp->right->tag == ASTLEFT) {
                            return sometmp;
                        }

                        switch(typestate) {
                            case 0: // awaiting a type
                                if(!istypeast(sometmp->right->tag)) {
                                    return ASTLeft(0, 0, "expected type in `:` form");
                                } else if(issimpletypeast(sometmp->right->tag)) {
                                    typestate = 2;
                                } else {
                                    typestate = 1;
                                }
                                vectmp[idx++] = sometmp->right;
                                break;
                            case 1: // awaiting either TOF or an end
                                if(sometmp->right->tag == TOF) {
                                    typestate = 0;
                                } else if(sometmp->right->tag == TEQ) {
                                    typestate = 3;
                                } else {
                                    return ASTLeft(0, 0, "expected either an `of` or a `=`");
                                }
                                break;
                            case 2:
                            case 3:
                                break;
                        }
                        if(typestate == 2 || typestate == 3) {
                            break;
                        }
                    }
                    /* collapse the above type states here... */
                    tmp = (AST *) hmalloc(sizeof(AST));
                    tmp->tag = TCOMPLEXTYPE;
                    tmp->lenchildren = idx - flag;
                    tmp->children = (AST **) hmalloc(sizeof(AST *) * tmp->lenchildren);
                    for(int cidx = 0, tidx = flag, tlen = tmp->lenchildren; cidx < tlen; cidx++, tidx++) {
                        tmp->children[cidx] = vectmp[tidx];
                    }
                    vectmp[flag] = tmp;
                    idx = flag;
                    flag = 0;
                    head->children[1] = tmp;
                }
               
                if(typestate != 3) {

                    sometmp = readexpression(fdin);

                    if(sometmp->tag == ASTLEFT) {
                        return sometmp;
                    } else if(sometmp->right->tag != TEQ) {
                        return ASTLeft(0, 0, "a `val` type definition *must* be followed by an `=`...");
                    }
                }

            } else {
                head->lenchildren = 1;
                head->children = (AST **)hmalloc(sizeof(AST *));
            }

            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else {
                head->children[0] = sometmp->right;
            }

            return ASTRight(head);
            break;
        case TARRAY:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TARRAY;
            return ASTRight(head);
        case TCOMMA:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TCOMMA;
            return ASTRight(head);
        case TOARR:
            flag = idx;
            sometmp = readexpression(fdin);
            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            }
            while(sometmp->right->tag != TCARR) {
                if(sometmp->right->tag != TCOMMA) {
                    vectmp[idx++] = sometmp->right;
                }
                sometmp = readexpression(fdin);
            }
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TARRAYLITERAL;
            head->lenchildren = idx - flag;
            head->children = (AST **)hmalloc(sizeof(AST *) * (idx - flag));
            for(int cidx = 0; flag < idx; cidx++, flag++) {
                head->children[cidx] = vectmp[flag];
            }
            return ASTRight(head);
        case TCARR:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TCARR;
            return ASTRight(head);
        case TRECORD:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TRECORD;
            
            sometmp = llreadexpression(fdin, 1);
            /* I like this 3-case block-style
             * I think *this* should be the
             * "expect" function, but it's
             * close enough to have this pattern
             * throughout...
             */
            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else if(sometmp->right->tag != TIDENT) {
                return ASTLeft(0, 0, "record's name *must* be an identifier: `record IDENTIFIER record-definition`");
            } else {
                tmp = sometmp->right;
                head->value = tmp->value;
            }

            /* so now we are at the point where
             * we have read the _name_ of the 
             * record, and we have read the 
             * `=`, so now we need to read 
             * the definition of the record.
             * I have some thought that we may
             * want to allow a record definition
             * to be <ident (: type)> | { <ident (: type)> + }
             * but I also don't know if there
             * will be a huge number of
             * use-cases for single-member
             * records...
             */

            ltmp = next(fdin, &buffer[0], 512);

            if(ltmp != TBEGIN) {
                return ASTLeft(0, 0, "record-defitintion *must* begin with BEGIN");
            }

            /* here, we just need to read:
             * 1. an ident.
             * 2. either a `:` or #\n
             * 3. if `:`, we then need to read a type.
             */

            while(tmp->tag != TEND) {
                sometmp = llreadexpression(fdin, 1);
                
                if(sometmp->tag == ASTLEFT) {
                    return sometmp;
                } else if(sometmp->right->tag == TNEWL || sometmp->right->tag == TSEMI) {
                    continue;
                } else if(sometmp->right->tag != TIDENT && sometmp->right->tag != TEND) {
                    printf("%d", sometmp->right->tag);
                    return ASTLeft(0, 0, "a `record`'s members *must* be identifiers: `name (: type)`"); 
                } else if(sometmp->right->tag == TEND) {
                    break;
                } else {
                    tmp = sometmp->right;
                }
                vectmp[idx++] = tmp;

                sometmp = llreadexpression(fdin, 1);
                if(sometmp->tag == ASTLEFT) {
                    return sometmp;
                } else if(sometmp->right->tag == TCOLON) {
                    sometmp = llreadexpression(fdin, 1);
                    if(sometmp->tag == ASTLEFT) {
                        return sometmp;
                    } if(!istypeast(sometmp->right->tag)) {
                        return ASTLeft(0, 0, "a `:` form *must* be followed by a type definition...");
                    } else if(issimpletypeast(sometmp->right->tag)) {
                        vectmp[idx] = sometmp->right;
                        sometmp = llreadexpression(fdin, 1);
                        if(sometmp->tag == ASTLEFT) {
                            return sometmp;
                        } else if(sometmp->right->tag != TNEWL && sometmp->right->tag != TSEMI) {
                            return ASTLeft(0, 0, "a simple type *must* be followed by a new line or semi-colon...");
                        }
                        /* we have determined a `val <name> : <simple-type>` form
                         * here, so store them in the record's definition.
                         */
                        tmp = (AST *)hmalloc(sizeof(AST));
                        tmp->tag = TRECDEF;
                        tmp->lenchildren = 2;
                        tmp->children = (AST **)hmalloc(sizeof(AST *) * 2);
                        tmp->children[0] = vectmp[idx - 1];
                        tmp->children[1] = vectmp[idx];
                        vectmp[idx - 1] = tmp;
                    } else {
                        /* complex type...
                         h*/
                        flag = idx;
                        /* we hit a complex type,
                         * now we're looking for 
                         * either `of` or `=`.
                         */
                        vectmp[idx++] = sometmp->right;
                        typestate = 1; 
                        while(sometmp->right->tag != TEQ) {

                            sometmp = llreadexpression(fdin, 1);
                            if(sometmp->right->tag == ASTLEFT) {
                                return sometmp;
                            }

                            switch(typestate) {
                                case 0: // awaiting a type
                                    if(!istypeast(sometmp->right->tag)) {
                                        return ASTLeft(0, 0, "expected type in `:` form");
                                    } else if(issimpletypeast(sometmp->right->tag)) {
                                        typestate = 2;
                                    } else {
                                        typestate = 1;
                                    }
                                    vectmp[idx++] = sometmp->right;
                                    break;
                                case 1: // awaiting either TOF or an end
                                    if(sometmp->right->tag == TOF) {
                                        typestate = 0;
                                    } else if(sometmp->right->tag == TNEWL || sometmp->right->tag == TSEMI) {
                                        typestate = 3;
                                    } else {
                                        return ASTLeft(0, 0, "expected either a newline or a `;`");
                                    }
                                    break;
                                case 2:
                                case 3:
                                    break;
                            }
                            if(typestate == 2 || typestate == 3) {
                                break;
                            }
                        }

                        /* collapse the above type states here... */
                        AST *ctmp = (AST *) hmalloc(sizeof(AST));
                        ctmp->tag = TCOMPLEXTYPE;
                        ctmp->lenchildren = idx - flag;
                        ctmp->children = (AST **) hmalloc(sizeof(AST *) * ctmp->lenchildren);
                        for(int cidx = 0, tidx = flag, tlen = ctmp->lenchildren; cidx < tlen; cidx++, tidx++) {
                            ctmp->children[cidx] = vectmp[tidx];
                        }

                        /* create the record field defition holder */
                        tmp = (AST *)hmalloc(sizeof(AST));
                        tmp->tag = TRECDEF;
                        tmp->lenchildren = 2;
                        tmp->children = (AST **)hmalloc(sizeof(AST *) * 2);
                        tmp->children[0] = vectmp[flag - 1];
                        tmp->children[1] = ctmp;
                        vectmp[flag - 1] = tmp;
                        idx = flag;
                        flag = 0;
                        if(typestate != 3) {

                            sometmp = llreadexpression(fdin, 1);

                            if(sometmp->tag == ASTLEFT) {
                                return sometmp;
                            } else if(sometmp->right->tag != TSEMI && sometmp->right->tag != TNEWL && sometmp->right->tag != TEND) {
                                return ASTLeft(0, 0, "a `record` type definition *must* be followed by a newline, a semicolon or an END");
                            }
                        }
                    }
                } else if(sometmp->right->tag == TNEWL || sometmp->right->tag == TSEMI) {
                    tmp = (AST *)hmalloc(sizeof(AST));
                    tmp->tag = TRECDEF;
                    tmp->lenchildren = 1;
                    tmp->children = (AST **)hmalloc(sizeof(AST *));
                    tmp->children[0] = vectmp[idx - 1];
                    vectmp[idx - 1] = tmp;
                } else {
                    /* we didn't see a `:` or a #\n, so that's
                     * an error.
                     */
                    return ASTLeft(0, 0, "malformed record definition.");
                }

            }
            /* so, we've constructed a list of record members.
             * now, we just need to actually _make_ the record
             * structure.
             */
            head->lenchildren = idx;
            head->children = (AST **)hmalloc(sizeof(AST *) * idx);
            for(int i = 0; i < idx; i++){
                head->children[i] = vectmp[i];
            }
            return ASTRight(head);
            break;
        case TFALSE:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TFALSE;
            return ASTRight(head);
        case TTRUE:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TTRUE;
            return ASTRight(head);
        case THEX:
        case TOCT:
        case TBIN:
        case TINT:
        case TFLOAT:
        case TSTRING:
        case TCHAR:
        case TBOOL:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = ltype;
            head->value = hstrdup(buffer);
            return ASTRight(head);
        case TEQ:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TEQ;
            return ASTRight(head);
        case TNEWL:
            if(!nltreatment) {
                return llreadexpression(fdin, nltreatment);
            } else {
                head = (AST *)hmalloc(sizeof(AST));
                head->tag = TNEWL;
                return ASTRight(head);
            }
        case TCHART:
        case TSTRT:
        case TINTT:
        case TBOOLT:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = ltype;
            return ASTRight(head);
        case TCOLON:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TCOLON;
            return ASTRight(head);
        case TSEMI:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TSEMI;
            return ASTRight(head);
    }
    return ASTLeft(0, 0, "unable to parse statement"); 
}

/* almost should be renamed, but I _think_ I want to try and
 * rip out all the custom code used in `let`, `letrec`, and `val`
 * in favor of this, which is why I included a type... For example,
 * declare forms could break on TNEWL, whereas val could break on
 * TEQ.
 */
AST *
mung_declare(const char **pdecls, const int **plexemes, int len, int haltstate) {
    int idx = 0, substate = 0;
    for(; idx < len ; idx++) {
        switch(substate) {
            
        }
    }
    return nil;
}

/* read in a single type, and return it as an AST node
 */
ASTOffset *
mung_single_type(const char **pdecls, const int **plexemes, int len, int haltstate, int offset) {
    int substate = 0, flag = -1, stackptr = 0, idx = 0;
    const int *lexemes = *plexemes;
    AST *tmp = nil, *stack[128] = {nil};

    if(issimpletypeast(lexemes[idx])) {
        tmp = (AST *)hmalloc(sizeof(AST));
        tmp->tag = lexemes[idx];
        return ASTOffsetRight(tmp, idx);
    } else if(lexemes[idx] == TIDENT) {
        /* here, we need to check if the
         * next element is a TOF, because
         * we don't _really_ know if we
         * have a complex type or a simple
         * type based on the fact that we
         * have an identifier here...
         */
        if((idx + 1) < len && lexemes[idx + 1] != TOF) {
            tmp = (AST *)hmalloc(sizeof(AST));
            tmp->tag = TUSERT;
            tmp->value = hstrdup(pdecls[idx]);
            return ASTOffsetRight(tmp, idx);
        }
    }

    /* ok, now we're in a complex type here... */

    for(flag = offset, idx = offset; idx < len; idx ++){
        switch(lexemes[idx]) {
            case TCHART:
            case TSTRT:
            case TINTT:
            case TFLOATT:
                tmp = (AST *)hmalloc(sizeof(AST));
                tmp->tag = lexemes[idx];
                if(flag == -1) {
                    return ASTOffsetRight(tmp, idx);
                } else {
                    if(substate == 1) {
                    } else if(substate == 2) {
                        break;
                    } else {
                        substate = 99;
                    } 
                    stack[stackptr] = tmp;
                    stackptr += 1;
                }
                break;
            case TOPAREN:
                /* Ok, here, we need to know
                 * what state we're in. It's
                 * partially context dependent
                 * (yuck), but it means that we
                 * can easily parse procedures
                 * versus tuples (for sum types)
                 */
                if(substate == 0) {
                    /* procedure:
                     * read single types and
                     * a fat arrow (=>) until
                     * we hit a TCPAREN
                     */
                } else if(substate == 2) {
                    /* tuple for sum types:
                     * read a type, then a TCOMMA,
                     * then a type, until we hit 
                     * TCPAREN
                     */
                } else {
                    substate = 99;
                }
                break;
            case TIDENT:
                /* ... */
                break;
            case TOF:
                /* ... */
                if(substate == 1) {
                    substate = 2;
                } else {
                    substate = 99; /* parse error */
                }
                break;
            case TARRAY:
            case TDEQUET:
                /* there's a few cases to consider here:
                 * - that the next token is TOF
                 * - that the next token is *not* TOF
                 * - that the next token is not TOF *and* that this is the terminal type.
                 * for example:
                 * array of int
                 * array
                 * array of array
                 */
                tmp = (AST *)hmalloc(sizeof(AST));
                tmp->tag = lexemes[idx];

                /* should I check for TOF here, or in another state?
                 * _almost_ seems like another state is "cleaner"...
                 * oooo, and that way too we can see if the () is correct
                 */
                if(flag == -1) {
                    flag = idx + 1;
                    substate = 1;
                } 
                stack[stackptr] = tmp;
                stackptr += 1;
                break;
        }
    }
    return ASTOffsetRight(tmp, idx);
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
        case TFN:
        case TDEF:
            if(head->tag == TFN) {
                printf("(fn ");
            } else {
                printf("(define %s ", head->value);
            }
            if(head->children[0] != nil) {
                walk(head->children[0], level);
            }
            printf("\n");
            walk(head->children[1], level + 1);
            printf(")");
            break;
        case TVAL:
        case TVAR:
            if(head->tag == TVAL) {
                printf("(define-value %s ", head->value);
            } else {
                printf("(define-mutable-value %s ", head->value);
            }
            walk(head->children[0], 0);
            if(head->lenchildren == 2) {
                printf(" ");
                walk(head->children[1], 0);
            }
            printf(")");
            break;
        case TLET:
        case TLETREC:
            if(head->tag == TLET) {
                printf("(let ");
            } else {
                printf("(letrec ");
            }

            printf("%s ", head->value);

            walk(head->children[0], 0);
            
            if(head->lenchildren == 3) {
                /* if we have a type,
                 * go ahead and print it.
                 */
                printf(" ");
                walk(head->children[2], 0);
            }

            printf("\n");
            walk(head->children[1], level + 1);

            printf(")");
            break;
        case TWHEN:
            printf("(when ");
            walk(head->children[0], 0);
            printf("\n");
            walk(head->children[1], level + 1);
            printf(")");
            break;
        case TPARAMLIST:
            printf("(parameter-list ");
            for(;idx < head->lenchildren; idx++) {
                walk(head->children[idx], 0); 
                if(idx < (head->lenchildren - 1)){
                    printf(" ");
                }
            }
            printf(") ");
            break;
        case TARRAY:
            printf("(type array)");
            break;
        case TCOMPLEXTYPE:
            printf("(complex-type ");
            for(;idx < head->lenchildren; idx++) {
                walk(head->children[idx], 0); 
                if(idx < (head->lenchildren - 1)){
                    printf(" ");
                }
            }
            printf(") ");
            break;
        case TARRAYLITERAL:
            printf("(array-literal ");
            for(;idx < head->lenchildren; idx++) {
                walk(head->children[idx], 0); 
                if(idx < (head->lenchildren - 1)){
                    printf(" ");
                }
            }
            printf(") ");
            break;
        case TCALL:
            printf("(call ");
            for(int i = 0; i < head->lenchildren; i++) {
                walk(head->children[i], 0);
                if(i < (head->lenchildren - 1)) {
                    printf(" ");
                }
            }
            printf(")");
            break;
        case TIF:
            printf("(if ");
            walk(head->children[0], 0);
            printf("\n");
            walk(head->children[1], level + 1);
            printf("\n");
            walk(head->children[2], level + 1);
            printf(")");
            break;
        case TIDENT:
            printf("(identifier %s)", head->value);
            break;
        case TTAG:
            printf("(tag %s)", head->value);
            break;
        case TBOOL:
            printf("(bool ");
            if(head->value[0] == 0) {
                printf("true)"); 
            } else {
                printf("false)");
            }
            break;
        case TCHAR:
            printf("(character #\\");
            switch(head->value[0]) {
                case '\b':
                    printf("backspace");
                    break;
                case '\n':
                    printf("newline");
                    break;
                case '\r':
                    printf("carriage");
                    break;
                case '\v':
                    printf("vtab");
                    break;
                case '\t':
                    printf("tab");
                    break;
                case '\0':
                    printf("nul");
                    break;
                default:
                    printf("%s", head->value);
                    break;
            }
            printf(")");
            break;
        case TFLOAT:
            printf("(float %s)", head->value);
            break;
        case TFALSE:
        case TTRUE:
            printf("(boolean ");
            if(head->tag == TFALSE) {
                printf("false)");
            } else {
                printf("true)");
            }
            break;
        case THEX:
            printf("(hex-integer %s)", head->value);
            break;
        case TOCT:
            printf("(octal-integer %s)", head->value);
            break;
        case TBIN:
            printf("(binary-integer %s)", head->value);
            break;
        case TINT:
            printf("(integer %s)", head->value);
            break;
        case TINTT:
            printf("(type integer)");
            break;
        case TFLOATT:
            printf("(type float)");
            break;
        case TCHART:
            printf("(type char)");
            break;
        case TSTRT:
            printf("(type string)");
            break;
        case TSTRING:
            printf("(string \"%s\")", head->value);
            break;
        case TRECORD:
            printf("(define-record %s\n", head->value);
            for(int i = 0; i < head->lenchildren; i++) {
                walk(head->children[i], level + 1);
                if(i < (head->lenchildren - 1)) {
                    printf("\n");
                }
            }
            printf(")");
            break;
        case TRECDEF:
            printf("(record-member ");
            walk(head->children[0], 0);
            if(head->lenchildren == 2) {
                printf(" ");
                walk(head->children[1], 0);
            }
            printf(")");
            break;
        case TBEGIN:
            printf("(begin\n");
            for(idx = 0; idx < head->lenchildren; idx++){
                walk(head->children[idx], level + 1);
                if(idx < (head->lenchildren - 1)) {
                    printf("\n");
                }
            }
            printf(")");
            break;
        case TEND:
            break;
        case TUNIT:
            printf("()");
            break;
        default:
            printf("(tag %d)", head->tag);
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
