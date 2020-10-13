//
// Created by xia0 on 2019/11/5.
//
#include <assert.h>
#include <vector>
#include "XIL2CppDumper.h"
#include "XB1nLib/XB1nLib.h"
#include <stdarg.h>

XIL2CppDumper* XIL2CppDumper::m_pInstance = NULL;

XIL2CppDumper* XIL2CppDumper::GetInstance() {
    if(m_pInstance == NULL)
        m_pInstance = new XIL2CppDumper();
    return m_pInstance;
}


void XIL2CppDumper::initMetadata(const char *metadataFile, const char *il2cpBinFile) {
    metadata = map_file_2_mem(metadataFile);
    metadataHeader = (const Il2CppGlobalMetadataHeader*)metadata;
    il2cppbin = map_file_2_mem(il2cpBinFile);

    metadataVersion = metadataHeader->version;
    XILOG("metadata version:%d\n", metadataVersion);
    metadataImageDefinitionTable = (const Il2CppImageDefinition*)((const char*)metadata + metadataHeader->imagesOffset);
    metadataTypeDefinitionTable = (const Il2CppTypeDefinition*)((const char*)metadata + metadataHeader->typeDefinitionsOffset);
    metadataInterfaceTable = (uint32_t*)((const char*)metadata + metadataHeader->interfacesOffset);
    metadataFieldDefinitionTable = (const Il2CppFieldDefinition*)((const char*)metadata + metadataHeader->fieldsOffset);
    metadataFieldDefaultValueTable = (const Il2CppFieldDefaultValue*)((const char*)metadata + metadataHeader->fieldDefaultValuesOffset);
    metadataPropertyTable = (const Il2CppPropertyDefinition*)((const char*)metadata + metadataHeader->propertiesOffset);
    metadataMethodDefinitionTable = (const Il2CppMethodDefinition*)((const char*)metadata + metadataHeader->methodsOffset);
    metadataParameterDefinitionTable = (const Il2CppParameterDefinition*)((const char*)metadata + metadataHeader->parametersOffset);

    // should check bin file is iOS Macho64 or android so ELF
    binParser = new IL2CppBinParser(il2cppbin, metadataVersion);

    g_CodeRegistration = (Il2CppCodeRegistration*)binParser->codeRegistration;
    g_MetadataRegistration = (Il2CppMetadataRegistration*)binParser->metadataRegistration;

    assert(g_CodeRegistration && g_MetadataRegistration);

    g_MethodPointers = (Il2CppMethodPointer*)idaAddr2MemAddr((void*)(g_CodeRegistration->methodPointers));
    methodPointersCount = g_CodeRegistration->methodPointersCount;

    void* ida_metadataUsages = (void*)g_MetadataRegistration->metadataUsages;
    g_MetadataUsages = (void** const*)idaAddr2MemAddr(ida_metadataUsages);
    metadataUsagesCount = g_MetadataRegistration->metadataUsagesCount;

    g_Il2CppTypeTable = (const Il2CppType**)(idaAddr2MemAddr((void*)g_MetadataRegistration->types));
    g_Il2CppTypeTableCount = (int32_t)(g_MetadataRegistration->typesCount);

    XILOG("g_Il2CppTypeTable=0x%lx g_Il2CppTypeTableCount=%d\n", g_MetadataRegistration->types ,g_Il2CppTypeTableCount );

    // open file for write
#if X_DEBUG
    outfile.open("../dump/dump.cs", ios::out | ios::trunc);
    scriptfile.open("../dump/script.py", ios::out | ios::trunc);
#else
    outfile.open("dump.cs", ios::out | ios::trunc);
    scriptfile.open("script.py", ios::out | ios::trunc);
#endif
    initScript();
}

// il2cpp function
void* XIL2CppDumper::RAW2RVA(uint64_t raw) {
    return binParser->RAW2RVA(raw);
}

uint64_t XIL2CppDumper::RVA2RAW(void *rva) {
    return binParser->RVA2RAW(rva);
}

