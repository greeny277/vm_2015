case 0x20: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("AND r/m8, reg8\n");
	#endif

	if(likely(cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT))){
		uint8_t reg = cpu_read_byte_from_reg(s_op.reg, IS_HIGH(s_op.reg));

		uint8_t op1;
		if(s_op.regmem_type == MEMORY)
			op1 = cpu_read_byte_from_mem(cpu_state, s_op.regmem_mem, DATA);
		else
			op1 = cpu_read_byte_from_reg(s_op.regmem_reg, IS_HIGH(s_op.regmem));

		uint8_t result = reg & op1;
		
		if(s_op.regmem_type == MEMORY)
			cpu_write_byte_in_mem(cpu_state, result, s_op.regmem_mem, DATA);
		else
			cpu_write_byte_in_reg(s_op.regmem_reg, result, IS_HIGH(s_op.regmem));

		cpu_set_sign_flag(cpu_state, result, EIGHT_BIT);
		cpu_set_zero_flag(cpu_state, result);
		cpu_set_parity_flag(cpu_state, result);
		cpu_clear_flag(cpu_state, OVERFLOW_FLAG);
		cpu_clear_flag(cpu_state, CARRY_FLAG);

		return true;
	}
	break;
}

case 0x21: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("AND r/m32, reg32\n");
	#endif

	if(likely(cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT))){
		uint32_t reg = cpu_read_doubleword_from_reg(s_op.reg);

		uint32_t op1;
		if(s_op.regmem_type == MEMORY)
			op1 = cpu_read_doubleword_from_mem(cpu_state, s_op.regmem_mem, DATA);
		else
			op1 = cpu_read_doubleword_from_reg(s_op.regmem_reg);

		uint32_t result = reg & op1;
		
		if(s_op.regmem_type == MEMORY)
			cpu_write_doubleword_in_mem(cpu_state, result, s_op.regmem_mem, DATA);
		else
			cpu_write_doubleword_in_reg(s_op.regmem_reg, result);

		cpu_set_sign_flag(cpu_state, result, !EIGHT_BIT);
		cpu_set_zero_flag(cpu_state, result);
		cpu_set_parity_flag(cpu_state, result);
		cpu_clear_flag(cpu_state, OVERFLOW_FLAG);
		cpu_clear_flag(cpu_state, CARRY_FLAG);

		return true;
	}
	break;
}

case 0x22: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("AND r8 rm8 \n");
	#endif

	/*AND r8 r/m8.*/
	if(likely(cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT))){
		uint8_t op1 = cpu_read_byte_from_reg(s_op.reg, IS_HIGH(s_op.reg));
		uint8_t op2;
		if(s_op.regmem_type == MEMORY)
			op2 = cpu_read_byte_from_mem(cpu_state, s_op.regmem_mem, DATA);
		else
			op2 = cpu_read_byte_from_reg(s_op.regmem_reg, IS_HIGH(s_op.regmem));

		uint8_t result = op1 & op2;

		cpu_write_byte_in_reg(s_op.reg, result, IS_HIGH(s_op.reg));

		cpu_set_overflow_add(cpu_state, op1, op2, result, EIGHT_BIT);
		cpu_set_sign_flag(cpu_state, result, EIGHT_BIT);
		cpu_set_zero_flag(cpu_state, result);
		cpu_set_carry_add(cpu_state, op1, result);
		cpu_set_parity_flag(cpu_state, result);


		return true;

	}
	break;
}
case 0x23: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("AND r32 rm32 \n");
	#endif

	/*ADD r32 r/m32.*/
	if(likely(cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT))){
		uint32_t op1 = cpu_read_doubleword_from_reg(s_op.reg);
		uint32_t op2;
		if(s_op.regmem_type == MEMORY)
			op2 = cpu_read_doubleword_from_mem(cpu_state, s_op.regmem_mem, DATA);
		else
			op2 = cpu_read_doubleword_from_reg(s_op.regmem_reg);

		uint32_t result = op1 & op2;

		cpu_write_doubleword_in_reg(s_op.reg, result);

		cpu_set_overflow_add(cpu_state, op1, op2, result, !EIGHT_BIT);
		cpu_set_sign_flag(cpu_state, result, !EIGHT_BIT);
		cpu_set_zero_flag(cpu_state, result);
		cpu_set_carry_add(cpu_state, op1, result);
		cpu_set_parity_flag(cpu_state, result);

		return true;
	}
	break;
}
case 0x24: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("AND AL, imm8\n");
	#endif

	uint8_t imm = cpu_consume_byte_from_mem(cpu_state);
	uint8_t al = cpu_read_byte_from_reg(&(cpu_state->eax),!HIGH_BYTE);
	
	uint8_t result = al & imm;

	cpu_set_parity_flag(cpu_state, result);
	cpu_set_zero_flag(cpu_state, result);
	cpu_set_sign_flag(cpu_state, result, EIGHT_BIT);

	cpu_clear_flag(cpu_state, OVERFLOW_FLAG);
	cpu_clear_flag(cpu_state, CARRY_FLAG);
	cpu_write_byte_in_reg(&(cpu_state->eax), result, !HIGH_BYTE);
	
	return true;
}

case 0x25: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("AND EAX, imm32\n");
	#endif

	uint32_t imm = cpu_consume_doubleword_from_mem(cpu_state);
	uint32_t eax = cpu_read_doubleword_from_reg(&(cpu_state)->eax);

	uint32_t result = imm & eax;
	
	cpu_set_parity_flag(cpu_state, result);

	cpu_set_zero_flag(cpu_state, result);
	cpu_set_sign_flag(cpu_state, result, !EIGHT_BIT);

	cpu_clear_flag(cpu_state, OVERFLOW_FLAG);
	cpu_clear_flag(cpu_state, CARRY_FLAG);

	cpu_write_doubleword_in_reg(&(cpu_state->eax), result);

	return true;
}
