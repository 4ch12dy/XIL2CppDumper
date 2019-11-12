#include <iostream>
#include "XIL2CppDumper.h"
#include "il2cpp-metadata.h"
#include "XB1nLib/XB1nLib.h"

void printUasge(){
    printf( "\n\n"
            "+--------------------------------------------------------------------------------------+\n"
            "| XIL2CppDumper | a tool of C++ version IL2CppDumper power by xia0@2019                |\n"
            "+--------------------------------------------------------------------------------------+\n"
            "| Info          | version: 0.1 support: iOS[arm64] Android[]                           |\n"
            "+--------------------------------------------------------------------------------------+\n"
            "| Usage         | XIL2CppDumper unity_metadata_file_path il2cpp_so_or_macho_file_path  |\n"
            "+--------------------------------------------------------------------------------------+\n"
            "| Blog          | http://4ch12dy.site                                                  |\n"
            "+--------------------------------------------------------------------------------------+\n"
            "| Github        | https://github.com/4ch12dy                                           |\n"
            "+--------------------------------------------------------------------------------------+\n"
            "| Specail thanks to Perfare's Il2CppDumper:https://github.com/Perfare/Il2CppDumper     |\n"
            "+--------------------------------------------------------------------------------------+\n"
            "\n"
    );
}

int main(int argc, char* argv[]) {

    if (argc != 3){
        printUasge();
        return 1;
    }
    // init
    const char* metadataFileFullPath = argv[1];
    const char* il2cppbinFileFullPath = argv[2];

    XIL2CppDumper* xdump = XIL2CppDumper::GetInstance();
    xdump->initMetadata(metadataFileFullPath, il2cppbinFileFullPath);
    xdump->dump();

    xdump->clean();
    return 0;
}