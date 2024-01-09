/* Disassemble greyfox instructions.
   Copyright (C) 2024 Jake Merdich

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include <stdio.h>
#define STATIC_TABLE
#define DEFINE_TABLE

#include "opcode/greyfox.h"
#include "disassemble.h"

extern const greyfox_opc_info_t greyfox_opc_info[128];

static fprintf_ftype fpr;
static void* stream;

int
print_insn_greyfox(bfd_vma addr, struct disassemble_info *info)
{
	int status;
	stream = info->stream;
	unsigned char opcode;
	fpr = info->fprintf_func;

	if ((status = info->read_memory_func(addr, &opcode, 1, info)))
		goto fail;

	fpr(stream, "%s", greyfox_opc_info[opcode].name);

	return 1;

fail:
	info->memory_error_func(status, addr, info);
	return -1;
}
