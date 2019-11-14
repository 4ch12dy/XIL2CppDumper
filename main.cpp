#include <iostream>
#include "XIL2CppDumper.h"
#include "XB1nLib/XB1nLib.h"

void printUasge(){
    printf( "\n\n"
            "+--------------------------------------------------------------------------------------+\n"
            "| XIL2CppDumper | a tool of C++ version IL2CppDumper made by xia0@2019                 |\n"
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

void banner(){
    printf("\n"
           " __  _____ _     ____   ____             ____                                  \n"
           " \\ \\/ /_ _| |   |___ \\ / ___|_ __  _ __ |  _ \\ _   _ _ __ ___  _ __   ___ _ __ \n"
           "  \\  / | || |     __) | |   | '_ \\| '_ \\| | | | | | | '_ ` _ \\| '_ \\ / _ \\ '__|\n"
           "  /  \\ | || |___ / __/| |___| |_) | |_) | |_| | |_| | | | | | | |_) |  __/ |   \n"
           " /_/\\_\\___|_____|_____|\\____| .__/| .__/|____/ \\__,_|_| |_| |_| .__/ \\___|_|   \n"
           "                            |_|   |_|                         |_|            "
           "\n\n");
}

int main(int argc, char* argv[]) {

    banner();

#if X_DEBUG
    // debug
    const char* metadataFileFullPath = IL2CPP_TEST_METADATA;
    const char* il2cppbinFileFullPath = IL2CPP_TEST_BIN;
#else
    if (argc != 3){
        printUasge();
        return 1;
    }
    const char* metadataFileFullPath = argv[1];
    const char* il2cppbinFileFullPath = argv[2];
#endif

    XIL2CppDumper* xdump = XIL2CppDumper::GetInstance();
    xdump->initMetadata(metadataFileFullPath, il2cppbinFileFullPath);
    xdump->dump();
//    xdump->dumpAllImages();
    xdump->clean();
    return 0;
}