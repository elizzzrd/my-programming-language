#include <stdint.h>
#include <stdio.h>

#include "my_elf.h"

typedef struct 
{
    uint8_t * data;
    size_t    size;
    size_t    capacity;
} section_t;

void section_init(section_t * section, size_t capacity);
void section_destroy(section_t * section);
void section_emit_byte(section_t * section, uint8_t byte);
void section_emit_bytes(section_t * section, const void * data, size_t len);


typedef enum {
    RAX = 0,
    RCX = 1,
    RDX = 2,
    RBX = 3,
    RSP = 4,
    RBP = 5,
    RSI = 6,
    RDI = 7,
    R8  = 8,
    R9  = 9,
    R10 = 10,
    R11 = 11,
    R12 = 12,
    R13 = 13,
    R14 = 14,
    R15 = 15
} reg_t;

#define REX_PREFIX  0b0100'0000   // 0x40
#define REX_W       0b0000'1000   // 64-bit operand
#define REX_R       0b0000'0100   // (ModRM)
#define REX_X       0b0000'0010   // (SIB)
#define REX_B       0b0000'0001   

#define REX_WB      (REX_PREFIX | REX_W | REX_B)
#define REX_WR      (REX_PREFIX | REX_W | REX_R)
#define REX_WRB     (REX_PREFIX | REX_W | REX_R | REX_B)
#define REX_WX      (REX_PREFIX | REX_W | REX_X)
#define REX_WXB     (REX_PREFIX | REX_W | REX_X | REX_B)

#define REG_EXTENDED(reg)   ((reg) >= 8)
#define REG_CODE(reg)       ((reg) & 7)

// ModRm mod
typedef enum {
    MODRM_MOD_REG        = 0b11,       // 3 - reg-reg
    MODRM_MOD_DISP32     = 0b10,       // 2 - [base + disp32]
    MODRM_MOD_DISP8      = 0b01,       // 1 - [base + disp8]
    MODRM_MOD_NO_DISP    = 0b00,       // 0 - [base]
} modrm_mod_t;

#define MODRM_REG_REG (MODRM_MOD_REG << 6)  // 0xC0


// ================================= STACK =================================

void emit_push(section_t * section, reg_t reg);
void emit_pop(section_t * section, reg_t reg);
void emit_sub_rsp(section_t * section, uint32_t n);
void emit_add_rsp(section_t * section, uint32_t n);

// ================================== CALL/RET/JUMPS ================================

void emit_call(section_t * section, int64_t addr); 
void emit_ret(section_t * section);

void emit_jmp_rel8(section_t * section, int8_t offset);
void emit_je_rel8(section_t * section, int8_t offset);
void emit_jne_rel8(section_t * section, int8_t offset);
void emit_jg_rel8(section_t * section, int8_t offset);
void emit_jge_rel8(section_t * section, int8_t offset);
void emit_jl_rel8(section_t * section, int8_t offset);
void emit_jle_rel8(section_t * section, int8_t offset);
void emit_jmp_rel32(section_t * section, int32_t offset);

void emit_syscall(section_t* section);

// ==================================== MOVE ==================================

void emit_move_rr(section_t * section, reg_t reg_dest, reg_t reg_src);
void emit_mov_ri(section_t * section, reg_t dst, uint64_t imm);

// ================================ ARITHMETIC ================================

void emit_cmp(section_t * section, reg_t reg_a, reg_t reg_b);
void emit_add_rr(section_t * section, reg_t reg_dest, reg_t reg_src);
void emit_sub_rr(section_t * section, reg_t reg_dest, reg_t reg_src);
void emit_div_rr(section_t * section, reg_t reg_dest, reg_t reg_src);
void emit_mul_rr(section_t * section, reg_t reg_dest, reg_t reg_src);


// ================================== XMM ==================================
// from mem: movsd xmm_dst, [base + offset]
void emit_movsd_rm(section_t * section, int xmm_dst, reg_t base, int32_t offset);
// in mem: movsd [base + offset], xmm_src
void emit_movsd_mr(section_t * section, reg_t base, int32_t offset, int xmm_src);
// movsd xmm_dst, xmm_src
void emit_movsd(section_t * section, int xmm_dst, int xmm_src);


void emit_addsd(section_t * sec, int xmm_dst, int xmm_src);
void emit_subsd(section_t * sec, int xmm_dst, int xmm_src);
void emit_mulsd(section_t * sec, int xmm_dst, int xmm_src);
void emit_divsd(section_t * sec, int xmm_dst, int xmm_src);
void emit_xorpd(section_t * sec, int xmm_dst, int xmm_src);
void emit_comisd(section_t * sec, int xmm_a, int xmm_b);
void emit_cvtsi2sd(section_t * sec, int xmm_dst, reg_t src);
void emit_cvtsd2si(section_t * sec, reg_t dst, int xmm_src);


void emit_nop(section_t* section);
void emit_int3(section_t* section);