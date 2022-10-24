function [tM tG] = summary (tm, tg, ktrials)
tM = median (tm (1:ktrials)) ;
tG = median (tg (1:ktrials)) ;
