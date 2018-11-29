//------------------------------------------------------------------------------
// GB_mx_semiring: get a built-in semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

//------------------------------------------------------------------------------
// built-in semirings
//------------------------------------------------------------------------------

// Using built-in types and operators, 960 unique semirings can be built.  This
// count excludes redundant Boolean operators (for example GxB_TIMES_BOOL and
// GxB_LAND_BOOL are different operators but they are redundant since they
// always return the same result):

// 680 semirings with a multiply operator TxT -> T where T is non-Boolean, from
// the complete cross product of:

//      4 add monoids (MIN, MAX, PLUS, TIMES)
//      17 multiply operators:
//          (FIRST, SECOND, MIN, MAX, PLUS, MINUS, TIMES, DIV,
//           ISEQ, ISNE, ISGT, ISLT, ISGE, ISLE,
//           LOR, LAND, LXOR)
//      10 non-Boolean types, T

// 240 semirings with a comparison operator TxT -> bool, where T is
// non-Boolean, from the complete cross product of:

//      4 Boolean add monoids: (LAND, LOR, LXOR, EQ)
//      6 multiply operators: (EQ, NE, GT, LT, GE, LE)
//      10 non-Boolean types, T

// 40 semirings with purely Boolean types, bool x bool -> bool, from the
// complete cross product of:

//      4 Boolean add monoids (LAND, LOR, LXOR, EQ)
//      10 multiply operators:
//          (FIRST, SECOND, LOR, LAND, LXOR, EQ, GT, LT, GE, LE)

// In the names below, each semiring has a name of the form GxB_add_mult_T
// where add is the additive monoid, mult is the multiply operator, and T is
// the type.  The type T is always the type of x and y for the z=mult(x,y)
// operator.  The monoid's three types and the ztype of the mult operator are
// always the same.  This is the type T for the first set, and Boolean for
// the second and third sets of semirngs.

//------------------------------------------------------------------------------

