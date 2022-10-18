# SPDX-License-Identifier: Apache-2.0
# Generate test instances from a large tensor product set of options

GB_TYPE_PREFIX = "GrB"

SUPPORTED_TYPES = {
    "int32_t": "INT32",
    "uint32_t": "UINT32",
    "int64_t": "INT64",
    "uint64_t": "UINT64",
    "bool": "BOOL",
    "float": "FP32",
    "double": "FP64"
}

DOT3_BUCKETS = [1, 2]    # NBUCKETS, hard-coded

DataShapes ={
    "nanoxnano": {'N':32, 'Anz':64, 'Bnz':56, 'Cnz': 256},
    "tinyxtiny": {'N':128, 'Anz':1256, 'Bnz':1028, 'Cnz': 1640},
    "smallxsmall": {'N':1024, 'Anz': 65_536, 'Bnz':65_536, 'Cnz': 10000},
    "ti_denxti_den": {'N':32, 'Anz':1024, 'Bnz':1024, 'Cnz': 1024},
    "ti_spaxti_den": {'N':32, 'Anz':256, 'Bnz':1024, 'Cnz': 1024},
    "medxmed": {'N':4096, 'Anz': 2**20, 'Bnz':2**20},
    "largexlarge": {'N':2**16, 'Anz': 64*2**20, 'Bnz':64*2**20}
}

FORMATS = { "sparse": ["phase1", "phase2", "mxm_sparse"],
            "dense": ["dense_phase1", "mxm_dense"],
            "sparse_dense": ["dense_phase1", "mxm_sparse_dense"],
            "reduce": ["reduce"]}

FORMAT_INPUTS = {
    "sparse": [("GxB_SPARSE", "GxB_SPARSE")],
    "dense": [("GxB_FULL", "GxB_FULL"), ("GxB_BITMAP", "GxB_BITMAP")],
    "sparse_dense": [("GxB_SPARSE", "GxB_FULL")],
    "reduce": [("GxB_SPARSE", "GxB_SPARSE")]
}

FORMAT_DATASETS = {
    "sparse": ["nanoxnano", "tinyxtiny", "smallxsmall"],
    "dense": ["ti_denxti_den"],
    "sparse_dense": ["ti_spaxti_den"],
    "reduce": ["nanoxnano", "smallxsmall", "ti_denxti_den", "ti_spaxti_den"]
}

def std_type_to_gb_type(t):
    return SUPPORTED_TYPES[t]

def build_gb_monioid(t, m):
    # Example: GrB_PLUS_MONIOD_UINT64
    gb_type = std_type_to_gb_type(t)
    return f"{GB_TYPE_PREFIX}_{m}_MONOID_{gb_type}"

def build_gb_binop(t, b):
    # Example: GrB_TIMES_UINT64
    gb_type = std_type_to_gb_type(t)
    return f"{GB_TYPE_PREFIX}_{b}_{gb_type}"




