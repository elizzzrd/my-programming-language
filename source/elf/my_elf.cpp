#include <elf.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "my_elf.h"

bool write_elf_header(FILE * f, uint64_t entry, uint64_t phoff, uint16_t phnum)
{
    assert(f);

    Elf64_Ehdr header = {
        .e_ident = {
            ELFMAG0,
            ELFMAG1,
            ELFMAG2,
            ELFMAG3,
            ELFCLASS64,
            ELFDATA2LSB,
            EV_CURRENT,
            ELFOSABI_SYSV,
            0, 0, 0, 0, 0, 0, 0, 0
        },
        .e_type = ET_EXEC,
        .e_machine = EM_X86_64,
        .e_version = EV_CURRENT,
        .e_entry = entry,
        .e_phoff = phoff,
        .e_shoff = 0,                   // no sections
        .e_flags = 0,
        .e_ehsize = sizeof(Elf64_Ehdr),
        .e_phentsize = sizeof(Elf64_Phdr),
        .e_phnum = phnum,
        .e_shentsize = 0,               // no sections
        .e_shnum = 0,
        .e_shstrndx = SHN_UNDEF
    };

    size_t written = fwrite(&header, 1, sizeof(header), f);
    if (written != sizeof(header))
    {
        perror("fwrite");
        return false;
    }

    return true;
}



bool write_program_header(FILE * f, uint32_t type, uint32_t flags,
                        uint64_t offset, uint64_t vaddr,
                        uint64_t filesz, uint64_t memsz)
{
    assert(f);

    Elf64_Phdr phdr = {
        .p_type = type,
        .p_flags = flags,
        .p_offset = offset,
        .p_vaddr = vaddr,
        .p_paddr = vaddr,
        .p_filesz = filesz,
        .p_memsz = memsz,
        .p_align = 0x1000
    };

    size_t written = fwrite(&phdr, 1, sizeof(phdr), f);
    if (written != sizeof(phdr))
    {
        perror("fwrite");
        return false;
    }

    return true;
}
