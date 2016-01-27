case 0: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("INC rm32\n");
	#endif
	/* Increment r/m word by 1 */
	uint32_t src;
	if(s_op.regmem_type == MEMORY){
		src = cpu_read_doubleword_from_mem(cpu_state, s_op.regmem_mem, DATA);
		src++;
		cpu_write_doubleword_in_mem(cpu_state, src, s_op.regmem_mem, DATA);
	} else {
		src = cpu_read_doubleword_from_reg(s_op.reg);
		src++;
		cpu_write_doubleword_in_reg(s_op.regmem_reg, src);
	}


	return true;
}
case 1: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("DEC rm32\n");
	#endif
	/* Decrement r/m word by 1 */
	uint32_t src;
	if(s_op.regmem_type == MEMORY){
		src = cpu_read_doubleword_from_mem(cpu_state, s_op.regmem_mem, DATA);
		src--;
		cpu_write_doubleword_in_mem(cpu_state, src, s_op.regmem_mem, DATA);
	} else {
		src = cpu_read_doubleword_from_reg(s_op.reg);
		src--;
		cpu_write_doubleword_in_reg(s_op.regmem_reg, src);
	}


	return true;
}
case 2: {
	/*  CALL r/m32 */
	break;
}

case 3: {
	break;
}

case 4: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("JMP rm32\n");
	#endif
	/* jump near, absolute indirect, address given in r/m32 (r). */
	cpu_set_eip(cpu_state, cpu_read_doubleword_from_reg(s_op.regmem_reg));


	return true;
}

case 6: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("PUSH RM32 \n");
	#endif
	uint32_t src;
	if(s_op.regmem_type == MEMORY){
		src = cpu_read_doubleword_from_mem(cpu_state, s_op.regmem_mem, DATA);
	} else {
		src = cpu_read_doubleword_from_reg(s_op.reg);
	}
	cpu_stack_push_doubleword(cpu_state, src);
}

default:
	break;
