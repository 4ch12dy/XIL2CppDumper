//
// Created by xia0 on 2019/11/18.
//

#import "IL2CppBinParser.h"
#include <assert.h>
#include <exception>

IL2CppBinParser::IL2CppBinParser(void *bin, uint32_t version) {
    il2cppbin = bin;
    metadataVersion = version;
    uint32_t fileID = *((uint32_t*)il2cppbin);
    if (fileID == 0xFEEDFACF){
        isMacho64 = true;
        initWithMacho64();
    } else if(fileID == 0x464C457F){
        isElf64 = true;
//        fixElf64Relocation();
        initWithElf64();
    }else{
        assert(isMacho64 || isMacho || isElf64 || isElf);
    }
}

void* IL2CppBinParser::seek2Offset(uint64_t offset) {
    return (void*)((uint8_t*)il2cppbin + offset);
}

void* IL2CppBinParser::RAW2RVA(uint64_t raw) {
    if (isMacho64){
        return (void*)(raw + 0x100000000);
    }
    return (void*)raw;
}

uint64_t IL2CppBinParser::RVA2RAW(void *rva) {
    if (isElf64){
        uint64_t targetAddr = (uint64_t)rva;
        Elf64_Ehdr* elf64h = (Elf64_Ehdr*)il2cppbin;
        uint64_t elf64ProgramTablelOffset = elf64h->e_phoff;
        int elf64ProgramTableCount = elf64h->e_phnum;
        Elf64_Phdr* elf64ProgramTable = (Elf64_Phdr*)seek2Offset(elf64ProgramTablelOffset);
        for (int i = 0; i < elf64ProgramTableCount; ++i) {
            Elf64_Phdr* curPhdr = elf64ProgramTable + i;
            if (targetAddr >= curPhdr->p_vaddr && targetAddr <= curPhdr->p_vaddr + curPhdr->p_memsz){
                return targetAddr - (curPhdr->p_vaddr - curPhdr->p_offset);
            }
        }
        XDLOG("address not found elf64 progarm segment, here return orig address\n");
        return (uint64_t)rva;
    } else if (isMacho64){
        return (uint64_t)((uint64_t)rva - 0x100000000);
    }
}

void* IL2CppBinParser::idaAddr2MemAddr(void *idaAddr) {
    uint64_t offset = RVA2RAW(idaAddr);
    void* mem = seek2Offset(offset);
    // XDLOG("ida address:0x%lx il2cppbin address:0x%lx mem address:0x%lx\n", idaAddr, il2cppbin, mem);
    return mem;
}

int64_t IL2CppBinParser::fixElf64Relocation(uint64_t needFixAddr) {
    // http://www.sco.com/developers/gabi/2003-12-17/ch4.reloc.html
    // https://topsrc.cn/archives/170/
    // http://netwinder.osuosl.org/users/p/patb/public_html/elf_relocs.html
    Elf64_Ehdr* elfh = (Elf64_Ehdr*)il2cppbin;
    // .dynsym .dynstr
    XRange* range = elf64_get_sec_range_by_name(elfh, ".rela.dyn");

    Elf64_Rela* rela = (Elf64_Rela*)(range->start);
    int count = (range->end - range->start) / sizeof(Elf64_Rela);

    for (int i = 0; i < count; ++i) {
        Elf64_Rela* curRela = rela + i;
        uint64_t curOffset = curRela->r_offset;
        if(curOffset == needFixAddr){
            XDLOG("found target need fix relocation address:0x%lx to 0x%lx\n", needFixAddr, curRela->r_addend);
            return curRela->r_addend;
        }
    }
    XDLOG("not found target need fix relocation address, here return orig address\n");
    return needFixAddr;
}

