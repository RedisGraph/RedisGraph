function [C_replace Mask_comp Atrans Btrans Mask_struct descriptor] = ...
    GB_spec_descriptor (descriptor)
%GB_SPEC_DESCRIPTOR return components of a descriptor
%
% Returns the components of the descriptor struct.  Defaults are used if not
% present, or if the descriptor itself is empty.
%
% desc fields:
%
% outp:  'default' or 'replace'
% mask:  'default' or 'scmp'
% inp0:  'default' or 'tran'
% inp1:  'default' or 'tran'

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (isempty (descriptor))
    descriptor = struct ;
end

if (~isfield (descriptor, 'outp'))
    % use 'replace' to clear C before writing to it on output via the mask.
    % See GB_spec_mask.m for details.
    descriptor.outp = 'default' ;
end
if (~isfield (descriptor, 'mask'))
    % default is to use Mask, not ~Mask if 'scmp'
    descriptor.mask = 'default' ;
end
if (~isfield (descriptor, 'inp0'))
    % default is to use A, or A' if 'tran'
    descriptor.inp0 = 'default' ;
end
if (~isfield (descriptor, 'inp1'))
    % default is to use B, or B' if 'tran'
    descriptor.inp1 = 'default' ;
end

C_replace = isequal (descriptor.outp, 'replace') ;
Atrans    = isequal (descriptor.inp0, 'tran') ;
Btrans    = isequal (descriptor.inp1, 'tran') ;

switch (descriptor.mask)
    case {'scmp', 'complement'}
        Mask_comp = true ;
        Mask_struct = false ;
    case {'structural'}
        Mask_comp = false ;
        Mask_struct = true ;
    case {'structural complement'}
        Mask_comp = true ;
        Mask_struct = true ;
    otherwise
        Mask_comp = false ;
        Mask_struct = false ;
    end
end


