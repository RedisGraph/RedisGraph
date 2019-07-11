function flops = flopcount (M,A,B) ;
%FLOPCOUNT returns cumulative sum of flop counts for A*B or C<M>=A*B
%
%   flops = flopcount (M,A,B) ;
%
% flops(j) is the flops to compute A*B(1:j-1), and flops(n+1) is the total
% flopcount, if B is m-by-n.
%
% Each 'flop' counted is actually a multiply-add pair.  M can be [ ]. The
% flopcount m-file returns the same thing as GB_AxB_flopcount.

n = size (B,2) ;
flops = zeros (1,n) ;

if (isempty (M))

    for j = 1:n
        brows = find (B (:,j)) ;
        brows = brows (:)' ;
        for k = brows
            flops (j) = flops (j) + nnz (A (:,k)) ;
        end
    end

else

    for j = 1:n
        brows = find (B (:,j)) ;
        mrows = find (M (:,j)) ;
        if (isempty (brows) || isempty (mrows))
            continue ;
        end
        % mrows
        imin = min (mrows) ;
        imax = max (mrows) ;
        brows = brows (:)' ;
        for k = brows
            [arows ignore] = find (A (:,k)) ;
            if (isempty (arows))
                % A(:,k) is empty
                continue ;
            end
            amin = min (arows) ;
            amax = max (arows) ;
            if (amax < imin || amin > imax)
                % intersection of A(:,k) and M(:,j) is empty
                continue ;
            end
            flops (j) = flops (j) + nnz (A (:,k)) ;
        end
    end

end
flops = cumsum ([0 flops]) ;
