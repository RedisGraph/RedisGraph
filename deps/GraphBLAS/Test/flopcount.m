function [flops mwork] = flopcount (M,Mask_complement,A,B) ;
%FLOPCOUNT cumulative sum of flop counts for A*B, C<M>=A*B, C<!M>=A*B
%
% flops = flopcount (M,Mask_complementA,B) ;
%
% flops(j) is the flops to compute A*B(1:j-1), and flops(n+1) is the total
% flopcount, if B is m-by-n.
%
% Each 'flop' counted is actually a multiply-add.  M can be [ ]. The
% flopcount m-file returns the same thing as GB_AxB_flopcount.  Also
% included in flops(j) is the work needed to access the mask M(:,j).

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

n = size (B,2) ;
flops = zeros (1,n) ;
mwork = 0 ;

if (isempty (M))

    % C = A*B

    for j = 1:n
        brows = find (B (:,j)) ;
        brows = brows (:)' ;
        for k = brows
            flops (j) = flops (j) + nnz (A (:,k)) ;
        end
    end

else

    % C<M>=A*B and C<!M>=A*B

    mask_is_M = (~Mask_complement) ;    % true for C<M>=A*B

    for j = 1:n
        brows = find (B (:,j)) ;
        if (isempty (brows))
            continue ;
        end
        mrows = find (M (:,j)) ;
        mjnz = length (mrows) ;
        
        if (mask_is_M & mjnz == 0)
            continue ;
        end
        imin = min (mrows) ;
        imax = max (mrows) ;
        brows = brows (:)' ;

        flops (j) = flops (j) + mjnz ;
        mwork = mwork + mjnz ;

        for k = brows
            [arows ignore] = find (A (:,k)) ;
            if (isempty (arows))
                % A(:,k) is empty
                continue ;
            end
            if (mask_is_M)
                amin = min (arows) ;
                amax = max (arows) ;
                if (amax < imin || amin > imax)
                    % intersection of A(:,k) and M(:,j) is empty
                    continue ;
                end
            end
            flops (j) = flops (j) + nnz (A (:,k)) ;
        end
    end

end

flops = cumsum ([0 flops]) ;
