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

#define TARGET_LITTLE_SYM 	greyfox_elfNN_vec
#define TARGET_LITTLE_NAME	"elfNN-greyfox"
#define ELF_ARCH	bfd_arch_greyfox
#define ELF_MACHINE_CODE	EM_GREYFOX
#define ELF_MAXPAGESIZE	1
#define bfd_elfNN_bfd_reloc_type_lookup bfd_default_reloc_type_lookup
#define bfd_elfNN_bfd_reloc_name_lookup _bfd_norelocs_bfd_reloc_name_lookup
#define elf_info_to_howto	_bfd_elf_no_info_to_howto

#include "elfNN-target.h"
