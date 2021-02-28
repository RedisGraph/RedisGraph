%TTEST run triangle counting tests

index = ssget ;
f = find (index.nnz > 1e6 & index.nrows == index.ncols) ;
[ignore i] = sort (index.nnz (f)) ;
f = f (i) ;
nmat = length (f) ;

winners = zeros (12,1) ;
totals = zeros (12,1) ;

for k = 1:nmat
    id = f (k) ;
    Prob = ssget (id, index)
    A = spones (Prob.A) ;
    [m, n] = size (A) ;
    if (m ~= n)
        A = [speye(m) A ; A' speye(n)] ;
    else
        A = A|A' ;
    end

    [s, times, best] = tric (A) ;
    winners (best) = winners (best) + 1 ;

    totals = totals + times ;

    fprintf ('\nwinner count:\n') ;
    for trial = 1:12
        fprintf ('  %2d : %12.2f  %d\n', trial, totals (trial), ...
            winners (trial)) ;
    end

end


