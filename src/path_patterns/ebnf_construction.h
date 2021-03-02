#pragma once

#include "ebnf.h"
#include "path_pattern_ctx.h"
#include <astnode.h>

EBNFBase *EBNFBase_Build(const cypher_astnode_t *path_pattern, PathPatternCtx *path_pattern_ctx);