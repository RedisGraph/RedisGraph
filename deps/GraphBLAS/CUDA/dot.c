
consider these methods on the GPU: (see ../Source/Template/GB_AxB_dot_cij.c)

            while (pA < pA_end && pB < pB_end)
            {
                int64_t ia = Ai [pA] ;
                int64_t ib = Bi [pB] ;

                #if 0
                if (ia == ib)
                {
                    GB_DOT (ia, pA, pB) ;
                    pA++ ;
                    pB++ ;
                }
                else
                { 
                    pA += (ia < ib) ;
                    pB += (ib < ia) ;
                }
                #endif

                #if 0
                // this might be fastest on the GPU
                #if GB_IS_PLUS_PAIR_REAL_SEMIRING && GB_CTYPE_IGNORE_OVERFLOW
                cij += (ia == ib) ;
                pA += (ia <= ib) ;
                pB += (ib <= ia) ;
                #endif
                #endif
            }
