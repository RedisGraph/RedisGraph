% test kron.c by comparing it with MATLAB
%
% usage: kron_test

clear
system ('make kron ; ./kron a.tsv b.tsv c.tsv') ;
load a.tsv
load b.tsv
load c.tsv

A = spconvert (a) ;
B = spconvert (b) ;
C = kron (A,B) ;

C1 = spconvert (c) ;
err = norm (C-C1, 1) ;
assert (err == 0) ;

for np = 1:16
    fprintf ('\nnp: %d\n', np) ;
    cc = zeros (0,3) ;
    for pid = 0:np-1
        system (sprintf ('./kron a.tsv b.tsv cc.tsv %d %d', np, pid)) ;
        if (np > 1)
            myc = sprintf ('%d_cc.tsv', pid) ;
        else
            myc = 'cc.tsv' ;
        end
        c_pid = load (myc) ;
        delete (myc) ;
        cc = [cc ; c_pid] ;
    end
    C2 = spconvert (cc) ;
    err = norm (C-C2, 1) ;
    assert (err == 0) ;
end
