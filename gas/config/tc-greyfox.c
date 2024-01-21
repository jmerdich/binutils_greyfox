/* tc-greyfox.c -- Assemble code for greyfox
   Copyright 2024 Jake Merdich

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to
   the Free Software Foundation, 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#include "as.h"
#include "safe-ctype.h"
#include "opcode/greyfox.h"
#include <assert.h>

extern const greyfox_opc_info_t greyfox_opc_info[128];

const char comment_chars[]        = "#";
const char line_separator_chars[] = ";";
const char line_comment_chars[]   = "#";

static int pending_reloc;
static htab_t opcode_hash_control;

const pseudo_typeS md_pseudo_table[] =
{
  {0, 0, 0}
};

const char FLT_CHARS[] = "rRsSfFdDxXpP";
const char EXP_CHARS[] = "eE";

void
md_operand (expressionS *op __attribute__((unused)))
{
  /* Empty for now. */
}

/* This function is called once, at assembler startup time.  It sets
   up the hash table with all the opcodes in it, and also initializes
   some aliases for compatibility with other assemblers.  */

void
md_begin (void)
{
  const greyfox_opc_info_t *opcode;
  opcode_hash_control = str_htab_create ();

  /* Insert names into hash table.  */
  for (opcode = greyfox_opc_info; opcode->name; opcode++)
    str_hash_insert (opcode_hash_control, opcode->name, (char *) opcode, 0);

  bfd_set_arch_mach (stdoutput, TARGET_ARCH, 0);
}

static int
parse_register_operand(char** ptr, char kind)
{
	int reg = 0;
	if (**ptr != kind)
	{
		// Not 'i##' or 'f##'
		goto bad_reg;
	}
	(*ptr)++;

	if (**ptr > '9' || **ptr < '0')
	{
		// Need at least one digit
		goto bad_reg;
	}
	reg = **ptr - '0';
	(*ptr)++;

	if (reg > 0 && **ptr <= '9' && **ptr >= '0')
	{
		// Try to parse second digit if present
		reg = (reg * 10) + (**ptr - '0');
		(*ptr)++;
	}

	if (**ptr <= '9' && **ptr >= '0')
	{
		// 3+ digit numbers are bad
		goto bad_reg;
	}

	if (reg > 15)
	{
		// Out of bounds
		goto bad_reg;
	}
	return reg;

	bad_reg:
		as_bad("expecting register %c0-%c15", kind, kind);
		ignore_rest_of_line();
		return -1;
}

/* This is the guts of the machine-dependent assembler.  STR points to
   a machine dependent instruction.  This function is supposed to emit
   the frags/bytes it assembles to.  */

