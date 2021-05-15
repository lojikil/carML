/*
 * @(#) the main header file, meant to keep definitions clean
 * @(#) and help make integration of generated code smoother
*/
#ifndef __CARMLC_H
#define __CARMLC_H

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
#define gwalk(head, level) llgwalk(head, level, NO)
#define indent(level) llindent(level, NO)
#define gindent(level) llindent(level, YES)

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
    LAND0, LAND1, LAND2, LA0, LAN0, LEXTERN0, LEXTERN1, LEXTERN2,
    LEXTERN3, LEXTERN4, LGIVEN0, LGIVEN1, LGIVEN2, LGIVEN3,
    LGIVEN4, LLOW0, LLOW1, LU0, LUNION0, LUNION1, LUNION2,
    LUNION3, LUNION4, LWALRUS0, LMODNS0,
} LexStates;

/* AST tag enum.
 * basically, this is all the AST types, and is
 * used both for determining the type of AST
#define gindent(level) llindent(level, YES)
 * that is represented within the tree, as well as
 * returned from the tokenizer to say what type
 * of object it thinks is in buffer
 */
typedef enum {
    TDEF, TBEGIN, TEND, TEQUAL, TCOREFORM, // 4
    TIDENT, TCALL, TOPAREN, TCPAREN, TMATCH, // 9
    TIF, TELSE, TTHEN, TTYPE, TPOLY, TVAL, // 15
    TARRAY, TRECORD, TINT, TFLOAT, TSTRING, // 20
    TCHAR, TBOOL, TEQ, TSEMI, TEOF, TPARAMLIST, // 26
    TTDECL, TWHEN, TNEWL, TDO, TUNIT, TERROR,  // 32
    TLETREC, TLET, TFN, TCASE, TSTRT, TCHART, // 38
    TINTT, TFLOATT, TCOMMENT, TREF, TDEQUET, // 42
    TBOOLT, TWITH, TOF, TDECLARE, TFALSE, // 47
    TTRUE, TUSE, TIN, TCOLON, TRECDEF, // 52
    TCOMPLEXTYPE, TCOMMA, TOARR, TCARR, // 56
    TARRAYLITERAL, TBIN, TOCT, THEX, // 60
    TARROW, TFATARROW, TCUT, TDOLLAR, // 64
    TPIPEARROW, TUSERT, TVAR, TTAG, // 68
    TPARAMDEF, TTYPEDEF, TWHILE, TFOR, // 72
    TTUPLET, TFUNCTIONT, TPROCEDURET, // 75
    TAND, TANY, TGUARD, TEXTERN, TGIVEN, // 80
    TLOW, TUNION, TWALRUS, TMODNS, // 84
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

// external defs, written in carML
extern int self_tco_p(const char *, AST *);
extern AST *shadow_ident(AST *);
extern AST *make_set_bang(AST *, AST *);
extern AST *shadow_params(AST *, AST *);
extern char *shadow_name(char *);
extern char *get_parameter_name(AST *, int);
extern AST *get_parameter_ident(AST *, int);
extern AST *get_parameter_type(AST *, int);
extern AST *define_shadow_params(AST *, AST *);
extern AST *rewrite_tco(AST *);

#endif
