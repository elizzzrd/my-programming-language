#include "emiters.h"

#include <assert.h>
#include <cstdint>
#include <cstring>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#include "my_elf.h"


// ============================== SECTION ==============================

void section_init(section_t * section, size_t capacity)
{
    assert(section);

    section->data = (uint8_t *) calloc(capacity, 1);
    section->size = 0;
    section->capacity = capacity;
}


void section_destroy(section_t * section)
{
    assert(section);

    free(section->data);
    section->data = NULL;
    section->size = 0;
    section->capacity = 0;
}

static void check_capacity(section_t * section, size_t add)
{
    assert(section);

    if (section->size + add >= section->capacity)
    {
        size_t new_cap = section->capacity * 2;
        if (new_cap < section->size + add)
            new_cap = section->size + add + 64;
        section->data = (u_int8_t *) realloc(section->data, new_cap);
        section->capacity = new_cap;
    }
}


void section_emit_byte(section_t * section, uint8_t byte)
{
    assert(section);
    check_capacity(section, 1);
    section->data[section->size++] = byte;
}
#define EMIT(_X_) section_emit_byte(section, (_X_))


void section_emit_bytes(section_t * section, const void * data, size_t len)
{
    assert(section);
    check_capacity(section, len);
    memcpy(section->data + section->size, data, len);
    section->size += len;
}


// REX.W 
static inline uint8_t make_rex(int dst_ext, int src_ext) {
    uint8_t rex = REX_PREFIX | REX_W;  
    if (dst_ext) rex |= REX_R;
    if (src_ext) rex |= REX_B;
    return rex;
}

static inline uint8_t make_rex_wb(int src_ext) {
    return REX_PREFIX | REX_W | (src_ext ? REX_B : 0);
}

// REX.WR 
static inline uint8_t make_rex_wr(int dst_ext) {
    return REX_PREFIX | REX_W | (dst_ext ? REX_R : 0);
}

// ================================= EMITERS ==================================


void emit_push(section_t * section, reg_t reg)
{
    assert(section);

    const uint8_t op_code = 0x50;
    if (REG_EXTENDED(reg))
    {
        EMIT(REX_PREFIX | REX_B);
        EMIT(op_code + REG_CODE(reg));
    }
    else
    {
        EMIT(op_code + reg);
    }
}

void emit_pop(section_t * section, reg_t reg)
{
    assert(section);

    const uint8_t op_code = 0x58;
    if (REG_EXTENDED(reg))
    {
        EMIT(REX_PREFIX | REX_B);
        EMIT(op_code + REG_CODE(reg));
    }
    else
    {
        EMIT(op_code + reg);
    }
}


void emit_sub_rsp(section_t * section, uint32_t n)
{
    assert(section);

    if (n == 0)     return;
    
    if (n < 128)
    {
        // sub rsp, byte n -- 48 83 EC n
        EMIT(0x48);
        EMIT(0x83);
        EMIT(0xEC);
        EMIT((uint8_t) n);
    }
    else
    {
        // sub rsp, dword n -- 48 81 EC n
        EMIT(0x48);
        EMIT(0x81);
        EMIT(0xEC);
        section_emit_bytes(section, &n, 4);
    }
}


void emit_add_rsp(section_t * section, uint32_t n)
{
    assert(section);

    if (n == 0)     return;
    
    if (n < 128)
    {
        // add rsp, byte n -- 48 83 C4 n
        EMIT(0x48);
        EMIT(0x83);
        EMIT(0xC4);
        EMIT((uint8_t) n);
    }
    else
    {
        // add rsp, dword n -- 48 81 C4 n
        EMIT(0x48);
        EMIT(0x81);
        EMIT(0xC4);
        section_emit_bytes(section, &n, 4);
    }
}


void emit_call(section_t * section, int64_t addr)
{
    assert(section);
    EMIT(0xE8);
    section_emit_bytes(section, &addr, 4);
}

void emit_ret(section_t * section)
{
    assert(section);
    EMIT(0xC3);
}

void emit_syscall(section_t * section)
{
    assert(section);
    EMIT(0x0F);
    EMIT(0x05);
}

void emit_nop(section_t* section)
{
    assert(section);
    EMIT(0x90);
}

void emit_int3(section_t* section)
{
    assert(section);
    EMIT(0xCC);
}


// ----------------------------------------------------------------------------

// mov dst, src -- 48 89 /r
void emit_move_rr(section_t * section, reg_t dst, reg_t src)
{
    assert(section);

    uint8_t rex = make_rex(REG_EXTENDED(dst), REG_EXTENDED(src));
    EMIT(rex);
    EMIT(0x89);
    EMIT(MODRM_REG_REG | (REG_CODE(dst) << 3) | REG_CODE(src));
}

