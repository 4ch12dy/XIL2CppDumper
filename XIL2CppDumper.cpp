//
// Created by xia0 on 2019/11/5.
//
#include <assert.h>
#include <vector>
#include "XIL2CppDumper.h"
#include "XB1nLib/XB1nLib.h"
#include "il2cpp-runtime-metadata.h"
#include "il2cpp-tabledefs.h"

XIL2CppDumper* XIL2CppDumper::m_pInstance = NULL;

XIL2CppDumper* XIL2CppDumper::GetInstance() {
    if(m_pInstance == NULL)
        m_pInstance = new XIL2CppDumper();
    return m_pInstance;
}

void XIL2CppDumper::initWithMacho64(void *il2cppbin) {
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

    // init done
    g_CodeRegistration = (Il2CppCodeRegistration*)((char*)il2cppbin + (uint64_t)ida_g_CodeRegistration - 0x100000000);
    g_MetadataRegistration = (Il2CppMetadataRegistration*)((char*)il2cppbin + (uint64_t)ida_g_MetadataRegistration - 0x100000000);

    XDLOG("g_CodeRegistration methodPointersCount:%ld\n", g_CodeRegistration->methodPointersCount);
}
void XIL2CppDumper::initMetadata(const char *metadataFile, const char *il2cpBinFile) {
    metadata = map_file_2_mem(metadataFile);
    metadataHeader = (const Il2CppGlobalMetadataHeader*)metadata;
    il2cppbin = map_file_2_mem(il2cpBinFile);

    metadataImageDefinitionTable = (const Il2CppImageDefinition*)((const char*)metadata + metadataHeader->imagesOffset);
    metadataTypeDefinitionTable = (const Il2CppTypeDefinition*)((const char*)metadata + metadataHeader->typeDefinitionsOffset);
    metadataInterfaceTable = (uint32_t*)((const char*)metadata + metadataHeader->interfacesOffset);

    // should check bin file is iOS Macho64 or android so ELF
    // here is iOS Macho64
    initWithMacho64(il2cppbin);
    assert(g_CodeRegistration && g_MetadataRegistration);

    g_Il2CppTypeTable = (const Il2CppType**)((char*)il2cppbin + (uint64_t)(g_MetadataRegistration->types) - 0x100000000);
    g_Il2CppTypeTableCount = (int32_t)(g_MetadataRegistration->typesCount);

    XILOG("g_Il2CppTypeTable=%lx g_Il2CppTypeTableCount=%d\n", g_Il2CppTypeTable ,g_Il2CppTypeTableCount );

    // open file for write
    outfile.open("/Users/zhangshun/xia0/game-sec/XIL2CppDumper/dump/dump.cs", ios::out | ios::trunc);
}

const Il2CppType* XIL2CppDumper::getTypeFromIl2CppTypeTableByIndex(TypeIndex index) {
    assert(g_Il2CppTypeTable && index >=0 && index <= g_Il2CppTypeTableCount);
    return (const Il2CppType*)((char*)il2cppbin + (uint64_t)(g_Il2CppTypeTable[index]) - 0x100000000);
}

void* XIL2CppDumper::idaAddr2MemAddr(void *idaAddr) {
    void* mem = (void*)((char*)il2cppbin + (uint64_t)idaAddr - 0x100000000);
//    XDLOG("ida address:0x%lx il2cppbin address:0x%lx mem address:0x%lx\n", idaAddr, il2cppbin, mem);
    return mem;
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
const char* XIL2CppDumper::getStringByIndex(StringIndex index) {
    assert(index <= metadataHeader->stringCount);
    const char* strings = ((const char*) metadata +  metadataHeader->stringOffset) + index;
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
        const char* imageName = this->getStringByIndex(imageDefinition->nameIndex);
        XILOG("[%d] image name :%s\n", imageIndex, imageName);
    }
}
const Il2CppTypeDefinition* XIL2CppDumper::getTypeDefinitionByIndex(TypeDefinitionIndex index) {
    const Il2CppTypeDefinition* typeDef = (const Il2CppTypeDefinition*)(metadataTypeDefinitionTable + index);
    return typeDef;
}

