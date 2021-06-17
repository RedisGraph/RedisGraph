function C = gb_diag (A, k)
%GB_DIAG Diagonal matrices and diagonals of a GraphBLAS matrix.
% Implements C = diag (A,k)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

[am, an, atype] = gbsize (A) ;
a_is_vector = (am == 1) || (an == 1) ;
desc.base = 'zero-based' ;

if (a_is_vector)

    % C = diag (v,k) is an m-by-m matrix if v is a vector
    n = am * an ;
    m = n + abs (k) ;

    if (am == 1)
        % A is a row vector
        if (k >= 0)
            [~, I, X] = gbextracttuples (A, desc) ;
            J = I + int64 (k) ;
        else
            [~, J, X] = gbextracttuples (A, desc) ;
            I = J - int64 (k) ;
        end
    else
        % A is a column vector
        if (k >= 0)
            [I, ~, X] = gbextracttuples (A, desc) ;
            J = I + int64 (k) ;
        else
            [J, ~, X] = gbextracttuples (A, desc) ;
            I = J - int64 (k) ;
        end
    end

    C = gbbuild (I, J, X, m, m, desc) ;

else

    % C = diag (A,k) is a column vector formed from the elements of the kth
    % diagonal of A

    if (k >= 0)
        m = min (an-k, am) ;
    else
        m = min (an, am+k) ;
    end

    if (m == 0)

        % A does not have a kth diagonal so C is empty
        I = [ ] ;

    else

        % extract the kth diagonal from A and convert into a column vector
        C = gbselect ('diag', A, k) ;
        if (k >= 0)
            [I, ~, X] = gbextracttuples (C, desc) ;
        else
            [~, I, X] = gbextracttuples (C, desc) ;
        end

    end

    if (isempty (I))
        % A does not have a kth diagonal, or diag (A,k) has no entries
        C = gbnew (m, 1, atype) ;
    else
        C = gbbuild (I, int64 (0), X, m, 1, desc) ;
    end

end

