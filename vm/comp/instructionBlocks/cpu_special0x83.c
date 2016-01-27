case 0: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("XOR rm32 imm8\n");
	#endif
	/*r/m32 ADD imm8.*/
	uint32_t op1;
	if(s_op.regmem_type == MEMORY)
		op1 = cpu_read_doubleword_from_mem(cpu_state, s_op.regmem_mem, DATA);
	else
		op1 = cpu_read_doubleword_from_reg(s_op.regmem_reg);

	int32_t op2 = (int8_t) cpu_consume_byte_from_mem(cpu_state);
	uint32_t result = op1 + op2;

	if(s_op.regmem_type == MEMORY)
		cpu_write_doubleword_in_mem(cpu_state, result, s_op.regmem_mem, DATA);
	else
		cpu_write_doubleword_in_reg(s_op.regmem_reg, result);

	cpu_set_overflow_add(cpu_state, op1, op2, result, !EIGHT_BIT);
	cpu_set_sign_flag(cpu_state, result, !EIGHT_BIT);
	cpu_set_zero_flag(cpu_state, result);
	cpu_set_carry_add(cpu_state, op1, result);
	cpu_set_parity_flag(cpu_state, result);


	return true;
}

case 6: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("XOR rm32 imm8\n");
	#endif
	/*r/m32 XOR imm8.*/
	uint32_t op1;
	if(s_op.regmem_type == MEMORY)
		op1 = cpu_read_doubleword_from_mem(cpu_state, s_op.regmem_mem, DATA);
	else
		op1 = cpu_read_doubleword_from_reg(s_op.regmem_reg);

	int32_t op2 = (int8_t) cpu_consume_byte_from_mem(cpu_state);
	uint32_t result = op1 ^ op2;

	if(s_op.regmem_type == MEMORY)
		cpu_write_doubleword_in_mem(cpu_state, result, s_op.regmem_mem, DATA);
	else
		cpu_write_doubleword_in_reg(s_op.regmem_reg, result);

	cpu_clear_flag(cpu_state, OVERFLOW_FLAG);
	cpu_clear_flag(cpu_state, CARRY_FLAG);
	cpu_set_sign_flag(cpu_state, result, !EIGHT_BIT);
	cpu_set_zero_flag(cpu_state, result);
	cpu_set_parity_flag(cpu_state, result);


	return true;
}

case 7: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("CMP rm32 imm8\n");
	#endif
	/*Compare imm8 with r/m32. */
	uint32_t subtrahend = (int8_t) cpu_consume_byte_from_mem(cpu_state);

	uint32_t minuend;
	if(s_op.regmem_type == MEMORY)
		minuend = cpu_read_doubleword_from_mem(cpu_state, s_op.regmem_mem, DATA);
	else
		minuend = cpu_read_doubleword_from_reg(s_op.regmem_reg);
	uint32_t result = minuend-subtrahend;

	cpu_set_eflag_arith(cpu_state, minuend, subtrahend, result,
			!EIGHT_BIT,SUBTRACTION);


	return true;

}
default:
	break;