string XIL2CppDumper::getGenericTypeParams(const Il2CppGenericInst *genericInst) {
    int argc = genericInst->type_argc;
    string argNames;
    int count = argc+argc-1+2;
    string retNames[count];
    retNames[0] = "<";
    for (int i = 0; i < argc; ++i) {
        const Il2CppType **typeArgv = (const Il2CppType **)idaAddr2MemAddr(genericInst->type_argv);
        const Il2CppType * type = (const Il2CppType*)idaAddr2MemAddr((void*)typeArgv[i]);
        retNames[i+1] = getTypeName(type);
        if (i+1 < argc){
            retNames[i+2] = ",";
        }
    }
    retNames[count-1] = ">";
    for (int j = 0; j < count; ++j) {
        argNames += retNames[j];
    }
    return argNames;
}

string XIL2CppDumper::getTypeDefinitionName(const Il2CppTypeDefinition *typeDefinition) {
    string ret = "";
    if (typeDefinition->declaringTypeIndex != -1){
        ret += getTypeName(getTypeFromIl2CppTypeTableByIndex(typeDefinition->declaringTypeIndex)) + ".";
    }
    ret += getStringByIndex(typeDefinition->nameIndex);

    return ret;
}

string XIL2CppDumper::getTypeName(const Il2CppType *type) {

    string ret;
    switch (type->type){

        case IL2CPP_TYPE_CLASS:
        case IL2CPP_TYPE_VALUETYPE:
        {
            const Il2CppTypeDefinition* typeDef = (const Il2CppTypeDefinition*)(metadataTypeDefinitionTable + type->data.klassIndex);
            ret = getTypeDefinitionName(typeDef);
            break;
        }
        case IL2CPP_TYPE_GENERICINST:
        {
            const Il2CppGenericClass* genericClass = (Il2CppGenericClass*)idaAddr2MemAddr((void*)type->data.generic_class);
            const Il2CppTypeDefinition* typeDef = getTypeDefinitionByIndex(genericClass->typeDefinitionIndex);
            ret = getStringByIndex(typeDef->nameIndex);
            Il2CppGenericContext context = genericClass->context;
            const Il2CppGenericInst* genericInst = (const Il2CppGenericInst*)idaAddr2MemAddr((void*)context.class_inst);
            string argNames = getGenericTypeParams(genericInst);
            ret += argNames;
            break;
        }
        case IL2CPP_TYPE_ARRAY:
        {
            const Il2CppArrayType* arrayType = (const Il2CppArrayType*)idaAddr2MemAddr((void*)type->data.array);
            const Il2CppType *tmpType = (const Il2CppType *)idaAddr2MemAddr((void*)arrayType->etype);
            ret = getTypeName(tmpType) + "[" + to_string(arrayType->rank) + "]";
            break;
        }
        case IL2CPP_TYPE_SZARRAY:
        {
            const Il2CppType *tmpType = (const Il2CppType *)idaAddr2MemAddr((void*)type->data.type);
            ret = getTypeName(tmpType) + "[]";
            break;
        }
        case IL2CPP_TYPE_PTR:
        {
            const Il2CppType *tmpType = (const Il2CppType *)idaAddr2MemAddr((void*)type->data.type);
            ret = getTypeName(tmpType) + "*";
            break;
        }
        default:

            break;
    }
    return ret;
}

