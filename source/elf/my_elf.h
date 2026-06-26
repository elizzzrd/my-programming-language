#pragma once

#include <stdio.h>
#include <elf.h>
#include <stdint.h>
#include <stdbool.h>



bool write_elf_header(FILE * f, uint64_t entry, uint64_t phoff, uint16_t phnum);
bool write_program_header(FILE * f, uint32_t type, uint32_t flags,
                        uint64_t offset, uint64_t vaddr,
                        uint64_t filesz, uint64_t memsz);