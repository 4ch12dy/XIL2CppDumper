//
// Created by zhangshun on 2019/11/5.
//

#ifndef XIL2CPPDUMPER_XIL2CPPDUMPER_H
#define XIL2CPPDUMPER_XIL2CPPDUMPER_H

#include "il2cpp-metadata.h"
#include <unistd.h>

class XIL2CppDumper
{

private:
    XIL2CppDumper()
    {
    }
    static XIL2CppDumper *m_pInstance;

public:
    const Il2CppGlobalMetadataHeader* metadataHeader;
    void* metadata;
    const Il2CppImageDefinition* metadataImageTable;
    const Il2CppTypeDefinition* metadataTypeDefinitionTable;

    static void* LoadMetadataFile(const char* fileName);
    static char* HexDump(void* targetAddr, uint64_t size);
    static void ShowHexDump(void* targetAddr, uint64_t size);

    template <typename T>
    static T MetadataOffset(void* metadata, size_t sectionOffset, size_t itemIndex)
    {
        return reinterpret_cast<T> (reinterpret_cast<uint8_t*> (metadata) + sectionOffset) + itemIndex;
    }

    // il2cpp function
    const char* getStringFromIndex(StringIndex index);
    char* getStringLiteralFromIndex(StringIndex index);
    char* removeAllChars(char* str, char c);
    void dumpString();

    static XIL2CppDumper * GetInstance()
    {
        if(m_pInstance == NULL)
            m_pInstance = new XIL2CppDumper();
        return m_pInstance;
    }
};

#endif //XIL2CPPDUMPER_XIL2CPPDUMPER_H
