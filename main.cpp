#include <iostream>
#include "XIL2CppDumper.h"
#include "il2cpp-metadata.h"
#include "XB1nLib/XB1nLib.h"

int main() {
    XILOG("====XIL2CppDumper====\n");

    // init
    XIL2CppDumper* dumper = XIL2CppDumper::GetInstance();
    const char* metadataFileFullPath = "/Users/zhangshun/xia0/game-sec/XIL2CppDumper/resource/global-metadata.dat";
    const char* il2cppbinFileFullPath = "/Users/zhangshun/xia0/game-sec/XIL2CppDumper/resource/ProductName";

    dumper->initMetadata(metadataFileFullPath, il2cppbinFileFullPath);

    return 0;
}