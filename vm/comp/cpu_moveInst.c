case 0x88:
	/* Copy r8 to r/m8 */
	if(!cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT, !IMMEDIATE)){
		uint8_t src = cpu_read_byte_from_reg(s_op.op1_reg, s_op.is_op1_high);
		if(s_op.op2_reg != 0){
			/* Write in a register */
			cpu_write_byte_in_reg(src, s_op.op2_reg, s_op.is_op2_high);
		} else {
			/* Write in memory */
			cpu_write_byte_in_ram(cpu_state, src, s_op.op2_mem);
		}
		return true;
	}
	break;
case 0x89:
	/* Copy r32 to r/m32 */
	if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT, !IMMEDIATE)){
		uint32_t src = *(s_op.op1_reg);
		if(s_op.op2_reg != 0){
			/* Write in a register */
			cpu_write_word_in_reg(src, s_op.op2_reg);
		} else {
			/* Write in memory */
			cpu_write_word_in_ram(cpu_state, src, s_op.op2_mem);
		}
		return true;
	}
	break;

case 0x8A:
	/* Copy r/m8 to r8. */
	if(!cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT, !IMMEDIATE)){
		uint8_t src;
		if(s_op.op2_reg != 0){
			/* Write in a register */
			src = cpu_read_byte_from_reg(s_op.op2_reg, s_op.is_op2_high);
		} else {
			/* Write in memory */
			src = cpu_peek_byte_from_ram(cpu_state, s_op.op2_mem);
		}
		cpu_write_byte_in_reg(src, s_op.op1_reg, s_op.is_op1_high);
		return true;
	}
	break;

case 0x8B:
	/* Copy r/m32 to r32. */
	if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT, !IMMEDIATE)){
		uint32_t src;
		if(s_op.op2_reg != 0){
			/* Write in a register */
			src = cpu_read_word_from_reg(s_op.op2_reg);
		} else {
			/* Write in memory */
			src = cpu_peek_word_from_ram(cpu_state, s_op.op2_mem);
		}
		cpu_write_word_in_reg(src, s_op.op1_reg);
		return true;
	}
	break;

case 0xA0:
	/* Copy byte at (seg:offset) to AL */
	fprintf(stderr, "0xA0 is not implemented yet\n");
	break;

case 0xA1:
	/* Copy doubleword at (seg:offset) to EAX */
	fprintf(stderr, "0xA1 is not implemented yet\n");
	break;

case 0xA2:
	/* Copy AL to (seg:offset) */
	fprintf(stderr, "0xA2 is not implemented yet\n");
	break;

case 0xA3:
	/* Copy EAX to (seg:offset) */
	fprintf(stderr, "0xA3 is not implemented yet\n");
	break;

case 0xB0:
	/* Copy imm8 to AL */
	eight_bit_src = cpu_read_byte_from_ram(cpu_state);
	cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->eax), !HIGH_BYTE);
	return true;

case 0xB1:
	/* Copy imm8 to CL */
	eight_bit_src = cpu_read_byte_from_ram(cpu_state);
	cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->ecx), !HIGH_BYTE);
	return true;

case 0xB2:
	/* Copy imm8 to DL */
	eight_bit_src = cpu_read_byte_from_ram(cpu_state);
	cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->edx), !HIGH_BYTE);
	return true;

case 0xB3:
	/* Copy imm8 to BL */
	eight_bit_src = cpu_read_byte_from_ram(cpu_state);
	cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->ebx), !HIGH_BYTE);
	return true;

case 0xB4:
	/* Copy imm8 to AH */
	eight_bit_src = cpu_read_byte_from_ram(cpu_state);
	cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->eax), HIGH_BYTE);
	return true;

case 0xB5:
	/* Copy imm8 to CH */
	eight_bit_src = cpu_read_byte_from_ram(cpu_state);
	cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->ecx), HIGH_BYTE);
	return true;

case 0xB6:
	/* Copy imm8 to DH */
	eight_bit_src = cpu_read_byte_from_ram(cpu_state);
	cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->edx), HIGH_BYTE);
	return true;

case 0xB7:
	/* Copy imm8 to BH */
	eight_bit_src = cpu_read_byte_from_ram(cpu_state);
	cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->ebx), HIGH_BYTE);
	return true;

case 0xB8:
	/* Copy imm32 to EAX */
	four_byte_src = cpu_read_word_from_ram(cpu_state);
	cpu_write_word_in_reg(four_byte_src, &(cpu_state->eax));
	return true;

case 0xB9:
	/* Copy imm32 to ECX */
	four_byte_src = cpu_read_word_from_ram(cpu_state);
	cpu_write_word_in_reg(four_byte_src, &(cpu_state->ecx));
	return true;

case 0xBA:
	/* Copy imm32 to EDX */
	four_byte_src = cpu_read_word_from_ram(cpu_state);
	cpu_write_word_in_reg(four_byte_src, &(cpu_state->edx));
	return true;

case 0xBB:
	/* Copy imm32 to EBX */
	four_byte_src = cpu_read_word_from_ram(cpu_state);
	cpu_write_word_in_reg(four_byte_src, &(cpu_state->ebx));
	return true;

case 0xC6:
	/* Copy imm8 to r/m8. */
	if(!cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT, IMMEDIATE)){
		uint8_t src = s_op.op1_const;
		if(s_op.op2_reg != 0){
			/* Write in a register */
			cpu_write_byte_in_reg(src, s_op.op2_reg, s_op.is_op2_high);
		} else {
			/* Write in memory */
			cpu_write_byte_in_ram(cpu_state, src, s_op.op2_mem);
		}
		return true;
	}
	break;
case 0xC7:
	/* Copy imm32 to r/m32. */
	if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT, IMMEDIATE)){
		uint32_t src = s_op.op1_const;
		if(s_op.op2_reg != 0){
			/* Write in a register */
			cpu_write_word_in_reg(src, s_op.op2_reg);
		} else {
			/* Write in memory */
			cpu_write_word_in_ram(cpu_state, src, s_op.op2_mem);
		}
		return true;
	}
	break;

/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */