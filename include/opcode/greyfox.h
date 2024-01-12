/* Opcode table for the greyfox processor
   Copyright (C) 2024 Jake Merdich
   
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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#define GREYFOX_OPC_TYPE_BAD                0x0
#define GREYFOX_OPC_TYPE_NO_OPERAND         0x1
#define GREYFOX_OPC_TYPE_ONE_OPERAND        0x2
#define GREYFOX_OPC_TYPE_TWO_OPERAND        0x3
#define GREYFOX_OPC_TYPE_THREE_OPERAND      0x4
#define GREYFOX_OPC_TYPE_SHORT_BRANCH       0x5
#define GREYFOX_OPC_TYPE_SVC                0x6
#define GREYFOX_OPC_TYPE_HVC                0x7
#define GREYFOX_OPC_TYPE_TWO_OPERAND_SAME   0x8

#define GREYFOX_OPC_TYPEFLAG_A_IS_FLOAT     0x0001
#define GREYFOX_OPC_TYPEFLAG_BC_IS_FLOAT    0x0002
#define GREYFOX_OPC_TYPEFLAG_REGS_ARE_FLOAT 0x0003
#define GREYFOX_OPC_TYPEFLAG_IMM_IS_FLOAT   0x0004
#define GREYFOX_OPC_TYPEFLAG_ALL_FLOAT      0x0007
#define GREYFOX_OPC_TYPEFLAG_A_IS_CP        0x0008
#define GREYFOX_OPC_TYPEFLAG_IMM_IS_SIGNED  0x0010
#define GREYFOX_OPC_TYPEFLAG_IMM_IS_PCREL   0x0020
#define GREYFOX_OPC_TYPEFLAG_IMM_IS_ADDR    0x0040
#define GREYFOX_OPC_TYPEFLAG_IMM_IS_RELADDR 0x0060


typedef struct greyfox_opc_info_t
{
	short opcode;    /* opcode (always within 16b )*/
	short mask;      /* mask of the opcode */
	char  type;      /* type of the op */
	char  len;       /* len of the op, including immediates */
	short typeflags; /* typeflags of the op (can derive len from this) */
	const char* name;
} greyfox_opc_info_t;
