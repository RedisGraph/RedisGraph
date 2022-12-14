
#include <vector>
#include <cstdint>

#pragma once

template<typename T_A, typename T_B, typename T_C, typename T_M>
class TestData {

public:
    TestData(  std::vector<std::int64_t> A_indptr_,
    std::vector<std::int64_t> A_indices_,
    std::vector<T_A> A_data_,

    std::vector<std::int64_t> B_indptr_,
    std::vector<std::int64_t> B_indices_,
    std::vector<T_B> B_data_,


    std::vector<std::int64_t> C_indptr_,
    std::vector<std::int64_t> C_indices_,
    std::vector<T_C> C_data_,

    std::vector<std::int64_t> M_indptr_,
    std::vector<std::int64_t> M_indices_,
    std::vector<T_M> M_data_):
        A_indptr(A_indptr_), A_indices(A_indices_), A_data(A_data_),
        B_indptr(B_indptr_), B_indices(B_indices_), B_data(B_data_),
        C_indptr(C_indptr_), C_indices(C_indices_), C_data(C_data_),
        M_indptr(M_indptr_), M_indices(M_indices_), M_data(M_data_){}


  std::vector<std::int64_t> A_indptr;
  std::vector<std::int64_t> A_indices;
  std::vector<T_A> A_data;
  
  std::vector<std::int64_t> B_indptr;
  std::vector<std::int64_t> B_indices;
  std::vector<T_B> B_data;
  
  
  std::vector<std::int64_t> C_indptr;
  std::vector<std::int64_t> C_indices;
  std::vector<T_C> C_data;

  std::vector<std::int64_t> M_indptr;
  std::vector<std::int64_t> M_indices;
  std::vector<T_M> M_data;

};

template<typename T_A, typename T_B, typename T_C, typename T_M>
std::unique_ptr<TestData<T_A, T_B, T_C, T_M>> make_karate_tricount() {

    std::vector<std::int64_t> A_indptr = { 0,16,24,32,35,37,40,41,41,44,45,45,45,45,46,48,50,50,50,52,53,55,55,57,
 62,65,66,68,69,71,73,75,77,78,78};
    std::vector<std::int64_t> A_indices = { 1, 2, 3, 4, 5, 6, 7, 8,10,11,12,13,17,19,21,31, 2, 3, 7,13,17,19,21,30,
  3, 7, 8, 9,13,27,28,32, 7,12,13, 6,10, 6,10,16,16,30,32,33,33,33,32,33,
 32,33,32,33,33,32,33,32,33,25,27,29,32,33,25,27,31,31,29,33,33,31,33,32,
 33,32,33,32,33,33};
    std::vector<T_A> A_data = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
 1,1,1,1};

    std::vector<std::int64_t> B_indptr = { 0, 0, 1, 3, 6, 7, 8,11,15,17,18,21,22,24,28,28,28,30,32,32,34,34,36,36,
 36,36,38,38,41,42,44,46,50,61,78};
    std::vector<std::int64_t> B_indices = { 0, 0, 1, 0, 1, 2, 0, 0, 0, 4, 5, 0, 1, 2, 3, 0, 2, 2, 0, 4, 5, 0, 0, 3,
  0, 1, 2, 3, 5, 6, 0, 1, 0, 1, 0, 1,23,24, 2,23,24, 2,23,26, 1, 8, 0,24,
 25,28, 2, 8,14,15,18,20,22,23,29,30,31, 8, 9,13,14,15,18,19,20,22,23,26,
 27,28,29,30,31,32};
    std::vector<T_B> B_data = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
 1,1,1,1};

    std::vector<std::int64_t> M_indptr = {  0, 16, 25, 35, 41, 44, 48, 52, 56, 61, 63, 66, 67, 69, 74, 76, 78, 80,
  82, 84, 87, 89, 91, 93, 98,101,104,106,110,113,117,121,127,139,156};
    std::vector<std::int64_t> M_indices = { 1, 2, 3, 4, 5, 6, 7, 8,10,11,12,13,17,19,21,31, 0, 2, 3, 7,13,17,19,21,
 30, 0, 1, 3, 7, 8, 9,13,27,28,32, 0, 1, 2, 7,12,13, 0, 6,10, 0, 6,10,16,
  0, 4, 5,16, 0, 1, 2, 3, 0, 2,30,32,33, 2,33, 0, 4, 5, 0, 0, 3, 0, 1, 2,
  3,33,32,33,32,33, 5, 6, 0, 1,32,33, 0, 1,33,32,33, 0, 1,32,33,25,27,29,
 32,33,25,27,31,23,24,31,29,33, 2,23,24,33, 2,31,33,23,26,32,33, 1, 8,32,
 33, 0,24,25,28,32,33, 2, 8,14,15,18,20,22,23,29,30,31,33, 8, 9,13,14,15,
 18,19,20,22,23,26,27,28,29,30,31,32};
    std::vector<T_M> M_data = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
 1,1,1,1,1,1,1,1};

    std::vector<std::int64_t> C_indptr = { 0, 0, 7,12,17,19,21,24,27,29,29,31,31,32,35,35,35,36,37,37,38,38,39,39,
 39,39,40,40,41,41,43,45,47,51,56};
    std::vector<std::int64_t> C_indices = { 2, 3, 7,13,17,19,21, 1, 3, 7, 8,13, 1, 2, 7,12,13, 6,10, 6,10, 4, 5,16,
  1, 2, 3, 2,32, 4, 5, 3, 1, 2, 3, 6, 1, 1, 1,31,33,32,33,32,33,25,33, 8,
 29,30,33,27,29,30,31,32};
    std::vector<T_C> C_data = { 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 2, 1, 2, 3, 1, 3, 1, 1, 1, 1, 1, 1, 1,
  1, 2, 3, 1, 1, 1, 1, 1, 1, 2, 3, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1,
  1, 1,10, 1, 2, 1, 1,10};

    TestData<T_A, T_B, T_C, T_M> karate_tricount(A_indptr, A_indices, A_data,
                                                 B_indptr, B_indices, B_data,
                                                 C_indptr, C_indices, C_data,
                                                 M_indptr, M_indices, M_data);

    return std::make_unique<TestData<T_A, T_B, T_C, T_M>>(karate_tricount);
}

