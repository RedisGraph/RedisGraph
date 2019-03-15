function floptest (M, A, B, floplimit, mflops)
%FLOPTEST compare flopcount with GB_mex_mxm_flops
% floptest (M, A, B, floplimit, mflops)
%
% compares the results of
% mflops = flopcount (M, A, B) ;
% with
% [result bflops] = GB_mex_mxm_flops (M,A,B,floplimit)
%
% However, flopcount(M,A,B) can only be computed when M, A, B are all MATLAB
% sparse matrices, not structs.  If the matrices are hypersparse, bflops has
% length B->nvec+1, not size(B,2).  In this case, the last entries of both
% mflops and bflops must match (equal to the total flops), and to do the test,
% mflops(end) is passed to this function in instead of all of mflops.

% get both the result of the test, and bflops
[result bflops] = GB_mex_mxm_flops (M, A, B, floplimit) ;
total_flops = bflops (end) ;
assert (result == (total_flops <= floplimit)) ;
if (isscalar (mflops))
    % mflops is just the total flop count
    assert (isequal (mflops, bflops (end)))
else
    % mflops is the cumulative sum
    assert (isequal (mflops, bflops))
end

% just get the result, not bflops:
result = GB_mex_mxm_flops (M, A, B, floplimit) ;
assert (result == (total_flops <= floplimit)) ;