void* XIL2CppDumper::idaAddr2MemAddr(void *idaAddr) {
    return binParser->idaAddr2MemAddr(idaAddr);
}

const Il2CppType* XIL2CppDumper::getTypeFromIl2CppTypeTableByIndex(TypeIndex index) {
    assert(g_Il2CppTypeTable && index >=0 && index <= g_Il2CppTypeTableCount);
    void* typeIDAAddr = (void*)g_Il2CppTypeTable[index];
    return (const Il2CppType*)(idaAddr2MemAddr(typeIDAAddr));
}

Il2CppMethodPointer XIL2CppDumper::getMethodPointerFromMethodPointersByIndex(MethodIndex index) {
    assert(g_MethodPointers && index >=0 && index <= methodPointersCount);
    Il2CppMethodPointer func = g_MethodPointers[index];
    void* funcAddr = (void*)func;
    return (Il2CppMethodPointer)idaAddr2MemAddr(funcAddr);
}

void* XIL2CppDumper::getMethodPointerIDAValueByIndex(MethodIndex index) {
    if (index == -1){
        // this Class all method index will = -1, I do not kown why now
        return (void*)-1;
    }

    assert(g_MethodPointers && index >=0 && index <= methodPointersCount);

    return (void*)g_MethodPointers[index];
}

void* XIL2CppDumper::getUsageAddressIDAValueByIndex(uint32_t index) {

    void* addr = (void*)g_MetadataUsages[index];
    return addr;
}

// metadata function
const char* XIL2CppDumper::getStringByIndex(StringIndex index) {

#if DEBUG_MODE == 1

    assert(index <= metadataHeader->stringCount && index >= 0);

#elif DEBUG_MODE == 2

    if(index > metadataHeader->stringCount){
        return "";
    }

#endif

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

const Il2CppImageDefinition* XIL2CppDumper::getImageDefinitionByIndex(ImageIndex index) {
    const Il2CppImageDefinition* imageDef = (const Il2CppImageDefinition*)(metadataImageDefinitionTable + index);
    return imageDef;
}

const Il2CppTypeDefinition* XIL2CppDumper::getTypeDefinitionByIndex(TypeDefinitionIndex index) {
    const Il2CppTypeDefinition* typeDef = (const Il2CppTypeDefinition*)(metadataTypeDefinitionTable + index);
    return typeDef;
}

const Il2CppFieldDefinition* XIL2CppDumper::getFieldDefinitionByIndex(FieldIndex index) {
    const Il2CppFieldDefinition* fieldDef = (const Il2CppFieldDefinition*)(metadataFieldDefinitionTable + index);
    return fieldDef;
}

const Il2CppFieldDefaultValue* XIL2CppDumper::getFieldDefaultValueByIndex(DefaultValueIndex index) {
    const Il2CppFieldDefaultValue* fieldDefaultValue = (const Il2CppFieldDefaultValue*)(metadataFieldDefinitionTable + index);
    return fieldDefaultValue;
}

const Il2CppPropertyDefinition* XIL2CppDumper::getPropertyDefinitionByIndex(PropertyIndex index) {
    const Il2CppPropertyDefinition* propertyDefinition = (const Il2CppPropertyDefinition*)(metadataPropertyTable + index);
    return propertyDefinition;
}

const Il2CppMethodDefinition* XIL2CppDumper::getMethodDefinitionByIndex(MethodIndex index) {
    const Il2CppMethodDefinition* methodDefinition = (const Il2CppMethodDefinition*)(metadataMethodDefinitionTable + index);
    return methodDefinition;
}

const Il2CppParameterDefinition* XIL2CppDumper::getParameterDefinitionByIndex(ParameterIndex index) {
    const Il2CppParameterDefinition* parameterDefinition = (const Il2CppParameterDefinition*)(metadataParameterDefinitionTable + index);
    return parameterDefinition;
}

string XIL2CppDumper::typeStringForID(int id) {
    int index = id;

#if DEBUG_MODE == 1

    assert(id <= 30 && id >=0);

#elif DEBUG_MODE == 2

    if (id > 30 || id < 0){
        return "ErrorType";
    }

#endif

    string typeDic[31] = {"","void", "bool", "char", "sbyte", "byte", "short", "ushort", "int", "uint", "long", "ulong", "float", "double", "string", \
                          "","", "", "", \
                          "T", "", "", \
                          "System.TypedReference","",\
                          "IntPtr", "UIntPtr","", "",\
                          "object", "", "T"};
    return typeDic[index];
}
string XIL2CppDumper::getTypeDefinitionName(const Il2CppTypeDefinition *typeDefinition) {
    string ret;
    if (typeDefinition->declaringTypeIndex != -1){
        ret += getTypeName(getTypeFromIl2CppTypeTableByIndex(typeDefinition->declaringTypeIndex)) + ".";
    }
    ret += getStringByIndex(typeDefinition->nameIndex);

    return ret;
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
            ret = typeStringForID(type->type);
            break;
    }
    return ret;
}

