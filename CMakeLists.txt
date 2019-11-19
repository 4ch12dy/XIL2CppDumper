cmake_minimum_required(VERSION 3.15)
project(XIL2CppDumper)

set(CMAKE_CXX_STANDARD 14)

add_executable(XIL2CppDumper main.cpp XB1nLib/xia0.h XIL2CppDumper.cpp XIL2CppDumper.h XB1nLib/macho64.h XB1nLib/XB1nLib.c XB1nLib/XB1nLib.h XB1nLib/macho64.h XB1nLib/arch/arm64.h XB1nLib/arch/arm64.c XB1nLib/arch/arm.h XB1nLib/arch/arm.c xia0.h il2cpp-header/il2cpp-misc.h XB1nLib/exec_elf.h IL2CppBinParser.h IL2CppBinParser.cpp)