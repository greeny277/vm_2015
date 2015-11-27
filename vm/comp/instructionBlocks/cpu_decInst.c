case 0x48 ... 0x4f: {
	#ifdef DEBUG_PRINT_INST
	switch(op_code & 0x7){
		case 0:
			fprintf(stderr, "DEC EAX\n");
			break;
		case 1:
			fprintf(stderr, "DEC ECX\n");
			break;
		case 2:
			fprintf(stderr, "DEC EDX\n");
			break;
		case 3:
			fprintf(stderr, "DEC EBX\n");
			break;
		case 4:
			fprintf(stderr, "DEC ESP\n");
			break;
		case 5:
			fprintf(stderr, "DEC EBP\n");
			break;
		case 6:
			fprintf(stderr, "DEC ESI\n");
			break;
		case 7:
			fprintf(stderr, "DEC EDI\n");
			break;
	}

	#endif
	/* Decrement word register eax-edi by 1 */
	uint32_t src = cpu_read_word_from_reg(&((&cpu_state->eax)[op_code & 0x7]));
	src--;
	cpu_write_word_in_reg(&((&cpu_state->eax)[op_code & 0x7]), src);

	cpu_set_overflow_add(cpu_state, src-1, 1, src, !EIGHT_BIT);
	cpu_set_aux_flag_add(cpu_state, src+1, 1, src, !EIGHT_BIT);
	cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
	cpu_set_zero_flag(cpu_state, src);

	return true;
}
