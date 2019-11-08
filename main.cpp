#include <iostream>
#include "XIL2CppDumper.h"
#include "il2cpp-metadata.h"
#include "XB1nLib/XB1nLib.h"

int main() {
    std::cout << "====XIL2CppDumper====" << std::endl;
    // load metadata file to memory
    char* metadataFilePath = (char*)"/Users/zhangshun/xia0/game-sec/XIL2CppDumper/resource/global-metadata.dat";
    void* metadata = XIL2CppDumper::LoadMetadataFile(metadataFilePath);

    const Il2CppGlobalMetadataHeader* metadataHeader = (const Il2CppGlobalMetadataHeader*)metadata;
    XIL2CppDumper* dumper = XIL2CppDumper::GetInstance();
    dumper->metadataHeader = metadataHeader;
    dumper->metadata = metadata;

    char* sanity = XIL2CppDumper::HexDump(metadata, 4);
    char* version = XIL2CppDumper::HexDump((void*)((char*)metadata+4), 4);
    XILOG("metadata sanity:%s version:%s\n", sanity, version);

    // dump all image name
    const Il2CppImageDefinition* imagesDefinitions = (const Il2CppImageDefinition*)((const char*)metadata + metadataHeader->imagesOffset);
    int32_t imageCount = metadataHeader->imagesCount / sizeof(Il2CppImageDefinition);
    for (int32_t imageIndex = 0; imageIndex < imageCount; imageIndex++)
    {
        const Il2CppImageDefinition* imageDefinition = imagesDefinitions + imageIndex;
        const char* imageName = dumper->getStringFromIndex(imageDefinition->nameIndex);
//        XILOG("[%d] image name :%s\n", imageIndex, imageName);
    }

    // dump all define types of first image
    const Il2CppImageDefinition* fisrtImage = imagesDefinitions + 23;
    const Il2CppTypeDefinition* typeDefinitions = (const Il2CppTypeDefinition*)((const char*)metadata + metadataHeader->typeDefinitionsOffset);
    const Il2CppTypeDefinition* imageTypeDefinitions = (const Il2CppTypeDefinition*)((const char*)typeDefinitions + fisrtImage->typeStart*
                                                                                                                     sizeof(Il2CppTypeDefinition));

    int32_t typeDefinitionCount = fisrtImage->typeCount;
    for (int32_t i = 0; i < typeDefinitionCount; i++) {
        const  Il2CppTypeDefinition* typeDefinition = imageTypeDefinitions + i;
        const char* typeName = dumper->getStringFromIndex(typeDefinition->nameIndex);
        const char* typeNamespace = dumper->getStringFromIndex(typeDefinition->namespaceIndex);
        XILOG("====\n[%d] %s %s\n", i, typeName, typeNamespace);
        if (typeDefinition->parentIndex >= 0){
            XILOG("parentIndex:%d\n", 25381 + typeDefinition->parentIndex);

        }

//
//        if (typeDefinition->interfaces_count > 0){
//            XDLOG("[%d] %s %s have parent has interface\n", i, typeName, typeNamespace);
//        }
    }

//    dumper->dumpString();

    void* macho64_ptr = map_file_2_mem("/Users/zhangshun/xia0/game-sec/XIL2CppDumper/resource/ProductName");

    char* mh_bytes_str = hex_dump(macho64_ptr, 20);
    XILOG("macho:%s\n", mh_bytes_str);
    free(mh_bytes_str);

    mach_header_64* mh = (mach_header_64*)(macho64_ptr);
    XRange* range = macho64_get_sec_range_by_name(mh, "__DATA", "__mod_init_func");
    XILOG("mod_init_func start:0x%lx end:0x%lx\n", range->start, range->end);
    uint64_t file_offset = range->start - 0x100000000;
    void** mod_init_func_list = (void**)((char*)mh + file_offset);
    void* first_init_func = *mod_init_func_list;
    XILOG("addr:%lx\n", first_init_func);
    void* addr = (void*)((char*)mh + (uint64_t)first_init_func - 0x100000000);
    show_hex_dump(addr, 4);
    uint8_t adrp_bytes[] = {0x80, 0x6b, 0x00, 0xb0};
    uint32_t adrp = bytes_2_uint32(adrp_bytes);
    XILOG("adrp:%x\n", adrp);
    void* p = arm64_adrp_decode((void*)0x10007ea54, adrp);

    printf("p=0x%lx\n", p);

    uint8_t add_bytes[] = {0x00,0x40,0x37,0x91};
    uint32_t add_insn = bytes_2_uint32(add_bytes);
    int y_n = arm64_is_add_imm(add_insn);
    XILOG("yes_or_no:%d", y_n);

    int64_t add_imm = arm64_add_decode_imm(add_insn);
    XILOG("add imm:%x\n", add_imm);

    p = (void*)((uint64_t)p + add_imm);
    printf("p=0x%lx\n", p);

    uint8_t adr_bytes[] = {0xe1, 0xfe, 0xff, 0x10};
    uint32_t adr = bytes_2_uint32(adr_bytes);
    void* pc = (void*)0x10007ea5c;
    XILOG("pc:0x%lx imm:0x%x adr pc:0x%lx\n", pc, arm64_adr_decode_imm(adr), arm64_adr_decode(pc, adr));
    return 0;
}