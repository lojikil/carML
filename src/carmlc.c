#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <gc.h>

#ifdef DEBUG
#define debugln printf("dying here on line %d?\n", __LINE__);
#define dprintf(...) printf(__VA_ARGS__)
#define dwalk(x, y) walk(x, y)
#else
#define debugln
#define dprintf(...)
#define dwalk(x, y) 
#endif

#define nil NULL
#define nul '\0'
#define YES 1
#define NO  0
#define hmalloc GC_MALLOC
#define cwalk(head, level) llcwalk(head, level, NO)

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
    LFN0, LFN1, LLET0, LLET1, LLET2, LLETREC0, LLETREC1, LLETREC2,
    LLETREC3, LLETREC4, LLETREC5, LLETREC6, LL0, LCHAR0,
    LCHAR1, LCHAR2, LCHAR3, LSTRT0, LSTRT1, LSTRT2, LSTRT3,
    LSTRT4, LSTRT5, LSTRT6, LINTT0, LINTT1, LINTT2, LFLOATT0,
    LFLOATT1, LFLOATT2, LFLOATT3, LFLOATT4, LFLOATT5, LINTT3,
    LARRAY0, LARRAY1, LARRAY2, LARRAY3, LARRAY4, LARRAY5,
    LB0, LI0, LUSE0, LF0, LC0, LR0, LW0, LOF0, LOF1,
    LDEQ0, LDEQ1, LDEQ2, LDEQ3, LDEC0, LDEC1, LDEC2, LDEC3,
    LDE0, LBOOL0, LBOOL1, LBOOL2, LREF0, LELSE0, LRE0, LTRUE2,
    LWITH0, LWITH1, LWITH2, LDEC4, LUSE1, LUSE2, LVAR2, LTAG0,
    LTAGIDENT, LWHILE1, LWHILE2, LWHILE3, LWHILE4, LWHILE5,
    LFOR0, LFOR1, LFOR2, LWH0, LTUPLE0, LTUPLE1, LTUPLE2, LTUPLE3,
    LTUPLE4, LFUNCTIONT0, LFUNCTIONT1, LFUNCTIONT2, LFUNCTIONT3,
	LFUNCTIONT4, LFUNCTIONT5, LFUNCTIONT6, LP0, LPROCEDURET0,
    LPROCEDURET1, LPROCEDURET2, LPROCEDURET3, LPROCEDURET4,
    LPROCEDURET5, LPROCEDURET6, LPROCEDURET7, LANY0, LANY1,
    LAND0, LAND1, LAND2, LA0, LAN0,
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
    TPARAMDEF, TTYPEDEF, TWHILE, TFOR, // 72
    TTUPLET, TFUNCTIONT, TPROCEDURET, // 75
    TAND, TANY, TGUARD, // 78
} TypeTag;