void IL2CppBinParser::initWithElf64() {
    Elf64_Ehdr* elfh = (Elf64_Ehdr*)il2cppbin;
    XRange* range =  elf64_get_sec_range_by_name(elfh, ".init_array");
    assert(range);
    void* init_register_ida_addr = NULL;
    XILOG("found elf64 init array start addr:0x%lx end addr:0x%lx\n", range->start, range->end);
    uint32_t **start = (uint32_t**)range->start;
    uint32_t **end = (uint32_t**)range->end;

    /**
     * ADRP            X0, #_ZL35s_Il2CppCodegenRegistrationVariable@PAGE ; s_Il2CppCodegenRegistrationVariable
     * ADRP            X1, #_Z27s_Il2CppCodegenRegistrationv@PAGE ; s_Il2CppCodegenRegistration(void)
     * ADD             X0, X0, #_ZL35s_Il2CppCodegenRegistrationVariable@PAGEOFF ; s_Il2CppCodegenRegistrationVariable
     * ADD             X1, X1, #_Z27s_Il2CppCodegenRegistrationv@PAGEOFF ; s_Il2CppCodegenRegistration(void)
     * MOV             X2, XZR
     * MOV             W3, WZR
     * B               _ZN6il2cpp5utils35RegisterRuntimeInitializeAndCleanupC2EPFvvES3_i ;
     */

    for (uint32_t **p = start; p < end; ++p) {
        uint32_t insn_1 = arm64_insn_from_addr(idaAddr2MemAddr(*p));
        uint32_t insn_2 = arm64_insn_from_addr((void*)((uint32_t*)idaAddr2MemAddr(*p) + 1));
        uint32_t insn_3 = arm64_insn_from_addr((void*)((uint32_t*)idaAddr2MemAddr(*p) + 2));
        uint32_t insn_4 = arm64_insn_from_addr((void*)((uint32_t*)idaAddr2MemAddr(*p) + 3));
        uint32_t insn_5 = arm64_insn_from_addr((void*)((uint32_t*)idaAddr2MemAddr(*p) + 4));
        uint32_t insn_6 = arm64_insn_from_addr((void*)((uint32_t*)idaAddr2MemAddr(*p) + 5));
        if ((metadataVersion == 24) && arm64_is_adrp(insn_1) && arm64_is_adrp(insn_2) && arm64_is_add_imm(insn_3) && arm64_is_add_imm(insn_4) && arm64_is_movr(insn_5) && arm64_is_movr(insn_6)){
            XILOG("found init register func:0x%lx\n", *p);
            init_register_ida_addr = *p;
            break;
        }
    }
    uint32_t *ida_pc = (uint32_t*)init_register_ida_addr;
    uint32_t *mem_pc = (uint32_t*)idaAddr2MemAddr(init_register_ida_addr);

    ida_pc += 1;
    mem_pc += 1;
    uint32_t insn = arm64_insn_from_addr(mem_pc);
    void* ida_s_Il2CppCodegenRegistration_page = arm64_adrp_decode(ida_pc, insn);

    ida_pc += 2;
    mem_pc += 2;
    insn = arm64_insn_from_addr(mem_pc);
    uint32_t ida_s_Il2CppCodegenRegistration_page_offset = arm64_add_decode_imm(insn);
    void* ida_s_Il2CppCodegenRegistration = (void*)((uint8_t*)ida_s_Il2CppCodegenRegistration_page + ida_s_Il2CppCodegenRegistration_page_offset);
    XILOG("decode s_Il2CppCodegenRegistration adrress from bin:0x%lx\n", ida_s_Il2CppCodegenRegistration);

    ida_pc = (uint32_t*)ida_s_Il2CppCodegenRegistration;
    mem_pc = (uint32_t*)idaAddr2MemAddr(ida_s_Il2CppCodegenRegistration);
    insn = arm64_insn_from_addr(mem_pc);
    void* ida_g_MetadataRegistration_page = arm64_adrp_decode(ida_pc, insn);

    ida_pc += 1;
    mem_pc += 1;
    insn = arm64_insn_from_addr(mem_pc);
    int64_t ida_g_MetadataRegistration_page_offset = arm64_ldr_decode_imm(insn);
    uint64_t * metadataRegistration_ptr = (uint64_t *)((uint8_t*)ida_g_MetadataRegistration_page + ida_g_MetadataRegistration_page_offset);
    void* ida_metadataRegistration = (void*)fixElf64Relocation((uint64_t)metadataRegistration_ptr);
    XILOG("decode g_MetadataRegistration adrress from bin:0x%lx\n", metadataRegistration);

    ida_pc += 1;
    mem_pc += 1;
    insn = arm64_insn_from_addr(mem_pc);
    void* g_CodeRegistration_page = arm64_adrp_decode(ida_pc, insn);

    ida_pc += 2;
    mem_pc += 2;
    insn = arm64_insn_from_addr(mem_pc);
    uint32_t g_CodeRegistration_page_offset = arm64_add_decode_imm(insn);
    void* ida_codeRegistration = (void*)((uint8_t*)g_CodeRegistration_page + g_CodeRegistration_page_offset);
    XILOG("decode g_CodeRegistration adrress from bin:0x%lx\n", ida_codeRegistration);

    codeRegistration = idaAddr2MemAddr(ida_codeRegistration);
    metadataRegistration = idaAddr2MemAddr(ida_metadataRegistration);
}

