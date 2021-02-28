% MATLAB interface for SuiteSparse:GraphBLAS
%
% GraphBLAS is a library for creating graph algorithms based on sparse linear
% algebraic operations over semirings.  Its MATLAB interface provides faster
% sparse matrix operations than the built-in methods in MATLAB, as well as
% sparse integer and single-precision matrices, and operations with arbitrary
% semirings.  See 'help GrB' for details.
%
% The constructor method is GrB.  If A is any matrix (GraphBLAS, MATLAB sparse
% or full), then:
%
%   C = GrB (A) ;            GraphBLAS copy of a matrix A, same type
%   C = GrB (m, n) ;         m-by-n GraphBLAS double matrix with no entries
%   C = GrB (..., type) ;    create or typecast to a different type
%   C = GrB (..., format) ;  create in a specified format
%
% The type can be 'double', 'single', 'logical', 'int8', 'int16', 'int32',
% 'int64', 'uint8', 'uint16', 'uint32', or 'uint64'.  The format is 'by row' or
% 'by col'.  
%
% Methods that overload the MATLAB function of the same name; at least
% one of the inputs must be a GraphBLAS matrix:
%
%                   fix             isreal          single
%   abs             flip            isscalar        size
%   all             floor           issparse        sparse
%   amd             fprintf         issymmetric     spfun
%   and             full            istril          spones
%   any             graph           istriu          sprand
%   assert          int16           isvector        sprandn
%   bandwidth       int32           kron            sprandsym
%   ceil            int64           length          sprintf
%   colamd          int8            logical         sqrt
%   complex         isa             max             sum
%   conj            isbanded        min             symamd
%   diag            isdiag          nnz             symrcm
%   digraph         isempty         nonzeros        tril
%   disp            isequal         norm            triu
%   display         isfinite        numel           true
%   dmperm          isfloat         nzmax           uint16
%   double          ishermitian     ones            uint32
%   eig             isinf           prod            uint64
%   end             isinteger       real            uint8
%   eps             islogical       repmat          xor
%   etree           ismatrix        reshape         zeros
%   false           isnan           round
%   find            isnumeric       sign
%
% Operator overloading (A and/or B a GraphBLAS matrix, C a GraphBLAS matrix):
%
%     A+B    A-B   A*B    A.*B   A./B   A.\B   A.^b    A/b    C=A(I,J)
%     -A     +A    ~A     A'     A.'    A&B    A|B     b\A    C(I,J)=A
%     A~=B   A>B   A==B   A<=B   A>=B   A<B    [A,B]   [A;B]  
%     A(1:end,1:end)
%
% These built-in MATLAB methods also work with any GraphBLAS matrices:
%
%   cast flipdim fliplr flipud isrow iscolumn ndims sprank etreeplot spy gplot
%   bicgstabl bicgstab cgs minres gmres bicg pcg qmr rjr tfqmr lsqr
%
% Static Methods: used as GrB.method; inputs can be any GraphBLAS or
% MATLAB matrix, in any combination.
%
%   init            finalize        burble
%   apply           emult           kronecker       select          
%   assign          entries         ktruss          selectopinfo    
%   bfs             expand          laplacian       semiringinfo    
%   binopinfo       extract         mis             speye           
%   build           extracttuples   monoidinfo      subassign       
%   chunk           eye             mxm             threads         
%   clear           format          nonz            trans           
%   compact         incidence       offdiag         tricount        
%   descriptorinfo  isbycol         pagerank        type            
%   dnn             isbyrow         prune           unopinfo        
%   eadd            isfull          random          vreduce         
%   empty           issigned        reduce 
%
% Tim Davis, Texas A&M University, http://faculty.cse.tamu.edu/davis/GraphBLAS