string XIL2CppDumper::getMethodAttribute(const Il2CppMethodDefinition *methodDef) {
    string str;
    uint16_t access = methodDef->flags & METHOD_ATTRIBUTE_MEMBER_ACCESS_MASK;
    switch (access){
        case METHOD_ATTRIBUTE_PRIVATE:
            str += "private ";
            break;
        case METHOD_ATTRIBUTE_PUBLIC:
            str += "public ";
            break;
        case METHOD_ATTRIBUTE_FAMILY:
            str += "protected ";
            break;
        case METHOD_ATTRIBUTE_ASSEM:
        case METHOD_ATTRIBUTE_FAM_AND_ASSEM:
            str += "internal ";
            break;
        case METHOD_ATTRIBUTE_FAM_OR_ASSEM:
            str += "protected internal ";
            break;
    }

    if ((methodDef->flags & METHOD_ATTRIBUTE_STATIC) != 0)
        str += "static ";
    if ((methodDef->flags & METHOD_ATTRIBUTE_ABSTRACT) != 0)
    {
        str += "abstract ";
        if ((methodDef->flags & METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK) == METHOD_ATTRIBUTE_REUSE_SLOT)
            str += "override ";
    }
    else if ((methodDef->flags & METHOD_ATTRIBUTE_FINAL) != 0)
    {
        if ((methodDef->flags & METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK) == METHOD_ATTRIBUTE_REUSE_SLOT)
            str += "sealed override ";
    }
    else if ((methodDef->flags & METHOD_ATTRIBUTE_VIRTUAL) != 0)
    {
        if ((methodDef->flags & METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK) == METHOD_ATTRIBUTE_NEW_SLOT)
            str += "virtual ";
        else
            str += "override ";
    }
    if ((methodDef->flags & METHOD_ATTRIBUTE_PINVOKE_IMPL) != 0)
        str += "extern ";

    return str;
}

