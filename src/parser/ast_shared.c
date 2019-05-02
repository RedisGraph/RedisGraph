#include "ast_shared.h"

void PropertyMap_Free(PropertyMap *map) {
    if (map == NULL) return;

    // TODO always freed elsewhere?
    for (uint i = 0; i < map->property_count; i++) {
        // SIValue_Free(&map->values[i]);
    }
    free(map->keys);
    free(map->values);
    free(map);
}

