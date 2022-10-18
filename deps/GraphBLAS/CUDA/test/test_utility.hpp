
#pragma once

// try calling a GrB_method and check the result
#define GRB_TRY(GrB_method)                                     \
{                                                               \
    GrB_Info GB_info_result = GrB_method ;                      \
    if (GB_info_result < GrB_SUCCESS)                           \
    {                                                           \
        printf ("test failure: file %s line %d status %d\n",    \
            __FILE__, __LINE__, GB_info_result) ;               \
        exit (EXIT_FAILURE) ;                                   \
    }                                                           \
}

