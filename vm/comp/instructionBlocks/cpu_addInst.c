case 0x00:{

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("ADD rm8 r8 \n");
	#endif

	/*ADD r/m8 rm8*/
	if(likely(cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT))){
		uint8_t op1;
		if(s_op.regmem_type == MEMORY)
			op1 = cpu_read_byte_from_mem(cpu_state, s_op.regmem_mem);
		else
			op1 = cpu_read_byte_from_reg(s_op.regmem_reg, IS_HIGH(s_op.regmem));

		uint8_t op2 = cpu_read_byte_from_reg(s_op.reg, IS_HIGH(s_op.reg));
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
	break;
}

case 0x01:{

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("ADD m32 r32 \n");
	#endif

	/*r/ADD m32 r32.*/
	if(likely(cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT))){
		uint32_t op1;
		if(s_op.regmem_type == MEMORY)
			op1 = cpu_read_doubleword_from_mem(cpu_state, s_op.regmem_mem);
		else
			op1 = cpu_read_doubleword_from_reg(s_op.regmem_reg);

		uint32_t op2 = cpu_read_doubleword_from_reg(s_op.reg);
		uint32_t result = op1 + op2;

		if(s_op.regmem_type == MEMORY)
			cpu_write_doubleword_in_mem(cpu_state, result, s_op.regmem_mem);
		else
			cpu_write_doubleword_in_reg(s_op.regmem_reg, result);

		cpu_set_overflow_add(cpu_state, op1, op2, result, EIGHT_BIT);
		cpu_set_sign_flag(cpu_state, result, EIGHT_BIT);
		cpu_set_zero_flag(cpu_state, result);
		cpu_set_carry_add(cpu_state, op1, result);
		cpu_set_parity_flag(cpu_state, result);


	  	return true;


	}
	break;
}

case 0x02:{

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("ADD r8 rm8 \n");
	#endif

	/*ADD r8 r/m8.*/
	if(likely(cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT))){
		uint8_t op1 = cpu_read_byte_from_reg(s_op.reg, IS_HIGH(s_op.reg));
		uint8_t op2;
		if(s_op.regmem_type == MEMORY)
			op2 = cpu_read_byte_from_mem(cpu_state, s_op.regmem_mem);
		else
			op2 = cpu_read_byte_from_reg(s_op.regmem_reg, IS_HIGH(s_op.regmem));

		uint8_t result = op1 + op2;

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

case 0x03:{

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("ADD r32 rm32 \n");
	#endif

	/*ADD r32 r/m32.*/
	if(likely(cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT))){
		uint32_t op1 = cpu_read_doubleword_from_reg(s_op.reg);
		uint32_t op2;
		if(s_op.regmem_type == MEMORY)
			op2 = cpu_read_doubleword_from_mem(cpu_state, s_op.regmem_mem);
		else
			op2 = cpu_read_doubleword_from_reg(s_op.regmem_reg);

		uint32_t result = op1 + op2;

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

case 0x04: {

#ifdef DEBUG_PRINT_INST
cpu_print_inst("ADD AL imm8 \n");
#endif

	/*ADD AL, imm8*/
	uint8_t op1 = cpu_read_byte_from_reg(&(cpu_state->eax), !HIGH_BYTE);
	uint8_t op2 = cpu_consume_byte_from_mem(cpu_state);
	uint8_t result = op1 + op2;
	cpu_write_byte_in_reg(&(cpu_state->eax), result, !HIGH_BYTE);

	cpu_set_overflow_add(cpu_state, op1, op2, result, EIGHT_BIT);
	cpu_set_sign_flag(cpu_state, result, EIGHT_BIT);
	cpu_set_zero_flag(cpu_state, result);
	cpu_set_carry_add(cpu_state, op1, result);
	cpu_set_parity_flag(cpu_state, result);


	return true;

}

case 0x05: {

#ifdef DEBUG_PRINT_INST
cpu_print_inst("ADD eax imm32 \n");
#endif

	/*ADD EAX, imm32*/
	uint8_t op1 = cpu_read_doubleword_from_reg(&(cpu_state->eax));
	uint8_t op2 = cpu_consume_doubleword_from_mem(cpu_state);
	uint8_t result = op1 + op2;
	cpu_write_doubleword_in_reg(&(cpu_state->eax), result);

	cpu_set_overflow_add(cpu_state, op1, op2, result, !EIGHT_BIT);
	cpu_set_sign_flag(cpu_state, result, !EIGHT_BIT);
	cpu_set_zero_flag(cpu_state, result);
	cpu_set_carry_add(cpu_state, op1, result);
	cpu_set_parity_flag(cpu_state, result);


	return true;

}

//case 0x80 0 is a special case, see cpu_special0x80.c
//case 0x81 0 is a special case, see cpu_special0x81.c
//case 0x83 0 is a special case, see cpu_special0x83.c
