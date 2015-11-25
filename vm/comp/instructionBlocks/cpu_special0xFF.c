case 0: {
	/* Increment r/m word by 1 */
	uint32_t src;
	if(s_op.regmem_type == MEMORY){
		src = cpu_read_word_from_mem(cpu_state, s_op.regmem_mem);
		src++;
		cpu_write_word_in_mem(cpu_state, src, s_op.regmem_mem);
	} else {
		src = cpu_read_word_from_reg(s_op.reg);
		src++;
		cpu_write_word_in_reg(s_op.regmem_reg, src);
	}

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "INC rm32\n");
	#endif

	return true;
}
case 1: {
	/* Decrement r/m word by 1 */
	uint32_t src;
	if(s_op.regmem_type == MEMORY){
		src = cpu_read_word_from_mem(cpu_state, s_op.regmem_mem);
		src--;
		cpu_write_word_in_mem(cpu_state, src, s_op.regmem_mem);
	} else {
		src = cpu_read_word_from_reg(s_op.reg);
		src--;
		cpu_write_word_in_reg(s_op.regmem_reg, src);
	}

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "DEC rm32\n");
	#endif

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
	/* jump near, absolute indirect, address given in r/m32 (r). */
	cpu_set_eip(cpu_state, cpu_read_word_from_reg(s_op.regmem_reg));

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "JMP rm32\n");
	#endif

	return true;
}

default:
	break;
