
/edges / {
    print "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"
    print "id = id + 1 ;"
    print "N (id) = ", $2, " ; "
    print "Nedges (id) = ", $5, " ; "
}

/triangles/ {
    print "Ntri (id) = ", $3, " ; "
}

/outer product method/ {
    print "T (id,1) = ", $3, " ; % outer product "
}

/U=triu\(A\) time/ {
    print "Tprep (id,1) = ", $3, " ; % outer product prep"
}


/dot product method/ {
    print "T (id,2) = ", $3, " ;  % dot product "
}

/L=tril\(A\) time/ {
    print "Tprep (id,2) = Tprep (id,1) + ", $3, " ; % dot product prep"
}

