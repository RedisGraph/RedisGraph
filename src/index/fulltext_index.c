#include "fulltext_index.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../graph/entities/node.h"

static int _getNodeAttribute(void* ctx, const char* fieldName, const void* id, char** strVal, double* doubleVal) {
    Node n;
    NodeID nId = *(NodeID*)id;
    GraphContext *gc = GraphContext_GetFromTLS();
    Graph *g = gc->g;
    
    assert(Graph_GetNode(g, nId, &n));
    Attribute_ID attrId = GraphContext_GetAttributeID(gc, fieldName);
    SIValue *v = GraphEntity_GetProperty((GraphEntity*)&n, attrId);
    int ret;
    if(v == PROPERTY_NOTFOUND) {
        ret = RSVALTYPE_NOTFOUND;
    } else if(v->type & T_STRING) {
        *strVal = v->stringval;
        ret = RSVALTYPE_STRING;
    } else if(v->type & SI_NUMERIC) {
        *doubleVal = SI_GET_NUMERIC(*v);
        ret = RSVALTYPE_DOUBLE;
    } else {
        // Skiping booleans.
        ret = RSVALTYPE_NOTFOUND;
    }
    return ret;
}

static void _populateIndex
(
    FullTextIndex *idx
)
{    
    GraphContext *gc = GraphContext_GetFromTLS();   
    Schema *s = GraphContext_GetSchema(gc, idx->label, SCHEMA_NODE);

    // Label doesn't exists.
    if(s == NULL) return;

    bool resolved_attribute = false;

    // Make sure at least one indexed attribute exists.
    for(uint i = 0; i < idx->fields_count; i++) {
        if(idx->fields_ids[i] != ATTRIBUTE_NOTFOUND) {
            resolved_attribute = true;
            break;
        }
    }

    // If all attributes are missing we can quickly return.
    if(!resolved_attribute) return;

    Node node;
    NodeID node_id;
    Graph *g = gc->g;
    int label_id = s->id;
    GxB_MatrixTupleIter *it;
    const GrB_Matrix label_matrix = Graph_GetLabelMatrix(g, label_id);
    GxB_MatrixTupleIter_new(&it, label_matrix);

    // Iterate over each labeled node.
    while(true) {
        bool depleted = false;
        GxB_MatrixTupleIter_next(it, NULL, &node_id, &depleted);
        if(depleted) break;

        Graph_GetNode(g, node_id, &node);
        FullTextIndex_IndexNode(idx, &node);
    }
    GxB_MatrixTupleIter_free(it);
}

// Create a new FullText index.
FullTextIndex* FullTextIndex_New
(
    const char *label   // Indexed label
)
{
    FullTextIndex *idx = rm_malloc(sizeof(FullTextIndex));
    idx->idx = NULL;
    idx->fields_count = 0;
    idx->label = rm_strdup(label);
    idx->fields = array_new(char*, 0);
    idx->fields_ids = array_new(Attribute_ID, 0);
    return idx;
}

// Adds field to index.
void FullTextIndex_AddField
(
    FullTextIndex *idx,
    const char *field
)
{
    assert(idx);
    if(FullTextIndex_ContainsField(idx, field)) return;

    idx->fields_count++;
    idx->fields = array_append(idx->fields, rm_strdup(field));

    /* It's OK for field to be missing from schema,
     * we'll try to resolve its ID later on. */
    GraphContext *gc = GraphContext_GetFromTLS();
    Attribute_ID fieldID = GraphContext_GetAttributeID(gc, field);
    idx->fields_ids = array_append(idx->fields_ids, fieldID);
}

// Removes fields from index.
void FullTextIndex_RemoveField
(
    FullTextIndex *idx,
    const char *field
)
{
    assert(idx);
    if(!FullTextIndex_ContainsField(idx, field)) return;

    for(uint i = 0; i < idx->fields_count; i++) {
        if(idx->fields[i] == field) {
            idx->fields_count--;
            rm_free(idx->fields[i]);
            array_del_fast(idx->fields, i);
            array_del_fast(idx->fields_ids, i);
            break;
        }
    }
}

