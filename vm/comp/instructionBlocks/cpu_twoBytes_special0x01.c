case 3: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("LIDT m16&m32\n");
	#endif
	/*  LIDT m16&32: Load m into IDTR. */

	if(unlikely(s_op.regmem_type != MEMORY)) {
		return false;
	}

	/* Next 2 bytes at src define IDTR limit */
	cpu_state->idtr_limit = cpu_read_word_from_mem(cpu_state, s_op.regmem_mem, DATA);
	/* Next 4 bytes at src+2 define IDTR base */
	cpu_state->idtr_base = cpu_read_doubleword_from_mem(cpu_state, s_op.regmem_mem+2, DATA);

	return true;
}

case 2: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("LGDT m16&m32\n");
	#endif
	/*  LGDT m16&32: Load m into GDTR. */

	if(unlikely(s_op.regmem_type != MEMORY)) {
		return false;
	}

	/* Next 2 bytes at src define GDTR limit */
	cpu_state->gdtr_limit = cpu_read_word_from_mem(cpu_state, s_op.regmem_mem, DATA);
	/* Next 4 bytes at src+2 define GDTR base */
	cpu_state->gdtr_base = cpu_read_doubleword_from_mem(cpu_state, s_op.regmem_mem+2, DATA);

	return true;
}