/* BFD support routines for the greyfox processor
   Copyright (C) 2024 Jake Merdich

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/greyfox.h"

static reloc_howto_type greyfox_elf_howto_table[] =
{
   // Note: this table must be indexable from R_GREYFOX_*
   HOWTO(R_GREYFOX_NONE, /* type*/
         0, /* right shift */
         0, /* size */
         0, /* bitsize*/
         false, /* pcrel */
         0, /* bitpos*/
         complain_overflow_dont,
         bfd_elf_generic_reloc, /* special func??? */
         "R_GREYFOX_NONE", /* name */
         false, /* partial implace */
         0, /* src mask*/
         0, /* dst mask*/
         0), /* pcrel offset */
   HOWTO(R_GREYFOX_ABS_16, /* type*/
         0, /* right shift */
         1, /* size (pow2) */
         16, /* bitsize*/
         false, /* pcrel */
         0, /* bitpos*/
         complain_overflow_unsigned,
         bfd_elf_generic_reloc, /* special func??? */
         "R_GREYFOX_ABS_16", /* name */
         false, /* partial implace */
         0, /* src mask*/
         0xFFFF, /* dst mask*/
         0), /* pcrel offset */
   HOWTO(R_GREYFOX_ABS_32, /* type*/
         0, /* right shift */
         2, /* size (pow2?) */
         32, /* bitsize*/
         false, /* pcrel */
         0, /* bitpos*/
         complain_overflow_unsigned,
         bfd_elf_generic_reloc, /* special func??? */
         "R_GREYFOX_ABS_32", /* name */
         false, /* partial implace */
         0, /* src mask*/
         0xFFFFFFFF, /* dst mask*/
         0), /* pcrel offset */
};

struct elf_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned char elf_reloc_val;
};

static const struct elf_reloc_map greyfox_reloc_map[] = {
   { BFD_RELOC_NONE, R_GREYFOX_NONE },
   { BFD_RELOC_32,   R_GREYFOX_ABS_32},
};

static reloc_howto_type*
greyfox_elfNN_type_lookup(bfd* abfd ATTRIBUTE_UNUSED,
                          bfd_reloc_code_real_type code)
{
   for (unsigned int i = 0; i < sizeof(greyfox_reloc_map)/sizeof(greyfox_reloc_map[0]); i++)
      if (greyfox_reloc_map[i].bfd_reloc_val == code)
         return &greyfox_elf_howto_table[(int)greyfox_reloc_map[i].elf_reloc_val];
   
   return NULL;
}

static reloc_howto_type*
greyfox_elfNN_name_lookup(bfd* abfd ATTRIBUTE_UNUSED,
                          const char* name)
{
   for (unsigned int i = 0; i < sizeof(greyfox_elf_howto_table)/sizeof(greyfox_elf_howto_table[0]); i++)
      if (greyfox_elf_howto_table[i].name != NULL &&
          strcasecmp(greyfox_elf_howto_table[i].name, name) == 0)
         return &greyfox_elf_howto_table[i];
   
   return NULL;
}

static bool
greyfox_elf_info_to_howto(bfd* abfd ATTRIBUTE_UNUSED,
			  arelent * cache_ptr,
			  Elf_Internal_Rela *dst)
{
	unsigned int r = ELFNN_R_TYPE(dst->r_info);

	if (r < R_GREYFOX_max)
	{
		cache_ptr->howto = &greyfox_elf_howto_table[r];
		return true;
	}
	/* xgettext:c-format */
	_bfd_error_handler (_("%pB: unsupported relocation type %#x"),
				abfd, r);
	bfd_set_error (bfd_error_bad_value);
	return false;

}

#define TARGET_LITTLE_SYM 	greyfox_elfNN_vec
#define TARGET_LITTLE_NAME	"elfNN-greyfox"
#define ELF_ARCH	bfd_arch_greyfox
#define ELF_MACHINE_CODE	EM_GREYFOX
#define ELF_MAXPAGESIZE	1
#define bfd_elfNN_bfd_reloc_type_lookup greyfox_elfNN_type_lookup
#define bfd_elfNN_bfd_reloc_name_lookup greyfox_elfNN_name_lookup
#define elf_info_to_howto	greyfox_elf_info_to_howto

#include "elfNN-target.h"
