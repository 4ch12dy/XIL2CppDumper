//
// Created by xia0 on 2019/11/13.
//

#ifndef XIL2CPPDUMPER_XIA0_H
#define XIL2CPPDUMPER_XIA0_H

/*  is open debug ?
 *  1   open
 *  0   close
 */
#define X_DEBUG 0

/*
 *  1: [strict] assert() check
 *  2: [normal] if else check, if failed, return null or ""
 *
 */
#define DEBUG_MODE  2

// v24.0
#define IL2CPP_TEST_METADATA_v24_0  "../il2cppTests/24.0/speedmobile/global-metadata.dat"
#define IL2CPP_TEST_BIN_v24_0       "../il2cppTests/24.0/speedmobile/speedmobile.decrypted"

// v24.1
#define IL2CPP_TEST_METADATA_v24_1  "../il2cppTests/android/24.1/Hero/global-metadata.dat"
#define IL2CPP_TEST_BIN_v24_1       "../il2cppTests/android/24.1/Hero/libil2cpp.so"

//====================current test version====================hero-demo
#define IL2CPP_TEST_METADATA        IL2CPP_TEST_METADATA_v24_1
#define IL2CPP_TEST_BIN             IL2CPP_TEST_BIN_v24_1


// ====================current running version====================
#define IL2CPP_VERSION IL2CPP_VERSION_24_DOT_1

// all support il2cpp version : http://1vr.cn/?p=568
/*
 * here is like 2018.3.14f1 string will in GLOBAL__sub_I_Runtime_Utilities_7.cpp of il2cpp_so or macho file
 */
#define IL2CPP_VERSION_23_DOT_0     230     // 23.0     unity5.6.x
#define IL2CPP_VERSION_24_DOT_0     240     // 24.0     unity2017.x-2018.2
#define IL2CPP_VERSION_24_DOT_1     241     // 24.1     unity2018.3-2019.x
#define IL2CPP_VERSION_24_DOT_2     242     // 24.2     unity2019.x+

// include corresponding il2cpp header for the il2cpp verison
#include "il2cpp-header/il2cpp-misc.h"

#if IL2CPP_VERSION == IL2CPP_VERSION_23_DOT_0

#elif IL2CPP_VERSION == IL2CPP_VERSION_24_DOT_0

#include "il2cpp-header/24.0/il2cpp-runtime-metadata.h"
#include "il2cpp-header/24.0/il2cpp-metadata.h"
#include "il2cpp-header/24.0/il2cpp-blob.h"
#include "il2cpp-header/24.0/il2cpp-class-internals.h"
#include "il2cpp-header/24.0/il2cpp-tabledefs.h"

#elif IL2CPP_VERSION == IL2CPP_VERSION_24_DOT_1

#include "il2cpp-header/24.1/il2cpp-runtime-metadata.h"
#include "il2cpp-header/24.1/il2cpp-metadata.h"
#include "il2cpp-header/24.1/il2cpp-blob.h"
#include "il2cpp-header/24.1/il2cpp-class-internals.h"
#include "il2cpp-header/24.1/il2cpp-tabledefs.h"

#elif IL2CPP_VERSION == IL2CPP_VERSION_24_DOT_2

#else

#error Not found IL2CPP_VERSION in support il2cpp macro list

#endif

#endif //XIL2CPPDUMPER_XIA0_H
