#include <stdio.h>
#include <stdint.h>
#include <strings.h>
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
    LSTRT4, LSTRT5, LSTRT6, LINT0, LINT1, LINT2, LFLOAT0,
    LFLOAT1, LFLOAT2, LFLOAT3, LFLOAT4, LFLOAT5, LINT3,
    LARRAY0, LARRAY1, LARRAY2, LARRAY3, LARRAY4, LARRAY5,
    LB0, LI0, LUSE0, LF0, LC0, LR0, LW0, LOF0, LOF1,
    LDEQ0, LDEQ1, LDEQ2, LDEQ3, LDEC0, LDEC1, LDEC2, LDEC3,
    LDE0, LBOOL0, LBOOL1, LBOOL2

} LexStates;

/* AST tag enum.
 * basically, this is all the AST types, and is
 * used both for determining the type of AST
 * that is represented within the tree, as well as
 * returned from the tokenizer to say what type
 * of object it thinks is in buffer
 */
typedef enum {
    TDEF, TBEGIN, TEND, TEQUAL, TCOREFORM,
    TIDENT, TCALL, TOPAREN, TCPAREN, TMATCH,
    TIF, TELSE, TTHEN, TTYPE, TPOLY, TVAL,
    TARRAY, TRECORD, TINT, TFLOAT, TSTRING,
    TCHAR, TBOOL, TEQ, TSEMI, TEOF, TPARAMLIST,
    TTDECL, TWHEN, TNEWL, TDO, TUNIT, TERROR, 
    TLETREC, TLET, TFN, TCASE, TSTRT, TCHART,
    TINTT, TFLOATT, TCOMMENT
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
    FILE *fdin = nil;
    GC_INIT();

    if(ac > 1) {
        if((fdin = fopen(al[1], "r")) == nil) {
            printf("cannot open file \"%s\"\n", al[1]);
            return 1;
        }
        do {
            ret = read(fdin);
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
        do {
            printf(">>> ");
            ret = read(stdin);
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
    return (c == '{' || c == '}' || c == '(' || c == ')' || c == '[' || c == ']');
}

int
isident(int c){
    return (!iswhite(c) && c != '\n' &&  c != ';' && c != '"' && c != '\'' && !isbrace(c));
}

int
next(FILE *fdin, char *buf, int buflen) {
    /* fetch the _next_ token from input, and tie it
     * to a token type. fill buffer with the current
     * item, so that we can return identifiers or
     * numbers.
     */
    int state = 0, rc = 0, cur = 0, idx = 0, substate = 0;
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
            case '=':
                return TEQ;
            case ';':
                return TSEMI;
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
                                substate = TFLOAT;
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
                            if((cur >= '0' && cur <= '9')) {
                                buf[idx++] = cur;
                            } else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                buf[idx] = '\0';
                                return TINT;
                            } else { 
                                strncpy(buf, "incorrectly formatted floating point numeral", 512);
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
                    buf[idx++] = cur;
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
                                    substate = LIDENT0;
                                    break;
                            }
                            break;
                        case LB0:
                            if(cur == 'e') {
                                substate = LBEGIN0;
                            } else if(cur == 'o') {
                                substate = LBOOL0;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LC0:
                            if(cur == 'h') {
                                substate = LCHAR0;
                            } else if(cur == 'a') {
                                substate = LCASE0;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LD0:
                            if(cur == 'o') {
                                substate = LDO0;
                            } else if(cur == 'e') {
                                substate = LDE0;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LDE0:
                            if(cur == 'f') {
                                substate = LDEF0;
                            } else if(cur == 'q') {
                                substate = LDEQ0;
                            } else if(cur == 'c') {
                                substate = LDEC0;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LDEF0:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            } else if(iswhite(cur) || cur == 'n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                buf[idx] = '\0';
                                return TDEF;
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
                        case LARRAY1:
                            if(cur == 'r') {
                                substate = LARRAY2;
                            } else {
                                substate= LIDENT0;
                            }
                            break;
                        case LARRAY2:
                            if(cur == 'a') {
                                substate = LARRAY3;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LARRAY3:
                            if(cur == 'y') {
                                substate = LARRAY4;
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
                        case LIDENT0:
                            if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
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

ASTEither *
read(FILE *fdin) {
    /* _read_ from `fdin` until a single AST is constructed, or EOF
     * is reached.
     */
    AST *head = nil, *tmp = nil, *vectmp[128];
    ASTEither *sometmp = nil;
    int ltype = 0, ltmp = 0, idx = 0, flag = -1;
    char buffer[512] = {0};

    ltype = next(fdin, &buffer[0], 512);
    switch(ltype) {
        case TERROR:
            return ASTLeft(0, 0, hstrdup(&buffer[0]));
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

            sometmp = read(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            }

            head->children[0] = sometmp->right;

            sometmp = read(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            }

            tmp = sometmp->right;

            if(tmp->tag != TTHEN) {
                return ASTLeft(0, 0, "missing THEN keyword after IF conditional: if conditional then expression else expression");
            }

            sometmp = read(fdin);
            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            }

            head->children[1] = sometmp->right;

            sometmp = read(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            }

            tmp = sometmp->right;

            if(tmp->tag != TELSE) {
                return ASTLeft(0, 0, "missing ELSE keyword after THEN value: if conditional then expression else expression");
            }

            sometmp = read(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            }

            head->children[2] = sometmp->right;

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
            sometmp = read(fdin);
            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            }

            tmp = sometmp->right;

            if(tmp->tag == TCPAREN) {
                tmp = (AST *)hmalloc(sizeof(AST));
                tmp->tag = TUNIT;
                return ASTRight(tmp);
            } else if(tmp->tag != TIDENT) {
                return ASTLeft(0, 0, "cannot call non-identifier object");
            }

            vectmp[idx++] = tmp;

            while(1) {
                sometmp = read(fdin);
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
            head->tag = TCALL;
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

            sometmp = read(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else {
                head->children[0] = sometmp->right;
            }

            sometmp = read(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else {
                tmp = sometmp->right;
            }

            if(tmp->tag != TDO) {
                return ASTLeft(0, 0, "missing `do` statement from `when`: when CONDITION do EXPRESSION");
            }

            sometmp = read(fdin);

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
        case TTYPE:
            break;
        case TPOLY:
            break;
        case TVAL:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TVAL;
            head->children = (AST **)hmalloc(sizeof(AST *) * 2);
            head->lenchildren = 2;
            
            sometmp = read(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else {
                tmp = sometmp->right;
            }

            if(tmp->tag != TIDENT) {
                return ASTLeft(0, 0, "val's name *must* be an identifier: `val IDENTIFIER = EXPRESSION`");
            } else {
                head->children[0] = tmp;
            }

            sometmp = read(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else {
                tmp = sometmp->right;
            }

            if(tmp->tag != TEQ) {
                return ASTLeft(0, 0, "val's identifiers *must* be followed by an `=`: `val IDENTIFIER = EXPRESSION`");
            }

            sometmp = read(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else {
                head->children[1] = sometmp->right;
            }

            return ASTRight(head);
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
    return ASTLeft(0, 0, "unable to parse statement"); 
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
            printf(")");
            break;
        case TVAL:
            printf("(define-value ");
            walk(head->children[0], 0);
            printf(" ");
            walk(head->children[1], 0);
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
        case TINT:
            printf("(integer %s)", head->value);
            break;
        case TSTRING:
            printf("(string \"%s\")", head->value);
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
