case 0x40 ... 0x47: {
	#ifdef DEBUG_PRINT_INST
	switch(op_code & 0x7){
		case 0:
			fprintf(stderr, "INC EAX\n");
			break;
		case 1:
			fprintf(stderr, "INC ECX\n");
			break;
		case 2:
			fprintf(stderr, "INC EDX\n");
			break;
		case 3:
			fprintf(stderr, "INC EBX\n");
			break;
		case 4:
			fprintf(stderr, "INC ESP\n");
			break;
		case 5:
			fprintf(stderr, "INC EBP\n");
			break;
		case 6:
			fprintf(stderr, "INC ESI\n");
			break;
		case 7:
			fprintf(stderr, "INC EDI\n");
			break;
	}

	#endif
	/* Increment word register eax-edi by 1 */
	uint32_t src = cpu_read_word_from_reg(&((&cpu_state->eax)[op_code & 0x7]));
	src++;
	cpu_write_word_in_reg(&((&cpu_state->eax)[op_code & 0x7]), src);

	cpu_set_overflow_add(cpu_state, src-1, 1, src, !EIGHT_BIT);
	cpu_set_aux_flag_add(cpu_state, src+1, 1, src, !EIGHT_BIT);
	cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
	cpu_set_zero_flag(cpu_state, src);

	return true;
}