void
md_assemble (char *str)
{
  char *op_start;
  char *op_end;

  greyfox_opc_info_t *opcode;
  char *output;
  char pend;

  int nlen = 0;

  /* Drop leading whitespace.  */
  while (*str == ' ')
    str++;

  /* Find the op code end.  */
  op_start = str;
  for (op_end = str;
       *op_end && !is_end_of_line[*op_end & 0xff] && *op_end != ' ';
       op_end++)
    nlen++;

  pend = *op_end;
  *op_end = 0;

  if (nlen == 0)
    as_bad (_("can't find opcode "));

  opcode = (greyfox_opc_info_t *) str_hash_find (opcode_hash_control, op_start);
  *op_end = pend;

  if (opcode == NULL)
    {
      as_bad (_("unknown opcode %s"), op_start);
      return;
    }

  output = frag_more (opcode->len);
  uint16_t instr = opcode->opcode;
  bool     parsedReg = false;
  switch (opcode->type)
  {
	case GREYFOX_OPC_TYPE_NO_OPERAND:
		break;
	case GREYFOX_OPC_TYPE_ONE_OPERAND:
	case GREYFOX_OPC_TYPE_TWO_OPERAND_SAME:
		parsedReg = true;
		while (ISSPACE(*op_end))
		  op_end++;
		if (opcode->typeflags & GREYFOX_OPC_TYPEFLAG_A_IS_CP)
		{
			// TODO
			as_bad("cp instructions not implemented!");
		}
		else
		{
		  char kind = (opcode->typeflags & GREYFOX_OPC_TYPEFLAG_A_IS_FLOAT) ? 'f' : 'i';
		  int regA = parse_register_operand(&op_end, kind);
		  instr |= regA;
		  if (opcode->type == GREYFOX_OPC_TYPE_TWO_OPERAND_SAME)
		    instr |= (regA << 4);
		}
		break;
	case GREYFOX_OPC_TYPE_TWO_OPERAND:
		parsedReg = true;
		while (ISSPACE(*op_end))
		  op_end++;
		{
		  char kindA = (opcode->typeflags & GREYFOX_OPC_TYPEFLAG_A_IS_FLOAT) ? 'f' : 'i';
		  int regA = parse_register_operand(&op_end, kindA);
		
		  if (*op_end != ',')
		   as_warn ("expecting comma delimited register operands");
		  op_end++;
		  while (ISSPACE(*op_end))
		    op_end++;

		  char kindB = (opcode->typeflags & GREYFOX_OPC_TYPEFLAG_BC_IS_FLOAT) ? 'f' : 'i';
		  int regB = parse_register_operand(&op_end, kindB);

		  instr |= regA;
		  instr |= (regB << 4);
		}
		break;
	case GREYFOX_OPC_TYPE_THREE_OPERAND:
		parsedReg = true;
		while (ISSPACE(*op_end))
		  op_end++;
		{
		  char kindA = (opcode->typeflags & GREYFOX_OPC_TYPEFLAG_A_IS_FLOAT) ? 'f' : 'i';
		  int regA = parse_register_operand(&op_end, kindA);
		
		  if (*op_end != ',')
		   as_warn ("expecting comma delimited register operands");
		  op_end++;
		  while (ISSPACE(*op_end))
		    op_end++;

		  char kindB = (opcode->typeflags & GREYFOX_OPC_TYPEFLAG_BC_IS_FLOAT) ? 'f' : 'i';
		  int regB = parse_register_operand(&op_end, kindB);

		  if (*op_end != ',')
		   as_warn ("expecting comma delimited register operands");
		  op_end++;
		  while (ISSPACE(*op_end))
		    op_end++;
		  int regC = parse_register_operand(&op_end, kindB);

		  instr |= regA;
		  instr |= (regB << 4);
		  instr |= (regC << 8);
		}
		break;
	case GREYFOX_OPC_TYPE_SHORT_BRANCH:
		while (ISSPACE(*op_end))
		  op_end++;
		{
			char* save = input_line_pointer;
			input_line_pointer = op_end;
			expressionS op;
			expression(&op);
			op_end = input_line_pointer;
			input_line_pointer = save;
			fix_new_exp(frag_now,
				(output - frag_now->fr_literal),
				2,
				&op,
				true,
				BFD_RELOC_12_PCREL);
		}
		break;
	case GREYFOX_OPC_TYPE_SVC:
	case GREYFOX_OPC_TYPE_HVC:
		while (ISSPACE(*op_end))
		  op_end++;
		{
			char* save = input_line_pointer;
			input_line_pointer = op_end;
			expressionS op = {0};
			expression_and_evaluate(&op);
			op_end = input_line_pointer;
			input_line_pointer = save;

			if (op.X_op != O_constant)
			{
				as_bad("Bad expression");
			}
			if (op.X_add_number < 0 || op.X_add_number > 0x1FF)
			{
				as_bad("Out of range");
			}
			instr += op.X_add_number;
		}
		break;
	default:
		abort();
  }

  // Parse immediates
  if (opcode->len > 2)
  {
	if (parsedReg)
	{
	  if (*op_end != ',')
	   as_warn ("expecting comma delimited register operands");
	  op_end++;
	}	
	while (ISSPACE(*op_end))
	  op_end++;

	if (opcode->typeflags & GREYFOX_OPC_TYPEFLAG_IMM_IS_FLOAT)
	{
		// TODO
		as_bad("float immediates not implemented!");
	}
	else
	{
		// Any integer can be a calculated value, so use that
		bool isPcRel = (opcode->typeflags & GREYFOX_OPC_TYPEFLAG_IMM_IS_PCREL) != 0;
		char* save = input_line_pointer;
		input_line_pointer = op_end;
		expressionS op;
		expression(&op);
		op_end = input_line_pointer;
		input_line_pointer = save;
		enum bfd_reloc_code_real relocType = 0;
		if (isPcRel) {
			switch (opcode->len) {
				case 4: 
				    relocType = BFD_RELOC_16_PCREL;
				    break;
				case 6: 
				    relocType = BFD_RELOC_32_PCREL;
				    break;
				case 10: 
				    relocType = BFD_RELOC_64_PCREL;
				    break;
			}
		} else {
			switch (opcode->len) {
				case 3: 
				    relocType = BFD_RELOC_8;
				    break;
				case 4: 
				    relocType = BFD_RELOC_16;
				    break;
				case 6: 
				    relocType = BFD_RELOC_32;
				    break;
				case 10: 
				    relocType = BFD_RELOC_64;
				    break;
			}

		}
		
		fix_new_exp(frag_now,
		            (output + 2 - frag_now->fr_literal),
			    opcode->len - 2,
			    &op,
			    isPcRel,
			    relocType);

	}
  }

  md_number_to_chars(output, instr, 2);
  
  while (ISSPACE (*op_end))
    op_end++;
    
  if (*op_end != 0)
    as_warn ("extra stuff on line ignored");
   
  if (pending_reloc)
    as_bad ("Something forgot to clean up\n");
}

