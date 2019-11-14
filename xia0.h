//
// Created by xia0 on 2019/11/13.
//

#ifndef XIL2CPPDUMPER_XIA0_H
#define XIL2CPPDUMPER_XIA0_H

#define X_DEBUG 1

// all support il2cpp version
#define IL2CPP_VERSION_23_DOT_0     230     // 23.0     unity5.6.x
#define IL2CPP_VERSION_24_DOT_0     240     // 24.0     unity2017.x-2018.2
#define IL2CPP_VERSION_24_DOT_1     241     // 24.1     unity2018.3-2019.x
#define IL2CPP_VERSION_24_DOT_2     242     // 24.2     unity2019.x+

// current running version
#define IL2CPP_VERSION IL2CPP_VERSION_24_DOT_1

// include corresponding il2cpp header for the il2cpp verison
#include "il2cpp-header/il2cpp-misc.h"

#if IL2CPP_VERSION == IL2CPP_VERSION_23_DOT_0

#elif IL2CPP_VERSION == IL2CPP_VERSION_24_DOT_0

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