void XIL2CppDumper::dump() {
    // dump all define types of first image
    uint32_t imageCount = metadataHeader->imagesCount / sizeof(Il2CppImageDefinition);
    for (int imageIndex = 0; imageIndex < imageCount; ++imageIndex) {
        const Il2CppImageDefinition* imageDefinition = getImageDefinitionByIndex(imageIndex);
        uint32_t typeEnd = imageDefinition->typeStart + imageDefinition->typeCount;

        DLOG("[%d] image name:%s type count:%d\n", imageIndex, getStringByIndex(imageDefinition->nameIndex), imageDefinition->typeCount);
        write2File(format("\n//[%d] imageName:%s typeCount:%d\n", imageIndex, getStringByIndex(imageDefinition->nameIndex), imageDefinition->typeCount));

        for (int32_t i = imageDefinition->typeStart; i < typeEnd; i++) {
            const  Il2CppTypeDefinition* typeDefinition = getTypeDefinitionByIndex(i);
            const char* typeName = getStringByIndex(typeDefinition->nameIndex);
            const char* typeNamespace = getStringByIndex(typeDefinition->namespaceIndex);
            DLOG("\ttype index:%d name:%s namespace:%s\n", i, typeName, typeNamespace);
            bool isStruct = false;
            bool isEnum = false;
            vector<string> extends;

            if (typeDefinition->parentIndex >= 0) {
                const Il2CppType* il2CppType = getTypeFromIl2CppTypeTableByIndex(typeDefinition->parentIndex);
                string name = getTypeName(getTypeFromIl2CppTypeTableByIndex(typeDefinition->parentIndex));
                DLOG("\t\tparent index:%d name:%s\n", typeDefinition->parentIndex, name.data());
                if (name == "ValueType"){
                    isStruct = true;
                } else if (name == "Enum"){
                    isEnum = true;
                }else if(name != "object"){
                    if (!name.empty()){
                        extends.push_back(name);
                    }
                }
            }

            if (typeDefinition->interfaces_count > 0){
                for (int i = 0; i < typeDefinition->interfaces_count; ++i) {
                    const Il2CppType* type = getTypeFromIl2CppTypeTableByIndex(*(metadataInterfaceTable+typeDefinition->interfacesStart + i));
                    string interfaceName = getTypeName(type).data();
                    extends.push_back(interfaceName);
                    DLOG("\t\tinterface index:%d count:%d name:%s\n", i, typeDefinition->interfaces_count, interfaceName.data());
                }
            }
            write2File(format("\n//NameSpace: %s\n", typeNamespace));
            uint32_t visibility = typeDefinition->flags & TYPE_ATTRIBUTE_VISIBILITY_MASK;

            switch (visibility){
                case TYPE_ATTRIBUTE_PUBLIC:
                case TYPE_ATTRIBUTE_NESTED_PUBLIC:
                    write2File("public ");
                    break;
                case TYPE_ATTRIBUTE_NOT_PUBLIC:
                case TYPE_ATTRIBUTE_NESTED_FAM_AND_ASSEM:
                case TYPE_ATTRIBUTE_NESTED_ASSEMBLY:
                    write2File("internal ");
                    break;
                case TYPE_ATTRIBUTE_NESTED_PRIVATE:
                    write2File("private ");
                    break;
                case TYPE_ATTRIBUTE_NESTED_FAMILY:
                    write2File("protected ");
                    break;
                case TYPE_ATTRIBUTE_NESTED_FAM_OR_ASSEM:
                    write2File("protected internal ");
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
                uint32_t  fieldEnd = typeDefinition->fieldStart + typeDefinition->field_count;

                for (int i = typeDefinition->fieldStart; i < fieldEnd; ++i) {
                    const Il2CppFieldDefinition* fieldDef =  getFieldDefinitionByIndex(i);
                    const Il2CppType* fieldType = getTypeFromIl2CppTypeTableByIndex(fieldDef->typeIndex);
                    const Il2CppFieldDefaultValue* fieldDefaultValue = getFieldDefaultValueByIndex(i);

                    unsigned int access = fieldType->attrs & FIELD_ATTRIBUTE_FIELD_ACCESS_MASK;

                    switch (access){
                        case FIELD_ATTRIBUTE_PRIVATE:
                            write2File("\tprivate ");
                            break;
                        case FIELD_ATTRIBUTE_PUBLIC:
                            write2File("\tpublic ");
                            break;
                        case FIELD_ATTRIBUTE_FAMILY:
                            write2File("\tprotected ");
                            break;
                        case FIELD_ATTRIBUTE_ASSEMBLY:
                        case FIELD_ATTRIBUTE_FAM_AND_ASSEM:
                            write2File("\tinternal ");
                            break;
                        case FIELD_ATTRIBUTE_FAM_OR_ASSEM:
                            write2File("\tprotected internal ");
                            break;
                    }

                    if ((fieldType->attrs & FIELD_ATTRIBUTE_LITERAL) != 0)
                    {
                        write2File("const ");
                    }
                    else
                    {
                        if ((fieldType->attrs & FIELD_ATTRIBUTE_STATIC) != 0)
                            write2File("static ");
                        if ((fieldType->attrs & FIELD_ATTRIBUTE_INIT_ONLY) != 0)
                            write2File("readonly ");
                    }

                    write2File(format("%s %s", getTypeName(fieldType).data(),getStringByIndex(fieldDef->nameIndex)));
                    DLOG("\t\tfield: %s %s\n", getTypeName(fieldType).data(), getStringByIndex(fieldDef->nameIndex));
                    if (fieldDefaultValue && fieldDefaultValue->dataIndex != -1){
                        // need improve it if I have time, here we do not dump the field default value
                        // write2File("; // this field has default value, but I do not dump now\n");
                    }
                    write2File(";\n");

                }
            }
            // dump property
            if (typeDefinition->property_count){
                write2File("\n\t// Properties\n");
                uint32_t propertyEnd = typeDefinition->propertyStart + typeDefinition->property_count;

                for (int i = typeDefinition->propertyStart; i < propertyEnd; ++i) {
                    const Il2CppPropertyDefinition* propertyDefinition = getPropertyDefinitionByIndex(i);
                    write2File("\t");
                    if (propertyDefinition->get >= 0){
                        const Il2CppMethodDefinition* methodDefinition = getMethodDefinitionByIndex(typeDefinition->methodStart + propertyDefinition->get);
                        write2File(getMethodAttribute(methodDefinition).data());
                        const Il2CppType* propertyType = getTypeFromIl2CppTypeTableByIndex(methodDefinition->returnType);
                        write2File(format("%s %s { ", getTypeName(propertyType).data(), getStringByIndex(propertyDefinition->nameIndex)));
                        DLOG("\t\tproperty: %s %s [get]\n", getTypeName(propertyType).data(), getStringByIndex(propertyDefinition->nameIndex));
                    }else if(propertyDefinition->set >= 0){
                        const Il2CppMethodDefinition* methodDefinition = getMethodDefinitionByIndex(typeDefinition->methodStart + propertyDefinition->set);
                        write2File(getMethodAttribute(methodDefinition).data());
                        const Il2CppType* propertyType = getTypeFromIl2CppTypeTableByIndex(methodDefinition->returnType);
                        write2File(format("%s %s { ", getTypeName(propertyType).data(), getStringByIndex(propertyDefinition->nameIndex)));
                        DLOG("\t\tproperty: %s %s [set]\n", getTypeName(propertyType).data(), getStringByIndex(propertyDefinition->nameIndex));
                    }

                    if (propertyDefinition->get >= 0){
                        write2File("get; ");
                    }
                    if (propertyDefinition->set >= 0){
                        write2File("set; ");
                    }
                    write2File("}\n");
                }
            }
            // dump method
            if (typeDefinition->method_count > 0){
                write2File("\n\t// Methods\n");
                uint32_t methodEnd = typeDefinition->methodStart + typeDefinition->method_count;

                for (int i = typeDefinition->methodStart; i < methodEnd; ++i) {
                    const Il2CppMethodDefinition* methodDefinition = getMethodDefinitionByIndex(i);
                    write2File("\t");
                    write2File(getMethodAttribute(methodDefinition));
                    const Il2CppType* methodReturnType = getTypeFromIl2CppTypeTableByIndex(methodDefinition->returnType);
                    //const char* methodTypeName = getTypeName(methodReturnType).data();  // use this to DLOG lead to wrong encode
                    const char* methodName = getStringByIndex(methodDefinition->nameIndex);
                    write2File(format("%s %s(", typeName, getTypeName(methodReturnType).data(), methodName));
                    DLOG("\t\tmethod: %s %s();\n", getTypeName(methodReturnType).data(), methodName); 
                    for (int j = 0; j < methodDefinition->parameterCount; ++j) {
                        string parameterStr = "";
                        const Il2CppParameterDefinition* parameterDefinition = getParameterDefinitionByIndex(methodDefinition->parameterStart + j);
                        const char* parameterName = getStringByIndex(parameterDefinition->nameIndex);
                        const Il2CppType* parameterType = getTypeFromIl2CppTypeTableByIndex(parameterDefinition->typeIndex);
                        string parameterTypeName = getTypeName(parameterType);
                        if ((parameterType->attrs & PARAM_ATTRIBUTE_OPTIONAL) != 0)
                            parameterStr += "optional ";
                        if ((parameterType->attrs & PARAM_ATTRIBUTE_OUT) != 0)
                            parameterStr += "out ";
                        parameterStr += format("%s %s", parameterTypeName.data(), parameterName);
                        write2File(parameterStr);
                        if (j + 1 < methodDefinition->parameterCount){
                            write2File(", ");
                        }
                    }
                    write2File(");");
                    // dump offset
                    void* methodPointer = getMethodPointerIDAValueByIndex(methodDefinition->methodIndex);
                    write2File(format(" // RVA: 0x%lX Offset:0x%lx\n", methodPointer, RVA2RAW(methodPointer)));
                    if ((uint64_t)methodPointer != 0xFFFFFFFFFFFFFFFF){
                        write2Script(format("SetName(0x%lX, \"%s$$%s\")\n", methodPointer, typeName, methodName));
                    }
                    DLOG("\t\t\t RVA: 0x%lX Offset:0x%lx\n", methodPointer, RVA2RAW(methodPointer));
                }

            }

            write2File("\n}");
        }
    }
    dumpUsage();
}

