cmake_minimum_required(VERSION 3.15)
project(SearchEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(search_engine src/main.cpp)
add_executable(pdf_extractor src/pdf_extractor.cpp)
add_executable(build_index src/build_index.cpp)
add_executable(test_bm25 src/test_bm25.cpp)
add_executable(test_trie src/test_trie.cpp)

set(pybind11_DIR "C:/Users/divav/AppData/Local/Python/pythoncore-3.14-64/Lib/site-packages/pybind11/share/cmake/pybind11")

set(PYTHON_EXECUTABLE "C:/Users/divav/AppData/Local/Python/pythoncore-3.14-64/python.exe" CACHE FILEPATH "" FORCE)
set(PYTHON_INCLUDE_DIR "C:/Users/divav/AppData/Local/Python/pythoncore-3.14-64/Include" CACHE PATH "" FORCE)
set(PYTHON_LIBRARY "C:/Users/divav/AppData/Local/Python/pythoncore-3.14-64/libs/python314.lib" CACHE FILEPATH "" FORCE)

find_package(pybind11 REQUIRED)

pybind11_add_module(search_engine_cpp src/bindings.cpp)