// mov r64, imm64 -- 48 B8 + rd imm64
void emit_mov_ri(section_t * section, reg_t dst, uint64_t imm)
{
    assert(section);

    uint8_t rex = make_rex_wb(REG_EXTENDED(dst));
    EMIT(rex);
    EMIT(0xB8 + REG_CODE(dst));
    section_emit_bytes(section, &imm, 8);
}

// from mem: movsd xmm_dst, [base + offset] 
void emit_movsd_rm(section_t * section, int xmm_dst, reg_t base, int32_t offset)
{
    assert(section);

    EMIT(0xF2);
    EMIT(0x0F);
    EMIT(0x10);
    
    if (REG_EXTENDED(base))
        EMIT(REX_PREFIX | REX_B);

    uint8_t modrm;
    if (offset == 0) 
    {
        modrm = (MODRM_MOD_NO_DISP << 6) | ((xmm_dst & 0b0111) << 3) | REG_CODE(base);
    } 
    else if (offset >= -128 && offset <= 127) {
        modrm = (MODRM_MOD_DISP8 << 6) | ((xmm_dst & 0b0111) << 3) | REG_CODE(base);
        EMIT(modrm);
        EMIT((uint8_t)offset);
        return;
    } 
    else 
    {
        modrm = (MODRM_MOD_DISP32 << 6) | ((xmm_dst & 0b0111) << 3) | REG_CODE(base);
        EMIT(modrm);
        section_emit_bytes(section, &offset, 4);
        return;
    }
    EMIT(modrm);
}   


// in mem: movsd [base + offset], xmm_src
void emit_movsd_mr(section_t * section, reg_t base, int32_t offset, int xmm_src)
{
    assert(section);

    EMIT(0xF2);
    EMIT(0x0F);
    EMIT(0x11);
    
    if (REG_EXTENDED(base))
        EMIT(REX_PREFIX | REX_B);

    uint8_t modrm;
    if (offset == 0) 
    {
        modrm = (MODRM_MOD_NO_DISP << 6) | ((xmm_src & 0b0111) << 3) | REG_CODE(base);
    } 
    else if (offset >= -128 && offset <= 127) {
        modrm = (MODRM_MOD_DISP8 << 6) | ((xmm_src & 0b0111) << 3) | REG_CODE(base);
        EMIT(modrm);
        EMIT((uint8_t)offset);
        return;
    } 
    else 
    {
        modrm = (MODRM_MOD_DISP32 << 6) | ((xmm_src & 0b0111) << 3) | REG_CODE(base);
        EMIT(modrm);
        section_emit_bytes(section, &offset, 4);
        return;
    }
    EMIT(modrm);
}   

// movsd xmm_dst, xmm_src
void emit_movsd(section_t * section, int xmm_dst, int xmm_src)
{
    assert(section);

    EMIT(0xF2);
    EMIT(0x0F);
    EMIT(0x10);
    EMIT(MODRM_REG_REG | ((xmm_dst & 0b0111)) | (xmm_src & 0b0111));
}

// ----------------------------------------------------------------------------

// cmp r64, r64 -- 48 39 /r
void emit_cmp(section_t * section, reg_t reg_a, reg_t reg_b)
{
    assert(section);

    uint8_t rex = make_rex(REG_EXTENDED(reg_a), REG_EXTENDED(reg_b));
    EMIT(rex);
    EMIT(0x39);
    EMIT(MODRM_REG_REG | (REG_CODE(reg_b) << 3) | (REG_CODE(reg_a)));
}

// add r/m, r
void emit_add_rr(section_t * section, reg_t reg_dest, reg_t reg_src)
{
    assert(section);

    uint8_t rex = make_rex(REG_EXTENDED(reg_dest), REG_EXTENDED(reg_src));
    EMIT(rex);
    EMIT(0x01);
    EMIT(MODRM_REG_REG | (REG_CODE(reg_src) << 3) | (REG_CODE(reg_dest)));
}

void emit_sub_rr(section_t * section, reg_t reg_dest, reg_t reg_src)
{
    assert(section);

    uint8_t rex = make_rex(REG_EXTENDED(reg_dest), REG_EXTENDED(reg_src));
    EMIT(rex);
    EMIT(0x29);
    EMIT(MODRM_REG_REG | (REG_CODE(reg_src) << 3) | (REG_CODE(reg_dest)));
}

