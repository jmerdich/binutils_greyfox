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

extern const greyfox_opc_info_t greyfox_opc_info[];

static fprintf_ftype fpr;
static void* stream;

bfd_signed_vma
sign_extend(bfd_vma input, int bit);

bfd_signed_vma
sign_extend(bfd_vma input, int bit)
{
	return ((bfd_signed_vma)input << (64 - bit)) >> (64 - bit);
}

int
print_insn_greyfox(bfd_vma addr, struct disassemble_info *info)
{
	int status;
	stream = info->stream;
	unsigned short opcode;
	fpr = info->fprintf_func;

	if ((status = info->read_memory_func(addr, (bfd_byte*)&opcode, 2, info)))
		goto fail;

	const struct greyfox_opc_info_t* opci;

	/* determine the opcode */
	for (opci = greyfox_opc_info; opci->name; opci++)
	{
		if ((opcode & opci->mask) != opci->opcode) {
			// wrong opcode
			continue;
		}
		if ((opci->type == GREYFOX_OPC_TYPE_TWO_OPERAND_SAME) &&
		    ((opcode & 0xF) != ((opcode >> 4) & 0xF))) {
			// need "same" but this instruction has different
			continue;
		}

		break; // Found the opcode! Last one is illegal.
	}

	// TODO: all sorts of type punning assumes little endian on this
	union {
		uint64_t u64;
		int64_t i64;
		double f64;
		float f32;
		bfd_byte bytes;
	} immdata;
	immdata.u64 = 0;
	if (opci->len > 2) {
		// Read the immediate if present. This must fail/pass before printing
		if ((status = info->read_memory_func(addr + 2, &immdata.bytes, opci->len - 2, info)))
			goto fail;
	}

	fpr(stream, "%s", opci->name);

	// Print registers and args in the opcode itself (not trailing immediate)
	bool immNeedsComma = false;
	switch (opci->type) {
		case GREYFOX_OPC_TYPE_BAD:
		case GREYFOX_OPC_TYPE_NO_OPERAND:
			break;
		case GREYFOX_OPC_TYPE_ONE_OPERAND:
		case GREYFOX_OPC_TYPE_TWO_OPERAND_SAME:
			if (opci->typeflags & GREYFOX_OPC_TYPEFLAG_A_IS_FLOAT) {
				fpr(stream, " f%d", opcode & 0xF);
			} else if (opci->typeflags & GREYFOX_OPC_TYPEFLAG_A_IS_CP) {
				fpr(stream, " cp_id%X", opcode & 0xF);
			} else {
				fpr(stream, " i%d", opcode & 0xF);
			}
			immNeedsComma = true;
			break;
		case GREYFOX_OPC_TYPE_TWO_OPERAND:
			OPCODES_ASSERT(!(opci->typeflags & GREYFOX_OPC_TYPEFLAG_A_IS_CP));
			fpr(stream,
			    " %c%d, %c%d",
			    (opci->typeflags & GREYFOX_OPC_TYPEFLAG_A_IS_FLOAT ? 'f' : 'i'),
			    opcode & 0xF,
			    (opci->typeflags & GREYFOX_OPC_TYPEFLAG_BC_IS_FLOAT ? 'f' : 'i'),
			    (opcode >> 4) & 0xF);
			immNeedsComma = true;
			break;
		case GREYFOX_OPC_TYPE_THREE_OPERAND:
			OPCODES_ASSERT(opci->len == 2);
			OPCODES_ASSERT(!(opci->typeflags & GREYFOX_OPC_TYPEFLAG_A_IS_CP));
			fpr(stream,
			    " %c%d, %c%d, %c%d",
			    (opci->typeflags & GREYFOX_OPC_TYPEFLAG_A_IS_FLOAT ? 'f' : 'i'),
			    opcode & 0xF,
			    (opci->typeflags & GREYFOX_OPC_TYPEFLAG_BC_IS_FLOAT ? 'f' : 'i'),
			    (opcode >> 4) & 0xF,
			    (opci->typeflags & GREYFOX_OPC_TYPEFLAG_BC_IS_FLOAT ? 'f' : 'i'),
			    (opcode >> 8) & 0xF);
			break;
		case GREYFOX_OPC_TYPE_SHORT_BRANCH:
			OPCODES_ASSERT(opci->len == 2);
			fpr(stream, " ");
			info->print_address_func(addr + sign_extend(opcode << 1 & 0xFFF, 12), info);
			break;
		case GREYFOX_OPC_TYPE_SVC:
		case GREYFOX_OPC_TYPE_HVC:
			OPCODES_ASSERT(opci->len == 2);
			// TODO: well-known hvc/svc calls
			fpr(stream, " 0x%x", opcode & 0x3FF);
			break;
	}

	if (opci->len > 2) {
		if (immNeedsComma) {
			fpr(stream, ",");
		}

		if (opci->typeflags & GREYFOX_OPC_TYPEFLAG_IMM_IS_ADDR) {
			fpr(stream, " ");
			OPCODES_ASSERT(!(opci->typeflags & GREYFOX_OPC_TYPEFLAG_IMM_IS_SIGNED));
			if (opci->typeflags & GREYFOX_OPC_TYPEFLAG_IMM_IS_PCREL) {
				immdata.u64 += addr;
			}
			info->print_address_func(immdata.u64, info);
		} else if (opci->typeflags & GREYFOX_OPC_TYPEFLAG_IMM_IS_FLOAT) {
			if (opci->len == 4) {
				// TODO: human readable float16
				fpr(stream, " 0x%04lx", immdata.u64);
			} else if (opci->len == 6) {
				fpr(stream, " %f", immdata.f32);
			} else {
				OPCODES_ASSERT(opci->len == 10);
				fpr(stream, " %f", immdata.f64);
			}
		} else if (opci->typeflags & GREYFOX_OPC_TYPEFLAG_IMM_IS_SIGNED) {
			immdata.i64 = sign_extend(immdata.u64, (opci->len - 2) * 8);
			fpr(stream, " %ld", immdata.i64);
		} else {
			fpr(stream, " %lu", immdata.u64);
		}
	}


	return opci->len;

fail:
	info->memory_error_func(status, addr, info);
	return -1;
}
