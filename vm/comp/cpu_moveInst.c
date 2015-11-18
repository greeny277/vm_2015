case 0x88:
	/* Copy r8 to r/m8 */
	if(!cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT)){
		uint8_t src = cpu_read_byte_from_reg(s_op.reg, s_op.reg_type == REGISTER_HIGH);
		if(s_op.regmem_type == MEMORY){
			cpu_write_byte_in_mem(cpu_state, src, s_op.regmem_mem);
		} else {
			cpu_write_byte_in_reg(src, s_op.regmem_reg, s_op.regmem_type == REGISTER_HIGH);
		}
		return true;
	}
	break;
case 0x89:
	/* Copy r32 to r/m32 */
	if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT)){
		uint32_t src = cpu_read_word_from_reg(s_op.reg);
		if(s_op.regmem_type == MEMORY){
			cpu_write_word_in_mem(cpu_state, src, s_op.regmem_mem);
		} else {
			cpu_write_word_in_reg(src, s_op.regmem_reg);
		}
		return true;
	}
	break;

case 0x8A:
	/* Copy r/m8 to r8. */
	if(!cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT)){
		uint8_t src;
		if(s_op.regmem_type == MEMORY){
			src = cpu_peek_byte_from_mem(cpu_state, s_op.regmem_mem);
		} else {
			src = cpu_read_byte_from_reg(s_op.regmem_reg, s_op.regmem_type == REGISTER_HIGH);
		}
		cpu_write_byte_in_reg(src, s_op.reg, s_op.regmem_type == REGISTER_HIGH);
		return true;
	}
	break;

case 0x8B:
	/* Copy r/m32 to r32. */
	if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT)){
		uint32_t src;
		if(s_op.regmem_type == MEMORY){
			src = cpu_peek_word_from_mem(cpu_state, s_op.regmem_mem);
		} else {
			src = cpu_read_word_from_reg(s_op.regmem_reg);
		}
		cpu_write_word_in_reg(src, s_op.reg);
		return true;
	}
	break;

case 0xA0: {
	/* Copy byte at (seg:offset) to AL */
	uint8_t src_addr = cpu_read_byte_from_mem(cpu_state);
	uint8_t src_byte = cpu_peek_byte_from_mem(cpu_state, src_addr);
	cpu_write_byte_in_reg(src_byte, &(cpu_state->eax), !HIGH_BYTE);

	return true;
}

case 0xA1: {
	/* Copy doubleword at (seg:offset) to EAX */
	uint32_t src_addr = cpu_read_word_from_mem(cpu_state);
	uint32_t src_doubleword = cpu_peek_word_from_mem(cpu_state, src_addr);
	cpu_write_word_in_reg(src_doubleword, &(cpu_state->eax));

	return true;
}

case 0xA2: {
	/* Copy AL to (seg:offset) */
	uint32_t dest_addr = cpu_read_byte_from_mem(cpu_state);
	uint8_t src_byte = cpu_read_byte_from_reg(&(cpu_state->eax), !HIGH_BYTE);

	cpu_write_byte_in_mem(cpu_state, src_byte, dest_addr);

	return true;
}

case 0xA3: {
	/* Copy EAX to (seg:offset) */

	uint32_t dest_addr = cpu_read_word_from_mem(cpu_state);
	uint32_t src_doubleword = cpu_read_word_from_reg(&(cpu_state->eax));

	cpu_write_word_in_mem(cpu_state, src_doubleword, dest_addr);

	return true;
}

case 0xB0:
	/* Copy imm8 to AL */
	eight_bit_src = cpu_read_byte_from_mem(cpu_state);
	cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->eax), !HIGH_BYTE);
	return true;

case 0xB1:
	/* Copy imm8 to CL */
	eight_bit_src = cpu_read_byte_from_mem(cpu_state);
	cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->ecx), !HIGH_BYTE);
	return true;

case 0xB2:
	/* Copy imm8 to DL */
	eight_bit_src = cpu_read_byte_from_mem(cpu_state);
	cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->edx), !HIGH_BYTE);
	return true;

case 0xB3:
	/* Copy imm8 to BL */
	eight_bit_src = cpu_read_byte_from_mem(cpu_state);
	cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->ebx), !HIGH_BYTE);
	return true;

case 0xB4:
	/* Copy imm8 to AH */
	eight_bit_src = cpu_read_byte_from_mem(cpu_state);
	cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->eax), HIGH_BYTE);
	return true;

case 0xB5:
	/* Copy imm8 to CH */
	eight_bit_src = cpu_read_byte_from_mem(cpu_state);
	cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->ecx), HIGH_BYTE);
	return true;

case 0xB6:
	/* Copy imm8 to DH */
	eight_bit_src = cpu_read_byte_from_mem(cpu_state);
	cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->edx), HIGH_BYTE);
	return true;

case 0xB7:
	/* Copy imm8 to BH */
	eight_bit_src = cpu_read_byte_from_mem(cpu_state);
	cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->ebx), HIGH_BYTE);
	return true;

case 0xB8:
	/* Copy imm32 to EAX */
	four_byte_src = cpu_read_word_from_mem(cpu_state);
	cpu_write_word_in_reg(four_byte_src, &(cpu_state->eax));
	return true;

case 0xB9:
	/* Copy imm32 to ECX */
	four_byte_src = cpu_read_word_from_mem(cpu_state);
	cpu_write_word_in_reg(four_byte_src, &(cpu_state->ecx));
	return true;

case 0xBA:
	/* Copy imm32 to EDX */
	four_byte_src = cpu_read_word_from_mem(cpu_state);
	cpu_write_word_in_reg(four_byte_src, &(cpu_state->edx));
	return true;

case 0xBB:
	/* Copy imm32 to EBX */
	four_byte_src = cpu_read_word_from_mem(cpu_state);
	cpu_write_word_in_reg(four_byte_src, &(cpu_state->ebx));
	return true;

/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
