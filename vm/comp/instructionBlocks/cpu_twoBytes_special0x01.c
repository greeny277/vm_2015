case 3: {
	/*  LIDT m16&32: Load m into IDTR. */
	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "LIDT m16&m32\n");
	#endif

	uint32_t src;
	if(likely(s_op.regmem_type == MEMORY)){
		src = cpu_read_word_from_mem(cpu_state, s_op.regmem_mem);
	}
	else{
		/* This should not happen */
		return false;
	}

	/* Next 2 bytes at src define IDTR limit */
	cpu_state->idtr_limit = cpu_read_word_from_mem(cpu_state, src) & 0xFFFF;
	/* Next 4 bytes at src+2 define IDTR base */
	cpu_state->idtr_base = cpu_read_word_from_mem(cpu_state, src+2);

	return true;
}