void XIL2CppDumper::dump() {
    // dump all define types of first image
    uint32_t imageCount = metadataHeader->imagesCount / sizeof(Il2CppImageDefinition);
    for (int i = 0; i < imageCount; ++i) {
        XILOG("[%d] \n", i);
        const Il2CppImageDefinition* curImage = metadataImageDefinitionTable + i;
        const Il2CppTypeDefinition* imageTypeDefinitions = (const Il2CppTypeDefinition*)((const char*)metadataTypeDefinitionTable + curImage->typeStart*
                                                                                                                        sizeof(Il2CppTypeDefinition));
        int32_t typeDefinitionCount = curImage->typeCount;
        for (int32_t i = 0; i < typeDefinitionCount; i++) {
            const  Il2CppTypeDefinition* typeDefinition = imageTypeDefinitions + i;
            const char* typeName = this->getStringByIndex(typeDefinition->nameIndex);
            const char* typeNamespace = this->getStringByIndex(typeDefinition->namespaceIndex);

            bool isStruct = false;
            bool isEnum = false;
            vector<string> extends;

            XILOG("====\n[%d] %s %s\n", i, typeName, typeNamespace);
            if (typeDefinition->parentIndex >= 0) {
                XILOG("parentIndex:%d\n", 25381 + typeDefinition->parentIndex);
                XILOG("idx:%lx\n", g_Il2CppTypeTable[typeDefinition->parentIndex]);
                const Il2CppType* il2CppType = getTypeFromIl2CppTypeTableByIndex(typeDefinition->parentIndex);

                string name = getTypeName(getTypeFromIl2CppTypeTableByIndex(typeDefinition->parentIndex));
                if (name == "ValueType"){
                    isStruct = true;
                } else if (name == "Enum"){
                    isEnum = true;
                }else if(name != "object"){
                    if (!name.empty()){
                        extends.push_back(name);
                    }
                }
                XILOG("parent name:%s\n", name.data());
            }

            if (typeDefinition->interfaces_count > 0){
                for (int i = 0; i < typeDefinition->interfaces_count; ++i) {
                    const Il2CppType* type = getTypeFromIl2CppTypeTableByIndex(*(metadataInterfaceTable+typeDefinition->interfacesStart + i));
                    extends.push_back(getTypeName(type).data());
                    XILOG("has interface start:%d count:%d idx:%d name:%s\n", typeDefinition->interfacesStart, typeDefinition->interfaces_count,*(metadataInterfaceTable+typeDefinition->interfacesStart), getTypeName(type).data());
                }
            }
            write2File(format("\n//NameSpace:%s\n", typeNamespace));
            uint32_t visibility = typeDefinition->flags & TYPE_ATTRIBUTE_VISIBILITY_MASK;

            switch (visibility){
                case TYPE_ATTRIBUTE_PUBLIC:
                case TYPE_ATTRIBUTE_NESTED_PUBLIC:
                    write2File("public ");
                    break;
                case TYPE_ATTRIBUTE_NOT_PUBLIC:
                case TYPE_ATTRIBUTE_NESTED_FAM_AND_ASSEM:
                case TYPE_ATTRIBUTE_NESTED_ASSEMBLY:
                    write2File("internal  ");
                    break;
                case TYPE_ATTRIBUTE_NESTED_PRIVATE:
                    write2File("private  ");
                    break;
                case TYPE_ATTRIBUTE_NESTED_FAMILY:
                    write2File("protected  ");
                    break;
                case TYPE_ATTRIBUTE_NESTED_FAM_OR_ASSEM:
                    write2File("protected internal  ");
                    break;
            }

            if ((typeDefinition->flags & TYPE_ATTRIBUTE_ABSTRACT) != 0 && (typeDefinition->flags & TYPE_ATTRIBUTE_SEALED) != 0)
                write2File("static ");
            else if ((typeDefinition->flags & TYPE_ATTRIBUTE_INTERFACE) == 0 && (typeDefinition->flags & TYPE_ATTRIBUTE_ABSTRACT) != 0)
                write2File("abstract ");
            else if (!isStruct && !isEnum && (typeDefinition->flags & TYPE_ATTRIBUTE_SEALED) != 0)
                write2File("sealed ");
            if ((typeDefinition->flags & TYPE_ATTRIBUTE_INTERFACE) != 0)
                write2File("interface ");
            else if (isStruct)
                write2File("struct ");
            else if (isEnum)
                write2File("enum ");
            else
                write2File("class ");

            write2File(getTypeDefinitionName(typeDefinition));

            if (extends.size() > 0){
                write2File(" : ");
                string tmp;
                for (int i = 0; i < extends.size(); ++i) {
                    tmp += extends[i];
                    if (i+1 < extends.size()){
                        tmp += ", ";
                    }
                }
                write2File(tmp);
            }

            write2File("\n{");

            // dump field
            if (typeDefinition->field_count > 0){
                write2File("\n\t// Fields\n");

            }

            write2File("\n}");
        }
    }

    outfile.close();
}
string XIL2CppDumper::format(const char *fmt, ...) {
    int size = 512;
    char* buffer = 0;
    buffer = new char[size];
    va_list vl;
    va_start(vl, fmt);
    int nsize = vsnprintf(buffer, size, fmt, vl);
    if(size<=nsize){ //fail delete buffer and try again
        delete[] buffer;
        buffer = 0;
        buffer = new char[nsize+1]; //+1 for /0
        nsize = vsnprintf(buffer, size, fmt, vl);
    }
    std::string ret(buffer);
    va_end(vl);
    delete[] buffer;
    return ret;
}



void XIL2CppDumper::write2File(string str) {
    outfile << str;
}

