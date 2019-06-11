
% Test lcc.m.  Requires ssget.

clear
index = ssget ;
f = find (index.nrows == index.ncols) ;
[~,i] = sort (index.nnz (f)) ;
f = f (i) ;

% f = [739 750 2662];
% f = [168 739 750] ; 
% f = [168] 
% f = [ 262 2459 ] ;

%   matrices = {
%   'HB/west0067'
%   'DIMACS10/road_usa'
%   'SNAP/roadNet-CA'
%   'McRae/ecology1'
%   'ND/nd3k'
%   'ND/nd12k'
%   'SNAP/soc-LiveJournal1'
%   'LAW/hollywood-2009'
%   'LAW/indochina-2004'
%   'DIMACS10/kron_g500-logn21' } ;

matrices = f ;
nmat = length (matrices)

for kk = 1:nmat % 1022:nmat

    % get a matrix
    matrix = matrices (kk) ;
    try
    Prob = ssget (matrix, index) ;
    catch
        continue
    end

    id = Prob.id ;
    A = Prob.A ;
    if (isfield (Prob, 'Zeros')) 
        A = A + Prob.Zeros ;
    end
    fprintf ('\n%4d %4d %-40s\n', kk, id, Prob.name) ;
    clear Prob 
    if (~isreal (A))
        A = spones (A) ;
    end

    % compute the result in MATLAB
    c1 = lcc (A) ;

    % compute the result in GraphBLAS
    c2 = lcc_graphblas (A) ;

    err = norm (c1-c2,1) ;
    if (err > 1e-12)
        err
        error ('!')
    end
end

