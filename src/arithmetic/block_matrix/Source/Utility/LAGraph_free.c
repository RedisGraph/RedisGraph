#include <stdlib.h>

void LAGraph_free
(
    void *p
)
{
    if (p != NULL)
    {
        free (p) ;
    }
}
