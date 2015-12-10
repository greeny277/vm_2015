case 4: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MULB rm8\n");
	#endif
	/* MUL r/m8
	unsigned multiply (AX ← AL ∗ r/m8)
	*/
	uint8_t op1 = cpu_read_byte_from_reg(&(cpu_state->eax), !HIGH_BYTE);

	uint8_t op2;
	if(s_op.regmem_type == MEMORY)
		op2 = cpu_read_byte_from_mem(cpu_state, s_op.regmem_mem);
	else
		op2 = cpu_read_byte_from_reg(s_op.regmem_reg, IS_HIGH(s_op.regmem));

	uint32_t result = (cpu_read_doubleword_from_reg(&(cpu_state->eax)) & 0xFFFF0000) | ((op1*op2) & 0xFFFF);

	cpu_write_doubleword_in_reg(&(cpu_state->eax), result);

	if(cpu_read_byte_from_reg(&(cpu_state->eax), HIGH_BYTE) == 0){
		cpu_clear_flag(cpu_state, OVERFLOW_FLAG);
		cpu_clear_flag(cpu_state, CARRY_FLAG);
	}else{
		cpu_raise_flag(cpu_state, OVERFLOW_FLAG);
		cpu_raise_flag(cpu_state, CARRY_FLAG);
	}


	return true;

}
