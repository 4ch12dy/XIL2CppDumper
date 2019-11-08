cmake_minimum_required(VERSION 3.15)
project(XIL2CppDumper)

set(CMAKE_CXX_STANDARD 14)

add_executable(XIL2CppDumper main.cpp XB1nLib/xia0.h XIL2CppDumper.cpp XIL2CppDumper.h il2cpp-metadata.h il2cpp-runtime-metadata.h XB1nLib/macho64.h XB1nLib/XB1nLib.c XB1nLib/XB1nLib.h XB1nLib/macho64.h XB1nLib/arch/arm64.h XB1nLib/arch/arm64.c XB1nLib/arch/arm.h XB1nLib/arch/arm.c)