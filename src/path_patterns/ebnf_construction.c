#include "ebnf_construction.h"
#include "path_pattern_ctx_construction.h"
#include "../util/rmalloc.h"

EBNFBase *_BuildEBNFBase(const cypher_astnode_t *node, PathPatternCtx *ctx) {
    if (cypher_astnode_instanceof(node, CYPHER_AST_PATH_PATTERN_EXPRESSION)) {
        EBNFBase *seq = EBNFSequence_New();
        unsigned int nelem = cypher_ast_path_pattern_expression_get_nelements(node);
        for (int i = 0; i < nelem; ++i) {
            const cypher_astnode_t *child_node = cypher_ast_path_pattern_expression_get_element(node, i);
            EBNFBase *child = _BuildEBNFBase(child_node, ctx);
            EBNFBase_AddChild(seq, child);
        }
        return seq;
    } else if (cypher_astnode_instanceof(node, CYPHER_AST_PATH_PATTERN_ALTERNATIVE)) {
        EBNFBase *alt = EBNFAlternative_New();
        unsigned int nelem = cypher_ast_path_pattern_alternative_get_nelements(node);
        for (int i = 0; i < nelem; ++i) {
            const cypher_astnode_t *child_node = cypher_ast_path_pattern_alternative_get_element(node, i);
            EBNFBase *child = _BuildEBNFBase(child_node, ctx);
            EBNFBase_AddChild(alt, child);
        }
        return alt;
    } else if (cypher_astnode_instanceof(node, CYPHER_AST_PATH_PATTERN_BASE)) {
		enum cypher_rel_direction dir = cypher_ast_path_pattern_base_get_direction(node);
		const cypher_astnode_t *range = cypher_ast_path_pattern_base_get_varlength(node);

		EBNFBase *group = EBNFGroup_New(dir, EBNF_ONE);
        EBNFBase *child = _BuildEBNFBase(cypher_ast_path_pattern_base_get_child(node), ctx);
		EBNFBase_AddChild(group, child);

		if(range) {
			const cypher_astnode_t *start = cypher_ast_range_get_start(range);
			const cypher_astnode_t *end = cypher_ast_range_get_end(range);
			assert(start == NULL && end == NULL);

			char *anon_name = PathPatternCtx_GetNextAnonName(ctx);

			EBNFBase *seq = EBNFSequence_New();
			EBNFBase_AddChild(seq, group);
			EBNFBase_AddChild(seq, EBNFReference_New(anon_name));

			EBNFBase *alt = EBNFAlternative_New();
			EBNFBase_AddChild(alt, seq);
			EBNFBase_AddChild(alt, EBNFNode_New(NULL));

			PathPatternCtx_BuildAndAddPathPattern(anon_name, alt, ctx);

			EBNFBase *ref = EBNFReference_New(anon_name);

			rm_free(anon_name);
			return ref;
		} else {
			return group;
		}
    } else if (cypher_astnode_instanceof(node, CYPHER_AST_PATH_PATTERN_EDGE)){
        const cypher_astnode_t *reltype = cypher_ast_path_pattern_edge_get_reltype(node);
        return EBNFEdge_New(cypher_ast_reltype_get_name(reltype));
    } else if (cypher_astnode_instanceof(node, CYPHER_AST_PATH_PATTERN_REFERENCE)) {
        const cypher_astnode_t *identifier = cypher_ast_path_pattern_reference_get_identifier(node);
        const char *name = cypher_ast_identifier_get_name(identifier);
        return EBNFReference_New(name);
    } else if (cypher_astnode_instanceof(node, CYPHER_AST_NODE_PATTERN)){
    	unsigned int nlabels = cypher_ast_node_pattern_nlabels(node);
    	assert((nlabels == 0 || nlabels == 1) && "PathNode must have 0 or 1 labels");

    	const char *label = NULL;
    	if (nlabels == 1) {
			const cypher_astnode_t *label_node = cypher_ast_node_pattern_get_label(node, 0);
			label = cypher_ast_label_get_name(label_node);
		}
    	return EBNFNode_New(label);
    } else {
		assert(false && "EBNF TRANSLATION NOT IMPLEMETED");
    }
}

EBNFBase *EBNFBase_Build(const cypher_astnode_t *path_pattern, PathPatternCtx *path_pattern_ctx) {
    REQUIRE_TYPE(path_pattern, CYPHER_AST_PATH_PATTERN, path_pattern_ctx);

    const cypher_astnode_t *expr = cypher_ast_path_pattern_get_expression(path_pattern);
    return _BuildEBNFBase(expr, path_pattern_ctx);
}
