#include "op_cartesian_product.h"

OpBase* NewCartesianProductOp() {
    return (OpBase*)NewCartesianProduct();
}

CartesianProduct* NewCartesianProduct() {
    CartesianProduct *cp = malloc(sizeof(CartesianProduct));
    cp->refresh = 1;

    // Set our Op operations
    cp->op.name = "Cartesian Product";
    cp->op.type = OPType_CARTESIAN_PRODUCT;
    cp->op.consume = CartesianProductConsume;
    cp->op.reset = CartesianProductReset;
    cp->op.free = CartesianProductFree;
    cp->op.modifies = NULL;

    return cp;
}

OpResult CartesianProductConsume(OpBase *opBase, QueryGraph* graph) {
    CartesianProduct *cp = (CartesianProduct*)opBase;

    if(cp->refresh) {
        cp->refresh = 0;
        return OP_REFRESH;
    }
    
    cp->refresh = 1;
    return OP_OK;
}

OpResult CartesianProductReset(OpBase *opBase) {
    return OP_OK;
}

void CartesianProductFree(OpBase *opBase) {
    CartesianProduct *cp = (CartesianProduct*)opBase;
    free(cp);
}
