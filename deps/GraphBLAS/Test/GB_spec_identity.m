function identity = GB_spec_identity (arg1,arg2)
%GB_SPEC_IDENTITY the additive identity of a monoid
%
% identity = GB_spec_identity (add) ;
% or
% identity = GB_spec_identity (add_op, add_class) ; % both strings
%
% Returns the additive identity of an operator of a monoid.
%
% The identity is that value such that x == add (x,identity) == add
% (identity,x).  For example, for 'plus', the value is zero.  for 'max' the
% value of identity is -infinity.
%
% The 8 addititive monoids supported are 'min', 'max', 'plus', 'times', 'or',
% 'and', 'xor', and 'eq'.   For the last 4 the class must be 'logical'

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin == 1)
    add = arg1 ;
    [add_operator add_class] = GB_spec_operator (add) ;
else
    add_operator = arg1 ;
    add_class = arg2 ;
end

switch add_operator

    case 'min'

        % x == min (x, inf)
        switch add_class
            case 'logical'
                identity = true ;
            case 'int8'
                identity = int8 (inf) ;
            case 'uint8'
                identity = uint8 (inf) ;
            case 'int16'
                identity = int16 (inf) ;
            case 'uint16'
                identity = uint16 (inf) ;
            case 'int32'
                identity = int32 (inf) ;
            case 'uint32'
                identity = uint32 (inf) ;
            case 'int64'
                identity = int64 (inf) ;
            case 'uint64'
                identity = uint64 (inf) ;
            case 'single'
                identity = single (inf) ;
            case 'double'
                identity = inf ;
        end

    case 'max'

        % x == max (x, -inf)
        switch add_class
            case 'logical'
                identity = false ;
            case 'int8'
                identity = int8 (-inf) ;
            case 'uint8'
                identity = 0 ;
            case 'int16'
                identity = int16 (-inf) ;
            case 'uint16'
                identity = 0 ;
            case 'int32'
                identity = int32 (-inf) ;
            case 'uint32'
                identity = 0 ;
            case 'int64'
                identity = int64 (-inf) ;
            case 'uint64'
                identity = 0 ;
            case 'single'
                identity = single (-inf) ;
            case 'double'
                identity = -inf ;
        end

    case 'plus'

        % x == x + 0
        identity = 0 ;

    case 'times'

        % x == x * 1
        identity = 1 ;

    case 'any'

        identity = [ ] ;

    case 'or'

        % x == x or false
        identity = false ;

        if (~isequal (add_class, 'logical'))
            error ('OR monoid must be logical') ;
        end

    case 'and'

        % x == x and true
        identity = true ;

        if (~isequal (add_class, 'logical'))
            error ('AND monoid must be logical') ;
        end

    case 'xor'

        % x == x xor false
        identity = false ;

        if (~isequal (add_class, 'logical'))
            error ('XOR monoid must be logical') ;
        end

    case 'eq'

        % x == (x == true)
        identity = true ;

        if (~isequal (add_class, 'logical'))
            error ('EQ monoid must be logical') ;
        end

    case 'iseq'

        % x == (x == true)
        identity = true ;

        if (~isequal (add_class, 'logical'))
            error ('ISEQ monoid must be logical') ;
        end

    otherwise

        error ('unsupported additive monoid') ;
end

identity = GB_mex_cast (identity, add_class) ;

