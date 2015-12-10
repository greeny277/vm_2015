case 0: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("ADD rm32 imm32\n");
	#endif
	/*r/m32 ADD imm32.*/
	uint8_t op1;
	if(s_op.regmem_type == MEMORY)
		op1 = cpu_read_word_from_mem(cpu_state, s_op.regmem_mem);
	else
		op1 = cpu_read_word_from_reg(s_op.regmem_reg);

	uint32_t op2 = cpu_consume_word_from_mem(cpu_state);
	uint32_t result = op1 + op2;

	if(s_op.regmem_type == MEMORY)
		cpu_write_word_in_mem(cpu_state, result, s_op.regmem_mem);
	else
		cpu_write_word_in_reg(s_op.regmem_reg, result);

	cpu_set_overflow_add(cpu_state, op1, op2, result, !EIGHT_BIT);
	cpu_set_sign_flag(cpu_state, result, !EIGHT_BIT);
	cpu_set_zero_flag(cpu_state, result);
	cpu_set_carry_add(cpu_state, op1, result);
	cpu_set_parity_flag(cpu_state, result);


	return true;
}

case 6: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("XOR rm32 imm32\n");
	#endif
	/*r/m32 XOR imm32.*/
	uint8_t op1;
	if(s_op.regmem_type == MEMORY)
		op1 = cpu_read_word_from_mem(cpu_state, s_op.regmem_mem);
	else
		op1 = cpu_read_word_from_reg(s_op.regmem_reg);

	uint32_t op2 = cpu_consume_word_from_mem(cpu_state);
	uint32_t result = op1 ^ op2;

	if(s_op.regmem_type == MEMORY)
		cpu_write_word_in_mem(cpu_state, result, s_op.regmem_mem);
	else
		cpu_write_word_in_reg(s_op.regmem_reg, result);

	cpu_clear_flag(cpu_state, OVERFLOW_FLAG);
	cpu_clear_flag(cpu_state, CARRY_FLAG);
	cpu_set_sign_flag(cpu_state, result, !EIGHT_BIT);
	cpu_set_zero_flag(cpu_state, result);
	cpu_set_parity_flag(cpu_state, result);


	return true;
}
case 7: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("CMP rm32 imm32\n");
	#endif
	/*Compare imm32 with r/m32. */
	uint32_t subtrahend = cpu_consume_word_from_mem(cpu_state);
	uint32_t minuend;
	if(s_op.regmem_type == MEMORY)
		minuend = cpu_read_word_from_mem(cpu_state, s_op.regmem_mem);
	else
		minuend = cpu_read_word_from_reg(s_op.regmem_reg);
	uint32_t result = minuend-subtrahend;

	cpu_set_eflag_arith(cpu_state, minuend, subtrahend, result,
			!EIGHT_BIT,SUBTRACTION);


	return true;
}
default:
	break;
