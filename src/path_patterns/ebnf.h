/* EBNF (Extended Backus-Naur Form) expression used to specify a more
 * convenient intermediate representation of path pattern. */

#pragma once
#include <cypher-parser.h>

/* Enum of types of ast node */
typedef enum {
    EBNF_SEQ,   // sequence of patterns "p1 p2", f.e. ":A :B"
    EBNF_ALT,   // alternative of patterns "p1 | p2", f.e ":A | :B"
    EBNF_GROUP, // direction and repetition parameters: <,>,+,*,?
    EBNF_EDGE,  // edge type pattern, f.e. ":A"
    EBNF_NODE,  // node type pattern, f.e. "(:X)"
    EBNF_REF    // named path pattern reference, f.e. "~S"
} EBNFBase_Type;

typedef enum {
    EBNF_ONE,           // default
    EBNF_AT_LEAST_ONE,  // +, don't supported
    EBNF_ZERO_OR_ONE,   // ?, don't supported
    EBNF_SPECIFIC       // *left..right, don't supported
} EBNFNode_Repetition;

typedef struct EBNFBase EBNFBase;
typedef EBNFBase*(*fpEBNFCopy)(EBNFBase *from);
typedef void(*fpEBNFFree)(EBNFBase *ebnf);

/* This structure represents node of path pattern EBNF.
 * These nodes form the expression tree specified by
 * the path pattern which is a more convenient intermediate
 * representation then cypher ast.
 * */
struct EBNFBase {
    EBNFBase_Type type;
    fpEBNFCopy copy;
    fpEBNFFree custom_free;
    EBNFBase** children;
};

typedef struct {
    EBNFBase base;
} EBNFAlternative;

typedef struct {
    EBNFBase base;
} EBNFSequence;

typedef struct {
    EBNFBase base;
    enum cypher_rel_direction direction;
    EBNFNode_Repetition repetition;
} EBNFGroup;

typedef struct {
    EBNFBase base;
    const char *reltype;
    int reltype_id;
} EBNFEdge;

typedef struct {
    EBNFBase base;
    const char *label;
} EBNFNode;

typedef struct {
    EBNFBase base;
    char *name;
} EBNFReference;

void EBNFBase_Init(EBNFBase *base, fpEBNFCopy fp, EBNFBase_Type type, fpEBNFFree free);
void EBNFBase_Free(EBNFBase *base);
void EBNFBase_AddChild(EBNFBase *root, EBNFBase *child);
EBNFBase *EBNFBase_Clone(EBNFBase *base);
char *EBNFBase_ToStr(EBNFBase *base);

EBNFBase *EBNFAlternative_New();
EBNFBase *EBNFSequence_New();
EBNFBase *EBNFGroup_New(enum cypher_rel_direction direction, EBNFNode_Repetition repetition);
EBNFBase *EBNFEdge_New(const char *label);
EBNFBase *EBNFNode_New(const char *label);
EBNFBase *EBNFReference_New(const char *name);

EBNFBase *EBNFAlternative_Copy(EBNFBase *base);
EBNFBase *EBNFSequence_Copy(EBNFBase *base);
EBNFBase *EBNFGroup_Copy(EBNFBase *base);
EBNFBase *EBNFEdge_Copy(EBNFBase *base);
EBNFBase *EBNFNode_Copy(EBNFBase *base);
EBNFBase *EBNFReference_Copy(EBNFBase *base);