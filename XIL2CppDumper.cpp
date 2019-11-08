//
// Created by xia0 on 2019/11/5.
//

#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "XIL2CppDumper.h"
#include "xia0-bin-lib/xia0-bin-lib.h"

XIL2CppDumper* XIL2CppDumper::m_pInstance = NULL;

void* XIL2CppDumper::LoadMetadataFile(const char *fileName) {
    int fd;
    int ret;
    struct stat st;
    size_t len_file;
    void *p;

    if ((fd = open(fileName, O_RDWR | O_CREAT,  S_IRWXU | S_IRGRP | S_IROTH)) < 0){
        XELOG("failed to open file:%s", fileName);
        perror("");
        return NULL;
    }
    if ((ret = fstat(fd,&st)) < 0){
        XELOG("error in fstat");
        return NULL;
    }
    len_file = st.st_size;

    if ((p = mmap(NULL,len_file,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0)) == MAP_FAILED){
        XELOG("error in mmap");
        return NULL;
    }

    if (close(fd)){
        XELOG("error in close");
    }

    return p;
}

char* XIL2CppDumper::HexDump(void *targetAddr, uint64_t size) {
    uint64_t hex_buffer_size = size*3 + 1;
    char* hex_buffer = (char*)malloc((unsigned long)hex_buffer_size);
    memset(hex_buffer, 0x0, hex_buffer_size);

    uint8_t* p = (uint8_t*)targetAddr;
    char* q = hex_buffer;

    for(int  i = 0; i < size ;i++ ){
        sprintf(q, "%02X ", *p);
        q += 3;
        p ++;
    };
    return hex_buffer;
}

void XIL2CppDumper::ShowHexDump(void *targetAddr, uint64_t size) {
    char* dumpStr = XIL2CppDumper::HexDump(targetAddr, size);
    XILOG("[HEX]:%s\n", dumpStr);
}

const char* XIL2CppDumper::getStringFromIndex(StringIndex index) {
    assert(index <= this->metadataHeader->stringCount);
    const char* strings = ((const char*) this->metadata +  this->metadataHeader->stringOffset) + index;
    return strings;
}

char* XIL2CppDumper::removeAllChars(char *str, char c) {
    char *pr = str, *pw = str;
    while (*pr) {
        *pw = *pr++;
        pw += (*pw != c);
    }
    *pw = '\0';

    return str;
}

char* XIL2CppDumper::getStringLiteralFromIndex(StringIndex index) {
    assert(index >= 0 && static_cast<uint32_t>(index) < metadataHeader->stringLiteralCount / sizeof(Il2CppStringLiteral) && "Invalid string literal index ");

    const Il2CppStringLiteral* stringLiteral = (const Il2CppStringLiteral*)((const char*)metadata + metadataHeader->stringLiteralOffset) + index;

    const char* srcStr = (const char*)metadata + metadataHeader->stringLiteralDataOffset + stringLiteral->dataIndex;

    char *dstStr = new char[stringLiteral->length+1];
    snprintf(dstStr, stringLiteral->length+1, "%s", srcStr);

//    dstStr = removeAllChars(dstStr, '\r');
//    dstStr = removeAllChars(dstStr, '\n');

    return dstStr;
}

void XIL2CppDumper::dumpString() {
    int usageListCount = this->metadataHeader->metadataUsageListsCount / sizeof(Il2CppMetadataUsageList);
    for (int n = 0; n < usageListCount; ++n) {
        uint32_t index = n;
        if(n != 7967){
            continue;
        }
        assert(this->metadataHeader->metadataUsageListsCount >= 0 && index <= static_cast<uint32_t>(this->metadataHeader->metadataUsageListsCount));

        const Il2CppMetadataUsageList* metadataUsageLists = MetadataOffset<const Il2CppMetadataUsageList*>(this->metadata, this->metadataHeader->metadataUsageListsOffset, index);

        uint32_t start = metadataUsageLists->start;
        uint32_t count = metadataUsageLists->count;
        XILOG("start:%d count:%d\n", start, count);
        for (uint32_t i = 0; i < count; i++)
        {
            uint32_t offset = start + i;
            assert(metadataHeader->metadataUsagePairsCount >= 0 && offset <= static_cast<uint32_t>(metadataHeader->metadataUsagePairsCount));
            const Il2CppMetadataUsagePair* metadataUsagePairs = MetadataOffset<const Il2CppMetadataUsagePair*>(metadata, metadataHeader->metadataUsagePairsOffset, offset);
            uint32_t destinationIndex = metadataUsagePairs->destinationIndex;

            uint32_t encodedSourceIndex = metadataUsagePairs->encodedSourceIndex;

            Il2CppMetadataUsage usage = GetEncodedIndexType(encodedSourceIndex);
            uint32_t decodedIndex = GetDecodedMethodIndex(encodedSourceIndex);
            switch (usage)
            {
                case kIl2CppMetadataUsageTypeInfo:
                    //metadataUsages[destinationIndex] = GetTypeInfoFromTypeIndex(decodedIndex);
                    break;
                case kIl2CppMetadataUsageIl2CppType:
                    //metadataUsages[destinationIndex] = const_cast<Il2CppType*>(GetIl2CppTypeFromIndex(decodedIndex));
                    //break;
                case kIl2CppMetadataUsageMethodDef:
                    //metadataUsages_method[destinationIndex] = const_cast<MethodInfo*>(GetMethodInfoFromMethodDefinitionIndex(encodedSourceIndex));
                    break;
                case kIl2CppMetadataUsageMethodRef:
                    //metadataUsages_method[destinationIndex] = const_cast<MethodInfo*>(GetMethodInfoFromMethodDefinitionIndex(encodedSourceIndex));
                    //metadataUsages[destinationIndex] = const_cast<char*>(GetMethodInfoFromIndex(encodedSourceIndex));
                    break;
                case kIl2CppMetadataUsageFieldInfo:
                    //metadataUsages[destinationIndex] = GetFieldInfoFromIndex(decodedIndex);
                    break;
                case kIl2CppMetadataUsageStringLiteral:{
                    char* theStr = getStringLiteralFromIndex(decodedIndex);
                    XILOG("[%d] %s\n", i, theStr);
                    break;
                }

                default:
                    //std::cout << "not implemented" << std::endl;
                    break;
            }
        }
    }
}