// div r64
void emit_div_rr(section_t * section, reg_t reg_dest, reg_t reg_src)
{
    assert(section);

    uint8_t rex = make_rex(0, REG_EXTENDED(reg_src));
    EMIT(rex);
    EMIT(0xF7);
    EMIT(0xF0 | REG_CODE(reg_src));
}

// mul r64
void emit_mul_rr(section_t * section, reg_t reg_dest, reg_t reg_src)
{
    assert(section);

    uint8_t rex = make_rex(0, REG_EXTENDED(reg_src));
    EMIT(rex);
    EMIT(0xF7);
    EMIT(0xE0 | REG_CODE(reg_src));
}



// ----------------------------------------------------------------------------

// F2 0F opcode /r
static void emit_sse2_xmm_xmm(section_t * section, uint8_t opcode, int xmm_dst, int xmm_src)
{
    assert(section);

    EMIT(0xF2);
    EMIT(0x0F);
    EMIT(opcode);
    EMIT(MODRM_REG_REG | ((xmm_dst & 0b0111) << 3) | (xmm_src & 0b0111));
}
#define EMIT_SSE(_opcode_)   emit_sse2_xmm_xmm(section, (_opcode_), xmm_dst, xmm_src);


void emit_addsd(section_t * section, int xmm_dst, int xmm_src)
{
    assert(section);
    EMIT_SSE(0x58);
}

void emit_subsd(section_t * section, int xmm_dst, int xmm_src)
{
    assert(section);
    EMIT_SSE(0x5C);
}

void emit_mulsd(section_t * section, int xmm_dst, int xmm_src)
{
    assert(section);
    EMIT_SSE(0x59);
}

void emit_divsd(section_t * section, int xmm_dst, int xmm_src)
{
    assert(section);
    EMIT_SSE(0x5E);
}

void emit_xorpd(section_t * section, int xmm_dst, int xmm_src)
{
    assert(section);
    EMIT(0x66);
    EMIT(0x0F);
    EMIT(0x57);
    EMIT(MODRM_REG_REG | ((xmm_dst & 0b0111) << 3) | (xmm_src & 0b0111));
}

void emit_comisd(section_t * section, int xmm_a, int xmm_b)
{
    assert(section);
    EMIT(0x66);
    EMIT(0x0F);
    EMIT(0x2F);
    EMIT(MODRM_REG_REG | ((xmm_a & 0b0111) << 3) | (xmm_b & 0b0111));
}

// F2 0F 2A /r
void emit_cvtsi2sd(section_t * section, int xmm_dst, reg_t src)
{
    assert(section);
    EMIT(0xF2);
    EMIT(0x0F);
    EMIT(0X2A);
    
    uint8_t rex = REX_PREFIX | REX_W | (REG_EXTENDED(src) ? REX_B : 0);
    EMIT(rex);
    EMIT(MODRM_REG_REG | ((xmm_dst & 0b0111) << 3) | (src & 0b0111));
}


// F2 0F 2D /r
void emit_cvtsd2si(section_t * section, reg_t dst, int xmm_src)
{
    assert(section);
    EMIT(0xF2);
    EMIT(0x0F);
    EMIT(0X2D);
    
    uint8_t rex = REX_PREFIX | REX_W | (REG_EXTENDED(dst) ? REX_R : 0);
    EMIT(rex);
    EMIT(MODRM_REG_REG | ((dst & 0b0111) << 3) | (xmm_src & 0b0111));
}

// ----------------------------------------------------------------------------

void emit_jmp_rel8(section_t * section, int8_t offset)
{
    assert(section);
    EMIT(0xEB);
    EMIT((uint8_t) offset);
}

void emit_je_rel8(section_t * section, int8_t offset)
{
    assert(section);
    EMIT(0x74);
    EMIT((uint8_t) offset);
}

void emit_jne_rel8(section_t * section, int8_t offset)
{
    assert(section);
    EMIT(0x75);
    EMIT((uint8_t) offset);
}

void emit_jg_rel8(section_t * section, int8_t offset)
{
    assert(section);
    EMIT(0x7F);
    EMIT((uint8_t) offset);
}

void emit_jge_rel8(section_t * section, int8_t offset)
{
    assert(section);
    EMIT(0x7D);
    EMIT((uint8_t) offset);
}

void emit_jl_rel8(section_t * section, int8_t offset)
{
    assert(section);
    EMIT(0x7C);
    EMIT((uint8_t) offset);
}

void emit_jle_rel8(section_t * section, int8_t offset)
{
    assert(section);
    EMIT(0x7E);
    EMIT((uint8_t) offset);
}

void emit_jmp_rel32(section_t * section, int32_t offset)
{
    assert(section);
    EMIT(0xE9);
    section_emit_bytes(section, &offset, 4);
}