void XIL2CppDumper::dumpUsage() {
    XDLOG("\ndump usage:\n");
    int usageListCount = metadataHeader->metadataUsageListsCount / sizeof(Il2CppMetadataUsageList);
    for (int n = 0; n < usageListCount; ++n) {
        uint32_t index = n;

        assert(this->metadataHeader->metadataUsageListsCount >= 0 && index <= static_cast<uint32_t>(this->metadataHeader->metadataUsageListsCount));

        const Il2CppMetadataUsageList* metadataUsageLists = MetadataOffset<const Il2CppMetadataUsageList*>(this->metadata, this->metadataHeader->metadataUsageListsOffset, index);
        uint32_t start = metadataUsageLists->start;
        uint32_t count = metadataUsageLists->count;
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
                    XILOG("[%d] %s  dstIndex:%d addr:0x%lX\n", i, theStr, destinationIndex, getUsageAddressIDAValueByIndex(destinationIndex));
                    write2Script(format("SetString(0x%lX, \"%s\")\n", getUsageAddressIDAValueByIndex(destinationIndex), toEscapedString(theStr)));
                    break;
                }

                default:
                    //std::cout << "not implemented" << std::endl;
                    break;
            }
        }
    }
}

// misc function

const char* XIL2CppDumper::toEscapedString(char *str){
    string retStr;
    while (*str) {
        switch (*str){
            case '\'':{
                retStr += "\\\'";
                break;
            }
            case '"':{
                retStr += "\\\"";
                break;
            }
            case '\t':{
                retStr += "\\\t";
                break;
            }
            case '\n':{
                retStr += "\\\n";
                break;
            }
            case '\r':{
                retStr += "\\\r";
                break;
            }
            case '\f':{
                retStr += "\\\f";
                break;
            }
            case '\b':{
                retStr += "\\\b";
                break;
            }
            case '\\':{
                retStr += "\\\\";
                break;
            }
            case '\0':{
                retStr += "\\\0";
                break;
            }
            default:
                retStr += *str;
                break;
        }
        str++;
    }
    return retStr.data();
}

