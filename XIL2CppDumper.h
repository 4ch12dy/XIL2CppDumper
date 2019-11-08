//
// Created by zhangshun on 2019/11/5.
//

#ifndef XIL2CPPDUMPER_XIL2CPPDUMPER_H
#define XIL2CPPDUMPER_XIL2CPPDUMPER_H

#include <unistd.h>
#include "il2cpp-metadata.h"
#include "il2cpp-runtime-metadata.h"

class XIL2CppDumper
{

private:
    XIL2CppDumper(){}
    static XIL2CppDumper *m_pInstance;

public:
    // metadata
    void* metadata;
    const Il2CppGlobalMetadataHeader* metadataHeader;
    const Il2CppImageDefinition* metadataImageTable;
    const Il2CppTypeDefinition* metadataTypeDefinitionTable;

    // binary file
    void* il2cppbin;
    const Il2CppCodeRegistration* g_CodeRegistration;
    const Il2CppMetadataRegistration* g_MetadataRegistration;

    // single instance
    static XIL2CppDumper * GetInstance();
    void initMetadata(const char* metadataFile, const char* il2cpBinFile);

    // some debug funcs
    static char* HexDump(void* targetAddr, uint64_t size);
    static void ShowHexDump(void* targetAddr, uint64_t size);

    // misc
    static void* LoadMetadataFile(const char* fileName);

    // il2cpp function
    const char* getStringFromIndex(StringIndex index);
    char* getStringLiteralFromIndex(StringIndex index);
    char* removeAllChars(char* str, char c);

    // test
    void dumpAllImages();
    void dumpString();
    void dumpTypes();

    template <typename T>
    static T MetadataOffset(void* metadata, size_t sectionOffset, size_t itemIndex)
    {
        return reinterpret_cast<T> (reinterpret_cast<uint8_t*> (metadata) + sectionOffset) + itemIndex;
    }
};

#endif //XIL2CPPDUMPER_XIL2CPPDUMPER_H