GrB_Semiring GB_mx_builtin_semiring // built-in semiring, or NULL if error
(
    const GrB_Monoid add_monoid,    // input monoid
    const GrB_BinaryOp mult         // input multiply operator
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_BinaryOp add  = add_monoid->op ;        // add operator

    GB_Opcode add_opcode  = add->opcode ;       // add opcode
    GB_Opcode mult_opcode = mult->opcode ;      // multiply opcode

    // add is a monoid
    ASSERT (add->xtype == add->ztype && add->ytype == add->ztype) ;

    // in a semiring, the ztypes of add and mult are always the same:
    ASSERT (add->ztype == mult->ztype) ;

    // The conditions above are true for any semiring and any A and B, whether
    // or not this function handles the semiring as hard-coded.  Now return for
    // cases this function does not handle.

    if (add_opcode >= GB_USER_C_opcode || mult_opcode >= GB_USER_C_opcode)
    {
        // This function handles only built-in operators.
        return (NULL) ;
    }

    // this condition is true for all built-in operators, but not required for
    // user-defined operators
    ASSERT (mult->xtype == mult->ytype) ;

    //--------------------------------------------------------------------------
    // rename redundant Boolean multiply operators
    //--------------------------------------------------------------------------

    GB_Type_code xycode = mult->xtype->code ;
    GB_Type_code zcode  = mult->ztype->code ;

    ASSERT (xycode <= GB_UDT_code) ;
    ASSERT (zcode  <= GB_UDT_code) ;

    if (xycode == GB_BOOL_code)
    {
        // z = mult(x,y) where both x and y are Boolean.
        // DIV becomes FIRST
        // MIN and TIMES become LAND
        // MAX and PLUS become LOR
        // NE, ISNE, and MINUS become LXOR
        // ISEQ becomes EQ
        // ISGT becomes GT
        // ISLT becomes LT
        // ISGE becomes GE
        // ISLE becomes LE
        mult_opcode = GB_boolean_rename (mult_opcode) ;
    }

    if (zcode == GB_BOOL_code)
    {
        // Only the LAND, LOR, LXOR, and EQ monoids remain if z is
        // Boolean.  MIN, MAX, PLUS, and TIMES are renamed.
        add_opcode = GB_boolean_rename (add_opcode) ;
    }

    // built-in binary operators always have this property.
    ASSERT (zcode == GB_BOOL_code || zcode == xycode) ;

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    if (zcode != GB_BOOL_code)
    {

        //----------------------------------------------------------------------
        // 640 semirings with TxT->T multiply operators
        //----------------------------------------------------------------------

        // x,y,z are all the same non-Boolean type

        switch (mult_opcode)
        {

            case GB_FIRST_opcode : // with (4 monoids) x (10 non-Boolean types)

                switch (add_opcode)
                {

                    case GB_MIN_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MIN_FIRST_INT8    ) ;
                            case GB_UINT8_code : return (GxB_MIN_FIRST_UINT8   ) ;
                            case GB_INT16_code : return (GxB_MIN_FIRST_INT16   ) ;
                            case GB_UINT16_code: return (GxB_MIN_FIRST_UINT16  ) ;
                            case GB_INT32_code : return (GxB_MIN_FIRST_INT32   ) ;
                            case GB_UINT32_code: return (GxB_MIN_FIRST_UINT32  ) ;
                            case GB_INT64_code : return (GxB_MIN_FIRST_INT64   ) ;
                            case GB_UINT64_code: return (GxB_MIN_FIRST_UINT64  ) ;
                            case GB_FP32_code  : return (GxB_MIN_FIRST_FP32    ) ;
                            case GB_FP64_code  : return (GxB_MIN_FIRST_FP64    ) ;
                            default : ; 
                        }
                        break; 

                    case GB_MAX_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MAX_FIRST_INT8    ) ;
                            case GB_UINT8_code : return (GxB_MAX_FIRST_UINT8   ) ;
                            case GB_INT16_code : return (GxB_MAX_FIRST_INT16   ) ;
                            case GB_UINT16_code: return (GxB_MAX_FIRST_UINT16  ) ;
                            case GB_INT32_code : return (GxB_MAX_FIRST_INT32   ) ;
                            case GB_UINT32_code: return (GxB_MAX_FIRST_UINT32  ) ;
                            case GB_INT64_code : return (GxB_MAX_FIRST_INT64   ) ;
                            case GB_UINT64_code: return (GxB_MAX_FIRST_UINT64  ) ;
                            case GB_FP32_code  : return (GxB_MAX_FIRST_FP32    ) ;
                            case GB_FP64_code  : return (GxB_MAX_FIRST_FP64    ) ;
                            default : ; 
                        }
                        break; 

                    case GB_PLUS_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_PLUS_FIRST_INT8   ) ;
                            case GB_UINT8_code : return (GxB_PLUS_FIRST_UINT8  ) ;
                            case GB_INT16_code : return (GxB_PLUS_FIRST_INT16  ) ;
                            case GB_UINT16_code: return (GxB_PLUS_FIRST_UINT16 ) ;
                            case GB_INT32_code : return (GxB_PLUS_FIRST_INT32  ) ;
                            case GB_UINT32_code: return (GxB_PLUS_FIRST_UINT32 ) ;
                            case GB_INT64_code : return (GxB_PLUS_FIRST_INT64  ) ;
                            case GB_UINT64_code: return (GxB_PLUS_FIRST_UINT64 ) ;
                            case GB_FP32_code  : return (GxB_PLUS_FIRST_FP32   ) ;
                            case GB_FP64_code  : return (GxB_PLUS_FIRST_FP64   ) ;
                            default : ; 
                        }
                        break; 

                    case GB_TIMES_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_TIMES_FIRST_INT8  ) ;
                            case GB_UINT8_code : return (GxB_TIMES_FIRST_UINT8 ) ;
                            case GB_INT16_code : return (GxB_TIMES_FIRST_INT16 ) ;
                            case GB_UINT16_code: return (GxB_TIMES_FIRST_UINT16) ;
                            case GB_INT32_code : return (GxB_TIMES_FIRST_INT32 ) ;
                            case GB_UINT32_code: return (GxB_TIMES_FIRST_UINT32) ;
                            case GB_INT64_code : return (GxB_TIMES_FIRST_INT64 ) ;
                            case GB_UINT64_code: return (GxB_TIMES_FIRST_UINT64) ;
                            case GB_FP32_code  : return (GxB_TIMES_FIRST_FP32  ) ;
                            case GB_FP64_code  : return (GxB_TIMES_FIRST_FP64  ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_SECOND_opcode : // with (4 monoids) x (10 non-Boolean types)

                switch (add_opcode)
                {

                    case GB_MIN_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MIN_SECOND_INT8   ) ;
                            case GB_UINT8_code : return (GxB_MIN_SECOND_UINT8  ) ;
                            case GB_INT16_code : return (GxB_MIN_SECOND_INT16  ) ;
                            case GB_UINT16_code: return (GxB_MIN_SECOND_UINT16 ) ;
                            case GB_INT32_code : return (GxB_MIN_SECOND_INT32  ) ;
                            case GB_UINT32_code: return (GxB_MIN_SECOND_UINT32 ) ;
                            case GB_INT64_code : return (GxB_MIN_SECOND_INT64  ) ;
                            case GB_UINT64_code: return (GxB_MIN_SECOND_UINT64 ) ;
                            case GB_FP32_code  : return (GxB_MIN_SECOND_FP32   ) ;
                            case GB_FP64_code  : return (GxB_MIN_SECOND_FP64   ) ;
                            default : ; 
                        }
                        break; 

                    case GB_MAX_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MAX_SECOND_INT8   ) ;
                            case GB_UINT8_code : return (GxB_MAX_SECOND_UINT8  ) ;
                            case GB_INT16_code : return (GxB_MAX_SECOND_INT16  ) ;
                            case GB_UINT16_code: return (GxB_MAX_SECOND_UINT16 ) ;
                            case GB_INT32_code : return (GxB_MAX_SECOND_INT32  ) ;
                            case GB_UINT32_code: return (GxB_MAX_SECOND_UINT32 ) ;
                            case GB_INT64_code : return (GxB_MAX_SECOND_INT64  ) ;
                            case GB_UINT64_code: return (GxB_MAX_SECOND_UINT64 ) ;
                            case GB_FP32_code  : return (GxB_MAX_SECOND_FP32   ) ;
                            case GB_FP64_code  : return (GxB_MAX_SECOND_FP64   ) ;
                            default : ; 
                        }
                        break; 

                    case GB_PLUS_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_PLUS_SECOND_INT8  ) ;
                            case GB_UINT8_code : return (GxB_PLUS_SECOND_UINT8 ) ;
                            case GB_INT16_code : return (GxB_PLUS_SECOND_INT16 ) ;
                            case GB_UINT16_code: return (GxB_PLUS_SECOND_UINT16) ;
                            case GB_INT32_code : return (GxB_PLUS_SECOND_INT32 ) ;
                            case GB_UINT32_code: return (GxB_PLUS_SECOND_UINT32) ;
                            case GB_INT64_code : return (GxB_PLUS_SECOND_INT64 ) ;
                            case GB_UINT64_code: return (GxB_PLUS_SECOND_UINT64) ;
                            case GB_FP32_code  : return (GxB_PLUS_SECOND_FP32  ) ;
                            case GB_FP64_code  : return (GxB_PLUS_SECOND_FP64  ) ;
                            default : ; 
                        }
                        break; 

                    case GB_TIMES_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_TIMES_SECOND_INT8 ) ;
                            case GB_UINT8_code : return (GxB_TIMES_SECOND_UINT8) ;
                            case GB_INT16_code : return (GxB_TIMES_SECOND_INT16) ;
                            case GB_UINT16_code: return (GxB_TIMES_SECOND_UINT16) ;
                            case GB_INT32_code : return (GxB_TIMES_SECOND_INT32) ;
                            case GB_UINT32_code: return (GxB_TIMES_SECOND_UINT32) ;
                            case GB_INT64_code : return (GxB_TIMES_SECOND_INT64) ;
                            case GB_UINT64_code: return (GxB_TIMES_SECOND_UINT64) ;
                            case GB_FP32_code  : return (GxB_TIMES_SECOND_FP32 ) ;
                            case GB_FP64_code  : return (GxB_TIMES_SECOND_FP64 ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_MIN_opcode : // with (4 monoids) x (10 non-Boolean types)

                switch (add_opcode)
                {

                    case GB_MIN_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MIN_MIN_INT8      ) ;
                            case GB_UINT8_code : return (GxB_MIN_MIN_UINT8     ) ;
                            case GB_INT16_code : return (GxB_MIN_MIN_INT16     ) ;
                            case GB_UINT16_code: return (GxB_MIN_MIN_UINT16    ) ;
                            case GB_INT32_code : return (GxB_MIN_MIN_INT32     ) ;
                            case GB_UINT32_code: return (GxB_MIN_MIN_UINT32    ) ;
                            case GB_INT64_code : return (GxB_MIN_MIN_INT64     ) ;
                            case GB_UINT64_code: return (GxB_MIN_MIN_UINT64    ) ;
                            case GB_FP32_code  : return (GxB_MIN_MIN_FP32      ) ;
                            case GB_FP64_code  : return (GxB_MIN_MIN_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_MAX_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MAX_MIN_INT8      ) ;
                            case GB_UINT8_code : return (GxB_MAX_MIN_UINT8     ) ;
                            case GB_INT16_code : return (GxB_MAX_MIN_INT16     ) ;
                            case GB_UINT16_code: return (GxB_MAX_MIN_UINT16    ) ;
                            case GB_INT32_code : return (GxB_MAX_MIN_INT32     ) ;
                            case GB_UINT32_code: return (GxB_MAX_MIN_UINT32    ) ;
                            case GB_INT64_code : return (GxB_MAX_MIN_INT64     ) ;
                            case GB_UINT64_code: return (GxB_MAX_MIN_UINT64    ) ;
                            case GB_FP32_code  : return (GxB_MAX_MIN_FP32      ) ;
                            case GB_FP64_code  : return (GxB_MAX_MIN_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_PLUS_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_PLUS_MIN_INT8     ) ;
                            case GB_UINT8_code : return (GxB_PLUS_MIN_UINT8    ) ;
                            case GB_INT16_code : return (GxB_PLUS_MIN_INT16    ) ;
                            case GB_UINT16_code: return (GxB_PLUS_MIN_UINT16   ) ;
                            case GB_INT32_code : return (GxB_PLUS_MIN_INT32    ) ;
                            case GB_UINT32_code: return (GxB_PLUS_MIN_UINT32   ) ;
                            case GB_INT64_code : return (GxB_PLUS_MIN_INT64    ) ;
                            case GB_UINT64_code: return (GxB_PLUS_MIN_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_PLUS_MIN_FP32     ) ;
                            case GB_FP64_code  : return (GxB_PLUS_MIN_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_TIMES_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_TIMES_MIN_INT8    ) ;
                            case GB_UINT8_code : return (GxB_TIMES_MIN_UINT8   ) ;
                            case GB_INT16_code : return (GxB_TIMES_MIN_INT16   ) ;
                            case GB_UINT16_code: return (GxB_TIMES_MIN_UINT16  ) ;
                            case GB_INT32_code : return (GxB_TIMES_MIN_INT32   ) ;
                            case GB_UINT32_code: return (GxB_TIMES_MIN_UINT32  ) ;
                            case GB_INT64_code : return (GxB_TIMES_MIN_INT64   ) ;
                            case GB_UINT64_code: return (GxB_TIMES_MIN_UINT64  ) ;
                            case GB_FP32_code  : return (GxB_TIMES_MIN_FP32    ) ;
                            case GB_FP64_code  : return (GxB_TIMES_MIN_FP64    ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_MAX_opcode : // with (4 monoids) x (10 non-Boolean types)

                switch (add_opcode)
                {

                    case GB_MIN_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MIN_MAX_INT8      ) ;
                            case GB_UINT8_code : return (GxB_MIN_MAX_UINT8     ) ;
                            case GB_INT16_code : return (GxB_MIN_MAX_INT16     ) ;
                            case GB_UINT16_code: return (GxB_MIN_MAX_UINT16    ) ;
                            case GB_INT32_code : return (GxB_MIN_MAX_INT32     ) ;
                            case GB_UINT32_code: return (GxB_MIN_MAX_UINT32    ) ;
                            case GB_INT64_code : return (GxB_MIN_MAX_INT64     ) ;
                            case GB_UINT64_code: return (GxB_MIN_MAX_UINT64    ) ;
                            case GB_FP32_code  : return (GxB_MIN_MAX_FP32      ) ;
                            case GB_FP64_code  : return (GxB_MIN_MAX_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_MAX_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MAX_MAX_INT8      ) ;
                            case GB_UINT8_code : return (GxB_MAX_MAX_UINT8     ) ;
                            case GB_INT16_code : return (GxB_MAX_MAX_INT16     ) ;
                            case GB_UINT16_code: return (GxB_MAX_MAX_UINT16    ) ;
                            case GB_INT32_code : return (GxB_MAX_MAX_INT32     ) ;
                            case GB_UINT32_code: return (GxB_MAX_MAX_UINT32    ) ;
                            case GB_INT64_code : return (GxB_MAX_MAX_INT64     ) ;
                            case GB_UINT64_code: return (GxB_MAX_MAX_UINT64    ) ;
                            case GB_FP32_code  : return (GxB_MAX_MAX_FP32      ) ;
                            case GB_FP64_code  : return (GxB_MAX_MAX_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_PLUS_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_PLUS_MAX_INT8     ) ;
                            case GB_UINT8_code : return (GxB_PLUS_MAX_UINT8    ) ;
                            case GB_INT16_code : return (GxB_PLUS_MAX_INT16    ) ;
                            case GB_UINT16_code: return (GxB_PLUS_MAX_UINT16   ) ;
                            case GB_INT32_code : return (GxB_PLUS_MAX_INT32    ) ;
                            case GB_UINT32_code: return (GxB_PLUS_MAX_UINT32   ) ;
                            case GB_INT64_code : return (GxB_PLUS_MAX_INT64    ) ;
                            case GB_UINT64_code: return (GxB_PLUS_MAX_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_PLUS_MAX_FP32     ) ;
                            case GB_FP64_code  : return (GxB_PLUS_MAX_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_TIMES_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_TIMES_MAX_INT8    ) ;
                            case GB_UINT8_code : return (GxB_TIMES_MAX_UINT8   ) ;
                            case GB_INT16_code : return (GxB_TIMES_MAX_INT16   ) ;
                            case GB_UINT16_code: return (GxB_TIMES_MAX_UINT16  ) ;
                            case GB_INT32_code : return (GxB_TIMES_MAX_INT32   ) ;
                            case GB_UINT32_code: return (GxB_TIMES_MAX_UINT32  ) ;
                            case GB_INT64_code : return (GxB_TIMES_MAX_INT64   ) ;
                            case GB_UINT64_code: return (GxB_TIMES_MAX_UINT64  ) ;
                            case GB_FP32_code  : return (GxB_TIMES_MAX_FP32    ) ;
                            case GB_FP64_code  : return (GxB_TIMES_MAX_FP64    ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_PLUS_opcode : // with (4 monoids) x (10 non-Boolean types)

                switch (add_opcode)
                {

                    case GB_MIN_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MIN_PLUS_INT8     ) ;
                            case GB_UINT8_code : return (GxB_MIN_PLUS_UINT8    ) ;
                            case GB_INT16_code : return (GxB_MIN_PLUS_INT16    ) ;
                            case GB_UINT16_code: return (GxB_MIN_PLUS_UINT16   ) ;
                            case GB_INT32_code : return (GxB_MIN_PLUS_INT32    ) ;
                            case GB_UINT32_code: return (GxB_MIN_PLUS_UINT32   ) ;
                            case GB_INT64_code : return (GxB_MIN_PLUS_INT64    ) ;
                            case GB_UINT64_code: return (GxB_MIN_PLUS_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_MIN_PLUS_FP32     ) ;
                            case GB_FP64_code  : return (GxB_MIN_PLUS_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_MAX_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MAX_PLUS_INT8     ) ;
                            case GB_UINT8_code : return (GxB_MAX_PLUS_UINT8    ) ;
                            case GB_INT16_code : return (GxB_MAX_PLUS_INT16    ) ;
                            case GB_UINT16_code: return (GxB_MAX_PLUS_UINT16   ) ;
                            case GB_INT32_code : return (GxB_MAX_PLUS_INT32    ) ;
                            case GB_UINT32_code: return (GxB_MAX_PLUS_UINT32   ) ;
                            case GB_INT64_code : return (GxB_MAX_PLUS_INT64    ) ;
                            case GB_UINT64_code: return (GxB_MAX_PLUS_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_MAX_PLUS_FP32     ) ;
                            case GB_FP64_code  : return (GxB_MAX_PLUS_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_PLUS_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_PLUS_PLUS_INT8    ) ;
                            case GB_UINT8_code : return (GxB_PLUS_PLUS_UINT8   ) ;
                            case GB_INT16_code : return (GxB_PLUS_PLUS_INT16   ) ;
                            case GB_UINT16_code: return (GxB_PLUS_PLUS_UINT16  ) ;
                            case GB_INT32_code : return (GxB_PLUS_PLUS_INT32   ) ;
                            case GB_UINT32_code: return (GxB_PLUS_PLUS_UINT32  ) ;
                            case GB_INT64_code : return (GxB_PLUS_PLUS_INT64   ) ;
                            case GB_UINT64_code: return (GxB_PLUS_PLUS_UINT64  ) ;
                            case GB_FP32_code  : return (GxB_PLUS_PLUS_FP32    ) ;
                            case GB_FP64_code  : return (GxB_PLUS_PLUS_FP64    ) ;
                            default : ; 
                        }
                        break; 

                    case GB_TIMES_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_TIMES_PLUS_INT8   ) ;
                            case GB_UINT8_code : return (GxB_TIMES_PLUS_UINT8  ) ;
                            case GB_INT16_code : return (GxB_TIMES_PLUS_INT16  ) ;
                            case GB_UINT16_code: return (GxB_TIMES_PLUS_UINT16 ) ;
                            case GB_INT32_code : return (GxB_TIMES_PLUS_INT32  ) ;
                            case GB_UINT32_code: return (GxB_TIMES_PLUS_UINT32 ) ;
                            case GB_INT64_code : return (GxB_TIMES_PLUS_INT64  ) ;
                            case GB_UINT64_code: return (GxB_TIMES_PLUS_UINT64 ) ;
                            case GB_FP32_code  : return (GxB_TIMES_PLUS_FP32   ) ;
                            case GB_FP64_code  : return (GxB_TIMES_PLUS_FP64   ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_MINUS_opcode : // with (4 monoids) x (10 non-Boolean types)

                switch (add_opcode)
                {

                    case GB_MIN_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MIN_MINUS_INT8    ) ;
                            case GB_UINT8_code : return (GxB_MIN_MINUS_UINT8   ) ;
                            case GB_INT16_code : return (GxB_MIN_MINUS_INT16   ) ;
                            case GB_UINT16_code: return (GxB_MIN_MINUS_UINT16  ) ;
                            case GB_INT32_code : return (GxB_MIN_MINUS_INT32   ) ;
                            case GB_UINT32_code: return (GxB_MIN_MINUS_UINT32  ) ;
                            case GB_INT64_code : return (GxB_MIN_MINUS_INT64   ) ;
                            case GB_UINT64_code: return (GxB_MIN_MINUS_UINT64  ) ;
                            case GB_FP32_code  : return (GxB_MIN_MINUS_FP32    ) ;
                            case GB_FP64_code  : return (GxB_MIN_MINUS_FP64    ) ;
                            default : ; 
                        }
                        break; 

                    case GB_MAX_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MAX_MINUS_INT8    ) ;
                            case GB_UINT8_code : return (GxB_MAX_MINUS_UINT8   ) ;
                            case GB_INT16_code : return (GxB_MAX_MINUS_INT16   ) ;
                            case GB_UINT16_code: return (GxB_MAX_MINUS_UINT16  ) ;
                            case GB_INT32_code : return (GxB_MAX_MINUS_INT32   ) ;
                            case GB_UINT32_code: return (GxB_MAX_MINUS_UINT32  ) ;
                            case GB_INT64_code : return (GxB_MAX_MINUS_INT64   ) ;
                            case GB_UINT64_code: return (GxB_MAX_MINUS_UINT64  ) ;
                            case GB_FP32_code  : return (GxB_MAX_MINUS_FP32    ) ;
                            case GB_FP64_code  : return (GxB_MAX_MINUS_FP64    ) ;
                            default : ; 
                        }
                        break; 

                    case GB_PLUS_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_PLUS_MINUS_INT8   ) ;
                            case GB_UINT8_code : return (GxB_PLUS_MINUS_UINT8  ) ;
                            case GB_INT16_code : return (GxB_PLUS_MINUS_INT16  ) ;
                            case GB_UINT16_code: return (GxB_PLUS_MINUS_UINT16 ) ;
                            case GB_INT32_code : return (GxB_PLUS_MINUS_INT32  ) ;
                            case GB_UINT32_code: return (GxB_PLUS_MINUS_UINT32 ) ;
                            case GB_INT64_code : return (GxB_PLUS_MINUS_INT64  ) ;
                            case GB_UINT64_code: return (GxB_PLUS_MINUS_UINT64 ) ;
                            case GB_FP32_code  : return (GxB_PLUS_MINUS_FP32   ) ;
                            case GB_FP64_code  : return (GxB_PLUS_MINUS_FP64   ) ;
                            default : ; 
                        }
                        break; 

                    case GB_TIMES_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_TIMES_MINUS_INT8  ) ;
                            case GB_UINT8_code : return (GxB_TIMES_MINUS_UINT8 ) ;
                            case GB_INT16_code : return (GxB_TIMES_MINUS_INT16 ) ;
                            case GB_UINT16_code: return (GxB_TIMES_MINUS_UINT16) ;
                            case GB_INT32_code : return (GxB_TIMES_MINUS_INT32 ) ;
                            case GB_UINT32_code: return (GxB_TIMES_MINUS_UINT32) ;
                            case GB_INT64_code : return (GxB_TIMES_MINUS_INT64 ) ;
                            case GB_UINT64_code: return (GxB_TIMES_MINUS_UINT64) ;
                            case GB_FP32_code  : return (GxB_TIMES_MINUS_FP32  ) ;
                            case GB_FP64_code  : return (GxB_TIMES_MINUS_FP64  ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_TIMES_opcode : // with (4 monoids) x (10 non-Boolean types)

                switch (add_opcode)
                {

                    case GB_MIN_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MIN_TIMES_INT8    ) ;
                            case GB_UINT8_code : return (GxB_MIN_TIMES_UINT8   ) ;
                            case GB_INT16_code : return (GxB_MIN_TIMES_INT16   ) ;
                            case GB_UINT16_code: return (GxB_MIN_TIMES_UINT16  ) ;
                            case GB_INT32_code : return (GxB_MIN_TIMES_INT32   ) ;
                            case GB_UINT32_code: return (GxB_MIN_TIMES_UINT32  ) ;
                            case GB_INT64_code : return (GxB_MIN_TIMES_INT64   ) ;
                            case GB_UINT64_code: return (GxB_MIN_TIMES_UINT64  ) ;
                            case GB_FP32_code  : return (GxB_MIN_TIMES_FP32    ) ;
                            case GB_FP64_code  : return (GxB_MIN_TIMES_FP64    ) ;
                            default : ; 
                        }
                        break; 

                    case GB_MAX_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MAX_TIMES_INT8    ) ;
                            case GB_UINT8_code : return (GxB_MAX_TIMES_UINT8   ) ;
                            case GB_INT16_code : return (GxB_MAX_TIMES_INT16   ) ;
                            case GB_UINT16_code: return (GxB_MAX_TIMES_UINT16  ) ;
                            case GB_INT32_code : return (GxB_MAX_TIMES_INT32   ) ;
                            case GB_UINT32_code: return (GxB_MAX_TIMES_UINT32  ) ;
                            case GB_INT64_code : return (GxB_MAX_TIMES_INT64   ) ;
                            case GB_UINT64_code: return (GxB_MAX_TIMES_UINT64  ) ;
                            case GB_FP32_code  : return (GxB_MAX_TIMES_FP32    ) ;
                            case GB_FP64_code  : return (GxB_MAX_TIMES_FP64    ) ;
                            default : ; 
                        }
                        break; 

                    case GB_PLUS_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_PLUS_TIMES_INT8   ) ;
                            case GB_UINT8_code : return (GxB_PLUS_TIMES_UINT8  ) ;
                            case GB_INT16_code : return (GxB_PLUS_TIMES_INT16  ) ;
                            case GB_UINT16_code: return (GxB_PLUS_TIMES_UINT16 ) ;
                            case GB_INT32_code : return (GxB_PLUS_TIMES_INT32  ) ;
                            case GB_UINT32_code: return (GxB_PLUS_TIMES_UINT32 ) ;
                            case GB_INT64_code : return (GxB_PLUS_TIMES_INT64  ) ;
                            case GB_UINT64_code: return (GxB_PLUS_TIMES_UINT64 ) ;
                            case GB_FP32_code  : return (GxB_PLUS_TIMES_FP32   ) ;
                            case GB_FP64_code  : return (GxB_PLUS_TIMES_FP64   ) ;
                            default : ; 
                        }
                        break; 

                    case GB_TIMES_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_TIMES_TIMES_INT8  ) ;
                            case GB_UINT8_code : return (GxB_TIMES_TIMES_UINT8 ) ;
                            case GB_INT16_code : return (GxB_TIMES_TIMES_INT16 ) ;
                            case GB_UINT16_code: return (GxB_TIMES_TIMES_UINT16) ;
                            case GB_INT32_code : return (GxB_TIMES_TIMES_INT32 ) ;
                            case GB_UINT32_code: return (GxB_TIMES_TIMES_UINT32) ;
                            case GB_INT64_code : return (GxB_TIMES_TIMES_INT64 ) ;
                            case GB_UINT64_code: return (GxB_TIMES_TIMES_UINT64) ;
                            case GB_FP32_code  : return (GxB_TIMES_TIMES_FP32  ) ;
                            case GB_FP64_code  : return (GxB_TIMES_TIMES_FP64  ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_DIV_opcode : // with (4 monoids) x (10 non-Boolean types)

                switch (add_opcode)
                {

                    case GB_MIN_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MIN_DIV_INT8      ) ;
                            case GB_UINT8_code : return (GxB_MIN_DIV_UINT8     ) ;
                            case GB_INT16_code : return (GxB_MIN_DIV_INT16     ) ;
                            case GB_UINT16_code: return (GxB_MIN_DIV_UINT16    ) ;
                            case GB_INT32_code : return (GxB_MIN_DIV_INT32     ) ;
                            case GB_UINT32_code: return (GxB_MIN_DIV_UINT32    ) ;
                            case GB_INT64_code : return (GxB_MIN_DIV_INT64     ) ;
                            case GB_UINT64_code: return (GxB_MIN_DIV_UINT64    ) ;
                            case GB_FP32_code  : return (GxB_MIN_DIV_FP32      ) ;
                            case GB_FP64_code  : return (GxB_MIN_DIV_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_MAX_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MAX_DIV_INT8      ) ;
                            case GB_UINT8_code : return (GxB_MAX_DIV_UINT8     ) ;
                            case GB_INT16_code : return (GxB_MAX_DIV_INT16     ) ;
                            case GB_UINT16_code: return (GxB_MAX_DIV_UINT16    ) ;
                            case GB_INT32_code : return (GxB_MAX_DIV_INT32     ) ;
                            case GB_UINT32_code: return (GxB_MAX_DIV_UINT32    ) ;
                            case GB_INT64_code : return (GxB_MAX_DIV_INT64     ) ;
                            case GB_UINT64_code: return (GxB_MAX_DIV_UINT64    ) ;
                            case GB_FP32_code  : return (GxB_MAX_DIV_FP32      ) ;
                            case GB_FP64_code  : return (GxB_MAX_DIV_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_PLUS_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_PLUS_DIV_INT8     ) ;
                            case GB_UINT8_code : return (GxB_PLUS_DIV_UINT8    ) ;
                            case GB_INT16_code : return (GxB_PLUS_DIV_INT16    ) ;
                            case GB_UINT16_code: return (GxB_PLUS_DIV_UINT16   ) ;
                            case GB_INT32_code : return (GxB_PLUS_DIV_INT32    ) ;
                            case GB_UINT32_code: return (GxB_PLUS_DIV_UINT32   ) ;
                            case GB_INT64_code : return (GxB_PLUS_DIV_INT64    ) ;
                            case GB_UINT64_code: return (GxB_PLUS_DIV_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_PLUS_DIV_FP32     ) ;
                            case GB_FP64_code  : return (GxB_PLUS_DIV_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_TIMES_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_TIMES_DIV_INT8    ) ;
                            case GB_UINT8_code : return (GxB_TIMES_DIV_UINT8   ) ;
                            case GB_INT16_code : return (GxB_TIMES_DIV_INT16   ) ;
                            case GB_UINT16_code: return (GxB_TIMES_DIV_UINT16  ) ;
                            case GB_INT32_code : return (GxB_TIMES_DIV_INT32   ) ;
                            case GB_UINT32_code: return (GxB_TIMES_DIV_UINT32  ) ;
                            case GB_INT64_code : return (GxB_TIMES_DIV_INT64   ) ;
                            case GB_UINT64_code: return (GxB_TIMES_DIV_UINT64  ) ;
                            case GB_FP32_code  : return (GxB_TIMES_DIV_FP32    ) ;
                            case GB_FP64_code  : return (GxB_TIMES_DIV_FP64    ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_ISEQ_opcode : // with (4 monoids) x (10 non-Boolean types)

                switch (add_opcode)
                {

                    case GB_MIN_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MIN_ISEQ_INT8     ) ;
                            case GB_UINT8_code : return (GxB_MIN_ISEQ_UINT8    ) ;
                            case GB_INT16_code : return (GxB_MIN_ISEQ_INT16    ) ;
                            case GB_UINT16_code: return (GxB_MIN_ISEQ_UINT16   ) ;
                            case GB_INT32_code : return (GxB_MIN_ISEQ_INT32    ) ;
                            case GB_UINT32_code: return (GxB_MIN_ISEQ_UINT32   ) ;
                            case GB_INT64_code : return (GxB_MIN_ISEQ_INT64    ) ;
                            case GB_UINT64_code: return (GxB_MIN_ISEQ_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_MIN_ISEQ_FP32     ) ;
                            case GB_FP64_code  : return (GxB_MIN_ISEQ_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_MAX_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MAX_ISEQ_INT8     ) ;
                            case GB_UINT8_code : return (GxB_MAX_ISEQ_UINT8    ) ;
                            case GB_INT16_code : return (GxB_MAX_ISEQ_INT16    ) ;
                            case GB_UINT16_code: return (GxB_MAX_ISEQ_UINT16   ) ;
                            case GB_INT32_code : return (GxB_MAX_ISEQ_INT32    ) ;
                            case GB_UINT32_code: return (GxB_MAX_ISEQ_UINT32   ) ;
                            case GB_INT64_code : return (GxB_MAX_ISEQ_INT64    ) ;
                            case GB_UINT64_code: return (GxB_MAX_ISEQ_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_MAX_ISEQ_FP32     ) ;
                            case GB_FP64_code  : return (GxB_MAX_ISEQ_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_PLUS_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_PLUS_ISEQ_INT8    ) ;
                            case GB_UINT8_code : return (GxB_PLUS_ISEQ_UINT8   ) ;
                            case GB_INT16_code : return (GxB_PLUS_ISEQ_INT16   ) ;
                            case GB_UINT16_code: return (GxB_PLUS_ISEQ_UINT16  ) ;
                            case GB_INT32_code : return (GxB_PLUS_ISEQ_INT32   ) ;
                            case GB_UINT32_code: return (GxB_PLUS_ISEQ_UINT32  ) ;
                            case GB_INT64_code : return (GxB_PLUS_ISEQ_INT64   ) ;
                            case GB_UINT64_code: return (GxB_PLUS_ISEQ_UINT64  ) ;
                            case GB_FP32_code  : return (GxB_PLUS_ISEQ_FP32    ) ;
                            case GB_FP64_code  : return (GxB_PLUS_ISEQ_FP64    ) ;
                            default : ; 
                        }
                        break; 

                    case GB_TIMES_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_TIMES_ISEQ_INT8   ) ;
                            case GB_UINT8_code : return (GxB_TIMES_ISEQ_UINT8  ) ;
                            case GB_INT16_code : return (GxB_TIMES_ISEQ_INT16  ) ;
                            case GB_UINT16_code: return (GxB_TIMES_ISEQ_UINT16 ) ;
                            case GB_INT32_code : return (GxB_TIMES_ISEQ_INT32  ) ;
                            case GB_UINT32_code: return (GxB_TIMES_ISEQ_UINT32 ) ;
                            case GB_INT64_code : return (GxB_TIMES_ISEQ_INT64  ) ;
                            case GB_UINT64_code: return (GxB_TIMES_ISEQ_UINT64 ) ;
                            case GB_FP32_code  : return (GxB_TIMES_ISEQ_FP32   ) ;
                            case GB_FP64_code  : return (GxB_TIMES_ISEQ_FP64   ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_ISNE_opcode : // with (4 monoids) x (10 non-Boolean types)

                switch (add_opcode)
                {

                    case GB_MIN_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MIN_ISNE_INT8     ) ;
                            case GB_UINT8_code : return (GxB_MIN_ISNE_UINT8    ) ;
                            case GB_INT16_code : return (GxB_MIN_ISNE_INT16    ) ;
                            case GB_UINT16_code: return (GxB_MIN_ISNE_UINT16   ) ;
                            case GB_INT32_code : return (GxB_MIN_ISNE_INT32    ) ;
                            case GB_UINT32_code: return (GxB_MIN_ISNE_UINT32   ) ;
                            case GB_INT64_code : return (GxB_MIN_ISNE_INT64    ) ;
                            case GB_UINT64_code: return (GxB_MIN_ISNE_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_MIN_ISNE_FP32     ) ;
                            case GB_FP64_code  : return (GxB_MIN_ISNE_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_MAX_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MAX_ISNE_INT8     ) ;
                            case GB_UINT8_code : return (GxB_MAX_ISNE_UINT8    ) ;
                            case GB_INT16_code : return (GxB_MAX_ISNE_INT16    ) ;
                            case GB_UINT16_code: return (GxB_MAX_ISNE_UINT16   ) ;
                            case GB_INT32_code : return (GxB_MAX_ISNE_INT32    ) ;
                            case GB_UINT32_code: return (GxB_MAX_ISNE_UINT32   ) ;
                            case GB_INT64_code : return (GxB_MAX_ISNE_INT64    ) ;
                            case GB_UINT64_code: return (GxB_MAX_ISNE_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_MAX_ISNE_FP32     ) ;
                            case GB_FP64_code  : return (GxB_MAX_ISNE_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_PLUS_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_PLUS_ISNE_INT8    ) ;
                            case GB_UINT8_code : return (GxB_PLUS_ISNE_UINT8   ) ;
                            case GB_INT16_code : return (GxB_PLUS_ISNE_INT16   ) ;
                            case GB_UINT16_code: return (GxB_PLUS_ISNE_UINT16  ) ;
                            case GB_INT32_code : return (GxB_PLUS_ISNE_INT32   ) ;
                            case GB_UINT32_code: return (GxB_PLUS_ISNE_UINT32  ) ;
                            case GB_INT64_code : return (GxB_PLUS_ISNE_INT64   ) ;
                            case GB_UINT64_code: return (GxB_PLUS_ISNE_UINT64  ) ;
                            case GB_FP32_code  : return (GxB_PLUS_ISNE_FP32    ) ;
                            case GB_FP64_code  : return (GxB_PLUS_ISNE_FP64    ) ;
                            default : ; 
                        }
                        break; 

                    case GB_TIMES_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_TIMES_ISNE_INT8   ) ;
                            case GB_UINT8_code : return (GxB_TIMES_ISNE_UINT8  ) ;
                            case GB_INT16_code : return (GxB_TIMES_ISNE_INT16  ) ;
                            case GB_UINT16_code: return (GxB_TIMES_ISNE_UINT16 ) ;
                            case GB_INT32_code : return (GxB_TIMES_ISNE_INT32  ) ;
                            case GB_UINT32_code: return (GxB_TIMES_ISNE_UINT32 ) ;
                            case GB_INT64_code : return (GxB_TIMES_ISNE_INT64  ) ;
                            case GB_UINT64_code: return (GxB_TIMES_ISNE_UINT64 ) ;
                            case GB_FP32_code  : return (GxB_TIMES_ISNE_FP32   ) ;
                            case GB_FP64_code  : return (GxB_TIMES_ISNE_FP64   ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_ISGT_opcode : // with (4 monoids) x (10 non-Boolean types)

                switch (add_opcode)
                {

                    case GB_MIN_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MIN_ISGT_INT8     ) ;
                            case GB_UINT8_code : return (GxB_MIN_ISGT_UINT8    ) ;
                            case GB_INT16_code : return (GxB_MIN_ISGT_INT16    ) ;
                            case GB_UINT16_code: return (GxB_MIN_ISGT_UINT16   ) ;
                            case GB_INT32_code : return (GxB_MIN_ISGT_INT32    ) ;
                            case GB_UINT32_code: return (GxB_MIN_ISGT_UINT32   ) ;
                            case GB_INT64_code : return (GxB_MIN_ISGT_INT64    ) ;
                            case GB_UINT64_code: return (GxB_MIN_ISGT_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_MIN_ISGT_FP32     ) ;
                            case GB_FP64_code  : return (GxB_MIN_ISGT_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_MAX_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MAX_ISGT_INT8     ) ;
                            case GB_UINT8_code : return (GxB_MAX_ISGT_UINT8    ) ;
                            case GB_INT16_code : return (GxB_MAX_ISGT_INT16    ) ;
                            case GB_UINT16_code: return (GxB_MAX_ISGT_UINT16   ) ;
                            case GB_INT32_code : return (GxB_MAX_ISGT_INT32    ) ;
                            case GB_UINT32_code: return (GxB_MAX_ISGT_UINT32   ) ;
                            case GB_INT64_code : return (GxB_MAX_ISGT_INT64    ) ;
                            case GB_UINT64_code: return (GxB_MAX_ISGT_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_MAX_ISGT_FP32     ) ;
                            case GB_FP64_code  : return (GxB_MAX_ISGT_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_PLUS_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_PLUS_ISGT_INT8    ) ;
                            case GB_UINT8_code : return (GxB_PLUS_ISGT_UINT8   ) ;
                            case GB_INT16_code : return (GxB_PLUS_ISGT_INT16   ) ;
                            case GB_UINT16_code: return (GxB_PLUS_ISGT_UINT16  ) ;
                            case GB_INT32_code : return (GxB_PLUS_ISGT_INT32   ) ;
                            case GB_UINT32_code: return (GxB_PLUS_ISGT_UINT32  ) ;
                            case GB_INT64_code : return (GxB_PLUS_ISGT_INT64   ) ;
                            case GB_UINT64_code: return (GxB_PLUS_ISGT_UINT64  ) ;
                            case GB_FP32_code  : return (GxB_PLUS_ISGT_FP32    ) ;
                            case GB_FP64_code  : return (GxB_PLUS_ISGT_FP64    ) ;
                            default : ; 
                        }
                        break; 

                    case GB_TIMES_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_TIMES_ISGT_INT8   ) ;
                            case GB_UINT8_code : return (GxB_TIMES_ISGT_UINT8  ) ;
                            case GB_INT16_code : return (GxB_TIMES_ISGT_INT16  ) ;
                            case GB_UINT16_code: return (GxB_TIMES_ISGT_UINT16 ) ;
                            case GB_INT32_code : return (GxB_TIMES_ISGT_INT32  ) ;
                            case GB_UINT32_code: return (GxB_TIMES_ISGT_UINT32 ) ;
                            case GB_INT64_code : return (GxB_TIMES_ISGT_INT64  ) ;
                            case GB_UINT64_code: return (GxB_TIMES_ISGT_UINT64 ) ;
                            case GB_FP32_code  : return (GxB_TIMES_ISGT_FP32   ) ;
                            case GB_FP64_code  : return (GxB_TIMES_ISGT_FP64   ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_ISLT_opcode : // with (4 monoids) x (10 non-Boolean types)

                switch (add_opcode)
                {

                    case GB_MIN_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MIN_ISLT_INT8     ) ;
                            case GB_UINT8_code : return (GxB_MIN_ISLT_UINT8    ) ;
                            case GB_INT16_code : return (GxB_MIN_ISLT_INT16    ) ;
                            case GB_UINT16_code: return (GxB_MIN_ISLT_UINT16   ) ;
                            case GB_INT32_code : return (GxB_MIN_ISLT_INT32    ) ;
                            case GB_UINT32_code: return (GxB_MIN_ISLT_UINT32   ) ;
                            case GB_INT64_code : return (GxB_MIN_ISLT_INT64    ) ;
                            case GB_UINT64_code: return (GxB_MIN_ISLT_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_MIN_ISLT_FP32     ) ;
                            case GB_FP64_code  : return (GxB_MIN_ISLT_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_MAX_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MAX_ISLT_INT8     ) ;
                            case GB_UINT8_code : return (GxB_MAX_ISLT_UINT8    ) ;
                            case GB_INT16_code : return (GxB_MAX_ISLT_INT16    ) ;
                            case GB_UINT16_code: return (GxB_MAX_ISLT_UINT16   ) ;
                            case GB_INT32_code : return (GxB_MAX_ISLT_INT32    ) ;
                            case GB_UINT32_code: return (GxB_MAX_ISLT_UINT32   ) ;
                            case GB_INT64_code : return (GxB_MAX_ISLT_INT64    ) ;
                            case GB_UINT64_code: return (GxB_MAX_ISLT_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_MAX_ISLT_FP32     ) ;
                            case GB_FP64_code  : return (GxB_MAX_ISLT_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_PLUS_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_PLUS_ISLT_INT8    ) ;
                            case GB_UINT8_code : return (GxB_PLUS_ISLT_UINT8   ) ;
                            case GB_INT16_code : return (GxB_PLUS_ISLT_INT16   ) ;
                            case GB_UINT16_code: return (GxB_PLUS_ISLT_UINT16  ) ;
                            case GB_INT32_code : return (GxB_PLUS_ISLT_INT32   ) ;
                            case GB_UINT32_code: return (GxB_PLUS_ISLT_UINT32  ) ;
                            case GB_INT64_code : return (GxB_PLUS_ISLT_INT64   ) ;
                            case GB_UINT64_code: return (GxB_PLUS_ISLT_UINT64  ) ;
                            case GB_FP32_code  : return (GxB_PLUS_ISLT_FP32    ) ;
                            case GB_FP64_code  : return (GxB_PLUS_ISLT_FP64    ) ;
                            default : ; 
                        }
                        break; 

                    case GB_TIMES_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_TIMES_ISLT_INT8   ) ;
                            case GB_UINT8_code : return (GxB_TIMES_ISLT_UINT8  ) ;
                            case GB_INT16_code : return (GxB_TIMES_ISLT_INT16  ) ;
                            case GB_UINT16_code: return (GxB_TIMES_ISLT_UINT16 ) ;
                            case GB_INT32_code : return (GxB_TIMES_ISLT_INT32  ) ;
                            case GB_UINT32_code: return (GxB_TIMES_ISLT_UINT32 ) ;
                            case GB_INT64_code : return (GxB_TIMES_ISLT_INT64  ) ;
                            case GB_UINT64_code: return (GxB_TIMES_ISLT_UINT64 ) ;
                            case GB_FP32_code  : return (GxB_TIMES_ISLT_FP32   ) ;
                            case GB_FP64_code  : return (GxB_TIMES_ISLT_FP64   ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_ISGE_opcode : // with (4 monoids) x (10 non-Boolean types)

                switch (add_opcode)
                {

                    case GB_MIN_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MIN_ISGE_INT8     ) ;
                            case GB_UINT8_code : return (GxB_MIN_ISGE_UINT8    ) ;
                            case GB_INT16_code : return (GxB_MIN_ISGE_INT16    ) ;
                            case GB_UINT16_code: return (GxB_MIN_ISGE_UINT16   ) ;
                            case GB_INT32_code : return (GxB_MIN_ISGE_INT32    ) ;
                            case GB_UINT32_code: return (GxB_MIN_ISGE_UINT32   ) ;
                            case GB_INT64_code : return (GxB_MIN_ISGE_INT64    ) ;
                            case GB_UINT64_code: return (GxB_MIN_ISGE_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_MIN_ISGE_FP32     ) ;
                            case GB_FP64_code  : return (GxB_MIN_ISGE_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_MAX_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MAX_ISGE_INT8     ) ;
                            case GB_UINT8_code : return (GxB_MAX_ISGE_UINT8    ) ;
                            case GB_INT16_code : return (GxB_MAX_ISGE_INT16    ) ;
                            case GB_UINT16_code: return (GxB_MAX_ISGE_UINT16   ) ;
                            case GB_INT32_code : return (GxB_MAX_ISGE_INT32    ) ;
                            case GB_UINT32_code: return (GxB_MAX_ISGE_UINT32   ) ;
                            case GB_INT64_code : return (GxB_MAX_ISGE_INT64    ) ;
                            case GB_UINT64_code: return (GxB_MAX_ISGE_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_MAX_ISGE_FP32     ) ;
                            case GB_FP64_code  : return (GxB_MAX_ISGE_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_PLUS_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_PLUS_ISGE_INT8    ) ;
                            case GB_UINT8_code : return (GxB_PLUS_ISGE_UINT8   ) ;
                            case GB_INT16_code : return (GxB_PLUS_ISGE_INT16   ) ;
                            case GB_UINT16_code: return (GxB_PLUS_ISGE_UINT16  ) ;
                            case GB_INT32_code : return (GxB_PLUS_ISGE_INT32   ) ;
                            case GB_UINT32_code: return (GxB_PLUS_ISGE_UINT32  ) ;
                            case GB_INT64_code : return (GxB_PLUS_ISGE_INT64   ) ;
                            case GB_UINT64_code: return (GxB_PLUS_ISGE_UINT64  ) ;
                            case GB_FP32_code  : return (GxB_PLUS_ISGE_FP32    ) ;
                            case GB_FP64_code  : return (GxB_PLUS_ISGE_FP64    ) ;
                            default : ; 
                        }
                        break; 

                    case GB_TIMES_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_TIMES_ISGE_INT8   ) ;
                            case GB_UINT8_code : return (GxB_TIMES_ISGE_UINT8  ) ;
                            case GB_INT16_code : return (GxB_TIMES_ISGE_INT16  ) ;
                            case GB_UINT16_code: return (GxB_TIMES_ISGE_UINT16 ) ;
                            case GB_INT32_code : return (GxB_TIMES_ISGE_INT32  ) ;
                            case GB_UINT32_code: return (GxB_TIMES_ISGE_UINT32 ) ;
                            case GB_INT64_code : return (GxB_TIMES_ISGE_INT64  ) ;
                            case GB_UINT64_code: return (GxB_TIMES_ISGE_UINT64 ) ;
                            case GB_FP32_code  : return (GxB_TIMES_ISGE_FP32   ) ;
                            case GB_FP64_code  : return (GxB_TIMES_ISGE_FP64   ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_ISLE_opcode : // with (4 monoids) x (10 non-Boolean types)

                switch (add_opcode)
                {

                    case GB_MIN_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MIN_ISLE_INT8     ) ;
                            case GB_UINT8_code : return (GxB_MIN_ISLE_UINT8    ) ;
                            case GB_INT16_code : return (GxB_MIN_ISLE_INT16    ) ;
                            case GB_UINT16_code: return (GxB_MIN_ISLE_UINT16   ) ;
                            case GB_INT32_code : return (GxB_MIN_ISLE_INT32    ) ;
                            case GB_UINT32_code: return (GxB_MIN_ISLE_UINT32   ) ;
                            case GB_INT64_code : return (GxB_MIN_ISLE_INT64    ) ;
                            case GB_UINT64_code: return (GxB_MIN_ISLE_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_MIN_ISLE_FP32     ) ;
                            case GB_FP64_code  : return (GxB_MIN_ISLE_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_MAX_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MAX_ISLE_INT8     ) ;
                            case GB_UINT8_code : return (GxB_MAX_ISLE_UINT8    ) ;
                            case GB_INT16_code : return (GxB_MAX_ISLE_INT16    ) ;
                            case GB_UINT16_code: return (GxB_MAX_ISLE_UINT16   ) ;
                            case GB_INT32_code : return (GxB_MAX_ISLE_INT32    ) ;
                            case GB_UINT32_code: return (GxB_MAX_ISLE_UINT32   ) ;
                            case GB_INT64_code : return (GxB_MAX_ISLE_INT64    ) ;
                            case GB_UINT64_code: return (GxB_MAX_ISLE_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_MAX_ISLE_FP32     ) ;
                            case GB_FP64_code  : return (GxB_MAX_ISLE_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_PLUS_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_PLUS_ISLE_INT8    ) ;
                            case GB_UINT8_code : return (GxB_PLUS_ISLE_UINT8   ) ;
                            case GB_INT16_code : return (GxB_PLUS_ISLE_INT16   ) ;
                            case GB_UINT16_code: return (GxB_PLUS_ISLE_UINT16  ) ;
                            case GB_INT32_code : return (GxB_PLUS_ISLE_INT32   ) ;
                            case GB_UINT32_code: return (GxB_PLUS_ISLE_UINT32  ) ;
                            case GB_INT64_code : return (GxB_PLUS_ISLE_INT64   ) ;
                            case GB_UINT64_code: return (GxB_PLUS_ISLE_UINT64  ) ;
                            case GB_FP32_code  : return (GxB_PLUS_ISLE_FP32    ) ;
                            case GB_FP64_code  : return (GxB_PLUS_ISLE_FP64    ) ;
                            default : ; 
                        }
                        break; 

                    case GB_TIMES_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_TIMES_ISLE_INT8   ) ;
                            case GB_UINT8_code : return (GxB_TIMES_ISLE_UINT8  ) ;
                            case GB_INT16_code : return (GxB_TIMES_ISLE_INT16  ) ;
                            case GB_UINT16_code: return (GxB_TIMES_ISLE_UINT16 ) ;
                            case GB_INT32_code : return (GxB_TIMES_ISLE_INT32  ) ;
                            case GB_UINT32_code: return (GxB_TIMES_ISLE_UINT32 ) ;
                            case GB_INT64_code : return (GxB_TIMES_ISLE_INT64  ) ;
                            case GB_UINT64_code: return (GxB_TIMES_ISLE_UINT64 ) ;
                            case GB_FP32_code  : return (GxB_TIMES_ISLE_FP32   ) ;
                            case GB_FP64_code  : return (GxB_TIMES_ISLE_FP64   ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_LOR_opcode : // with (4 monoids) x (10 non-Boolean types)

                switch (add_opcode)
                {

                    case GB_MIN_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MIN_LOR_INT8      ) ;
                            case GB_UINT8_code : return (GxB_MIN_LOR_UINT8     ) ;
                            case GB_INT16_code : return (GxB_MIN_LOR_INT16     ) ;
                            case GB_UINT16_code: return (GxB_MIN_LOR_UINT16    ) ;
                            case GB_INT32_code : return (GxB_MIN_LOR_INT32     ) ;
                            case GB_UINT32_code: return (GxB_MIN_LOR_UINT32    ) ;
                            case GB_INT64_code : return (GxB_MIN_LOR_INT64     ) ;
                            case GB_UINT64_code: return (GxB_MIN_LOR_UINT64    ) ;
                            case GB_FP32_code  : return (GxB_MIN_LOR_FP32      ) ;
                            case GB_FP64_code  : return (GxB_MIN_LOR_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_MAX_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MAX_LOR_INT8      ) ;
                            case GB_UINT8_code : return (GxB_MAX_LOR_UINT8     ) ;
                            case GB_INT16_code : return (GxB_MAX_LOR_INT16     ) ;
                            case GB_UINT16_code: return (GxB_MAX_LOR_UINT16    ) ;
                            case GB_INT32_code : return (GxB_MAX_LOR_INT32     ) ;
                            case GB_UINT32_code: return (GxB_MAX_LOR_UINT32    ) ;
                            case GB_INT64_code : return (GxB_MAX_LOR_INT64     ) ;
                            case GB_UINT64_code: return (GxB_MAX_LOR_UINT64    ) ;
                            case GB_FP32_code  : return (GxB_MAX_LOR_FP32      ) ;
                            case GB_FP64_code  : return (GxB_MAX_LOR_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_PLUS_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_PLUS_LOR_INT8     ) ;
                            case GB_UINT8_code : return (GxB_PLUS_LOR_UINT8    ) ;
                            case GB_INT16_code : return (GxB_PLUS_LOR_INT16    ) ;
                            case GB_UINT16_code: return (GxB_PLUS_LOR_UINT16   ) ;
                            case GB_INT32_code : return (GxB_PLUS_LOR_INT32    ) ;
                            case GB_UINT32_code: return (GxB_PLUS_LOR_UINT32   ) ;
                            case GB_INT64_code : return (GxB_PLUS_LOR_INT64    ) ;
                            case GB_UINT64_code: return (GxB_PLUS_LOR_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_PLUS_LOR_FP32     ) ;
                            case GB_FP64_code  : return (GxB_PLUS_LOR_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_TIMES_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_TIMES_LOR_INT8    ) ;
                            case GB_UINT8_code : return (GxB_TIMES_LOR_UINT8   ) ;
                            case GB_INT16_code : return (GxB_TIMES_LOR_INT16   ) ;
                            case GB_UINT16_code: return (GxB_TIMES_LOR_UINT16  ) ;
                            case GB_INT32_code : return (GxB_TIMES_LOR_INT32   ) ;
                            case GB_UINT32_code: return (GxB_TIMES_LOR_UINT32  ) ;
                            case GB_INT64_code : return (GxB_TIMES_LOR_INT64   ) ;
                            case GB_UINT64_code: return (GxB_TIMES_LOR_UINT64  ) ;
                            case GB_FP32_code  : return (GxB_TIMES_LOR_FP32    ) ;
                            case GB_FP64_code  : return (GxB_TIMES_LOR_FP64    ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_LAND_opcode : // with (4 monoids) x (10 non-Boolean types)

                switch (add_opcode)
                {

                    case GB_MIN_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MIN_LAND_INT8     ) ;
                            case GB_UINT8_code : return (GxB_MIN_LAND_UINT8    ) ;
                            case GB_INT16_code : return (GxB_MIN_LAND_INT16    ) ;
                            case GB_UINT16_code: return (GxB_MIN_LAND_UINT16   ) ;
                            case GB_INT32_code : return (GxB_MIN_LAND_INT32    ) ;
                            case GB_UINT32_code: return (GxB_MIN_LAND_UINT32   ) ;
                            case GB_INT64_code : return (GxB_MIN_LAND_INT64    ) ;
                            case GB_UINT64_code: return (GxB_MIN_LAND_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_MIN_LAND_FP32     ) ;
                            case GB_FP64_code  : return (GxB_MIN_LAND_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_MAX_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MAX_LAND_INT8     ) ;
                            case GB_UINT8_code : return (GxB_MAX_LAND_UINT8    ) ;
                            case GB_INT16_code : return (GxB_MAX_LAND_INT16    ) ;
                            case GB_UINT16_code: return (GxB_MAX_LAND_UINT16   ) ;
                            case GB_INT32_code : return (GxB_MAX_LAND_INT32    ) ;
                            case GB_UINT32_code: return (GxB_MAX_LAND_UINT32   ) ;
                            case GB_INT64_code : return (GxB_MAX_LAND_INT64    ) ;
                            case GB_UINT64_code: return (GxB_MAX_LAND_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_MAX_LAND_FP32     ) ;
                            case GB_FP64_code  : return (GxB_MAX_LAND_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_PLUS_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_PLUS_LAND_INT8    ) ;
                            case GB_UINT8_code : return (GxB_PLUS_LAND_UINT8   ) ;
                            case GB_INT16_code : return (GxB_PLUS_LAND_INT16   ) ;
                            case GB_UINT16_code: return (GxB_PLUS_LAND_UINT16  ) ;
                            case GB_INT32_code : return (GxB_PLUS_LAND_INT32   ) ;
                            case GB_UINT32_code: return (GxB_PLUS_LAND_UINT32  ) ;
                            case GB_INT64_code : return (GxB_PLUS_LAND_INT64   ) ;
                            case GB_UINT64_code: return (GxB_PLUS_LAND_UINT64  ) ;
                            case GB_FP32_code  : return (GxB_PLUS_LAND_FP32    ) ;
                            case GB_FP64_code  : return (GxB_PLUS_LAND_FP64    ) ;
                            default : ; 
                        }
                        break; 

                    case GB_TIMES_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_TIMES_LAND_INT8   ) ;
                            case GB_UINT8_code : return (GxB_TIMES_LAND_UINT8  ) ;
                            case GB_INT16_code : return (GxB_TIMES_LAND_INT16  ) ;
                            case GB_UINT16_code: return (GxB_TIMES_LAND_UINT16 ) ;
                            case GB_INT32_code : return (GxB_TIMES_LAND_INT32  ) ;
                            case GB_UINT32_code: return (GxB_TIMES_LAND_UINT32 ) ;
                            case GB_INT64_code : return (GxB_TIMES_LAND_INT64  ) ;
                            case GB_UINT64_code: return (GxB_TIMES_LAND_UINT64 ) ;
                            case GB_FP32_code  : return (GxB_TIMES_LAND_FP32   ) ;
                            case GB_FP64_code  : return (GxB_TIMES_LAND_FP64   ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_LXOR_opcode : // with (4 monoids) x (10 non-Boolean types)

                switch (add_opcode)
                {

                    case GB_MIN_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MIN_LXOR_INT8     ) ;
                            case GB_UINT8_code : return (GxB_MIN_LXOR_UINT8    ) ;
                            case GB_INT16_code : return (GxB_MIN_LXOR_INT16    ) ;
                            case GB_UINT16_code: return (GxB_MIN_LXOR_UINT16   ) ;
                            case GB_INT32_code : return (GxB_MIN_LXOR_INT32    ) ;
                            case GB_UINT32_code: return (GxB_MIN_LXOR_UINT32   ) ;
                            case GB_INT64_code : return (GxB_MIN_LXOR_INT64    ) ;
                            case GB_UINT64_code: return (GxB_MIN_LXOR_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_MIN_LXOR_FP32     ) ;
                            case GB_FP64_code  : return (GxB_MIN_LXOR_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_MAX_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_MAX_LXOR_INT8     ) ;
                            case GB_UINT8_code : return (GxB_MAX_LXOR_UINT8    ) ;
                            case GB_INT16_code : return (GxB_MAX_LXOR_INT16    ) ;
                            case GB_UINT16_code: return (GxB_MAX_LXOR_UINT16   ) ;
                            case GB_INT32_code : return (GxB_MAX_LXOR_INT32    ) ;
                            case GB_UINT32_code: return (GxB_MAX_LXOR_UINT32   ) ;
                            case GB_INT64_code : return (GxB_MAX_LXOR_INT64    ) ;
                            case GB_UINT64_code: return (GxB_MAX_LXOR_UINT64   ) ;
                            case GB_FP32_code  : return (GxB_MAX_LXOR_FP32     ) ;
                            case GB_FP64_code  : return (GxB_MAX_LXOR_FP64     ) ;
                            default : ; 
                        }
                        break; 

                    case GB_PLUS_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_PLUS_LXOR_INT8    ) ;
                            case GB_UINT8_code : return (GxB_PLUS_LXOR_UINT8   ) ;
                            case GB_INT16_code : return (GxB_PLUS_LXOR_INT16   ) ;
                            case GB_UINT16_code: return (GxB_PLUS_LXOR_UINT16  ) ;
                            case GB_INT32_code : return (GxB_PLUS_LXOR_INT32   ) ;
                            case GB_UINT32_code: return (GxB_PLUS_LXOR_UINT32  ) ;
                            case GB_INT64_code : return (GxB_PLUS_LXOR_INT64   ) ;
                            case GB_UINT64_code: return (GxB_PLUS_LXOR_UINT64  ) ;
                            case GB_FP32_code  : return (GxB_PLUS_LXOR_FP32    ) ;
                            case GB_FP64_code  : return (GxB_PLUS_LXOR_FP64    ) ;
                            default : ; 
                        }
                        break; 

                    case GB_TIMES_opcode :

                        switch (zcode)
                        {
                            case GB_INT8_code  : return (GxB_TIMES_LXOR_INT8   ) ;
                            case GB_UINT8_code : return (GxB_TIMES_LXOR_UINT8  ) ;
                            case GB_INT16_code : return (GxB_TIMES_LXOR_INT16  ) ;
                            case GB_UINT16_code: return (GxB_TIMES_LXOR_UINT16 ) ;
                            case GB_INT32_code : return (GxB_TIMES_LXOR_INT32  ) ;
                            case GB_UINT32_code: return (GxB_TIMES_LXOR_UINT32 ) ;
                            case GB_INT64_code : return (GxB_TIMES_LXOR_INT64  ) ;
                            case GB_UINT64_code: return (GxB_TIMES_LXOR_UINT64 ) ;
                            case GB_FP32_code  : return (GxB_TIMES_LXOR_FP32   ) ;
                            case GB_FP64_code  : return (GxB_TIMES_LXOR_FP64   ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            default : ;
        }
    }
    else if (xycode != GB_BOOL_code)
    {

        //----------------------------------------------------------------------
        // 240 semirings with TxT->bool multiply operators
        //----------------------------------------------------------------------

        // x,y are one of the 10 non-Boolean types, z is Boolean

        switch (mult_opcode)
        {

            case GB_EQ_opcode : // with (4 bool monoids) x (10 non-bool types)

                switch (add_opcode)
                {

                    case GB_LOR_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_LOR_EQ_INT8       ) ;
                            case GB_UINT8_code :  return (GxB_LOR_EQ_UINT8      ) ;
                            case GB_INT16_code :  return (GxB_LOR_EQ_INT16      ) ;
                            case GB_UINT16_code:  return (GxB_LOR_EQ_UINT16     ) ;
                            case GB_INT32_code :  return (GxB_LOR_EQ_INT32      ) ;
                            case GB_UINT32_code:  return (GxB_LOR_EQ_UINT32     ) ;
                            case GB_INT64_code :  return (GxB_LOR_EQ_INT64      ) ;
                            case GB_UINT64_code:  return (GxB_LOR_EQ_UINT64     ) ;
                            case GB_FP32_code  :  return (GxB_LOR_EQ_FP32       ) ;
                            case GB_FP64_code  :  return (GxB_LOR_EQ_FP64       ) ;
                            default : ; 
                        }
                        break; 

                    case GB_LAND_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_LAND_EQ_INT8      ) ;
                            case GB_UINT8_code :  return (GxB_LAND_EQ_UINT8     ) ;
                            case GB_INT16_code :  return (GxB_LAND_EQ_INT16     ) ;
                            case GB_UINT16_code:  return (GxB_LAND_EQ_UINT16    ) ;
                            case GB_INT32_code :  return (GxB_LAND_EQ_INT32     ) ;
                            case GB_UINT32_code:  return (GxB_LAND_EQ_UINT32    ) ;
                            case GB_INT64_code :  return (GxB_LAND_EQ_INT64     ) ;
                            case GB_UINT64_code:  return (GxB_LAND_EQ_UINT64    ) ;
                            case GB_FP32_code  :  return (GxB_LAND_EQ_FP32      ) ;
                            case GB_FP64_code  :  return (GxB_LAND_EQ_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_LXOR_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_LXOR_EQ_INT8      ) ;
                            case GB_UINT8_code :  return (GxB_LXOR_EQ_UINT8     ) ;
                            case GB_INT16_code :  return (GxB_LXOR_EQ_INT16     ) ;
                            case GB_UINT16_code:  return (GxB_LXOR_EQ_UINT16    ) ;
                            case GB_INT32_code :  return (GxB_LXOR_EQ_INT32     ) ;
                            case GB_UINT32_code:  return (GxB_LXOR_EQ_UINT32    ) ;
                            case GB_INT64_code :  return (GxB_LXOR_EQ_INT64     ) ;
                            case GB_UINT64_code:  return (GxB_LXOR_EQ_UINT64    ) ;
                            case GB_FP32_code  :  return (GxB_LXOR_EQ_FP32      ) ;
                            case GB_FP64_code  :  return (GxB_LXOR_EQ_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_EQ_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_EQ_EQ_INT8        ) ;
                            case GB_UINT8_code :  return (GxB_EQ_EQ_UINT8       ) ;
                            case GB_INT16_code :  return (GxB_EQ_EQ_INT16       ) ;
                            case GB_UINT16_code:  return (GxB_EQ_EQ_UINT16      ) ;
                            case GB_INT32_code :  return (GxB_EQ_EQ_INT32       ) ;
                            case GB_UINT32_code:  return (GxB_EQ_EQ_UINT32      ) ;
                            case GB_INT64_code :  return (GxB_EQ_EQ_INT64       ) ;
                            case GB_UINT64_code:  return (GxB_EQ_EQ_UINT64      ) ;
                            case GB_FP32_code  :  return (GxB_EQ_EQ_FP32        ) ;
                            case GB_FP64_code  :  return (GxB_EQ_EQ_FP64        ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_NE_opcode : // with (4 bool monoids) x (10 non-bool types)

                switch (add_opcode)
                {

                    case GB_LOR_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_LOR_NE_INT8       ) ;
                            case GB_UINT8_code :  return (GxB_LOR_NE_UINT8      ) ;
                            case GB_INT16_code :  return (GxB_LOR_NE_INT16      ) ;
                            case GB_UINT16_code:  return (GxB_LOR_NE_UINT16     ) ;
                            case GB_INT32_code :  return (GxB_LOR_NE_INT32      ) ;
                            case GB_UINT32_code:  return (GxB_LOR_NE_UINT32     ) ;
                            case GB_INT64_code :  return (GxB_LOR_NE_INT64      ) ;
                            case GB_UINT64_code:  return (GxB_LOR_NE_UINT64     ) ;
                            case GB_FP32_code  :  return (GxB_LOR_NE_FP32       ) ;
                            case GB_FP64_code  :  return (GxB_LOR_NE_FP64       ) ;
                            default : ; 
                        }
                        break; 

                    case GB_LAND_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_LAND_NE_INT8      ) ;
                            case GB_UINT8_code :  return (GxB_LAND_NE_UINT8     ) ;
                            case GB_INT16_code :  return (GxB_LAND_NE_INT16     ) ;
                            case GB_UINT16_code:  return (GxB_LAND_NE_UINT16    ) ;
                            case GB_INT32_code :  return (GxB_LAND_NE_INT32     ) ;
                            case GB_UINT32_code:  return (GxB_LAND_NE_UINT32    ) ;
                            case GB_INT64_code :  return (GxB_LAND_NE_INT64     ) ;
                            case GB_UINT64_code:  return (GxB_LAND_NE_UINT64    ) ;
                            case GB_FP32_code  :  return (GxB_LAND_NE_FP32      ) ;
                            case GB_FP64_code  :  return (GxB_LAND_NE_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_LXOR_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_LXOR_NE_INT8      ) ;
                            case GB_UINT8_code :  return (GxB_LXOR_NE_UINT8     ) ;
                            case GB_INT16_code :  return (GxB_LXOR_NE_INT16     ) ;
                            case GB_UINT16_code:  return (GxB_LXOR_NE_UINT16    ) ;
                            case GB_INT32_code :  return (GxB_LXOR_NE_INT32     ) ;
                            case GB_UINT32_code:  return (GxB_LXOR_NE_UINT32    ) ;
                            case GB_INT64_code :  return (GxB_LXOR_NE_INT64     ) ;
                            case GB_UINT64_code:  return (GxB_LXOR_NE_UINT64    ) ;
                            case GB_FP32_code  :  return (GxB_LXOR_NE_FP32      ) ;
                            case GB_FP64_code  :  return (GxB_LXOR_NE_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_EQ_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_EQ_NE_INT8        ) ;
                            case GB_UINT8_code :  return (GxB_EQ_NE_UINT8       ) ;
                            case GB_INT16_code :  return (GxB_EQ_NE_INT16       ) ;
                            case GB_UINT16_code:  return (GxB_EQ_NE_UINT16      ) ;
                            case GB_INT32_code :  return (GxB_EQ_NE_INT32       ) ;
                            case GB_UINT32_code:  return (GxB_EQ_NE_UINT32      ) ;
                            case GB_INT64_code :  return (GxB_EQ_NE_INT64       ) ;
                            case GB_UINT64_code:  return (GxB_EQ_NE_UINT64      ) ;
                            case GB_FP32_code  :  return (GxB_EQ_NE_FP32        ) ;
                            case GB_FP64_code  :  return (GxB_EQ_NE_FP64        ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_GT_opcode : // with (4 bool monoids) x (10 non-bool types)

                switch (add_opcode)
                {

                    case GB_LOR_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_LOR_GT_INT8       ) ;
                            case GB_UINT8_code :  return (GxB_LOR_GT_UINT8      ) ;
                            case GB_INT16_code :  return (GxB_LOR_GT_INT16      ) ;
                            case GB_UINT16_code:  return (GxB_LOR_GT_UINT16     ) ;
                            case GB_INT32_code :  return (GxB_LOR_GT_INT32      ) ;
                            case GB_UINT32_code:  return (GxB_LOR_GT_UINT32     ) ;
                            case GB_INT64_code :  return (GxB_LOR_GT_INT64      ) ;
                            case GB_UINT64_code:  return (GxB_LOR_GT_UINT64     ) ;
                            case GB_FP32_code  :  return (GxB_LOR_GT_FP32       ) ;
                            case GB_FP64_code  :  return (GxB_LOR_GT_FP64       ) ;
                            default : ; 
                        }
                        break; 

                    case GB_LAND_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_LAND_GT_INT8      ) ;
                            case GB_UINT8_code :  return (GxB_LAND_GT_UINT8     ) ;
                            case GB_INT16_code :  return (GxB_LAND_GT_INT16     ) ;
                            case GB_UINT16_code:  return (GxB_LAND_GT_UINT16    ) ;
                            case GB_INT32_code :  return (GxB_LAND_GT_INT32     ) ;
                            case GB_UINT32_code:  return (GxB_LAND_GT_UINT32    ) ;
                            case GB_INT64_code :  return (GxB_LAND_GT_INT64     ) ;
                            case GB_UINT64_code:  return (GxB_LAND_GT_UINT64    ) ;
                            case GB_FP32_code  :  return (GxB_LAND_GT_FP32      ) ;
                            case GB_FP64_code  :  return (GxB_LAND_GT_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_LXOR_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_LXOR_GT_INT8      ) ;
                            case GB_UINT8_code :  return (GxB_LXOR_GT_UINT8     ) ;
                            case GB_INT16_code :  return (GxB_LXOR_GT_INT16     ) ;
                            case GB_UINT16_code:  return (GxB_LXOR_GT_UINT16    ) ;
                            case GB_INT32_code :  return (GxB_LXOR_GT_INT32     ) ;
                            case GB_UINT32_code:  return (GxB_LXOR_GT_UINT32    ) ;
                            case GB_INT64_code :  return (GxB_LXOR_GT_INT64     ) ;
                            case GB_UINT64_code:  return (GxB_LXOR_GT_UINT64    ) ;
                            case GB_FP32_code  :  return (GxB_LXOR_GT_FP32      ) ;
                            case GB_FP64_code  :  return (GxB_LXOR_GT_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_EQ_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_EQ_GT_INT8        ) ;
                            case GB_UINT8_code :  return (GxB_EQ_GT_UINT8       ) ;
                            case GB_INT16_code :  return (GxB_EQ_GT_INT16       ) ;
                            case GB_UINT16_code:  return (GxB_EQ_GT_UINT16      ) ;
                            case GB_INT32_code :  return (GxB_EQ_GT_INT32       ) ;
                            case GB_UINT32_code:  return (GxB_EQ_GT_UINT32      ) ;
                            case GB_INT64_code :  return (GxB_EQ_GT_INT64       ) ;
                            case GB_UINT64_code:  return (GxB_EQ_GT_UINT64      ) ;
                            case GB_FP32_code  :  return (GxB_EQ_GT_FP32        ) ;
                            case GB_FP64_code  :  return (GxB_EQ_GT_FP64        ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_LT_opcode : // with (4 bool monoids) x (10 non-bool types)

                switch (add_opcode)
                {

                    case GB_LOR_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_LOR_LT_INT8       ) ;
                            case GB_UINT8_code :  return (GxB_LOR_LT_UINT8      ) ;
                            case GB_INT16_code :  return (GxB_LOR_LT_INT16      ) ;
                            case GB_UINT16_code:  return (GxB_LOR_LT_UINT16     ) ;
                            case GB_INT32_code :  return (GxB_LOR_LT_INT32      ) ;
                            case GB_UINT32_code:  return (GxB_LOR_LT_UINT32     ) ;
                            case GB_INT64_code :  return (GxB_LOR_LT_INT64      ) ;
                            case GB_UINT64_code:  return (GxB_LOR_LT_UINT64     ) ;
                            case GB_FP32_code  :  return (GxB_LOR_LT_FP32       ) ;
                            case GB_FP64_code  :  return (GxB_LOR_LT_FP64       ) ;
                            default : ; 
                        }
                        break; 

                    case GB_LAND_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_LAND_LT_INT8      ) ;
                            case GB_UINT8_code :  return (GxB_LAND_LT_UINT8     ) ;
                            case GB_INT16_code :  return (GxB_LAND_LT_INT16     ) ;
                            case GB_UINT16_code:  return (GxB_LAND_LT_UINT16    ) ;
                            case GB_INT32_code :  return (GxB_LAND_LT_INT32     ) ;
                            case GB_UINT32_code:  return (GxB_LAND_LT_UINT32    ) ;
                            case GB_INT64_code :  return (GxB_LAND_LT_INT64     ) ;
                            case GB_UINT64_code:  return (GxB_LAND_LT_UINT64    ) ;
                            case GB_FP32_code  :  return (GxB_LAND_LT_FP32      ) ;
                            case GB_FP64_code  :  return (GxB_LAND_LT_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_LXOR_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_LXOR_LT_INT8      ) ;
                            case GB_UINT8_code :  return (GxB_LXOR_LT_UINT8     ) ;
                            case GB_INT16_code :  return (GxB_LXOR_LT_INT16     ) ;
                            case GB_UINT16_code:  return (GxB_LXOR_LT_UINT16    ) ;
                            case GB_INT32_code :  return (GxB_LXOR_LT_INT32     ) ;
                            case GB_UINT32_code:  return (GxB_LXOR_LT_UINT32    ) ;
                            case GB_INT64_code :  return (GxB_LXOR_LT_INT64     ) ;
                            case GB_UINT64_code:  return (GxB_LXOR_LT_UINT64    ) ;
                            case GB_FP32_code  :  return (GxB_LXOR_LT_FP32      ) ;
                            case GB_FP64_code  :  return (GxB_LXOR_LT_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_EQ_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_EQ_LT_INT8        ) ;
                            case GB_UINT8_code :  return (GxB_EQ_LT_UINT8       ) ;
                            case GB_INT16_code :  return (GxB_EQ_LT_INT16       ) ;
                            case GB_UINT16_code:  return (GxB_EQ_LT_UINT16      ) ;
                            case GB_INT32_code :  return (GxB_EQ_LT_INT32       ) ;
                            case GB_UINT32_code:  return (GxB_EQ_LT_UINT32      ) ;
                            case GB_INT64_code :  return (GxB_EQ_LT_INT64       ) ;
                            case GB_UINT64_code:  return (GxB_EQ_LT_UINT64      ) ;
                            case GB_FP32_code  :  return (GxB_EQ_LT_FP32        ) ;
                            case GB_FP64_code  :  return (GxB_EQ_LT_FP64        ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_GE_opcode : // with (4 bool monoids) x (10 non-bool types)

                switch (add_opcode)
                {

                    case GB_LOR_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_LOR_GE_INT8       ) ;
                            case GB_UINT8_code :  return (GxB_LOR_GE_UINT8      ) ;
                            case GB_INT16_code :  return (GxB_LOR_GE_INT16      ) ;
                            case GB_UINT16_code:  return (GxB_LOR_GE_UINT16     ) ;
                            case GB_INT32_code :  return (GxB_LOR_GE_INT32      ) ;
                            case GB_UINT32_code:  return (GxB_LOR_GE_UINT32     ) ;
                            case GB_INT64_code :  return (GxB_LOR_GE_INT64      ) ;
                            case GB_UINT64_code:  return (GxB_LOR_GE_UINT64     ) ;
                            case GB_FP32_code  :  return (GxB_LOR_GE_FP32       ) ;
                            case GB_FP64_code  :  return (GxB_LOR_GE_FP64       ) ;
                            default : ; 
                        }
                        break; 

                    case GB_LAND_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_LAND_GE_INT8      ) ;
                            case GB_UINT8_code :  return (GxB_LAND_GE_UINT8     ) ;
                            case GB_INT16_code :  return (GxB_LAND_GE_INT16     ) ;
                            case GB_UINT16_code:  return (GxB_LAND_GE_UINT16    ) ;
                            case GB_INT32_code :  return (GxB_LAND_GE_INT32     ) ;
                            case GB_UINT32_code:  return (GxB_LAND_GE_UINT32    ) ;
                            case GB_INT64_code :  return (GxB_LAND_GE_INT64     ) ;
                            case GB_UINT64_code:  return (GxB_LAND_GE_UINT64    ) ;
                            case GB_FP32_code  :  return (GxB_LAND_GE_FP32      ) ;
                            case GB_FP64_code  :  return (GxB_LAND_GE_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_LXOR_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_LXOR_GE_INT8      ) ;
                            case GB_UINT8_code :  return (GxB_LXOR_GE_UINT8     ) ;
                            case GB_INT16_code :  return (GxB_LXOR_GE_INT16     ) ;
                            case GB_UINT16_code:  return (GxB_LXOR_GE_UINT16    ) ;
                            case GB_INT32_code :  return (GxB_LXOR_GE_INT32     ) ;
                            case GB_UINT32_code:  return (GxB_LXOR_GE_UINT32    ) ;
                            case GB_INT64_code :  return (GxB_LXOR_GE_INT64     ) ;
                            case GB_UINT64_code:  return (GxB_LXOR_GE_UINT64    ) ;
                            case GB_FP32_code  :  return (GxB_LXOR_GE_FP32      ) ;
                            case GB_FP64_code  :  return (GxB_LXOR_GE_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_EQ_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_EQ_GE_INT8        ) ;
                            case GB_UINT8_code :  return (GxB_EQ_GE_UINT8       ) ;
                            case GB_INT16_code :  return (GxB_EQ_GE_INT16       ) ;
                            case GB_UINT16_code:  return (GxB_EQ_GE_UINT16      ) ;
                            case GB_INT32_code :  return (GxB_EQ_GE_INT32       ) ;
                            case GB_UINT32_code:  return (GxB_EQ_GE_UINT32      ) ;
                            case GB_INT64_code :  return (GxB_EQ_GE_INT64       ) ;
                            case GB_UINT64_code:  return (GxB_EQ_GE_UINT64      ) ;
                            case GB_FP32_code  :  return (GxB_EQ_GE_FP32        ) ;
                            case GB_FP64_code  :  return (GxB_EQ_GE_FP64        ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            case GB_LE_opcode : // with (4 bool monoids) x (10 non-bool types)

                switch (add_opcode)
                {

                    case GB_LOR_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_LOR_LE_INT8       ) ;
                            case GB_UINT8_code :  return (GxB_LOR_LE_UINT8      ) ;
                            case GB_INT16_code :  return (GxB_LOR_LE_INT16      ) ;
                            case GB_UINT16_code:  return (GxB_LOR_LE_UINT16     ) ;
                            case GB_INT32_code :  return (GxB_LOR_LE_INT32      ) ;
                            case GB_UINT32_code:  return (GxB_LOR_LE_UINT32     ) ;
                            case GB_INT64_code :  return (GxB_LOR_LE_INT64      ) ;
                            case GB_UINT64_code:  return (GxB_LOR_LE_UINT64     ) ;
                            case GB_FP32_code  :  return (GxB_LOR_LE_FP32       ) ;
                            case GB_FP64_code  :  return (GxB_LOR_LE_FP64       ) ;
                            default : ; 
                        }
                        break; 

                    case GB_LAND_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_LAND_LE_INT8      ) ;
                            case GB_UINT8_code :  return (GxB_LAND_LE_UINT8     ) ;
                            case GB_INT16_code :  return (GxB_LAND_LE_INT16     ) ;
                            case GB_UINT16_code:  return (GxB_LAND_LE_UINT16    ) ;
                            case GB_INT32_code :  return (GxB_LAND_LE_INT32     ) ;
                            case GB_UINT32_code:  return (GxB_LAND_LE_UINT32    ) ;
                            case GB_INT64_code :  return (GxB_LAND_LE_INT64     ) ;
                            case GB_UINT64_code:  return (GxB_LAND_LE_UINT64    ) ;
                            case GB_FP32_code  :  return (GxB_LAND_LE_FP32      ) ;
                            case GB_FP64_code  :  return (GxB_LAND_LE_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_LXOR_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_LXOR_LE_INT8      ) ;
                            case GB_UINT8_code :  return (GxB_LXOR_LE_UINT8     ) ;
                            case GB_INT16_code :  return (GxB_LXOR_LE_INT16     ) ;
                            case GB_UINT16_code:  return (GxB_LXOR_LE_UINT16    ) ;
                            case GB_INT32_code :  return (GxB_LXOR_LE_INT32     ) ;
                            case GB_UINT32_code:  return (GxB_LXOR_LE_UINT32    ) ;
                            case GB_INT64_code :  return (GxB_LXOR_LE_INT64     ) ;
                            case GB_UINT64_code:  return (GxB_LXOR_LE_UINT64    ) ;
                            case GB_FP32_code  :  return (GxB_LXOR_LE_FP32      ) ;
                            case GB_FP64_code  :  return (GxB_LXOR_LE_FP64      ) ;
                            default : ; 
                        }
                        break; 

                    case GB_EQ_opcode :

                        switch (xycode)
                        {
                            case GB_INT8_code  :  return (GxB_EQ_LE_INT8        ) ;
                            case GB_UINT8_code :  return (GxB_EQ_LE_UINT8       ) ;
                            case GB_INT16_code :  return (GxB_EQ_LE_INT16       ) ;
                            case GB_UINT16_code:  return (GxB_EQ_LE_UINT16      ) ;
                            case GB_INT32_code :  return (GxB_EQ_LE_INT32       ) ;
                            case GB_UINT32_code:  return (GxB_EQ_LE_UINT32      ) ;
                            case GB_INT64_code :  return (GxB_EQ_LE_INT64       ) ;
                            case GB_UINT64_code:  return (GxB_EQ_LE_UINT64      ) ;
                            case GB_FP32_code  :  return (GxB_EQ_LE_FP32        ) ;
                            case GB_FP64_code  :  return (GxB_EQ_LE_FP64        ) ;
                            default : ; 
                        }
                        break; 

                    default : ;
                }

            default : ;
        }
    }
    else
    {

        //----------------------------------------------------------------------
        // 40 purely Boolean semirings
        //----------------------------------------------------------------------

        // x,y,z are all Boolean, and all operators are Boolean

        switch (mult_opcode)
        {

            case GB_FIRST_opcode :

                switch (add_opcode)
                {
                    case GB_LOR_opcode        :  return (GxB_LOR_FIRST_BOOL    ) ;
                    case GB_LAND_opcode       :  return (GxB_LAND_FIRST_BOOL   ) ;
                    case GB_LXOR_opcode       :  return (GxB_LXOR_FIRST_BOOL   ) ;
                    case GB_EQ_opcode         :  return (GxB_EQ_FIRST_BOOL     ) ;
                    default : ;
                }

            case GB_SECOND_opcode :

                switch (add_opcode)
                {
                    case GB_LOR_opcode        :  return (GxB_LOR_SECOND_BOOL   ) ;
                    case GB_LAND_opcode       :  return (GxB_LAND_SECOND_BOOL  ) ;
                    case GB_LXOR_opcode       :  return (GxB_LXOR_SECOND_BOOL  ) ;
                    case GB_EQ_opcode         :  return (GxB_EQ_SECOND_BOOL    ) ;
                    default : ;
                }

            case GB_LOR_opcode :

                switch (add_opcode)
                {
                    case GB_LOR_opcode        :  return (GxB_LOR_LOR_BOOL      ) ;
                    case GB_LAND_opcode       :  return (GxB_LAND_LOR_BOOL     ) ;
                    case GB_LXOR_opcode       :  return (GxB_LXOR_LOR_BOOL     ) ;
                    case GB_EQ_opcode         :  return (GxB_EQ_LOR_BOOL       ) ;
                    default : ;
                }

            case GB_LAND_opcode :

                switch (add_opcode)
                {
                    case GB_LOR_opcode        :  return (GxB_LOR_LAND_BOOL     ) ;
                    case GB_LAND_opcode       :  return (GxB_LAND_LAND_BOOL    ) ;
                    case GB_LXOR_opcode       :  return (GxB_LXOR_LAND_BOOL    ) ;
                    case GB_EQ_opcode         :  return (GxB_EQ_LAND_BOOL      ) ;
                    default : ;
                }

            case GB_LXOR_opcode :

                switch (add_opcode)
                {
                    case GB_LOR_opcode        :  return (GxB_LOR_LXOR_BOOL     ) ;
                    case GB_LAND_opcode       :  return (GxB_LAND_LXOR_BOOL    ) ;
                    case GB_LXOR_opcode       :  return (GxB_LXOR_LXOR_BOOL    ) ;
                    case GB_EQ_opcode         :  return (GxB_EQ_LXOR_BOOL      ) ;
                    default : ;
                }

            case GB_EQ_opcode :

                switch (add_opcode)
                {
                    case GB_LOR_opcode        :  return (GxB_LOR_EQ_BOOL       ) ;
                    case GB_LAND_opcode       :  return (GxB_LAND_EQ_BOOL      ) ;
                    case GB_LXOR_opcode       :  return (GxB_LXOR_EQ_BOOL      ) ;
                    case GB_EQ_opcode         :  return (GxB_EQ_EQ_BOOL        ) ;
                    default : ;
                }

            case GB_GT_opcode :

                switch (add_opcode)
                {
                    case GB_LOR_opcode        :  return (GxB_LOR_GT_BOOL       ) ;
                    case GB_LAND_opcode       :  return (GxB_LAND_GT_BOOL      ) ;
                    case GB_LXOR_opcode       :  return (GxB_LXOR_GT_BOOL      ) ;
                    case GB_EQ_opcode         :  return (GxB_EQ_GT_BOOL        ) ;
                    default : ;
                }

            case GB_LT_opcode :

                switch (add_opcode)
                {
                    case GB_LOR_opcode        :  return (GxB_LOR_LT_BOOL       ) ;
                    case GB_LAND_opcode       :  return (GxB_LAND_LT_BOOL      ) ;
                    case GB_LXOR_opcode       :  return (GxB_LXOR_LT_BOOL      ) ;
                    case GB_EQ_opcode         :  return (GxB_EQ_LT_BOOL        ) ;
                    default : ;
                }

            case GB_GE_opcode :

                switch (add_opcode)
                {
                    case GB_LOR_opcode        :  return (GxB_LOR_GE_BOOL       ) ;
                    case GB_LAND_opcode       :  return (GxB_LAND_GE_BOOL      ) ;
                    case GB_LXOR_opcode       :  return (GxB_LXOR_GE_BOOL      ) ;
                    case GB_EQ_opcode         :  return (GxB_EQ_GE_BOOL        ) ;
                    default : ;
                }

            case GB_LE_opcode :

                switch (add_opcode)
                {
                    case GB_LOR_opcode        :  return (GxB_LOR_LE_BOOL       ) ;
                    case GB_LAND_opcode       :  return (GxB_LAND_LE_BOOL      ) ;
                    case GB_LXOR_opcode       :  return (GxB_LXOR_LE_BOOL      ) ;
                    case GB_EQ_opcode         :  return (GxB_EQ_LE_BOOL        ) ;
                    default : ;
                }

            default : ;
        }
    }

    //--------------------------------------------------------------------------
    // not a built-in semiring
    //--------------------------------------------------------------------------

    return (NULL) ;
}

