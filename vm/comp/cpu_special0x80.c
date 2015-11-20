case 0: {
	/*ADD r/m8  imm8.*/
	uint8_t op1;
	if(s_op.regmem_type == MEMORY)
		op1 = cpu_peek_byte_from_mem(cpu_state, s_op.regmem_mem);
	else
		op1 = cpu_read_byte_from_reg(s_op.regmem_reg, IS_HIGH(s_op.regmem));

	uint8_t op2 = cpu_read_byte_from_mem(cpu_state);
	uint8_t result = op1 + op2;

	if(s_op.regmem_type == MEMORY)
		cpu_write_byte_in_mem(cpu_state, result, s_op.regmem_mem);
	else
		cpu_write_byte_in_reg(s_op.regmem_reg, result, IS_HIGH(s_op.regmem));

	cpu_set_overflow_add(cpu_state, op1, op2, result, EIGHT_BIT);
	cpu_set_sign_flag(cpu_state, result, EIGHT_BIT);
	cpu_set_zero_flag(cpu_state, result);
	cpu_set_carry_add(cpu_state, op1, result);
	cpu_set_parity_flag(cpu_state, result);
	return true;
}

case 6: {
	/*r/m8 XOR imm8.*/
	uint8_t op1;
	if(s_op.regmem_type == MEMORY)
		op1 = cpu_peek_byte_from_mem(cpu_state, s_op.regmem_mem);
	else
		op1 = cpu_read_byte_from_reg(s_op.regmem_reg, IS_HIGH(s_op.regmem));

	uint8_t op2 = cpu_read_byte_from_mem(cpu_state);
	uint8_t result = op1 ^ op2;

	if(s_op.regmem_type == MEMORY)
		cpu_write_byte_in_mem(cpu_state, result, s_op.regmem_mem);
	else
		cpu_write_byte_in_reg(s_op.regmem_reg, result, IS_HIGH(s_op.regmem));

	cpu_clear_flag(cpu_state, OVERFLOW_FLAG);
	cpu_clear_flag(cpu_state, CARRY_FLAG);
	cpu_set_sign_flag(cpu_state, result, EIGHT_BIT);
	cpu_set_zero_flag(cpu_state, result);
	cpu_set_parity_flag(cpu_state, result);

	return true;
}

case 7: {
	/*Compare imm8 with r/m8. */
	uint8_t subtrahend = cpu_read_byte_from_reg(s_op.reg, IS_HIGH(s_op.reg));
	uint8_t minuend;
	if(s_op.regmem_type == MEMORY)
		minuend = cpu_peek_byte_from_mem(cpu_state, s_op.regmem_mem);
	else
		minuend = cpu_read_byte_from_reg(s_op.regmem_reg, IS_HIGH(s_op.regmem));
	uint8_t result = minuend-subtrahend;

	cpu_set_eflag_arith(cpu_state, minuend, subtrahend, result,
			EIGHT_BIT,SUBTRACTION);

	return true;
}
default:
	break;
