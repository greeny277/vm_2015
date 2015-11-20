case 0x3C:{
	/*Compare imm8 with AL*/
	uint8_t subtrahend = cpu_consume_byte_from_mem(cpu_state);
	uint8_t minuend = (uint8_t)(cpu_state->eax);
	uint8_t result = minuend - subtrahend;

	cpu_set_eflag_arith(cpu_state, minuend, subtrahend, result,
			EIGHT_BIT,SUBTRACTION);
	return true;
}
case 0x3D: {
	/*Compare imm32 with AL*/
	uint32_t subtrahend = cpu_consume_word_from_mem(cpu_state);
	uint32_t minuend = cpu_state->eax;
	uint32_t result = minuend - subtrahend;

	cpu_set_eflag_arith(cpu_state, minuend, subtrahend, result,
			!EIGHT_BIT,SUBTRACTION);

	return true;
}

case 0x38: {
	/*Compare r8 with r/m8. */
	if(!cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT)){
		uint8_t minuend, subtrahend;

		subtrahend = cpu_read_byte_from_reg(s_op.reg, IS_HIGH(s_op.reg));
		if(s_op.regmem_type == MEMORY)
			minuend = cpu_read_byte_from_mem(cpu_state, s_op.regmem_mem);
		else
			minuend = cpu_read_byte_from_reg(s_op.regmem_reg, IS_HIGH(s_op.regmem));
		uint8_t result = minuend - subtrahend;

		cpu_set_eflag_arith(cpu_state, minuend, subtrahend, result,
				EIGHT_BIT,SUBTRACTION);
		return true;
	}
	break;
}
case 0x39: {
	/*Compare r32 with r/m32. */
	if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT)){
		uint32_t minuend, subtrahend;

		subtrahend = cpu_read_word_from_reg(s_op.reg);
		if(s_op.regmem_type == MEMORY)
			minuend = cpu_read_word_from_mem(cpu_state, s_op.regmem_mem);
		else
			minuend = cpu_read_word_from_reg(s_op.regmem_reg);
		uint32_t result = minuend - subtrahend;

		cpu_set_eflag_arith(cpu_state, minuend, subtrahend, result,
				!EIGHT_BIT,SUBTRACTION);
		return true;
	}
	break;
}
case 0x3A: {
	/* Compare r/m8 with r8. */
	if(!cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT)){
		uint8_t minuend, subtrahend;

		minuend = cpu_read_byte_from_reg(s_op.reg, IS_HIGH(s_op.reg));
		if(s_op.regmem_type == MEMORY)
			subtrahend = cpu_read_byte_from_mem(cpu_state, s_op.regmem_mem);
		else
			subtrahend = cpu_read_byte_from_reg(s_op.regmem_reg, IS_HIGH(s_op.regmem));
		uint8_t result = minuend - subtrahend;

		cpu_set_eflag_arith(cpu_state, minuend, subtrahend, result,
				EIGHT_BIT,SUBTRACTION);
		return true;
	}
	break;
}

case 0x3B: {
	/* Compare r/m32 with r32. */
	if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT)){
		uint32_t minuend, subtrahend;

		minuend = cpu_read_word_from_reg(s_op.reg);
		if(s_op.regmem_type == MEMORY)
			subtrahend = cpu_read_word_from_mem(cpu_state, s_op.regmem_mem);
		else
			subtrahend = cpu_read_word_from_reg(s_op.regmem_reg);
		uint32_t result = minuend - subtrahend;

		cpu_set_eflag_arith(cpu_state, minuend, subtrahend, result,
				!EIGHT_BIT,SUBTRACTION);
		return true;
	}
	break;
}
