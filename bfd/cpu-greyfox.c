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

const bfd_arch_info_type bfd_greyfox_arch =
  {
	64, /* 64 bits in a word*/
	64, /* 64 bits in an address */
	8, /* 8 bits in a byte */
	bfd_arch_greyfox,
	bfd_mach_greyfox,
	"greyfox", /* arch name */
	"greyfox", /* printable name */
	3, /* section alignment power*/
	true, /* This is the default (and only) machine for this type */
	bfd_default_compatible,
	bfd_default_scan,
	bfd_arch_default_fill,
	NULL,
	0 /* max offset into insn for reloc */
  };