/* Turn a string in input_line_pointer into a floating point constant
   of type type, and store the appropriate bytes in *LITP.  The number
   of LITTLENUMS emitted is stored in *SIZEP .  An error message is
   returned, or NULL on OK.  */

const char *
md_atof (int type, char *litP, int *sizeP)
{
  int prec;
  LITTLENUM_TYPE words[4];
  char *t;
  int i;

  switch (type)
    {
    case 'f':
      prec = 2;
      break;

    case 'd':
      prec = 4;
      break;

    default:
      *sizeP = 0;
      return _("bad call to md_atof");
    }

  t = atof_ieee (input_line_pointer, type, words);
  if (t)
    input_line_pointer = t;

  *sizeP = prec * 2;

  for (i = prec - 1; i >= 0; i--)
    {
      md_number_to_chars (litP, (valueT) words[i], 2);
      litP += 2;
    }

  return NULL;
}

const char *md_shortopts = "";

struct option md_longopts[] =
{
  {NULL, no_argument, NULL, 0}
};
size_t md_longopts_size = sizeof (md_longopts);

/* We have no target specific options yet, so these next
   two functions are empty.  */
int
md_parse_option (int c ATTRIBUTE_UNUSED, const char *arg ATTRIBUTE_UNUSED)
{
  return 0;
}

void
md_show_usage (FILE *stream ATTRIBUTE_UNUSED)
{
}

/* Apply a fixup to the object file.  */

void
md_apply_fix (fixS *fixP, valueT * valP, segT seg ATTRIBUTE_UNUSED)
{
  char *buf = fixP->fx_where + fixP->fx_frag->fr_literal;
  valueT val = *valP;
  valueT max, min;
//   int shift;

  max = min = 0;
//   shift = 0;
  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_12_PCREL:
      buf[0] = val >> 1;
      buf[1] |= (val >> 9) & 0x7;
      break;
    case BFD_RELOC_8:
      *buf++ = val;
      break;
    case BFD_RELOC_16:
    case BFD_RELOC_16_PCREL:
      *buf++ = val >> 0;
      *buf++ = val >> 8;
      break;
    case BFD_RELOC_32:
    case BFD_RELOC_32_PCREL:
      *buf++ = val >> 0;
      *buf++ = val >> 8;
      *buf++ = val >> 16;
      *buf++ = val >> 24;
      break;
    case BFD_RELOC_64:
    case BFD_RELOC_64_PCREL:
      *buf++ = val >> 0;
      *buf++ = val >> 8;
      *buf++ = val >> 16;
      *buf++ = val >> 24;
      *buf++ = val >> 32;
      *buf++ = val >> 40;
      *buf++ = val >> 48;
      *buf++ = val >> 56;
      break;

    default:
      abort ();
    }

  if (max != 0 && (val < min || val > max))
    as_bad_where (fixP->fx_file, fixP->fx_line, _("offset out of range"));

  if (fixP->fx_addsy == NULL && fixP->fx_pcrel == 0)
    fixP->fx_done = 1;
}


/* Put number into target byte order (little endian).  */

void
md_number_to_chars (char *ptr, valueT use, int nbytes)
{
  number_to_chars_littleendian (ptr, use, nbytes);
}

/* Translate internal representation of relocation info to BFD target
   format.  */

arelent *
tc_gen_reloc (asection *section ATTRIBUTE_UNUSED, fixS *fixp)
{
  arelent *rel;
  bfd_reloc_code_real_type r_type;

  rel = xmalloc (sizeof (arelent));
  rel->sym_ptr_ptr = xmalloc (sizeof (asymbol *));
  *rel->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  rel->address = fixp->fx_frag->fr_address + fixp->fx_where;

  r_type = fixp->fx_r_type;
  rel->addend = fixp->fx_addnumber;
  rel->howto = bfd_reloc_type_lookup (stdoutput, r_type);

  if (rel->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("Cannot represent relocation type %s"),
		    bfd_get_reloc_code_name (r_type));
      /* Set howto to a garbage value so that we can keep going.  */
      rel->howto = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_32);
      assert (rel->howto != NULL);
    }

  return rel;
}
