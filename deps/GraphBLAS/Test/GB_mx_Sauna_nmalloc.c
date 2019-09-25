//------------------------------------------------------------------------------
// GB_mx_Sauna_nmalloc: get the stats on Sauna usage
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

int GB_mx_Sauna_nmalloc ( )   // return # of mallocs in Saunas in use
{
    
    int nmallocs_in_use = 0 ;

    for (int Sauna_id = 0 ; Sauna_id < GxB_NTHREADS_MAX ; Sauna_id++)
    {

        GB_Sauna Sauna = GB_Global_Saunas_get (Sauna_id) ;
        if (Sauna != NULL)
        { 
            nmallocs_in_use++ ;             // the Sauna header
            if (Sauna->Sauna_Mark != NULL)
            {
                nmallocs_in_use++ ;
            }
            if (Sauna->Sauna_Work != NULL)
            {
                nmallocs_in_use++ ;
            }
        }
    }

    return (nmallocs_in_use) ;
}

