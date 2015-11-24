case 0x82: {
	/* JB rel32
	 * Jump short if below (CF=1).
	 */
	int32_t offset = cpu_consume_word_from_mem(cpu_state);

	if(cpu_get_carry_flag(cpu_state)){
		cpu_set_eip(cpu_state, cpu_state->eip + offset);
	}

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "JB rel32\n");
	#endif

	return true;
}

case 0x84: {
	/* JE rel32
	 * Jump short if equal (ZF=1).
	 */
	int32_t offset = cpu_consume_word_from_mem(cpu_state);

	if(cpu_get_zero_flag(cpu_state)){
		cpu_set_eip(cpu_state, cpu_state->eip + offset);
	}

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "JE rel32\n");
	#endif

	return true;
}

case 0x85: {
	/* JNE rel32
	 * Jump short if not equal (ZF=0).
	 */
	int32_t offset = cpu_consume_word_from_mem(cpu_state);

	if(!cpu_get_zero_flag(cpu_state)){
		cpu_set_eip(cpu_state, cpu_state->eip + offset);
	}

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "JNE rel32\n");
	#endif

	return true;
}
