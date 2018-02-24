
k = 10e6 ;
fprintf ('\nbuilding random sparse matrices %d by M\n', k) ;
for m = [1:8 10:2:50 100 1000:1000:10000]
    d = 0.0001 ;
    % A = sprandn (k, m, 0.1) ;
    % B = sprandn (k, m, 0.1) ;
    % Mask = spones (sprandn (m, m, 0.5)) ;
    % A (:,m) = sparse (rand (k,1)) ;
    % B (:,m) = sparse (rand (k,1)) ;

    anz = k*(m-1)*d + m ;
    bnz = k*(m-1)*d + m ;
    cnz = m*m ;

    awork = anz + k + m ;
    bwork = bnz + k + m ;
    cwork = cnz ;

    fprintf ('m %4d %10.6e    %10.6e : %g\n', m, cwork, awork, cwork/awork) ;
end

