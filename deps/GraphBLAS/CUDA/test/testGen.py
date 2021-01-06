# SPDX-License-Identifier: Apache-2.0 
# Generate test instances from a large tensor product set of options

Monoids = ["PLUS","MIN","MAX","TIMES","ANY"]
Binops  = ["TIMES", "PLUS", "MIN", "MAX", "DIV","MINUS", "RDIV","RMINUS","FIRST","SECOND","PAIR"]
Semirings = ["PLUS_TIMES", "MIN_PLUS", "MAX_PLUS"]
#Semirings = ["PLUS_TIMES"]#,"MIN_PLUS"] #, "MAX_PLUS"]

#DataTypes = ["bool","int8_t","int16_t", "int32_t", "int64_t",
#                    "uint8_t", "uint16_t", "uint32_t", "uint64_t",
#                    "float","double"]
DataTypes = ["int32_t", "int64_t", "uint32_t","uint64_t","float","double"]
#DataTypes = ["float","double"]
DataTypes = ["int32_t","uint64_t"]

DataShapes ={
             "tinyxtiny": {'N':32, 'Anz':256, 'Bnz':128}, 
             "smallxsmall": {'N':1024, 'Anz': 65_536, 'Bnz':65_536}
            # "medxmed": {'N':4096, 'Anz': 2**20, 'Bnz':2**20} 
            # "largexlarge": {'N':2**16, 'Anz': 64*2**20, 'Bnz':64*2**20} 
             }

Kernels= ["warp","mp", "vsvs","dndn", "spdn","vssp"]
Kernels= ["warp"] #, "vsvs","dndn", "spdn","vssp"]



def buildTest(ts="TestsuiteName",kern="vsvs", ds= "tiny-tiny", SR = "PLUS_TIMES",phase=3, 
              typeC="int",typeM="int",typeA="int",typeB="int",type_x="int",type_y="int",type_z="int"):

    # build string interpolation from pieces
    Test_name = f"{ds}{SR}C{typeC}M{typeM}A{typeA}B{typeB}X{type_x}Y{type_y}Z{type_z}"

    Test_suite = ts
    #print(Test_suite)
    TEST_HEAD = f"""TEST( {Test_suite}, {Test_name})""" 
    #print(TEST_HEAD)
    N = DataShapes[ds]['N']
    Anz = DataShapes[ds]['Anz']
    Bnz = DataShapes[ds]['Bnz']
    phase1_body= f""" test_AxB_dot3_phase1_factory< {typeC}, {typeM}, {typeA}, {typeB}>( 5, {N}, {Anz},{Bnz});""" 
    phase2_body= f""" test_AxB_dot3_phase2_factory< {typeC} >( 5, {N}, {Anz},{Bnz});"""  
    phase3_body = f""" test_AxB_dot3_{kern}_factory< {typeC},{typeM},{typeA},{typeB},{type_x},{type_y},{type_z} > (5, {N}, {Anz}, {Bnz}, SR);"""
    #print( TEST_BODY)
    phasedict = { 1: phase1_body, 2: phase2_body, 3: phase3_body}
    TEST_BODY= phasedict[phase] 

    return TEST_HEAD,TEST_BODY 


if __name__ == "__main__":


   #print( buildTest()) #test if anything works

    
   outfile = f"""AxB_dot3_test_instances.hpp""" 
   fp = open(outfile, 'w')


   for k in Kernels:
       Test_suite = f'AxB_dot3_tests_{k}'
       for SR in Semirings:
           for dtC in DataTypes:
               dtX = dtC 
               dtY = dtC 
               dtZ = dtC
               for dtM in ["bool", "int32_t"]: 
                   for dtA in DataTypes:
                       for dtB in DataTypes:
                           for ds in DataShapes:
                               for phase in [3]: 

                                   TEST_HEAD, TEST_BODY = buildTest( Test_suite, k, ds, SR, phase,
                                                                     dtC, dtM, dtA, dtB, dtX, dtY, dtZ)  
                                   fp.write( TEST_HEAD)
                                   fp.write( """{ std::string SR = "%s"; """%SR)
                                   fp.write( TEST_BODY)
                                   fp.write( "}\n")

          
   fp.close()

