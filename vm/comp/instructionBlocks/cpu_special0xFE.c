case 0: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("INC rm8\n");
	#endif
	/* Increment r/m byte by 1 */
	uint8_t src;
	if(s_op.regmem_type == MEMORY){
		src = cpu_read_byte_from_mem(cpu_state, s_op.regmem_mem, DATA);
		src++;
		cpu_write_byte_in_mem(cpu_state, src, s_op.regmem_mem, DATA);
	} else {
		src = cpu_read_byte_from_reg(s_op.reg, s_op.regmem_type == REGISTER_HIGH);
		src++;
		cpu_write_byte_in_reg(s_op.regmem_reg, src ,IS_HIGH(s_op.regmem));
	}

	cpu_set_overflow_add(cpu_state, src-1, 1, src, !EIGHT_BIT);
	cpu_set_aux_flag_add(cpu_state, src-1, 1, src, !EIGHT_BIT);
	cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
	cpu_set_zero_flag(cpu_state, src);


	return true;
}

case 1: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("DEC rm8\n");
	#endif
	/* Decrement r/m byte by 1 */
	uint8_t src;
	if(s_op.regmem_type == MEMORY){
		src = cpu_read_byte_from_mem(cpu_state, s_op.regmem_mem, DATA);
		src--;
		cpu_write_byte_in_mem(cpu_state, src, s_op.regmem_mem, DATA);
	} else {
		src = cpu_read_byte_from_reg(s_op.reg, s_op.regmem_type == REGISTER_HIGH);
		src--;
		cpu_write_byte_in_reg(s_op.regmem_reg, src ,IS_HIGH(s_op.regmem));
	}

	cpu_set_overflow_sub(cpu_state, src+1, 1, src, !EIGHT_BIT);
	cpu_set_aux_flag_sub(cpu_state, src+1, 1, !EIGHT_BIT);
	cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
	cpu_set_zero_flag(cpu_state, src);


	return true;
}


dafault:
	break;