string XIL2CppDumper::format(const char *fmt, ...) {
    int size = 512;
    char* buffer = 0;
    buffer = new char[size];
    va_list vl;
    va_start(vl, fmt);
    int nsize = vsnprintf(buffer, size, fmt, vl);
    if(size<=nsize){ //fail, delete buffer and try again
        va_end(vl);
        va_start(vl, fmt); // needed
        delete[] buffer;
        buffer = 0;
        buffer = new char[nsize+1]; //+1 for /0
        nsize = vsnprintf(buffer, nsize, fmt, vl);
    }
    std::string ret(buffer);
    va_end(vl);
    delete[] buffer;
    return ret;
}

void XIL2CppDumper::write2File(string str) {
    outfile << str;
}

void XIL2CppDumper::write2Script(string str) {
    scriptfile << str;
}

void XIL2CppDumper::clean() {
//    munmap(metadata, len);
//    munmap(il2cppbin, len);
    outfile.close();
    scriptfile.close();
}

// test function

void XIL2CppDumper::dumpAllImages() {
    // dump all image name

    const Il2CppImageDefinition* imagesDefinitions = (const Il2CppImageDefinition*)((const char*)metadata + metadataHeader->imagesOffset);
    int32_t imageCount = metadataHeader->imagesCount / sizeof(Il2CppImageDefinition);
    for (int32_t imageIndex = 0; imageIndex < imageCount; imageIndex++)
    {
        const Il2CppImageDefinition* imageDefinition = imagesDefinitions + imageIndex;
        const char* imageName = this->getStringByIndex(imageDefinition->nameIndex);
        XILOG("[%d] image name :%s type start:%d type count:%d\n", imageIndex, imageName,imageDefinition->typeStart ,imageDefinition->typeCount);
    }
}