void FullTextIndex_IndexNode
(
    FullTextIndex *idx,
    Node *n
)
{
    double score = 0;           // Default score.
    const char* lang = NULL;    // Default language.
    RSIndex *rsIdx = idx->idx;
    NodeID node_id = ENTITY_GET_ID(n);

    // Create a document out of node.
    RSDoc *doc = RediSearch_CreateDocument(&node_id, sizeof(EntityID), score, lang);

    // Add document field for each indexed property.
    for(uint i = 0; i < idx->fields_count; i++) {
        if(idx->fields_ids[i] == ATTRIBUTE_NOTFOUND) continue;

        SIValue *v = GraphEntity_GetProperty((GraphEntity*)n, idx->fields_ids[i]);

        // Value must be of type string.
        if(v == PROPERTY_NOTFOUND || v->type != T_STRING) continue;

        RediSearch_DocumentAddFieldString(doc,
                                          idx->fields[i],
                                          v->stringval,
                                          strlen(v->stringval),
                                          RSFLDTYPE_FULLTEXT);
    }

    RediSearch_SpecAddDocument(rsIdx, doc);
}

// Constructs index.
void FullTextIndex_Construct
(
    FullTextIndex *idx
)
{
    assert(idx);
    
    /* RediSearch index already exists
     * re-construct */
    if(idx->idx) RediSearch_DropIndex(idx->idx);

    RSIndex *rsIdx = NULL;
    RSIndexOptions *idx_options = RediSearch_CreateIndexOptions();
    RediSearch_IndexOptionsSetGetValueCallback(idx_options, _getNodeAttribute, NULL);
    rsIdx = RediSearch_CreateIndex(idx->label, idx_options);
    RediSearch_FreeIndexOptions(idx_options);

    // Create indexed fields, one per specified field.
    for(uint i = 0; i < idx->fields_count; i++) {
        // Introduce text fields.
        RediSearch_CreateTextField(rsIdx, idx->fields[i]);
    }

    idx->idx = rsIdx;

    _populateIndex(idx);
}

// Query index.
RSResultsIterator* FullTextIndex_Query
(
    const FullTextIndex *idx,
    const char *query,           // Query to execute
    char** err
)
{
    assert(idx && query);
    return RediSearch_IterateQuery(idx->idx, query, strlen(query), err);
}

// Return indexed label.
char* FullTextIndex_GetLabel
(
    const FullTextIndex *idx
)
{
    assert(idx);
    return rm_strdup(idx->label);
}

// Returns number of fields indexed.
uint FullTextIndex_FieldsCount
(
    const FullTextIndex *idx
)
{
    assert(idx);
    return idx->fields_count;
}

// Returns indexed fields.
void FullTextIndex_GetFields
(
    const FullTextIndex *idx,
    char **fields    // Array of size FullTextIndex_FieldsCount
)
{
    assert(idx && fields);
    for(uint i = 0; i < idx->fields_count; i++) {
        fields[i] = rm_strdup(idx->fields[i]);
    }
}

// Checks if given field is indexed.
bool FullTextIndex_ContainsField
(
    const FullTextIndex *idx,
    const char *field
)
{
    assert(idx && field);

    for(uint i = 0; i < idx->fields_count; i++) {
        if(strcmp(idx->fields[i], field) == 0)
        return true;
    }

    return false;
}

// Free fulltext index.
void FullTextIndex_Free
(
    FullTextIndex *idx
)
{
    assert(idx);
    if(idx->idx) RediSearch_DropIndex(idx->idx);
    
    rm_free(idx->label);
    
    for(uint i = 0; i < idx->fields_count; i++) {
        rm_free(idx->fields[i]);
    }

    array_free(idx->fields);
    array_free(idx->fields_ids);

    rm_free(idx);
}
