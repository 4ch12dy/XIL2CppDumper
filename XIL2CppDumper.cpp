//
// Created by xia0 on 2019/11/5.
//
#include <assert.h>
#include "XIL2CppDumper.h"
#include "XB1nLib/XB1nLib.h"

XIL2CppDumper* XIL2CppDumper::m_pInstance = NULL;

XIL2CppDumper* XIL2CppDumper::GetInstance() {
    if(m_pInstance == NULL)
        m_pInstance = new XIL2CppDumper();
    return m_pInstance;
}

void XIL2CppDumper::initMetadata(const char *metadataFile, const char *il2cpBinFile) {
    metadata = map_file_2_mem(metadataFile);
    il2cppbin = map_file_2_mem(il2cpBinFile);

    metadataHeader = (const Il2CppGlobalMetadataHeader*)metadata;

    // should check bin file is iOS Macho64 or android so ELF
    // here is iOS Macho64

    mach_header_64* mh = (mach_header_64*)il2cppbin;
    XDLOG("macho header magic number:%X\n", mh->magic);
    XRange* range = macho64_get_sec_range_by_name(mh, "__DATA", "__mod_init_func");
    XDLOG("macho64 mod_init_func address start:0x%lx end:0x%lx\n", range->start, range->end);

    void** mod_init_func_list = (void**)((char*)il2cppbin + range->start - 0x100000000);
    void* first_init_func = *mod_init_func_list;
    XDLOG("mod_init_func first func addr:0x%lx\n", first_init_func);

    uint32_t * mem_pc = (uint32_t *)((char*)il2cppbin + (uint64_t)first_init_func - 0x100000000);

    uint32_t* ida_pc = (uint32_t*)first_init_func;
    ida_pc += 2;
    mem_pc += 2;
    uint32_t insn = arm64_insn_from_addr(mem_pc);
    void* ida_s_Il2CppCodegenRegistration = arm64_adr_decode(ida_pc, insn);
    XDLOG("decode s_Il2CppCodegenRegistration adrress from bin:0x%lx\n", ida_s_Il2CppCodegenRegistration);

    ida_pc = (uint32_t*)ida_s_Il2CppCodegenRegistration;
    mem_pc = (uint32_t *)((char*)il2cppbin + (uint64_t)ida_s_Il2CppCodegenRegistration - 0x100000000);
    insn = arm64_insn_from_addr(mem_pc);
    void* ida_g_CodeRegistration = (void*)((uint64_t)arm64_adrp_decode(ida_pc, insn) + arm64_add_decode_imm(arm64_insn_from_addr(mem_pc+1)));
    XDLOG("decode g_CodeRegistration adrress from bin:0x%lx\n", ida_g_CodeRegistration);

    ida_pc += 2;
    mem_pc += 2;
    insn = arm64_insn_from_addr(mem_pc);
    void* ida_g_MetadataRegistration = (void*)((uint64_t)arm64_adrp_decode(ida_pc, insn) + arm64_add_decode_imm(arm64_insn_from_addr(mem_pc+1)));
    XDLOG("decode g_MetadataRegistration adrress from bin:0x%lx\n", ida_g_MetadataRegistration);

    g_CodeRegistration = (Il2CppCodeRegistration*)((char*)il2cppbin + (uint64_t)ida_g_CodeRegistration - 0x100000000);
    g_MetadataRegistration = (Il2CppMetadataRegistration*)((char*)il2cppbin + (uint64_t)ida_g_MetadataRegistration - 0x100000000);

    XDLOG("g_CodeRegistration methodPointersCount:%ld\n", g_CodeRegistration->methodPointersCount);
}

void* XIL2CppDumper::LoadMetadataFile(const char *fileName) {
    void* p = map_file_2_mem(fileName);

    return p;
}

char* XIL2CppDumper::HexDump(void *targetAddr, uint64_t size) {
    return hex_dump(targetAddr, size);
}

void XIL2CppDumper::ShowHexDump(void *targetAddr, uint64_t size) {
    show_hex_dump(targetAddr, size);
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

/**
 * get string by index from metadata
 * @param index
 * @return
 */
const char* XIL2CppDumper::getStringFromIndex(StringIndex index) {
    assert(index <= this->metadataHeader->stringCount);
    const char* strings = ((const char*) this->metadata +  this->metadataHeader->stringOffset) + index;
    return strings;
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

/**
 * dump string from metadata
 */
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

void XIL2CppDumper::dumpAllImages() {
    // dump all image name
    const Il2CppImageDefinition* imagesDefinitions = (const Il2CppImageDefinition*)((const char*)metadata + metadataHeader->imagesOffset);
    int32_t imageCount = metadataHeader->imagesCount / sizeof(Il2CppImageDefinition);
    for (int32_t imageIndex = 0; imageIndex < imageCount; imageIndex++)
    {
        const Il2CppImageDefinition* imageDefinition = imagesDefinitions + imageIndex;
        const char* imageName = this->getStringFromIndex(imageDefinition->nameIndex);
        XILOG("[%d] image name :%s\n", imageIndex, imageName);
    }
}

void XIL2CppDumper::dumpTypes() {
    // dump all define types of first image
    const Il2CppImageDefinition* imagesDefinitions = (const Il2CppImageDefinition*)((const char*)metadata + metadataHeader->imagesOffset);
    const Il2CppImageDefinition* fisrtImage = imagesDefinitions + 23;
    const Il2CppTypeDefinition* typeDefinitions = (const Il2CppTypeDefinition*)((const char*)metadata + metadataHeader->typeDefinitionsOffset);
    const Il2CppTypeDefinition* imageTypeDefinitions = (const Il2CppTypeDefinition*)((const char*)typeDefinitions + fisrtImage->typeStart*
                                                                                                                    sizeof(Il2CppTypeDefinition));

    int32_t typeDefinitionCount = fisrtImage->typeCount;
    for (int32_t i = 0; i < typeDefinitionCount; i++) {
        const  Il2CppTypeDefinition* typeDefinition = imageTypeDefinitions + i;
        const char* typeName = this->getStringFromIndex(typeDefinition->nameIndex);
        const char* typeNamespace = this->getStringFromIndex(typeDefinition->namespaceIndex);
        XILOG("====\n[%d] %s %s\n", i, typeName, typeNamespace);
        if (typeDefinition->parentIndex >= 0) {
            XILOG("parentIndex:%d\n", 25381 + typeDefinition->parentIndex);

        }
    }
}