void XIL2CppDumper::dumpString() {
    int usageListCount = metadataHeader->metadataUsageListsCount / sizeof(Il2CppMetadataUsageList);
    for (int n = 0; n < usageListCount; ++n) {
        uint32_t index = n;

        assert(this->metadataHeader->metadataUsageListsCount >= 0 && index <= static_cast<uint32_t>(this->metadataHeader->metadataUsageListsCount));

        const Il2CppMetadataUsageList* metadataUsageLists = MetadataOffset<const Il2CppMetadataUsageList*>(this->metadata, this->metadataHeader->metadataUsageListsOffset, index);
        uint32_t start = metadataUsageLists->start;
        uint32_t count = metadataUsageLists->count;
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
                    XILOG("[%d] %s  dstIndex:%d realIndex:%d addr:0x%lX\n", i, theStr, destinationIndex, destinationIndex+9652, getUsageAddressIDAValueByIndex(destinationIndex));
                    break;
                }

                default:
                    //std::cout << "not implemented" << std::endl;
                    break;
            }
        }
    }
}

void XIL2CppDumper::initScript() {
    write2Script("#encoding: utf-8\n");
    write2Script("import idaapi\n\n");
    write2Script("def SetString(addr, comm):\n");
    write2Script("\tglobal index\n");
    write2Script("\tname = \"StringLiteral_\" + str(index)\n");
    write2Script("\tret = idc.set_name(addr, name, SN_NOWARN)\n");
    write2Script("\tidc.set_cmt(addr, comm, 1)\n");
    write2Script("\tindex += 1\n");

    write2Script("def SetName(addr, name):\n");
    write2Script("\tret = idc.set_name(addr, name, SN_NOWARN | SN_NOCHECK)\n");
    write2Script("\tif ret == 0:\n");
    write2Script("\t\tnew_name = name + '_' + str(addr)\n");
    write2Script("\t\tret = idc.set_name(addr, new_name, SN_NOWARN | SN_NOCHECK)\n");

    write2Script("def MakeFunction(start, end):\n");
    write2Script("\tnext_func = idc.get_next_func(start)\n");
    write2Script("\tif next_func < end:\n");
    write2Script("\t\tend = next_func\n");
    write2Script("\tif idc.get_func_attr(start, FUNCATTR_START) == start:\n");
    write2Script("\t\tida_funcs.del_func(start)\n");
    write2Script("\tida_funcs.add_func(start, end)\n");

    write2Script("index = 1\n");
    write2Script("print('Making method name...')\n");
}
