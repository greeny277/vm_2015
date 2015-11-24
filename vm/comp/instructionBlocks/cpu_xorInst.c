case 0x30:{
	/*r/m8 XOR r8.*/
	if(cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT)){
		uint8_t src;
		if(s_op.regmem_type == MEMORY)
			src = cpu_read_byte_from_mem(cpu_state, s_op.regmem_mem);
		else
			src = cpu_read_byte_from_reg(s_op.regmem_reg, IS_HIGH(s_op.regmem));

		uint8_t imm = cpu_read_byte_from_reg(s_op.reg, IS_HIGH(s_op.reg));
		uint8_t result = src ^ imm;

		if(s_op.regmem_type == MEMORY)
			cpu_write_byte_in_mem(cpu_state, result, s_op.regmem_mem);
		else
			cpu_write_byte_in_reg(s_op.regmem_reg, result, IS_HIGH(s_op.regmem));

		cpu_clear_flag(cpu_state, OVERFLOW_FLAG);
		cpu_clear_flag(cpu_state, CARRY_FLAG);
		cpu_set_sign_flag(cpu_state, result, EIGHT_BIT);
		cpu_set_zero_flag(cpu_state, result);
		cpu_set_parity_flag(cpu_state, result);

		#ifdef DEBUG_PRINT_INST
		fprintf(stderr, "XOR rm8 r8\n");
		#endif

		return true;
	}
	break;
}

case 0x31:{
	/*r/m32 XOR r32.*/
	if(cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT)){
		uint32_t op1;
		if(s_op.regmem_type == MEMORY)
			op1 = cpu_read_word_from_mem(cpu_state, s_op.regmem_mem);
		else
			op1 = cpu_read_word_from_reg(s_op.regmem_reg);

		uint32_t op2 = cpu_read_word_from_reg(s_op.reg);
		uint32_t result = op1 ^ op2;

		if(s_op.regmem_type == MEMORY)
			cpu_write_word_in_mem(cpu_state, result, s_op.regmem_mem);
		else
			cpu_write_word_in_reg(s_op.regmem_reg, result);

		cpu_clear_flag(cpu_state, OVERFLOW_FLAG);
		cpu_clear_flag(cpu_state, CARRY_FLAG);
		cpu_set_sign_flag(cpu_state, result, EIGHT_BIT);
		cpu_set_zero_flag(cpu_state, result);
		cpu_set_parity_flag(cpu_state, result);

		#ifdef DEBUG_PRINT_INST
		fprintf(stderr, "XOR rm32 r32\n");
		#endif

		return true;
	}
	break;
}

case 0x32:{
	/*r8 XOR r/m8.*/
	if(cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT)){
		uint8_t op1 = cpu_read_byte_from_reg(s_op.reg, IS_HIGH(s_op.reg));
		uint8_t op2;
		if(s_op.regmem_type == MEMORY)
			op2 = cpu_read_byte_from_mem(cpu_state, s_op.regmem_mem);
		else
			op2 = cpu_read_byte_from_reg(s_op.regmem_reg, IS_HIGH(s_op.regmem));

		uint8_t result = op1 ^ op2;

		cpu_write_byte_in_reg(s_op.reg, result, IS_HIGH(s_op.reg));

		cpu_clear_flag(cpu_state, OVERFLOW_FLAG);
		cpu_clear_flag(cpu_state, CARRY_FLAG);
		cpu_set_sign_flag(cpu_state, result, EIGHT_BIT);
		cpu_set_zero_flag(cpu_state, result);
		cpu_set_parity_flag(cpu_state, result);

		#ifdef DEBUG_PRINT_INST
		fprintf(stderr, "XOR r8 rm8\n");
		#endif

		return true;
	}
	break;
}

case 0x33:{
	/*r32 XOR r/m32.*/
	if(cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT)){
		uint32_t op1 = cpu_read_word_from_reg(s_op.reg);
		uint32_t op2;
		if(s_op.regmem_type == MEMORY)
			op2 = cpu_read_word_from_mem(cpu_state, s_op.regmem_mem);
		else
			op2 = cpu_read_word_from_reg(s_op.regmem_reg);

		uint32_t result = op1 ^ op2;

		cpu_write_word_in_reg(s_op.reg, result);

		cpu_clear_flag(cpu_state, OVERFLOW_FLAG);
		cpu_clear_flag(cpu_state, CARRY_FLAG);
		cpu_set_sign_flag(cpu_state, result, EIGHT_BIT);
		cpu_set_zero_flag(cpu_state, result);
		cpu_set_parity_flag(cpu_state, result);

		#ifdef DEBUG_PRINT_INST
		fprintf(stderr, "XOR r32 rm32\n");
		#endif

		return true;
	}
	break;
}

case 0x34:{
	/*XOR AL, imm8*/
	uint8_t op1 = cpu_read_byte_from_reg(&(cpu_state->eax), !HIGH_BYTE);
	uint8_t op2 = cpu_consume_byte_from_mem(cpu_state);
	uint8_t result = op1 ^ op2;
	cpu_write_byte_in_reg(&(cpu_state->eax), result, !HIGH_BYTE);

	cpu_clear_flag(cpu_state, OVERFLOW_FLAG);
	cpu_clear_flag(cpu_state, CARRY_FLAG);
	cpu_set_sign_flag(cpu_state, result, EIGHT_BIT);
	cpu_set_zero_flag(cpu_state, result);
	cpu_set_parity_flag(cpu_state, result);

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "XOR AL imm8\n");
	#endif

	return true;
}

case 0x35:{
	/*XOR EAX, imm32*/
	uint8_t op1 = cpu_read_word_from_reg(&(cpu_state->eax));
	uint8_t op2 = cpu_consume_word_from_mem(cpu_state);
	uint8_t result = op1 ^ op2;
	cpu_write_word_in_reg(&(cpu_state->eax), result);

	cpu_clear_flag(cpu_state, OVERFLOW_FLAG);
	cpu_clear_flag(cpu_state, CARRY_FLAG);
	cpu_set_sign_flag(cpu_state, result, EIGHT_BIT);
	cpu_set_zero_flag(cpu_state, result);
	cpu_set_parity_flag(cpu_state, result);

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "XOR EAX imm32\n");
	#endif

	return true;
}


//case 0x80 /6 is a special instruction, see cpu_special0x80.c
//case 0x81 /6 is a special instruction, see cpu_special0x81.c
//case 0x83 /6 is a special instruction, see cpu_special0x83.c