struct _AST {
    TypeTag tag;
    /* there's going to have to be some
     * interpretation here, as we're not
     * breaking down the ASTs into discrete
     * objects, but rather just grouping them
     * into a generic a piece as possible.
     * so, for example, and IF block would
     * have a lenchildren == 3, no matter what
     * whereas a BEGIN block lenchildren == N,
     * where N >= 0. In the real thing, would
     * probably be best to make this a poly with
     * each member broken down, or an SRFI-57-style
     * struct.
     * in fact... if you have row-polymorphism, there's
     * no need for SRFI-57 inheritance, save for to
     * create convenience methods... hmm... :thinking_face:
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

/* probably should track unary vs binary
 * but for now I think this is enough...
 */
const char *coperators[] = {
    "sum", "+",
    "add", "+",
    "+", "+",
    "sub", "-",
    "-", "-",
    "div", "/",
    "/", "/",
    "mul", "*",
    "*", "*",
    "mod", "%",
    "%", "%",
    "<", "<",
    ">", ">",
    "<=", "<=",
    ">=", ">=",
    "eq?", "==",
    "!=", "!=",
    "/=", "!=",
    "<>", "!=",
    "lshift", "<<",
    "<<", "<<",
    "rshift", ">>",
    ">>", ">>",
    "xor", "^",
    "^", "^",
    "not", "!",
    "!", "!",
    "negate", "~",
    "~", "~",
    "land", "&&",
    "logical-and", "&&",
    "lor", "||",
    "logical-or", "||",
    "&&", "&&",
    "||", "||",
    "band", "&",
    "bitwise-and", "&",
    "&", "&",
    "bor", "|",
    "bitwise-or", "|",
    "|", "|",
    "set!", "=",
    ".", ".",
    "->", "->",
    "get", "get",
    "make-struct", "make-struct",
    "make-string", "make-string",
    "make-array", "make-array",
    "return", "return",
    0
};


char *upcase(const char *, char *, int);
char *downcase(const char *, char *, int);
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
AST *linearize_complex_type(AST *);
void indent(int);
void walk(AST *, int);
void llcwalk(AST *, int, int);
void generate_type_value(AST *, const char *); // generate a type/poly constructor
void generate_type_ref(AST *, const char *); // generate a type/poly reference constructor
int compile(FILE *, FILE *);
int iswhite(int);
int isident(int);
int isbrace(int);
int istypeast(int);
int issimpletypeast(int);
int iscomplextypeast(int);
int islambdatypeast(int);
int issyntacticform(int);
int isprimitivevalue(int);
int isvalueform(int);
int iscoperator(const char *);
char *typespec2c(AST *, char *, char*, int);
char *findtype(AST *);

int
main(int ac, char **al) {
    ASTEither *ret = nil;
    AST *tmp = nil;
    FILE *fdin = nil;
    int walkflag = 0;
    GC_INIT();

    if(ac > 1) {
        if((fdin = fopen(al[1], "r")) == nil) {
            printf("cannot open file \"%s\"\n", al[1]);
            return 1;
        }
        if(ac > 2 && !strncmp(al[2], "+c", 2)) {
            walkflag = 1;
        }
        do {
            ret = readexpression(fdin);
            if(ret->tag == ASTLEFT) {
                printf("parse error: %s\n", ret->left.message);
                break;
            }

            tmp = ret->right;
            if(tmp->tag != TNEWL && tmp->tag != TEOF) {
                if(walkflag) {
                    cwalk(tmp, 0);
                } else {
                    walk(tmp, 0);
                }
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
        printf("\t\tcarML/C 2018.0\n");
        printf("(c) 2016-2018 lojikil, released under the ISC License.\n\n");
        printf("%%c - turns on C code generation\n%%quit/%%q - quits\n\n");
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
                } else if(tmp->tag == TIDENT && !strncmp(tmp->value, "%quit", 4)) {
                    break;
                } else if(tmp->tag == TIDENT && !strncmp(tmp->value, "%q", 2)) {
                    break;
                } else if(tmp->tag == TIDENT && !strncmp(tmp->value, "%c", 2)) {
                    walkflag = !walkflag;
                    printf("[!] C generation is: %s", (walkflag ? "on" : "off"));
                } else if(tmp->tag != TNEWL) {
                    if(walkflag) {
                        cwalk(tmp, 0);
                    } else {
                        walk(tmp, 0);
                    }
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
    return (c == '{' || c == '}' || c == '(' || c == ')' || c == '[' || c == ']' || c == ';' || c == ',' || c == ':');
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
    //debugln;
    switch(tag) {
        case TARRAY:
        case TINTT:
        case TCHART:
        case TDEQUET:
        case TFLOATT:
        case TTUPLET:
        case TFUNCTIONT:
        case TPROCEDURET:
        case TSTRT:
        case TTAG: // user types 
        case TBOOLT:
        case TREF:
        case TANY:
            return 1;
        default:
            return 0;
    }
}

int
issimpletypeast(int tag) {
    //debugln;
    switch(tag) {
        case TINTT:
        case TCHART:
        case TFLOATT:
        case TSTRT:
        case TBOOLT:
        case TANY:
            return 1;
        default:
            return 0;
    }
}

int
iscomplextypeast(int tag) {
    //debugln;
    switch(tag) {
        case TARRAY:
        case TDEQUET:
		case TTUPLET:
        case TFUNCTIONT:
        case TPROCEDURET:
        case TCOMPLEXTYPE:
        case TREF:
        case TTAG: // user types 
            return 1;
        //case TARRAYLITERAL:
            // really... need to iterate through
            // the entire array to be sure here...
            //return 1;
        default:
            return 0;
    }
}

int
islambdatypeast(int tag) {
    switch(tag) {
        case TFUNCTIONT:
        case TPROCEDURET:
            return 1;
        default:
            return 0;
    }
}

int
isprimitivevalue(int tag) {
    switch(tag) {
        case TINT:
        case TFLOAT:
        case TARRAYLITERAL:
        case TSTRING:
        case TCHAR:
            return 1;

        default:
            return 0;
    }
}

int
isvalueform(int tag) {
    if(isprimitivevalue(tag)) {
        return 1;
    } else if(tag == TCALL) {
        return 1;
    } else if(tag == TIDENT) {
        return 1;
    } else {
        return 0;
    }
}

int
iscoperator(const char *potential) {
    int idx = 0;

    while(coperators[idx] != nil) {
        if(!strcmp(potential, coperators[idx])) {
            return idx + 1;
        }
        idx += 2;
    }

    return -1;
}

int
issyntacticform(int tag) {
    switch(tag) {
        case TVAL:
        case TVAR:
        case TLET:
        case TLETREC:
        case TWHEN:
        case TDO:
        case TMATCH:
        case TWHILE:
        case TFOR:
        case TIF:
            return 1;
        default:
            return 0;
    }
}

// So currently the type parser will give us:
// (complex-type (type array)
//      (tag Either)
//          (array-literal (type ref) (array-literal (type integer))
//          (type string)))
// for array[Either[ref[int] string]]
// which is wrong. We need to linearize those complex types better,
// and this function is meant to do that.
//
// this is a *really* terrible way of doing this; the better way
// would be to correctly parse types in the first place. However,
// I'm still thinking about how I want to handle some things within
// the type syntax, so until that is done, I think we can just leave
// this as is. The code is technically O(n^m): it iterates over each
// member of the array and then recurses the depths there of. The
// sole saving grace of this approach is that our types are small
// enough that this shouldn't matter: N should be < 10 worse case,
// and M wouldn't be much larger. That's the *only* reason why I'm
// not just diving into the types now. Fixing this, and fixing
// type parsing in general, won't be major undertakings once I am
// good with the type syntax & the associated calculus.
AST *
linearize_complex_type(AST *head) {
    AST *stack[512] = {nil}, *tmp = nil, *flatten_target = nil;
    int sp = 0;

    dprintf("here? %d\n", __LINE__);
    if(!iscomplextypeast(head->tag)){
        printf("here? %d\n", __LINE__);
        walk(head, 0);
        return head;
    }
    dprintf("here? %d: ", __LINE__);
    dwalk(head, 0);
    dprintf("\n");

    stack[sp] = head->children[0];
    sp++;

    for(int cidx = 1; cidx < head->lenchildren; cidx++) {
        if(iscomplextypeast(head->children[cidx]->tag) && cidx < (head->lenchildren - 1) && head->children[cidx + 1]->tag == TARRAYLITERAL) {
            flatten_target = head->children[cidx + 1];

            dprintf("here? %d: ", __LINE__);
            dwalk(flatten_target, 0);
            dprintf("\n");

            tmp = (AST *)hmalloc(sizeof(AST));
            tmp->tag = TCOMPLEXTYPE;
            tmp->lenchildren = 1 + flatten_target->lenchildren;
            tmp->children = (AST **)hmalloc(sizeof(AST *) * flatten_target->lenchildren);
            tmp->children[0] = head->children[cidx];
            for(int midx = 1; midx <= flatten_target->lenchildren; midx++) {
                dprintf("here? %d: ", __LINE__);
                dwalk(flatten_target->children[midx - 1], 0);
                dprintf("\n");
                tmp->children[midx] = flatten_target->children[midx - 1];
            }
            dprintf("here? %d: ", __LINE__);
            dwalk(tmp, 0);
            dprintf("here? %d\n", __LINE__);

            tmp = linearize_complex_type(tmp);
            stack[sp] = tmp;
            sp++;
            cidx++; // skip the next item, we have already consumed it
        } else {
            dprintf("here? %d\n", __LINE__);
            stack[sp] = head->children[cidx];
            sp++;
        }
    }

    dprintf("here? %d\n", __LINE__);

    // ok, we have collapsed types, now linearize them into
    // one TCOMPLEX TYPE
    tmp = (AST *)hmalloc(sizeof(AST));
    tmp->tag = TCOMPLEXTYPE;
    tmp->lenchildren = sp;
    tmp->children = (AST **)hmalloc(sizeof(AST *) * sp);
    for(int cidx = 0; cidx <= sp; cidx++) {
        tmp->children[cidx] = stack[cidx];
    }

    return tmp;
}

// find a type in a tree of complex types
// could be interesting to walk down...
char *
findtype(AST *head) {
    AST *hare = head, *turtle = head;
    char *stack[16] = {nil}, *typeval = nil;
    int sp = 0, speclen = head->lenchildren, typeidx = 0;
    int breakflag = 0, reslen = 0, pushf = 0;

    if(turtle->tag == TCOMPLEXTYPE) {
        hare = turtle->children[0];
    }

    dprintf("speclen: %d\n", speclen);
    dwalk(head, 0);
    dprintf("\n");
    debugln;
    while(!breakflag) {
        debugln;
        dprintf("current hare: ");
        dwalk(hare, 0);
        dprintf("\n");
        switch(hare->tag) {
            case TTAG:
                // really should be Tag *...
                typeval = hare->value;
                breakflag = 1;
                pushf = 1;
                break;
            case TINTT:
                typeval = "int";
                breakflag = 1;
                pushf = 1;
                break;
            case TFLOATT:
                typeval = "double";
                breakflag = 1;
                pushf = 1;
                break;
            case TSTRT:
                typeval = "char *";
                breakflag = 1;
                pushf = 1;
                break;
            case TCHART:
                typeval = "char";
                breakflag = 1;
                pushf = 1;
                break;
            case TBOOLT:
                typeval = "uint8_t";
                breakflag = 1;
                pushf = 1;
                break;
            case TREF:
            case TARRAY:
            case TDEQUET:
                typeval = "*";
                pushf = 1;
                break;
            case TCOMPLEXTYPE:
                turtle = turtle->children[typeidx];
                hare = turtle->children[0];
                typeidx = -1;
                speclen = turtle->lenchildren;
                pushf = 0;
                break;
            case TFUNCTIONT:
            case TPROCEDURET:
            default:
                typeval = "void *";
                breakflag = 1;
                break;
        }

        
        // another great place to have Option types
        // would be nice to be able to have the above
        // set an Option[string] and have that matched
        // here... 
        if(pushf) {
            dprintf("typeval: %s\n", typeval);
            stack[sp] = hstrdup(typeval);
            sp += 1;
        }
        debugln;
        // keep the length of the resulting
        // string updated each time, and add
        // one to the length for either a space
        // in between members or a NUL at the
        // end
        debugln;
        reslen += strlen(typeval) + 1;
        debugln;

        if(typeidx >= speclen) {
            dprintf("here on 716? typeidx: %d, speclen: %d\n", typeidx, speclen);
            break;
        } else if(breakflag) {
            debugln;
            break;
        } else {
            typeidx++;
            hare = turtle->children[typeidx];
        }
    }

    debugln;

    typeval = (char *)hmalloc(sizeof(char) * reslen);

    debugln;

    for(sp--; sp >= 0; sp--) {
        debugln;
        strncat(typeval, stack[sp], reslen);
        debugln;
        strncat(typeval, " ", reslen);
        debugln;
    }

    debugln;

    return typeval;
}

// yet another location where I'd rather
// return Option[String], sigh
// this works fine for function declarations, but 
// not really for variable decs... need to work
// out what *type* of signature we're generating...
char *
typespec2c(AST *typespec, char *dst, char *name, int len) {
    int strstart = 0, typeidx = 0, rewrite = 0, speclen = 0, sp = 0, breakflag = 0;
    char *typeval = nil, *typestack[16] = {nil};
    AST *tmp = nil;

    // we use the type stack to capture each level of a type...
    // probably should use a growable, but 16 levels deep of 
    // arrays/refs should be good for now...
    // (famous last words)

    if(typespec->lenchildren == 0 && istypeast(typespec->tag)) {
        switch(typespec->tag) {
            case TTAG:
                typeval = typespec->value;
                break;
            case TINTT:
                typeval = "int";
                break;
            case TFLOATT:
                typeval = "double";
                break;
            case TARRAY:
                typeval = "void *";
                break;
            case TREF:
                typeval = "void *";
                break;
            case TSTRT:
                typeval = "char *";
                break;
            case TCHART:
                typeval = "char";
                break;
            case TBOOLT:
                typeval = "uint8_t";
                break;
            case TFUNCTIONT:
            case TPROCEDURET:
                typeval = "void (*)(void)";
            default:
                typeval = "void *";
        }
        if(name != nil) {
            snprintf(dst, len, "%s %s", typeval, name);
        } else {
            snprintf(dst, len, "%s ", typeval);
        }
        return dst;
    } else if(typespec->lenchildren == 1) {
        if(typespec->children[0]->tag == TTAG) {
            snprintf(dst, len, "%s ", typespec->children[0]->value);
        } else {
            switch(typespec->children[0]->tag) {
                case TSTRT:
                    snprintf(dst, 10, "char * ");
                    break;
                case TDEQUET:
                    snprintf(dst, 10, "deque *");
                    break;
                case TFUNCTIONT:
                case TPROCEDURET:
                    break;
                case TARRAY:
                case TREF:
                default:
                    snprintf(dst, 10, "void * ");
                    break;
            }
        }

        if(name != nil) {
            strstart = strnlen(dst, 512);
            if(islambdatypeast(typespec->children[0]->tag)) {
                snprintf(&dst[strstart], 512 - strstart, "void (*%s)(void)", name); 
            } else {
                snprintf(&dst[strstart], 512 - strstart, "%s ", name);
            }
        }
    } else if(typespec->lenchildren > 1 && islambdatypeast(typespec->children[0]->tag)) {
        // handle functions & procedures here.
        char *frettype = nil, fnbuf[256] = {0};
        int tlen = 0;

        if(typespec->children[0]->tag == TFUNCTIONT) {
            tlen = typespec->lenchildren - 1;
            if(typespec->children[tlen]->tag == TUNIT) {
                frettype = "void";
            } else {
                frettype = typespec2c(typespec->children[tlen], fnbuf, nil, 256);
                frettype = hstrdup(fnbuf);
            }
            fnbuf[0] = 0;
        } else if(typespec->children[0]->tag == TPROCEDURET) {
            frettype = "void";
            tlen = typespec->lenchildren;
        }

        // ok, get the return type, then iterate over the
        // remaining list items and run typespec2c on each

        if(name == nil) {
            strstart = snprintf(dst, 512, "%s(*)(", frettype);
        } else {
            strstart = snprintf(dst, 512, "%s(*%s)(", frettype, name);
        }

        // we iterate from 1 instead of 0 because
        // the first member of the type spec is the
        // function/procedure type.
        for(int cidx = 1; cidx < tlen; cidx++) {
            frettype = typespec2c(typespec->children[cidx], fnbuf, nil, 256);
            strncat(dst, frettype, 512);

            if(cidx < (tlen - 1)) {
                strncat(dst, ", ", 512);
            }
        }

        strncat(dst, ")", 512);
    } else {
        speclen = typespec->lenchildren;

        /* the type domination algorithm is as follows:
         * 1. iterate through the type list
         * 1. if we hit a tag, that's the stop item
         * 1. if we hit a cardinal type, that's the stop
         * 1. invert the list from start to stop
         * 1. snprintf to C.
         * so, for example:
         * `array of array of Either of int` would become
         * Either **; I'd like to make something fat to hold
         * arrays, but for now we can just do it this way.
         * Honestly, I'd love to Specialize types, at least
         * to some degree, but for now...
         */
        tmp = typespec;
        dprintf("what does typespec look like here: ");
        dwalk(tmp, 0);
        dprintf("\n");

        dprintf("result? %s\n", findtype(typespec));

        dprintf("typespec[%d] == null? %s\n", typeidx, typespec->children[typeidx] == nil ? "yes" : "no");
        dprintf("here on %d, typeidx: %d, len: %d\n", __LINE__, typeidx, typespec->lenchildren);

        typeval = findtype(typespec);

        snprintf(&dst[strstart], (len - strstart), "%s ", typeval);
        strstart = strnlen(dst, len);
        if(name != nil) {
            snprintf(&dst[strstart], (len - strstart), "%s", name);
            strstart = strnlen(dst, len);
        }
    }
    return dst;
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
                if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                    buf[idx++] = '|';
                    buf[idx] = nul;
                    ungetc(cur, fdin);
                    return TIDENT;
                } else if(cur == '>') {
                    return TPIPEARROW;
                } else {
                    buf[idx++] = '|';
                }
                break;
            case '=':
                cur = fgetc(fdin);
                if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                    return TEQ;
                } else if(cur == '>') {
                    return TFATARROW;
                } else {
                    buf[idx++] = '=';
                }
                break;
            case '$':
                cur = fgetc(fdin);

                if(cur == '(') {
                    return TCUT;
                } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                    ungetc(cur, fdin);
                    return TDOLLAR;
                } else {
                    /* same jazz here as above with '='. */
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
                                buf[idx++] = '\\';
                                buf[idx++] = 'n';
                                break;
                            case 'r':
                                buf[idx++] = '\\';
                                buf[idx++] = 'r';
                                break;
                            case 't':
                                buf[idx++] = '\\';
                                buf[idx++] = 't';
                                break;
                            case 'v':
                                buf[idx++] = '\\';
                                buf[idx++] = 'v';
                                break;
                            case '0':
                                buf[idx++] = '\\';
                                buf[idx++] = '0';
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
                        case '\'':
                            buf[idx++] = '\'';
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

                    if(substate == LIDENT0 && (iswhite(cur) || cur == '\n' || isbrace(cur))) {
                        //debugln;
                        ungetc(cur, fdin);
                        buf[idx - 1] = '\0';
                        return TIDENT;
                    }

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
                                    substate = LA0;
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
                                    break;
                                case 'o':
                                    substate = LOF0;
                                    break;
                                case 'p':
                                    substate = LP0;
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
                                //debugln;
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
                            //debugln;
                            if(cur == 'o') {
                                //debugln;
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
                                //debugln;
                                substate = LBOOL2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            //debugln;
                            break;
                        case LBOOL2:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            /*} else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;*/
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
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LCHAR0:
                            if(cur == 'a') {
                                substate = LCHAR1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LCHAR1:
                            if(cur == 'r') {
                                substate = LCHAR2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LCHAR2:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            } else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                buf[idx] = '\0';
                                return TCHART;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
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
                            } else if(cur == 'l') {
                                substate = LFLOATT0;
                            } else if(cur == 'a') {
                                substate = LFALSE0;
                            } else if(cur == 'o') {
                                substate = LFOR0;
                            } else if(cur == 'u') {
                                substate = LFUNCTIONT0;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LFUNCTIONT0:
                            if(cur == 'n') {
                                substate = LFUNCTIONT1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LFUNCTIONT1:
                            if(cur == 'c') {
                                substate = LFUNCTIONT2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LFUNCTIONT2:
                            if(cur == 't') {
                                substate = LFUNCTIONT3;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LFUNCTIONT3:
                            if(cur == 'i') {
                                substate = LFUNCTIONT4;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LFUNCTIONT4:
                            if(cur == 'o') {
                                substate = LFUNCTIONT5;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LFUNCTIONT5:
                            if(cur == 'n') {
                                substate = LFUNCTIONT6;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LFUNCTIONT6:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TFUNCTIONT;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LFOR0:
                            if(cur == 'r') {
                                substate = LFOR1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LFOR1:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TFOR;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
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
                        case LA0:
                            if(cur == 'r') {
                                substate = LARRAY1;
                            } else if(cur == 'n') {
                                substate = LAN0;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
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
                            } else if(cur == 'u') {
                                substate = LTUPLE0;
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
                        case LTUPLE0:
                            if(cur == 'p') {
                                substate = LTUPLE1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;

                        case LTUPLE1:
                            if(cur == 'l') {
                                substate = LTUPLE2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;

                        case LTUPLE2:
                            if(cur == 'e') {
                                substate = LTUPLE3;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;

                        case LTUPLE3:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TTUPLET;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LAN0:
                            if(cur == 'y') {
                                substate = LANY0;
                            } else if(cur == 'd') {
                                substate = LAND0;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate= LIDENT0;
                            }
                            break;
                        case LANY0:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            } else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                buf[idx] = '\0';
                                return TANY;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
                        case LAND0:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            } else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                buf[idx] = '\0';
                                return TAND;
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
                                substate = LWH0;
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
                        case LWH0:
                            if(cur == 'e') {
                                substate = LWHEN1;
                            } else if(cur == 'i') {
                                substate = LWHILE1;
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
                        case LWHILE1:
                            if(cur == 'l') {
                                substate = LWHILE2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LWHILE2:
                            if(cur == 'e') {
                                substate = LWHILE3;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
                        case LWHILE3:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TWHILE;
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
                        case LPROCEDURET0:
                            if(cur == 'o') {
                                substate = LPROCEDURET1;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
            			case LPROCEDURET1:
                            if(cur == 'c') {
                                substate = LPROCEDURET2;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
            			case LPROCEDURET2:
                            if(cur == 'e') {
                                substate = LPROCEDURET3;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
            			case LPROCEDURET3:
                            if(cur == 'd') {
                                substate = LPROCEDURET4;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
            			case LPROCEDURET4:
                            if(cur == 'u') {
                                substate = LPROCEDURET5;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
            			case LPROCEDURET5:
                            if(cur == 'r') {
                                substate = LPROCEDURET6;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
            			case LPROCEDURET6:
                            if(cur == 'e') {
                                substate = LPROCEDURET7;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
                            }
                            break;
            			case LPROCEDURET7:
                            if(isident(cur)) {
                                substate = LIDENT0;
                            }else if(iswhite(cur) || cur == '\n' || isbrace(cur)) {
                                ungetc(cur, fdin);
                                return TPROCEDURET;
                            } else {
                                strncpy(buf, "malformed identifier", 512);
                                return TERROR;
                            }
                            break;
						case LP0:
						    if(cur == 'o') {
                                substate = LPOLY1;
                            } else if(cur == 'r') {
                                substate = LPROCEDURET0;
                            } else if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TIDENT;
                            } else {
                                substate = LIDENT0;
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
                            if(iswhite(cur) || isbrace(cur) || cur == '\n') {
                                ungetc(cur, fdin);
                                buf[idx - 1] = '\0';
                                return TTAG;
                            }
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
    int ltype = 0, ltmp = 0, idx = 0, flag = -1, typestate = 0, fatflag = 0;
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
             * ran into an interesting bug here.
             * because `#` eats until the newline, it
             * doesn't *return* a new line, which breaks
             * llread for certain types of newline-sensitive
             * operations, such as record members. So, we
             * have to check what the nltreatment value is,
             * and either convert this into a TNEWL or call
             * read expression again.
             */

            if(nltreatment) {
                tmp = (AST *)hmalloc(sizeof(AST));
                tmp->tag = TNEWL;
                sometmp = ASTRight(tmp);
            } else {
                sometmp = readexpression(fdin);
            }

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
                                } else if(sometmp->right->tag == TARRAYLITERAL) {
                                    tmp = sometmp->right;
                                    for(int cidx = 0; cidx < tmp->lenchildren; cidx++, idx++) {
                                        vectmp[idx] = tmp->children[cidx];
                                    }
                                    typestate = 2;
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
                    tmp = linearize_complex_type(tmp);
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
        case TUSE:
            head = (AST *) hmalloc(sizeof(AST));
            head->tag = TUSE;
            head->lenchildren = 1;
            head->children = (AST **)hmalloc(sizeof(AST *));
            
            sometmp = readexpression(fdin);
            tmp = sometmp->right;

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else if(tmp->tag != TIDENT && tmp->tag != TTAG && tmp->tag != TSTRING) {
                return ASTLeft(0, 0, "`use` must be followed by an ident, a tag, or a string.");
            }

            head->children[0] = sometmp->right;

            return ASTRight(head);
        case TFN:
        case TDEF:
            head = (AST *) hmalloc(sizeof(AST));
            head->tag = ltype;
            int loopflag = 1;
            AST *params = nil, *returntype = nil;

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
                            fatflag = idx;
                            typestate = 2;
                        } else if(sometmp->right->tag == TEQ) {
                            // body
                            //debugln;
                            typestate = 3;
                        } else {
                            //debugln;
                            //dprintf("tag == %d\n", sometmp->right->tag);
                            return ASTLeft(0, 0, "`def` must have either a parameter list, a fat-arrow, or an equals sign.");
                        } 
                        debugln;
                        dprintf("typestate == %d\n", typestate);
                        break;
                    case 1: // TIDENT
                        debugln;
                        vectmp[idx] = sometmp->right;
                        flag = idx;
                        idx++;
                        sometmp = readexpression(fdin);
                        //debugln;
                        if(sometmp->tag == ASTLEFT) {
                            //debugln;
                            return sometmp;
                        } else if(sometmp->right->tag == TIDENT) {
                            //debugln;
                            typestate = 1;
                        } else if(sometmp->right->tag == TFATARROW) {
                            debugln;
                            fatflag = idx;
                            typestate = 2;
                        } else if(sometmp->right->tag == TEQ) {
                            //debugln;
                            typestate = 3;
                        } else if(sometmp->right->tag == TCOLON) {
                            //debugln;
                            //dprintf("sometmp type: %d\n", sometmp->right->tag);
                            typestate = 4;
                        } else {
                            //debugln;
                            //dprintf("tag == %d\n", sometmp->right->tag);
                            return ASTLeft(0, 0, "`def` identifiers *must* be followed by `:`, `=>`, or `=`");
                        }
                        debugln;
                        dprintf("tag == %d, typestate == %d\n", sometmp->right->tag, typestate);
                        break;
                    case 3: // TEQ, start function
                        /* mark the parameter list loop as closed,
                         * and begin to process the elements on the
                         * stack as a parameter list.
                         */
                        //debugln;
                        loopflag = 0;
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
                        break;
                    // part of the problem here is that
                    // I tried to simplify the states, but
                    // ended up with messier code and more
                    // stateful stuff. Undoing some of that
                    // now, by unwinding the fatflag code
                    case 2: // TFATARROW, return
                        debugln;
                        sometmp = readexpression(fdin);
                        if(sometmp->tag == ASTLEFT) {
                            return sometmp;
                        } else if(!istypeast(sometmp->right->tag)) {
                            dprintf("somehow here, but... %d\n", sometmp->right->tag);
                            return ASTLeft(0, 0, "a `=>` form *must* be followed by a type definition...");
                        } else if(issimpletypeast(sometmp->right->tag)) {
                            debugln;
                            returntype = sometmp->right;
                            typestate = 6;
                        } else { // complex type
                            debugln;
                            vectmp[idx] = sometmp->right;
                            flag = idx;
                            idx++;
                            typestate = 21;
                        }
                        break;
                    case 21: // OF but for return
                        sometmp = readexpression(fdin);

                        if(sometmp->tag == ASTLEFT) {
                            return sometmp;
                        } else if(sometmp->right->tag == TOF) {
                            typestate = 22;
                        } else if(sometmp->right->tag == TARRAYLITERAL) {
                            tmp = sometmp->right;
                            for(int tidx = 0; tidx < tmp->lenchildren; tidx++, idx++) {
                                vectmp[idx] = tmp->children[tidx];
                            }
                            returntype = (AST *)hmalloc(sizeof(AST));
                            returntype->tag = TCOMPLEXTYPE;
                            returntype->lenchildren = idx - flag;
                            returntype->children = (AST **)hmalloc(sizeof(AST *) * returntype->lenchildren);
                            for(int cidx = 0, tidx = flag, tlen = returntype->lenchildren; cidx < tlen; cidx++, tidx++) {
                                returntype->children[cidx] = vectmp[tidx];
                            }
                            // this is a hack; because of the current grammar construction,
                            // we end up with array-literals in positions that typespec2c
                            // cannot handle. This solves that, but terribly. This will
                            // be solved by changing the syntax such that there is no
                            // need for the `of` form when specifying complex types,
                            // and going full-Scala (get it???) on the type syntax.
                            returntype = linearize_complex_type(returntype);
                            idx = flag;
                            flag = 0;
                            typestate = 6;
                        } else if(sometmp->right->tag == TEQ) {
                            returntype = (AST *)hmalloc(sizeof(AST));
                            returntype->tag = TCOMPLEXTYPE;
                            returntype->lenchildren = idx - flag;
                            returntype->children = (AST **)hmalloc(sizeof(AST *) * returntype->lenchildren);
                            for(int cidx = 0, tidx = flag, tlen = returntype->lenchildren; cidx < tlen; cidx++, tidx++) {
                                returntype->children[cidx] = vectmp[tidx];
                            }
                            idx = flag;
                            flag = 0;
                            typestate = 3;
                        } else {
                            return ASTLeft(0, 0, "a complex type in `=>` must be followed by `of`, `=`, or an array of types.");
                        }
                        break;
                    case 22: // complex end result, but for return
                        sometmp = readexpression(fdin);

                        if(sometmp->tag == ASTLEFT) {
                            return sometmp;
                        } else if(issimpletypeast(sometmp->right->tag) || sometmp->right->tag == TARRAYLITERAL) {
                            vectmp[idx] = sometmp->right;
                            idx++;
                            returntype = (AST *)hmalloc(sizeof(AST));
                            returntype->tag = TCOMPLEXTYPE;
                            returntype->lenchildren = idx - flag;
                            returntype->children = (AST **)hmalloc(sizeof(AST *) * returntype->lenchildren);
                            for(int cidx = 0, tidx = flag, tlen = returntype->lenchildren; cidx < tlen; cidx++, tidx++) {
                                returntype->children[cidx] = vectmp[tidx];
                            }
                            idx = flag;
                            flag = 0;
                            typestate = 6;
                        } else if(iscomplextypeast(sometmp->right->tag)) {
                            vectmp[idx] = sometmp->right;
                            idx++;
                            typestate = 21;
                        } else {
                            return ASTLeft(0, 0, "`of` in `=>` complex type *must* be followed by a type or an array of types.");
                        }
                        break;
                    case 4: // type
                    case 7: // of
                        //dprintf("%%debug: typestate = %d, sometmp->right->tag = %d\n", typestate, sometmp->right->tag);

                        sometmp = readexpression(fdin);

                        if(sometmp->tag == ASTLEFT) {
                            //debugln;
                            return sometmp;
                        } else if(!istypeast(sometmp->right->tag) && sometmp->right->tag != TARRAYLITERAL) {
                            //dprintf("type: %d\n", sometmp->right->tag);
                            //debugln;
                            return ASTLeft(0, 0, "a `:` form *must* be followed by a type definition...");
                        } else if(issimpletypeast(sometmp->right->tag)) { // simple type
                            if(typestate == 4) {
                                //debugln;
                                tmp = (AST *)hmalloc(sizeof(AST));
                                tmp->tag = TPARAMDEF;
                                tmp->lenchildren = 2;
                                tmp->children = (AST **)hmalloc(sizeof(AST *) * 2);
                                tmp->children[0] = vectmp[idx - 1];
                                tmp->children[1] = sometmp->right;
                                vectmp[idx - 1] = tmp;
                                typestate = 0;
                            } else if(typestate == 7) {
                                //debugln;
                                vectmp[idx] = sometmp->right;
                                AST *ctmp = (AST *)hmalloc(sizeof(AST));
                                ctmp->tag = TCOMPLEXTYPE;
                                ctmp->lenchildren = idx - flag;
                                ctmp->children = (AST **)hmalloc(sizeof(AST *) * ctmp->lenchildren); 

                                for(int tidx = flag + 1, cidx = 0; cidx < idx; cidx++, tidx++) {
                                    ctmp->children[cidx] = vectmp[tidx];
                                }

                                if(fatflag) {
                                    returntype = ctmp;
                                    idx = fatflag;
                                } else {
                                    tmp = (AST *)hmalloc(sizeof(AST));
                                    tmp->tag = TPARAMDEF;
                                    tmp->lenchildren = 2;
                                    tmp->children = (AST **)hmalloc(sizeof(AST *) * 2);
                                    tmp->children[0] = vectmp[flag];
                                    tmp->children[1] = ctmp;
                                    idx = flag + 1;
                                    vectmp[flag] = tmp;
                                }

                                typestate = 0;
                                // need to collapse from flag -> idx
                            } else {
                                returntype = sometmp->right;
                                typestate = 6;
                            }
                        } else { // complex type
                            debugln
                            vectmp[idx] = sometmp->right;
                            idx++;
                            typestate = 5;
                        }
                        break;
                    case 5: // complex type "of" game
                        //walk(sometmp->right, 0);
                        //debugln;
                        sometmp = readexpression(fdin);

                        // need to collapse the complex type in 
                        // the state transforms below, not just
                        // dispatch. It's close tho.
                        // this is why complex types aren't working in
                        // certain states (like `URL` as a stand alone)
                        // also need to trace down why other areas are 
                        // failing too (like after `=>`). Very close
                        if(sometmp->tag == ASTLEFT) {
                            return sometmp;
                        } else if(sometmp->right->tag == TOF) {
                            debugln;
                            typestate = 7;
                        } else if(sometmp->right->tag == TIDENT) {
                            debugln;
                            typestate = 1;
                        } else if(sometmp->right->tag == TEQ) {
                            typestate = 3;
                        } else if(sometmp->right->tag == TARRAYLITERAL) {
                            // which state here? need to check that the
                            // array only has types in it...
                            tmp = sometmp->right;
                            for(int cidx = 0; cidx < tmp->lenchildren; cidx++, idx++) {
                                vectmp[idx] = tmp->children[cidx];
                            }
                            typestate = 0;
                        } else if(sometmp->right->tag == TFATARROW) {
                            debugln;
                            fatflag = idx;
                            typestate = 2;
                        } else {
                            return ASTLeft(0, 0, "a complex type most be followed by an `of`, an ident, an array, a `=` or a `=>`");
                        }

                        // ok we have dispatched, now collapse
                        if(typestate != 7) {
                            debugln;
                            AST *ctmp = (AST *)hmalloc(sizeof(AST));
                            ctmp->tag = TCOMPLEXTYPE;
                            dprintf("typestate: %d, idx: %d, flag: %d\n", typestate, idx, flag);
                            ctmp->lenchildren = idx - flag - 1;
                            ctmp->children = (AST **)hmalloc(sizeof(AST *) * ctmp->lenchildren); 
                            dprintf("len of children should be: %d\n", ctmp->lenchildren);
                            for(int tidx = flag + 1, cidx = 0; tidx < idx ; cidx++, tidx++) {
                                debugln;
                                ctmp->children[cidx] = vectmp[tidx];
                            }

                            ctmp = linearize_complex_type(ctmp);

                            if(fatflag && fatflag != idx) {
                                returntype = ctmp;
                                /*
                                 * so the "correct" way of doing this would be to actually
                                 * break out the state for return, and then duplicate the
                                 * code there. It would be context-free, and would work
                                 * nicely. I didn't do that tho. I chose to de-dupe the
                                 * code, and just use flags. Best idea? Dunno, but the
                                 * code is smaller, and means I have to change fewer
                                 * locations. Honestly, this stuff should all be added
                                 * to *ONE* location that I can pull around to the various
                                 * places; right now records may not handle tags correctly,
                                 * for example. However, I think adding a "streaming" interface
                                 * in carML/carML (that is, self-hosting carML) would make
                                 * this a lot nicer looking internally.
                                 */
                                debugln;
                                idx = fatflag + 1;
                                flag = fatflag + 1;
                            } else {
                                debugln;
                                tmp = (AST *)hmalloc(sizeof(AST));
                                tmp->tag = TPARAMDEF;
                                tmp->lenchildren = 2;
                                tmp->children = (AST **)hmalloc(sizeof(AST *) * 2);
                                tmp->children[0] = vectmp[flag];
                                tmp->children[1] = ctmp;
                                vectmp[flag] = tmp;
                                idx = flag + 1;
                            }
                            //debugln;
                        }
                        break;
                    case 6: // post-fatarrow
                        sometmp = readexpression(fdin);

                        if(sometmp->tag == ASTLEFT) {
                            return sometmp;
                        } else if(sometmp->right->tag != TEQ) {
                            return ASTLeft(0, 0, "a `=>` return type must be followed by an `=`");
                        } else {
                            typestate = 3;
                        }
                        break;
                }
            }

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
            if(returntype != nil) {
                head->children = (AST **)hmalloc(sizeof(AST *) * 3);
                head->lenchildren = 3;
                head->children[2] = returntype;
            } else {
                head->children = (AST **)hmalloc(sizeof(AST *) * 2);
                head->lenchildren = 2;
            }
            head->children[0] = params;
            head->children[1] = tmp;
            return ASTRight(head);
        case TWHILE:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TWHILE;
            head->lenchildren = 2;
            head->children = (AST **)hmalloc(sizeof(AST *) * 2);

            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            }

            head->children[0] = sometmp->right;

            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else if(sometmp->right->tag != TDO) {
                return ASTLeft(0, 0, "While form conditions *must* be followed by a `do`...");
            }

            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            }

            head->children[1] = sometmp->right;

            return ASTRight(head);
        case TFOR:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TFOR;
            head->lenchildren = 2;
            head->children = (AST **)hmalloc(sizeof(AST *) * 2);

            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } if(sometmp->right->tag != TIDENT) {
                return ASTLeft(0, 0, "for-form's name must be an ident");
            }

            head->value = sometmp->right->value;

            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else if(sometmp->right->tag != TIN) {
                return ASTLeft(0, 0, "for-form binding *must* be followed by an `in`...");
            }
            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            }

            head->children[0] = sometmp->right;

            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else if(sometmp->right->tag != TDO) {
                return ASTLeft(0, 0, "for-form conditions *must* be followed by a `do`...");
            }

            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            }

            head->children[1] = sometmp->right;

            return ASTRight(head);
            break;

        case TWITH:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TWITH;
            head->lenchildren = 0;
            return ASTRight(head);
            break;
        case TMATCH:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = TMATCH;
            head->lenchildren = 2;
            head->children = (AST **)hmalloc(sizeof(AST *) * 2);
            AST *mcond = nil, *mval = nil, *mstack[128] = {nil};
            int msp = 0;

            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else {
                head->children[0] = sometmp->right;
            }

            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else if(sometmp->right->tag != TWITH) {
                return ASTLeft(0, 0, "a `match` form's expression *must* be followed by a `with`.");
            }

            // ok, we have an expression, and with have a WITH, now
            // we need to read our expressions.
            // expression => expression
            // else => expression
            // end $
            debugln;
            while(1) {
                sometmp = readexpression(fdin);
                debugln;
                if(sometmp->tag == ASTLEFT) {
                    return sometmp;
                } else if(sometmp->right->tag == TCALL) {
                    mcond = sometmp->right;
                    if(mcond->children[0]->tag != TTAG) {
                        return ASTLeft(0, 0, "match can only decompose sum-types.");
                    }
                } else if(sometmp->right->tag == TTAG || isprimitivevalue(sometmp->right->tag) || sometmp->right->tag == TELSE) {
                    mcond = sometmp->right;
                } else if(sometmp->right->tag == TEND) {
                    break;
                } else {
                    return ASTLeft(0, 0, "match cannot match against this value");
                }
                debugln;
                sometmp = readexpression(fdin);

                // need to figure out how to wedge guard clauses in here...
                if(sometmp->tag == ASTLEFT) {
                    return sometmp;
                } else if(sometmp->right->tag == TWHEN) {
                    // read in a guard clause
                    // then check for a TFATARROW
                } else if(sometmp->right->tag != TFATARROW) {
                    return ASTLeft(0, 0, "match conditions *must* be followed by a fat-arrow `=>`");
                } 
                debugln;
                sometmp = readexpression(fdin);

                if(sometmp->tag == ASTLEFT) {
                    return sometmp;
                } else {
                    mval = sometmp->right;
                }
                debugln;
                mstack[msp] = mcond;
                mstack[msp + 1] = mval;
                msp += 2;
                debugln;
            }
            debugln;
            tmp = (AST *)hmalloc(sizeof(AST));
            tmp->lenchildren = msp;
            tmp->children = (AST **)hmalloc(sizeof(AST *) * msp);

            for(int cidx = 0; cidx < msp; cidx += 2) {
                tmp->children[cidx] = mstack[cidx];
                tmp->children[cidx + 1] = mstack[cidx + 1];
            }
            tmp->tag = TBEGIN;
            head->children[1] = tmp;
            return ASTRight(head);
            break;
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
                if(tmp->tag == TIDENT || tmp->tag == TTAG) {
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
                    if(tmp->tag == TEND) {
                        debugln;
                        fatflag = 1;
                    }
                    /* collapse the call into a TCALL
                     * this has some _slight_ problems, since it 
                     * uses the same stack as the TBEGIN itself,
                     * so as we add items to the body, there will
                     * be less left over for intermediate calls...
                     * could add another stack, or could just do it
                     * this way to see how much of a problem it actually
                     * is...
                     */
                    if((idx - flag) > 1) {
                        AST *tcall = (AST *)hmalloc(sizeof(AST));
                        // preprocess the call for `$` first
                        for(int tidx = idx - 1, flag = idx; tidx > 0; tidx--) {
                            debugln;
                            if(vectmp[tidx]->tag == TDOLLAR) {
                                debugln;
                                AST *res = (AST *)hmalloc(sizeof(AST));
                                res->tag = TCALL;
                                res->lenchildren = flag - tidx - 1;
                                res->children = (AST **)hmalloc(sizeof(AST *) * (flag - tidx));
                                for(int ctidx = tidx + 1, ttidx = 0; ctidx < flag; ctidx++, ttidx++) {
                                    res->children[ttidx] = vectmp[ctidx];
                                }
                                debugln;
                                flag = tidx + 1;
                                idx = flag;
                                dprintf("idx == %d, flag == %d\n", idx, flag);
                                vectmp[tidx] = res;
                            }
                        }
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
                        tmp = tcall;
                    } else {
                        tmp = vectmp[flag];
                    }
                    idx = flag;
                    flag = -1;
                } else if(tmp->tag == TNEWL) {
                    continue;
                }

                /*walk(tmp, 0);
                debugln;
                printf("tmp->tag == %d\n", tmp->tag);*/

                if(fatflag) {
                    vectmp[idx++] = tmp;
                    break;
                } else if(tmp->tag == TEND) {
                    break;
                } else {
                    vectmp[idx++] = tmp;
                }

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

            // process these in *reverse*:
            // each time you hit a $, add it
            // to the stack...
            // is it worth it? lemme work it.
            // I put my stack down, flip it, and reverse it.
            // Ti esrever dna, ti pilf, nwod kcats ym tup i.
            // Ti esrever dna, ti pilf, nwod kcats ym tup i.
            dprintf("idx == %d\n", idx);
            for(int tidx = idx - 1, flag = idx; tidx > 0; tidx--) {
                debugln;
                if(vectmp[tidx]->tag == TDOLLAR) {
                    debugln;
                    AST *res = (AST *)hmalloc(sizeof(AST));
                    res->tag = TCALL;
                    res->lenchildren = flag - tidx - 1;
                    res->children = (AST **)hmalloc(sizeof(AST *) * (flag - tidx));
                    for(int ctidx = tidx + 1, ttidx = 0; ctidx < flag; ctidx++, ttidx++) {
                        res->children[ttidx] = vectmp[ctidx];
                    }
                    debugln;
                    flag = tidx + 1;
                    idx = flag;
                    dprintf("idx == %d, flag == %d\n", idx, flag);
                    vectmp[tidx] = res;
                }
            }
            dprintf("idx == %d\n", idx);
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
        case TPOLY:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = ltype;

            head->lenchildren = 2;
            head->children = (AST **)hmalloc(sizeof(AST *) * 2);

            AST *nstack[128] = {nil};
            int nsp = 0, collapse_complex = 0;

            sometmp = readexpression(fdin);

            if(sometmp->tag == ASTLEFT) {
                return sometmp;
            } else if(sometmp->right->tag != TTAG) {
                return ASTLeft(0, 0, "type/poly definition *must* be followed by a Tag name");
            } else {
                head->value = sometmp->right->value;
            }

            flag = idx;

            /* type|poly Tag type-var* {
             *     (Tag-name type-member*)+
             * }
             * need to put this in a loop, so that
             * we can read the various idents (typevars)
             * before the begin...
             */
            while(ltmp != TBEGIN) {
                ltmp = next(fdin, &buffer[0], 512);

                if(ltmp == TBEGIN) {
                    break;
                } else if(ltmp == TTAG) {
                    debugln;
                    tmp = (AST *)hmalloc(sizeof(AST));
                    tmp->tag = TTAG;
                    tmp->value = hstrdup(buffer);
                    vectmp[idx] = tmp;
                    idx++;
                } else {
                    return ASTLeft(0, 0, "record-definition *must* begin with BEGIN");
                }
            }

            if(idx > flag) {

                tmp = (AST *)hmalloc(sizeof(AST));
                tmp->tag = TPARAMLIST;
                tmp->lenchildren = idx - flag;
                tmp->children = (AST **)hmalloc(sizeof(AST *) * tmp->lenchildren);
                debugln;
                for(int tidx = 0; flag < idx; flag++, tidx++) {
                    dprintf("%d %d\n", tidx, flag);
                    tmp->children[tidx] = vectmp[flag];
                }

                head->children[0] = tmp;
                debugln;
                dwalk(tmp, 0);
                dprintf("\n");
                flag = 0;
                idx = 0;
            } else {
                // AGAIN with the option types...
                head->children[0] = nil;
            }

            typestate = -1;

            /* here, we just need to read:
             * 1. a Tag name
             * 2. some number of variables
             * Honestly, could almost lift the TDEF code instead...
             */
            while(sometmp->right->tag != TEND) {

                // so this below fixes the TNEWL
                // bug noted in test-type, but it 
                // introduces another bug of blank
                // constructors. I figured that would
                // be a problem, because the state
                // transition below seems a bit off
                // anyway. So, more to fix there, but
                // close...
                sometmp = llreadexpression(fdin, 1);
                //sometmp = readexpression(fdin);

                if(sometmp->tag == ASTLEFT) {
                    return sometmp;
                }
                debugln; 
                dwalk(sometmp->right, 0);
                dprintf("\n");
                dprintf("tag == TEND? %s\n", sometmp->right->tag == TEND ? "yes" : "no");
                dprintf("typestate == %d\n", typestate);
                dprintf("collapse == %d\n", typestate);
                switch(typestate) {
                    case -1:
                        if(sometmp->right->tag == TTAG) {
                            typestate = 0;
                            vectmp[idx] = sometmp->right;
                            idx++;
                        } else if(sometmp->right->tag == TEND || sometmp->right->tag == TNEWL || sometmp->right->tag == TSEMI){
                            typestate = -1;
                        } else {
                            return ASTLeft(0, 0, "type/poly constructors must be Tags.");
                        }
                        break;

                    case 0:
                        if(sometmp->right->tag == TIDENT) {
                            typestate = 1;
                        } else if(issimpletypeast(sometmp->right->tag)) {
                            typestate = 0;
                        } else if(iscomplextypeast(sometmp->right->tag)) {
                            flag = idx; // start of complex type
                            typestate = 3;
                        } else if(sometmp->right->tag == TEND || sometmp->right->tag == TNEWL || sometmp->right->tag == TSEMI) {
                            typestate = -1;
                        } else {
                            return ASTLeft(0, 0, "constructor's must be followed by a name or a type.");
                        }

                        if(typestate != -1) {
                            debugln;
                            vectmp[idx] = sometmp->right;
                            idx++;
                        }
                        break;

                    case 1: // we had an ident, now need a type
                        if(sometmp->right->tag == TCOLON) {
                            typestate = 2;
                        } else {
                            return ASTLeft(0, 0, "constructor named vars must be followed by a colon and a type specifier.");
                        }
                        break;

                    case 2:
                        debugln;
                        if(issimpletypeast(sometmp->right->tag)) {
                            typestate = 0;
                        } else if(iscomplextypeast(sometmp->right->tag)) {
                            typestate = 3;
                        } else {
                            return ASTLeft(0, 0, "expecting type in user-type definition");
                        }

                        vectmp[idx] = sometmp->right;
                        idx++;
                        break;
                    case 3:
                        debugln;
                        if(sometmp->right->tag == TOF) {
                            debugln;
                            typestate = 4;
                        } else if(sometmp->right->tag == TIDENT) {
                            debugln;
                            typestate = 1;
                        } else if(issimpletypeast(sometmp->right->tag)) {
                            debugln;
                            typestate = 0;
                        } else if(iscomplextypeast(sometmp->right->tag)) {
                            debugln;
                            typestate = 3;
                        } else if(sometmp->right->tag == TARRAYLITERAL) {
                            debugln;
                            typestate = 0;
                            tmp = sometmp->right;
                            for(int tidx = 0; tidx < tmp->lenchildren; tidx++, idx++) {
                                debugln;
                                vectmp[idx] = tmp->children[tidx];
                            }
                            debugln;
                        } else if(sometmp->right->tag == TEND || sometmp->right->tag == TNEWL || sometmp->right->tag == TSEMI) {
                            debugln;
                            typestate = -1;
                        }

                        debugln;

                        if(typestate != 4 && typestate != -1) {
                            debugln;
                            dprintf("typestate == %d\n", typestate);
                            tmp = (AST *)hmalloc(sizeof(AST));
                            tmp->tag = TCOMPLEXTYPE;
                            tmp->lenchildren = idx - flag;
                            tmp->children = (AST **)hmalloc(sizeof(AST *) * tmp->lenchildren);
                            for(int cidx = 0, tidx = flag, tlen = tmp->lenchildren; cidx < tlen; cidx++, tidx++) {
                                tmp->children[cidx] = vectmp[tidx];
                            }
                            vectmp[flag] = tmp;
                            flag = flag + 1;
                            idx = flag;
                            //flag = 0;
                            /*nstack[nsp] = tmp;
                            nsp++;
                            collapse_complex = 0;*/
                            // this is *very* gross
                            if(typestate == 3 || (typestate == 0 && sometmp->right->tag != TARRAYLITERAL)) {
                                vectmp[idx] = sometmp->right;
                                idx++;
                            }
                        }
                        break;
                    case 4:
                        if(sometmp->right->tag == TIDENT) {
                            debugln;
                            typestate = 1;
                        } else if(issimpletypeast(sometmp->right->tag)) {
                            debugln;
                            typestate = 0;
                        } else if(iscomplextypeast(sometmp->right->tag)) {
                            debugln;
                            typestate = 3;
                            vectmp[idx] = sometmp->right;
                            idx++;
                        } else if(sometmp->right->tag == TARRAYLITERAL) {
                            debugln;
                            typestate = 0;
                            tmp = sometmp->right;
                            for(int tidx = 0; tidx < tmp->lenchildren; tidx++, idx++) {
                                debugln;
                                vectmp[idx] = tmp->children[tidx];
                            }
                            debugln;
                        } else if(sometmp->right->tag == TEND || sometmp->right->tag == TNEWL || sometmp->right->tag == TSEMI) {
                            debugln;
                            typestate = -1;
                        }

                        if(typestate != 3 && typestate != -1) {
                            debugln;
                            dprintf("typestate == %d\n", typestate);
                            tmp = (AST *)hmalloc(sizeof(AST));
                            tmp->tag = TCOMPLEXTYPE;
                            tmp->lenchildren = idx - flag;
                            tmp->children = (AST **)hmalloc(sizeof(AST *) * tmp->lenchildren);
                            for(int cidx = 0, tidx = flag, tlen = tmp->lenchildren; cidx < tlen; cidx++, tidx++) {
                                tmp->children[cidx] = vectmp[tidx];
                            }
                            vectmp[flag] = tmp;
                            flag = flag + 1;
                            idx = flag;
                            //flag = 0;
                            /*nstack[nsp] = tmp;
                            nsp++;
                            collapse_complex = 0;*/
                            //vectmp[idx] = sometmp->right;
                            //idx++;
                        }
                        break;
                }

                if(collapse_complex) {
                }

                // ok, we got to the end of *something*
                // collapse it here
                if(typestate == -1 && idx > 0) {
                    params = (AST *) hmalloc(sizeof(AST));
                    params->lenchildren = idx;
                    params->children = (AST **)hmalloc(sizeof(AST *) * idx);

                    for(int i = 0; i < params->lenchildren; i++, flag++) {
                        params->children[i] = vectmp[i];
                    }

                    params->tag = TTYPEDEF;
                    nstack[nsp] = params;
                    flag = 0;
                    idx = flag;
                    nsp++;

                    debugln;

                    if(sometmp->right->tag == TEND) {
                        debugln;
                        break;
                    }
                }
            }
            // oh, right
            // it would be helpful to collect the above
            // and make them into like... an AST
            debugln;
            params = (AST *)hmalloc(sizeof(AST));
            dprintf("idx == %d\n", nsp);
            params->lenchildren = nsp;
            params->children = (AST **)hmalloc(sizeof(AST *) * nsp);

            for(int cidx = 0; cidx < nsp; cidx++) {
                params->children[cidx] = nstack[cidx];                
            }

            params->tag = TBEGIN;

            head->children[1] = params;

            debugln;
            return ASTRight(head);
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
                                } else if(sometmp->right->tag == TARRAYLITERAL) {
                                    tmp = sometmp->right;
                                    for(int cidx = 0; cidx < tmp->lenchildren; cidx++) {
                                        vectmp[idx++] = tmp->children[cidx];
                                    }
                                    typestate = 2;
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
                    tmp = linearize_complex_type(tmp);
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
            } else if(sometmp->right->tag != TTAG) {
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
                return ASTLeft(0, 0, "record-definition *must* begin with BEGIN");
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
                                    } else if(sometmp->right->tag == TARRAYLITERAL) {
                                        tmp = sometmp->right;
                                        for(int cidx = 0; cidx < tmp->lenchildren; cidx++, idx++) {
                                            vectmp[idx] = tmp->children[cidx];
                                        }
                                        typestate = 2;
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

                        ctmp = linearize_complex_type(ctmp);

                        /* create the record field definition holder */
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
        case THEX:
        case TOCT:
        case TBIN:
        case TINT:
        case TFLOAT:
        case TSTRING:
        case TCHAR:
        case TBOOL:
        case TARRAY:
        case TDEQUET:
		case TPROCEDURET:
		case TFUNCTIONT:
        case TCOMMA:
        case TTUPLET:
        case TANY:
        case TAND:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = ltype;
            head->value = hstrdup(buffer);
            return ASTRight(head);
        case TNEWL:
            if(!nltreatment) {
                return llreadexpression(fdin, nltreatment);
            } else {
                head = (AST *)hmalloc(sizeof(AST));
                head->tag = TNEWL;
                return ASTRight(head);
            }
        case TFALSE:
        case TTRUE:
        case TEQ:
        case TCHART:
        case TSTRT:
        case TINTT:
        case TFLOATT:
        case TBOOLT:
        case TCOLON:
        case TSEMI:
        case TFATARROW:
        case TPIPEARROW:
        case TREF:
            head = (AST *)hmalloc(sizeof(AST));
            head->tag = ltype;
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
    return nil;
}

/* read in a single type, and return it as an AST node
 */
ASTOffset *
mung_single_type(const char **pdecls, const int **plexemes, int len, int haltstate, int offset) {
    return nil;
}

void
indent(int level) {
    // should probably look to inline this
    // basically what we are replacing is the
    // inlined version of the same
    for(int idx = 0; idx < level; idx++) {
        printf("    ");
    }
}

void
walk(AST *head, int level) {
    int idx = 0;
    AST *tmp = nil;

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

            if(head->lenchildren == 3) {
                // indent nicely
                printf("\n");
                for(; idx < level + 1; idx++) {
                    printf("    ");
                }

                printf("(returns ");
                walk(head->children[2], 0);
                printf(")");
            }
            printf("\n");
            walk(head->children[1], level + 1);
            printf(")");
            break;
        case TUSE:
            printf("(use ");
            walk(head->children[0], 0);
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
        case TWHILE:
            printf("(while ");
            walk(head->children[0], 0);
            printf("\n");
            walk(head->children[1], level + 1);
            printf(")");
            break;
        case TFOR:
            printf("(for %s ", head->value);
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
        case TPARAMDEF:
            printf("(parameter-definition ");
            walk(head->children[0], 0);
            printf(" ");
            walk(head->children[1], 0);
            printf(")");
            break;
        case TTYPE:
        case TPOLY:
            if(head->tag == TPOLY) {
                printf("(polymorphic-type ");
            } else {
                printf("(type ");
            }
            printf("%s ", head->value);
            if(head->children[0] != nil)
            {
                walk(head->children[0], 0);
            }
            printf("\n");
            for(int cidx = 0; cidx < head->children[1]->lenchildren; cidx++) {
                walk(head->children[1]->children[cidx], level + 1);
                if(cidx < (head->children[1]->lenchildren - 1)) {
                    printf("\n");
                }
            }
            printf(")\n");
            break;
        case TTYPEDEF:
            printf("(type-constructor ");
            for(int cidx = 0; cidx < head->lenchildren; cidx++) {
                walk(head->children[cidx], 0);
                if(cidx < (head->lenchildren - 1)) {
                    printf(" ");
                }
            }
            printf(")");
            break;
        case TARRAY:
            printf("(type array)");
            break;
        case TDEQUET:
            printf("(type deque)");
            break;
        case TFUNCTIONT:
            printf("(type function)");
            break;
        case TPROCEDURET:
            printf("(type procedure)");
            break;
        case TTUPLET:
            printf("(type tuple)");
            break;
        case TREF:
            printf("(type ref)");
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
        case TMATCH:
            debugln;
            printf("(match ");
            walk(head->children[0], 0);
            printf("\n");
            tmp = head->children[1];
            for(int cidx = 0; cidx < tmp->lenchildren; cidx += 2) {
                if(tmp->children[cidx]->tag == TELSE) {
                    indent(level + 1);
                    printf("else");
                } else {
                    walk(tmp->children[cidx], level + 1);
                }
                printf(" => ");
                walk(tmp->children[cidx + 1], 0);
                if(cidx < (tmp->lenchildren - 2)) {
                    printf("\n");
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
        case TBOOLT:
            printf("(type boolean)");
            break;
        case TCHART:
            printf("(type char)");
            break;
        case TANY:
            printf("(type any)");
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

void
generate_type_value(AST *head, const char *name) {
    int midx = 0, cidx = 0;
    char *tbuf = nil, buf[512] = {0}, *rtbuf = nil, rbuf[512] = {0};
    char *member = nil, membuf[512] = {0};
    // setup a nice definition...
    printf("%s\n%s_%s(", name, name, head->children[0]->value);
    // dump our parameters...
    for(cidx = 1; cidx < head->lenchildren; cidx++) {
        snprintf(buf, 512, "m_%d", cidx); 
        rtbuf = typespec2c(head->children[cidx], rbuf, buf, 512);
        if(cidx < (head->lenchildren - 1)) {
            printf("%s, ", rtbuf);
        } else {
            printf("%s", rtbuf);
        }
    }
    printf(") {\n");

    // define our return value...
    indent(1);
    printf("%s res;\n", name);

    // grab the constructor name...
    member = head->children[0]->value;
    member = upcase(member, membuf, 512);

    // tag our type
    indent(1);
    printf("res.tag = TAG_%s_%s;\n", name, member);

    // set all members...
    for(cidx = 1; cidx < head->lenchildren; cidx++) {
        indent(1);
        snprintf(buf, 512, "m_%d", cidx);
        printf("res.members.%s_t->%s = %s;\n", member, buf, buf);
    }
    indent(1);
    printf("return res;\n}\n");
}

void
generate_type_ref(AST *head, const char *name) {
    int midx = 0, cidx = 0;
    char *tbuf = nil, buf[512] = {0}, *rtbuf = nil, rbuf[512] = {0};
    char *member = nil, membuf[512] = {0};
    // setup a nice definition...
    printf("%s *\n%s_%s_ref(", name, name, head->children[0]->value);
    // dump our parameters...
    for(cidx = 1; cidx < head->lenchildren; cidx++) {
        snprintf(buf, 512, "m_%d", cidx); 
        rtbuf = typespec2c(head->children[cidx], rbuf, buf, 512);
        if(cidx < (head->lenchildren - 1)) {
            printf("%s, ", rtbuf);
        } else {
            printf("%s", rtbuf);
        }
    }
    printf(") {\n");
    // define our return value...
    indent(1);
    printf("%s *res = (%s *)malloc(sizeof(%s));\n", name, name, name);

    // grab the constructor name...
    member = head->children[0]->value;
    member = upcase(member, membuf, 512);

    // tag our type
    indent(1);
    printf("res->tag = TAG_%s_%s;\n", name, member);

    // set all members...
    for(cidx = 1; cidx < head->lenchildren; cidx++) {
        indent(1);
        snprintf(buf, 512, "m_%d", cidx);
        printf("res->members.%s_t->%s = %s;\n", member, buf, buf);
    }
    indent(1);
    printf("return res;\n}\n");
}

void
llcwalk(AST *head, int level, int final) {
    int idx = 0, opidx = -1;
    char *tbuf = nil, buf[512] = {0}, rbuf[512] = {0}, *rtbuf = nil;
    AST *ctmp = nil, *htmp = nil;

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
                // need to lift lambdas...
                printf("(fn ");
            } else {
                if(head->lenchildren == 3) {
                    cwalk(head->children[2], 0);
                } else {
                    printf("void");
                }
                printf("\n%s", head->value);
            }
            if(head->children[0] != nil) {
                cwalk(head->children[0], level);
            } else {
                printf("()");
            }

            printf("{\n");
            // we need to check if we have a primitive 
            // value or a CALL here. Honestly, what does
            // it mean to have a $() form here tho?

            if(isvalueform(head->children[1]->tag)) {
                for(; idx < level + 1; idx++) {
                    printf("    ");
                }
                printf("return ");
                cwalk(head->children[1], 0);
                printf(";\n");
            } else if(issyntacticform(head->children[1]->tag)) {
                // does this need to be a thing? can we just pass
                // the YES to llcwalk and let the lower level
                // forms handle it?
                llcwalk(head->children[1], level + 1, YES);
            } else {
                llcwalk(head->children[1], level + 1, YES);
            }
            printf("}");
            break;
        case TVAL:
        case TVAR:
            if(head->tag == TVAL) {
                printf("const ");
            }

            if(head->lenchildren == 2) {
                if(head->children[1]->tag == TCOMPLEXTYPE) {
                    tbuf = typespec2c(head->children[1], buf, head->value, 512);
                    printf("%s= ", tbuf);
                } else {
                    cwalk(head->children[1], 0);
                    printf(" %s = ", head->value);
                }
            } else {
                printf("void *");
                printf(" %s = ", head->value);
            }


            cwalk(head->children[0], 0);

            printf(";");
            break;
        case TLET:
        case TLETREC:
            // need to do name rebinding...
            // but I think that's best meant
            // for a nanopass...
            if(head->tag == TLET) {
                printf("(let ");
            } else {
                printf("(letrec ");
            }

            printf("%s ", head->value);

            cwalk(head->children[0], 0);
            
            if(head->lenchildren == 3) {
                /* if we have a type,
                 * go ahead and print it.
                 */
                printf(" ");
                cwalk(head->children[2], 0);
            }

            printf("\n");
            cwalk(head->children[1], level + 1);

            printf(")");
            break;
        case TWHEN:
            printf("if(");
            cwalk(head->children[0], 0);
            printf("){\n");
            if(final && isvalueform(head->children[1]->tag)) {
                //indent(level + 1);
                printf("return ");
                cwalk(head->children[1], 0);
                printf(";\n");
            } else if(final) {
                llcwalk(head->children[1], level + 1, YES);
            } else {
                cwalk(head->children[1], level + 1);
                printf(";\n");
            }
            indent(level);
            printf("}");
            break;
        case TMATCH:
            // there are several different strategies to
            // use here...
            // 1. simple if-then-else chain for things like string compares
            // 2. switch block (can use FNV1a for strings => switch, straight for int/float)
            // 3. unpacking ADTs means we have to detect tag & collate those cases together

            if(head->children[0]->tag == TIDENT) {
                ctmp = head->children[0];
            } else {
                ctmp = (AST *)hmalloc(sizeof(AST));
                ctmp->tag = TIDENT;
                snprintf(&buf[0], 512, "l%d", rand());

                // need to demand a type here...
            }

            // need to:
            // demand a type from the results
            // make sure it reifies
            // generate if/else 
            // handle bindings
            // for now, just roll with it
            htmp = head->children[1];
            for(int tidx = 0; tidx < htmp->lenchildren; tidx+=2) {
                if(tidx == 0) {
                    printf("if(");
                } else if(htmp->children[tidx]->tag == TELSE) {
                    indent(level);
                    printf("} else ");
                } else {
                    indent(level);
                    printf("} else if(");
                }

                if(htmp->children[tidx]->tag != TELSE) {
                    switch(htmp->children[tidx]->tag) {
                        // add HEX, OCT, BIN here too
                        // have to figure out how to encode booleans as well
                        case TFLOAT:
                        case TINT:
                        case TTAG:
                            printf("%s == %s", ctmp->value, htmp->children[tidx]->value);
                            break;
                        case TCHAR:
                            printf("%s == '%s'", ctmp->value, htmp->children[tidx]->value);
                            break;
                        case TSTRING:
                            printf("!strncmp(%s, \"%s\", %lu)", ctmp->value, htmp->children[tidx]->value, strlen(htmp->children[tidx]->value));
                            break;
                        default:
                            break;
                    }
                    printf(") ");
                }
                printf("{\n");
                if(final && isvalueform(htmp->children[tidx + 1]->tag)) {
                    indent(level + 1);
                    printf("return ");
                    cwalk(htmp->children[tidx + 1], 0);
                    printf(";\n");
                } else if(final) {
                    llcwalk(htmp->children[tidx + 1], level + 1, YES);
                } else {
                    cwalk(htmp->children[tidx + 1], level + 1);
                    printf(";\n");
                }
            }
            indent(level);
            printf("}\n");
            break;
        case TWHILE:
            printf("while(");
            cwalk(head->children[0], 0);
            printf("){\n");
            if(head->children[1]->tag == TBEGIN) {
                ctmp = head->children[1];
                for(int widx = 0; widx < ctmp->lenchildren; widx++) {
                    cwalk(ctmp->children[widx], level + 1);
                    printf(";\n");
                }
                indent(level);
                printf("}");
            } else {
                cwalk(head->children[1], level + 1);
                printf(";\n}");
            }
            break;
        case TPARAMLIST:
            printf("(");
            for(;idx < head->lenchildren; idx++) {
                AST *dc_type = nil, *dc_name = nil;
                if(head->children[idx]->tag == TIDENT) {
                    dc_name = head->children[idx];
                    printf("void *%s", dc_name->value);
                } else {
                    // now we've reached a TPARAMDEF
                    // so just dump whatever is returned
                    // by typespec2c
                    dc_type = head->children[idx]->children[1];
                    dc_name = head->children[idx]->children[0];
                    tbuf = typespec2c(dc_type, buf, dc_name->value, 512);
                    if(tbuf != nil) {
                        printf("%s", tbuf);
                    } else {
                        printf("void *");
                    }
                }

                if(idx < (head->lenchildren - 1)){
                    printf(", ");
                }
            }
            printf(")");
            break;
        case TPARAMDEF:
            cwalk(head->children[1], 0);
            printf(" ");
            cwalk(head->children[0], 0);
            break;
        case TARRAY:
            printf("(type array)");
            break;
        case TCOMPLEXTYPE:
            tbuf = typespec2c(head, buf, nil, 512); 
            if(tbuf != nil) {
                printf("%s", tbuf);
            } else {
                printf("void *");
            }
            break;
        case TTYPE:
        case TPOLY:
            // setup our name
            tbuf = upcase(head->value, &buf[0], 512);
            htmp = head->children[1];

            // generate our enum for the various tags for this
            // type/poly (forEach constructor thereExists |Tag|)
            printf("enum Tags_%s {\n", tbuf);
            for(int cidx = 0; cidx < htmp->lenchildren; cidx++) {
                indent(level + 1);
                // I hate this, but it works
                rtbuf = upcase(htmp->children[cidx]->children[0]->value, rbuf, 512);
                printf("TAG_%s_%s,\n", head->value, rtbuf);
            }
            printf("};\n");

            // generate the rough structure to hold all 
            // constructor members 
            printf("typedef struct %s_t {\n", tbuf);
            indent(level + 1);
            printf("int tag;\n");
            indent(level + 1);
            printf("union {\n");
            // first pass: 
            // - dump all constructors into a union struct.
            // - upcase the constructor name
            // TODO: move structs to top-level structs?
            // TODO: optimization for null members (like None in Optional)
            // TODO: naming struct members based on names given by users
            // TODO: inline records
            for(int cidx = 0; cidx < htmp->lenchildren; cidx++) {
                debugln;
                indent(level + 2);
                printf("struct {\n");
                ctmp = htmp->children[cidx];
                debugln;
                for(int midx = 1; midx < ctmp->lenchildren; midx++) {
                    debugln;
                    dprintf("type tag of ctmp: %d\n", ctmp->tag);
                    dprintf("midx: %d, len: %d\n", midx, ctmp->lenchildren);
                    indent(level + 3);
                    snprintf(buf, 512, "m_%d", midx);
                    debugln;
                    dprintf("walking children...\n");
                    dwalk(ctmp->children[midx], level);
                    dprintf("done walking children...\n");
                    dprintf("ctmp->children[%d] == null? %s\n", midx, ctmp->children[midx] == nil ? "yes" : "no");
                    rtbuf = typespec2c(ctmp->children[midx], rbuf, buf, 512);
                    printf("%s;\n", rtbuf);
                    debugln;
                }
                indent(level + 2);
                tbuf = upcase(ctmp->children[0]->value, buf, 512);
                printf("} %s_t;\n", buf);
            }
            indent(level + 1);
            printf("} members;\n");
            printf("} %s;\n", head->value);

            // ok, now we have generated the structure, now we
            // need to generate the constructors.
            // we need to do two passes at that:
            // - one pass with values
            // - one pass with references

            for(int cidx = 0; cidx < htmp->lenchildren; cidx++) {
                generate_type_value(htmp->children[cidx], head->value);
                generate_type_ref(htmp->children[cidx], head->value);
            }
            break;
        case TARRAYLITERAL:
            printf("{");
            for(;idx < head->lenchildren; idx++) {
                cwalk(head->children[idx], 0); 
                if(idx < (head->lenchildren - 1)){
                    printf(", ");
                }
            }
            printf("}");
            break;
        case TCALL:
            // do the lookup of idents here, and if we have
            // a C operator, use that instead

            opidx = iscoperator(head->children[0]->value);

            if(head->children[0]->tag == TIDENT && opidx != -1) {
                if(head->lenchildren == 2) {
                    printf("%s ", coperators[opidx]);
                    cwalk(head->children[1], 0);
                } else if(!strncmp(head->children[0]->value, "make-struct", 11)) {
                    printf("{ ");
                    for(int cidx = 1; cidx < head->lenchildren; cidx++) {
                        cwalk(head->children[cidx], 0);
                        if(cidx < (head->lenchildren - 1)) {
                            printf(", ");
                        }
                    }
                    printf("}");
                } else {
                    cwalk(head->children[1], 0);
                    if(!strncmp(head->children[0]->value, ".", 2)) {
                        printf("%s", coperators[opidx]);
                        cwalk(head->children[2], 0);
                    } else if(!strncmp(head->children[0]->value, "->", 2)) {
                        printf("%s", coperators[opidx]);
                        cwalk(head->children[2], 0);
                    } else if(!strncmp(head->children[0]->value, "get", 3)) {
                        printf("[");
                        cwalk(head->children[2], 0);
                        printf("]");
                    } else {
                        printf(" %s ", coperators[opidx]);
                        cwalk(head->children[2], 0);
                    }
                }
            } else if(head->lenchildren == 1) {
                printf("%s()", head->children[0]->value);
            } else {
                printf("%s(", head->children[0]->value);
                for(int i = 1; i < head->lenchildren; i++) {
                    cwalk(head->children[i], 0);
                    if(i < (head->lenchildren - 1)) {
                        printf(", ");
                    }
                }
                printf(")");
            }
            break;
        case TIF:
            printf("if(");
            cwalk(head->children[0], 0);
            printf("){\n");

            // such a hack; I hate doing this at
            // the compiler level. Should be done as
            // a pass above.
            if(final && isvalueform(head->children[1]->tag)) {
                // need to extract all of these calls to an
                // actual indent function
                /*for(idx = 0; idx < level + 1; idx++) {
                    printf("    ");
                }*/
                indent(level + 1);
                printf("return ");
                cwalk(head->children[1], 0);
                printf(";");
            } else if(final) {
                llcwalk(head->children[1], level, YES);
            } else {
                cwalk(head->children[1], level + 1);
                printf(";");
            }

            printf("\n");

            for(idx = 0; idx < level; idx++) {
                printf("    ");
            }

            printf("} else {\n");

            if(final && isvalueform(head->children[2]->tag)) {
                for(idx = 0; idx < level + 1; idx++) {
                    printf("    ");
                }
                printf("return ");
                cwalk(head->children[2], 0);
                printf(";\n");
            } else if(final) {
                llcwalk(head->children[2], level + 1, YES);
            } else {
                cwalk(head->children[2], level + 1);
                printf(";\n");
            }

            for(idx = 0; idx < level; idx++) {
                printf("    ");
            }

            printf("}\n");
            break;
        case TIDENT:
            printf("%s", head->value);
            break;
        case TTAG:
            printf("%s", head->value);
            break;
        case TBOOL:
            /* really, would love to introduce a higher-level
             * boolean type, but not sure I care all that much
             * for the initial go-around in C...
             */
            if(head->value[0] == 0) {
                printf("1"); 
            } else {
                printf("0");
            }
            break;
        case TCHAR:
            switch(head->value[0]) {
                case '\n':
                    printf("'\\n'");
                    break;
                case '\r':
                    printf("'\\r'");
                    break;
                case '\t':
                    printf("'\\t'");
                    break;
                case '\v':
                    printf("'\\v'");
                    break;
                case '\0':
                    printf("'\\0'");
                    break;
                case '\'':
                    printf("'\''");
                    break;
                default:
                    printf("'%c'", head->value[0]);
                    break;
            }
            break;
        case TFLOAT:
            printf("%sf", head->value);
            break;
        case TFALSE:
        case TTRUE:
            if(head->tag == TFALSE) {
                printf("false");
            } else {
                printf("true");
            }
            break;
        case THEX:
            printf("0x%s", head->value);
            break;
        case TOCT:
            printf("0%s", head->value);
            break;
        case TBIN:
            /* convert to hex... */
            printf("(binary-integer %s)", head->value);
            break;
        case TINT:
            printf("%s", head->value);
            break;
        case TINTT:
            printf("int");
            break;
        case TFLOATT:
            printf("float");
            break;
        case TBOOLT:
            printf("bool");
            break;
        case TCHART:
            printf("char");
            break;
        case TSTRT:
            /* make a fat version of this?
             * or just track the length every
             * where?
             */
            printf("char *");
            break;
        case TSTRING:
            printf("\"%s\"", head->value);
            break;
        case TRECORD:
            printf("typedef struct {\n");
            for(int i = 0; i < head->lenchildren; i++) {
                cwalk(head->children[i], level + 1);
                if(i < (head->lenchildren - 1)) {
                    printf("\n");
                }
            }
            printf("\n} %s;", head->value);
            break;
        case TRECDEF:
            if(head->lenchildren == 2) {
                cwalk(head->children[1], 0);
            } else {
                printf("void *");
            }
            printf(" ");
            cwalk(head->children[0], 0);
            printf(";");
            break;
        case TBEGIN:
            // TODO: this code is super ugly & can be cleaned up
            for(idx = 0; idx < head->lenchildren; idx++){
                if(idx == 0) {
                    if(head->lenchildren == 1 && final && isvalueform(head->children[0]->tag)) {
                        printf("return ");
                        cwalk(head->children[idx], 0);
                        printf(";\n");
                    } else if(head->lenchildren == 1 && final) {
                        llcwalk(head->children[idx], 0, YES);
                    } else {
                        cwalk(head->children[idx], 0);
                        printf(";\n");
                    }
                } else if(idx < (head->lenchildren - 1)) {
                    cwalk(head->children[idx], level);
                    if(!issyntacticform(head->children[idx]->tag)){
                        printf(";\n");
                    } else {
                        printf("\n");
                    }
                } else if(idx < head->lenchildren) {
                    if(isvalueform(head->children[idx]->tag)) {
                        indent(level);
                        if(final) {
                            printf("return ");
                        }
                        cwalk(head->children[idx], 0);
                        printf(";\n");
                    } else {
                        if(final) {
                            llcwalk(head->children[idx], level, YES);
                        } else {
                            cwalk(head->children[idx], level);
                        }
                        if(!issyntacticform(head->children[idx]->tag)){
                            printf(";\n");
                        } else {
                            printf("\n");
                        }
                    }
                }
            }
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

char *
upcase(const char *src, char *dst, int len) {
    int idx = 0;

    for(; idx < len; idx++) {
        if(src[idx] == '\0') {
            dst[idx] = '\0';
            break;
        } else if(idx == (len - 1)) {
            dst[idx] = '\0';
            break;
        } else if(src[idx] >= 'a' && src[idx] <= 'z') {
            dst[idx] = 'A' + (src[idx] - 'a');
        } else {
            dst[idx] = src[idx];
        }
    }

    return dst;
}

char *
downcase(const char *src, char *dst, int len) {
    int idx = 0;

    for(; idx < len; idx++) {
        if(src[idx] == '\0') {
            dst[idx] = '\0';
            break;
        } else if(idx == (len - 1)) {
            dst[idx] = '\0';
            break;
        } else if(src[idx] >= 'A' && src[idx] <= 'Z') {
            dst[idx] = 'a' + (src[idx] - 'A');
        } else {
            dst[idx] = src[idx];
        }
    }
    return dst;
}
