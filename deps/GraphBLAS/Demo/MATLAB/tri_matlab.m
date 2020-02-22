%TRI_MATLAB run tricount tests in MATLAB

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

clear
diary tri_matlab_out.txt

files = {
'/research/davisgroup/GraphChallenge/ssget/Mallya/lhr71_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/ssget/Freescale/Freescale2_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/cit-HepPh/cit-HepPh_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/cit-HepTh/cit-HepTh_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/email-EuAll/email-EuAll_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/soc-Epinions1/soc-Epinions1_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/soc-Slashdot0811/soc-Slashdot0811_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/soc-Slashdot0902/soc-Slashdot0902_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/amazon0312/amazon0312_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/amazon0505/amazon0505_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/amazon0601/amazon0601_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/flickrEdges/flickrEdges_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/cit-Patents/cit-Patents_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/ssget/SNAP/soc-LiveJournal1_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/ssget/Gleich/wb-edu_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/amazon0302/amazon0302_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/as-caida20071105/as-caida20071105_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/as20000102/as20000102_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/ca-AstroPh/ca-AstroPh_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/ca-CondMat/ca-CondMat_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/ca-GrQc/ca-GrQc_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/ca-HepPh/ca-HepPh_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/ca-HepTh/ca-HepTh_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/email-Enron/email-Enron_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/facebook_combined/facebook_combined_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/loc-brightkite_edges/loc-brightkite_edges_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/loc-gowalla_edges/loc-gowalla_edges_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/oregon1_010331/oregon1_010331_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/oregon1_010407/oregon1_010407_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/oregon1_010414/oregon1_010414_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/oregon1_010421/oregon1_010421_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/oregon1_010428/oregon1_010428_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/oregon1_010505/oregon1_010505_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/oregon1_010512/oregon1_010512_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/oregon1_010519/oregon1_010519_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/oregon1_010526/oregon1_010526_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/oregon2_010331/oregon2_010331_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/oregon2_010407/oregon2_010407_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/oregon2_010414/oregon2_010414_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/oregon2_010421/oregon2_010421_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/oregon2_010428/oregon2_010428_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/oregon2_010505/oregon2_010505_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/oregon2_010512/oregon2_010512_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/oregon2_010519/oregon2_010519_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/oregon2_010526/oregon2_010526_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/p2p-Gnutella04/p2p-Gnutella04_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/p2p-Gnutella05/p2p-Gnutella05_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/p2p-Gnutella06/p2p-Gnutella06_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/p2p-Gnutella08/p2p-Gnutella08_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/p2p-Gnutella09/p2p-Gnutella09_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/p2p-Gnutella24/p2p-Gnutella24_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/p2p-Gnutella25/p2p-Gnutella25_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/p2p-Gnutella30/p2p-Gnutella30_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/p2p-Gnutella31/p2p-Gnutella31_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/roadNet-CA/roadNet-CA_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/roadNet-PA/roadNet-PA_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/snap/roadNet-TX/roadNet-TX_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/synthetic/image-grid/g-1045506-262144_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/synthetic/image-grid/g-16764930-4194304_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/synthetic/image-grid/g-260610-65536_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/synthetic/image-grid/g-268386306-67108864_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/synthetic/image-grid/g-4188162-1048576_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/ssget/DIMACS10/hugebubbles-00020_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/ssget/vanHeukelum/cage15_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/synthetic/graph500-scale18-ef16/graph500-scale18-ef16_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/synthetic/graph500-scale19-ef16/graph500-scale19-ef16_adj.tsv.gz'} ;

% out of memory (kills MATLAB on a 16GB laptop):
files = {
'/research/davisgroup/GraphChallenge/ssget/Freescale/circuit5M_adj.tsv.gz' 
'/research/davisgroup/GraphChallenge/synthetic/graph500-scale20-ef16/graph500-scale20-ef16_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/synthetic/graph500-scale21-ef16/graph500-scale21-ef16_adj.tsv.gz'
'/research/davisgroup/GraphChallenge/synthetic/graph500-scale22-ef16/graph500-scale22-ef16_adj.tsv.gz' } ;

nfiles = length (files) ;
for k = 1:nfiles
    filename = files {k} ;
    fprintf ('\nMatrix: %s\n', filename) ;
    system (sprintf ('gunzip -c %s > /tmp/adj', filename)) ;

    A = spconvert (load ('/tmp/adj')) ;

    % [ntri t] = tricount ('Sandia', A) ;
    tic
    U = triu (A, 1) ;
    tprep = toc ;
    clear A
    tic
    ntri = sum (sum (U*U .*U)) ;
    ttri = toc ;

    ttot = tprep + ttri ;

    fprintf ('triangles %d prep %g tri %g total %g rate %g\n', ...
        full (ntri), tprep, ttri, ttot, 1e-6*nnz(U)/ttot) ;

    clear U
    diary off
    diary on
end


