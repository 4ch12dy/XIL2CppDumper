//
// Created by zhangshun on 2019/11/5.
//

#ifndef XIL2CPPDUMPER_XIL2CPPDUMPER_H
#define XIL2CPPDUMPER_XIL2CPPDUMPER_H

#include <string>
#include <unistd.h>
#include <fstream>
#include "il2cpp-metadata.h"
#include "il2cpp-runtime-metadata.h"
using namespace std;

class XIL2CppDumper
{

private:
    XIL2CppDumper(){}
    static XIL2CppDumper *m_pInstance;
    void initWithMacho64(void* il2cppbin);
public:
    // metadata
    void* metadata;
    const Il2CppGlobalMetadataHeader* metadataHeader;
    const Il2CppImageDefinition* metadataImageDefinitionTable;
    const Il2CppTypeDefinition* metadataTypeDefinitionTable;
    uint32_t* metadataInterfaceTable;
    const Il2CppType** g_Il2CppTypeTable;
    int32_t g_Il2CppTypeTableCount;

    ofstream outfile;

    // binary file
    void* il2cppbin;
    const Il2CppCodeRegistration* g_CodeRegistration;
    const Il2CppMetadataRegistration* g_MetadataRegistration;

    // single instance
    static XIL2CppDumper * GetInstance();
    void initMetadata(const char* metadataFile, const char* il2cpBinFile);

    // execute format file
    void* idaAddr2MemAddr(void* idaAddr);

    // il2cpp function
    const char* getStringByIndex(StringIndex index);
    string getTypeName(const Il2CppType* type);
    string getTypeDefinitionName(const Il2CppTypeDefinition* typeDefinition);
    string getGenericTypeParams(const Il2CppGenericInst* genericInst);
    const Il2CppTypeDefinition* getTypeDefinitionByIndex(TypeDefinitionIndex index);
    const Il2CppType* getTypeFromIl2CppTypeTableByIndex(TypeIndex index);
    char* getStringLiteralFromIndex(StringIndex index);
    char* removeAllChars(char* str, char c);

    // misc
    inline string format(const char* fmt, ...);
    void write2File(string str);

    // test
    void dumpAllImages();
    void dumpString();
    void dump();

    template <typename T>
    static T MetadataOffset(void* metadata, size_t sectionOffset, size_t itemIndex)
    {
        return reinterpret_cast<T> (reinterpret_cast<uint8_t*> (metadata) + sectionOffset) + itemIndex;
    }
};

#endif //XIL2CPPDUMPER_XIL2CPPDUMPER_H
