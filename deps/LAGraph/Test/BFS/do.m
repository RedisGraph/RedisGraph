
% 
[a2 m2 s2 q2] = bfs2_n36000 ;               % push only
%[ap mp sp  q]= bfs_pushpull_n36000 ;       % push/pull
[ap mp sp  q] = bfs_pull_n36000 ;           % pull only
figure (2) ; what =  ('36000: nd12k') ;
doplots (a2,m2,s2,q2, ap,mp,sp,q, 1.42e7, what)

[a2 m2 s2  q] = bfs2_n9000 ;                % push only
%[ap mp sp q2]= bfs_pushpull_n9000 ;        % push/pull
[ap mp sp q2] = bfs_pull_n9000 ;            % pull only
figure (3) ; what = ('9000: nd3k') ;
doplots (a2,m2,s2,q2, ap,mp,sp,q, 3.3e6, what)

[a2 m2 s2 q2] = bfs2_n1000000 ;             % push only
%[ap mp sp q] = bfs_pushpull_n1000000 ;     % push/pull
[ap mp sp q] = bfs_pull_n1000000 ;          % pull only
figure (4) ; what = ('1000000: eco') ;
doplots (a2,m2,s2,q2, ap,mp,sp,q, 5e6, what)

[a2 m2 s2 q2] = bfs2_n1139905 ;             % push only
%[ap mp sp q] = bfs_pushpull_n1139905 ;     % push/pull
[ap mp sp q] = bfs_pull_n1139905 ;          % pull only
figure (5) ; what = ('1139905: holly') ;
doplots (a2,m2,s2,q2, ap,mp,sp,q, 1.1e8, what)

[a2 m2 s2 q2] = bfs2_n1971281 ;             % push only
%[ap mp sp q] = bfs_pushpull_n1971281 ;     % push/pull
[ap mp sp q] = bfs_pull_n1971281 ;          % pull only
figure (6) ; what = ('1971281: roadCA') ;
doplots (a2,m2,s2,q2, ap,mp,sp,q, 5.5e6, what)

[a2 m2 s2 q2] = bfs2_n2097152 ;             % push only
%[ap mp sp q] = bfs_pushpull_n2097152 ;     % push/pull
[ap mp sp q] = bfs_pull_n2097152 ;          % pull only
figure (7) ; what = ('2097152: kron') ;
doplots (a2,m2,s2,q2, ap,mp,sp,q, 1.8e8, what)

[a2 m2 s2 q2] = bfs2_n7414866 ;             % push only
%[ap mp sp q] = bfs_pushpull_n7414866 ;     % push/pull
[ap mp sp q] = bfs_pull_n7414866 ;          % pull only
figure (8) ; what = ('7414866: indo') ;
doplots (a2,m2,s2,q2, ap,mp,sp,q, 1.94e8, what)

[a2 m2 s2 q2] = bfs2_n4847571 ;             % push only
%[ap mp sp q] = bfs_pushpull_n4847571 ;     % push/pull
[ap mp sp q] = bfs_pull_n4847571 ;          % pull only
figure (9) ; what = ('4847571: livej') ;
doplots (a2,m2,s2,q2, ap,mp,sp,q, 6.9e7, what) 