def buildTest(ts="TestsuiteName", ds="tiny-tiny", df=("GxB_SPARSE", "GxB_SPARSE"),
              SUM="PLUS", PRODUCT="TIMES",
              typeC="int32_t",typeM="int32_t",
              typeA="int32_t",typeB="int32_t",
              type_x="int32_t", type_y="int32_t",type_z="int32_t"):

    # build string interpolation from pieces
    format_A, format_B = df

    Test_name = f"{ds}{SUM}_{PRODUCT}__{format_A}_{format_B}__C{typeC}M{typeM}A{typeA}B{typeB}X{type_x}Y{type_y}Z{type_z}"
    Test_suite = f"{ts}"

    N = DataShapes[ds]['N']
    Anz = DataShapes[ds]['Anz']
    Bnz = DataShapes[ds]['Bnz']
    Cnz = DataShapes[ds]['Cnz']

    gb_monoid = build_gb_monioid(typeC, SUM)
    gb_binop = build_gb_binop(typeC, PRODUCT)

    TEST_HEAD = f"""
    TEST( {Test_suite}, {Test_name}) {{

        /**************************
         * Create reference and input data
         */
        GrB_Monoid monoid = {gb_monoid}; 
        GrB_BinaryOp binop = {gb_binop};

        mxm_problem_spec<{typeC}, {typeM}, {typeA}, {typeB}> problem_spec(monoid, binop, {N}, {Anz}, {Bnz}, {Cnz},
                                                                          {format_A}, {format_B});
    """
    phase1_body= f""" test_AxB_phase1_factory< {typeC}, {typeM}, {typeA}, {typeB}>(problem_spec);"""
    phase2_body= f""" test_AxB_phase2_factory< {typeC}, {typeM}, {typeA}, {typeB} >(problem_spec);"""
    dense_phase1_body = f""" test_AxB_dense_phase1_factory<{typeC}, {typeM}, {typeA}, {typeB}>(problem_spec);"""
    mxm_sparse_body = f""" test_AxB_dot3_sparse_factory< {typeC},{typeM},{typeA},{typeB},{type_x},{type_y},{type_z} > (problem_spec);\n"""
    mxm_dense_body = f""" test_AxB_dot3_dense_factory< {typeC},{typeM},{typeA},{typeB},{type_x},{type_y},{type_z} > (problem_spec);\n"""
    mxm_sparse_dense_body = f""" test_AxB_dot3_sparse_dense_factory< {typeC},{typeM},{typeA},{typeB},{type_x},{type_y},{type_z} > (problem_spec);\n"""
    reduce_body = f""" test_reduce_factory<{typeC}, {typeM}, {typeA}, {typeB}>(problem_spec);"""
    phasedict = { "phase1": phase1_body,
                  "phase2": phase2_body,
                  "mxm_sparse": mxm_sparse_body,
                  "mxm_dense": mxm_dense_body,
                  "mxm_sparse_dense": mxm_sparse_dense_body,
                  "reduce": reduce_body,
                  "dense_phase1": dense_phase1_body }

    return TEST_HEAD, phasedict

def load_types(argv):
    test_suite_name = argv[2]
    Monoids = argv[3].split(";")
    Binops  = argv[4].split(";")
    Semirings = argv[5]
    DataTypes = argv[6].split(";")

    # Hard-coding data shapes for now
    Kernels= argv[7]

    return argv[1], test_suite_name, Monoids, Binops, Semirings, DataTypes, DataShapes, Kernels

def write_test_instances_header(test_suite_name, mat_format, tests, Monoids, Binops, Semirings, DataTypes, DataShapes, Kernels):
    outfile = f'{test_suite_name}_{Semirings}_{mat_format}_test_instances.hpp'
    with open(outfile, 'w') as fp:
        fp.write("#pragma once\n#include \"problem_spec.hpp\"\n");
        m, b = Semirings.split("_")
        Test_suite = f'{test_suite_name}_tests_{mat_format}_{m}_{b}'
        for dtC in DataTypes:
            dtX = dtC
            dtY = dtC
            dtZ = dtC
            for dtM in ["bool", "int32_t", "int64_t", "float", "double"]:
                for dtA in DataTypes:
                    for dtB in DataTypes:
                        for ds in FORMAT_DATASETS[mat_format]:
                            for df in FORMAT_INPUTS[mat_format]:
                                TEST_HEAD, TEST_BODY = buildTest( Test_suite, ds, df, m, b,
                                                                  dtC, dtM, dtA, dtB, dtX, dtY, dtZ)
                                fp.write( TEST_HEAD)
                                for test in tests:
                                    fp.write( TEST_BODY[test] )
                                fp.write( "}\n")

def write_cuda_test(source_dir, test_suite_name, mat_format, semiring, kernel):
    import shutil

    shutil.copy(f"{source_dir}/test/cuda_tests_template.cpp", f"{test_suite_name}_{semiring}_{mat_format}_cuda_tests.cpp")

    with open(f"{test_suite_name}_{semiring}_{mat_format}_cuda_tests.cpp", "a") as file_object:
        # Keeping this as a separate file for now to allow for further nesting
        # of test instances for each test_suite_name
        file_object.write(f"\n#include \"{test_suite_name}_{semiring}_{mat_format}_test_instances.hpp\"")

if __name__ == "__main__":
    import sys

    if(len(sys.argv) != 8):
        raise ValueError("Expected 7 arguments but only got %s" % len(sys.argv))

    """
    First load values
    """
    source_dir, test_suite_name, Monoids, Binops, Semirings, DataTypes, DataShapes, Kernels = load_types(sys.argv)

    for mat_format, tests in FORMATS.items():
        write_test_instances_header(test_suite_name, mat_format, tests, Monoids, Binops, Semirings, DataTypes, DataShapes, DOT3_BUCKETS)
        write_cuda_test(source_dir, test_suite_name, mat_format, Semirings, Kernels)
