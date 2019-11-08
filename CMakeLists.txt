cmake_minimum_required(VERSION 3.15)
project(XIL2CppDumper)

set(CMAKE_CXX_STANDARD 14)

add_executable(XIL2CppDumper main.cpp xia0-bin-lib/xia0.h XIL2CppDumper.cpp XIL2CppDumper.h il2cpp-metadata.h il2cpp-runtime-metadata.h xia0-bin-lib/macho64.h xia0-bin-lib/xia0-bin-lib.c xia0-bin-lib/xia0-bin-lib.h xia0-bin-lib/macho64.h xia0-bin-lib/arch/arm64.h xia0-bin-lib/arch/arm64.c xia0-bin-lib/arch/arm.h xia0-bin-lib/arch/arm.c)