void IL2CppBinParser::initWithMacho64() {
    mach_header_64* mh = (mach_header_64*)il2cppbin;
    XDLOG("macho header magic number:%X\n", mh->magic);
    XRange* range = macho64_get_sec_range_by_name(mh, "__DATA", "__mod_init_func");
    XDLOG("macho64 mod_init_func address start:0x%lx end:0x%lx\n", range->start, range->end);

    void* init_register_ida_addr = NULL;

    // search feature value to find il2cpp_codegen_register() function
    uint32_t ** mem_mod_init_func_start = (uint32_t **)idaAddr2MemAddr((void*)range->start);
    uint32_t ** mem_mod_init_func_end = (uint32_t **)idaAddr2MemAddr((void*)range->end);

    for (uint32_t **p = mem_mod_init_func_start; p < mem_mod_init_func_end; ++p) {
        uint32_t insn_1 = arm64_insn_from_addr(idaAddr2MemAddr(*p));
        uint32_t insn_2 = arm64_insn_from_addr((void*)((uint32_t*)idaAddr2MemAddr(*p) + 1));
        uint32_t insn_3 = arm64_insn_from_addr((void*)((uint32_t*)idaAddr2MemAddr(*p) + 2));
        uint32_t insn_4 = arm64_insn_from_addr((void*)((uint32_t*)idaAddr2MemAddr(*p) + 3));
        uint32_t insn_5 = arm64_insn_from_addr((void*)((uint32_t*)idaAddr2MemAddr(*p) + 4));
        uint32_t insn_6 = arm64_insn_from_addr((void*)((uint32_t*)idaAddr2MemAddr(*p) + 5));
        if ((metadataVersion == 24) && arm64_is_adrp(insn_1) && arm64_is_add_imm(insn_2) && arm64_is_adr(insn_3) && arm64_is_nop(insn_4) && arm64_is_movz(insn_5) && arm64_is_movz(insn_6)){
            XILOG("found init register func:0x%lx\n", *p);
            init_register_ida_addr = *p;
            break;
        }
    }

    assert(init_register_ida_addr);

    uint32_t * mem_pc = (uint32_t *)idaAddr2MemAddr(init_register_ida_addr);
    uint32_t* ida_pc = (uint32_t*)init_register_ida_addr;
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
    codeRegistration = (void*)((char*)il2cppbin + (uint64_t)ida_g_CodeRegistration - 0x100000000);
    metadataRegistration = (void*)((char*)il2cppbin + (uint64_t)ida_g_MetadataRegistration - 0x100000000);

}

