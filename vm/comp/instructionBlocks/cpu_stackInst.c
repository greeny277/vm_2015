case 0x50 ... 0x57: {
	#ifdef DEBUG_PRINT_INST
	switch(op_code & 0x7){
		case 0:
			fprintf(stderr, "PUSH EAX\n");
			break;
		case 1:
			fprintf(stderr, "PUSH ECX\n");
			break;
		case 2:
			fprintf(stderr, "PUSH EDX\n");
			break;
		case 3:
			fprintf(stderr, "PUSH EBX\n");
			break;
		case 4:
			fprintf(stderr, "PUSH ESP\n");
			break;
		case 5:
			fprintf(stderr, "PUSH EBP\n");
			break;
		case 6:
			fprintf(stderr, "PUSH ESI\n");
			break;
		case 7:
			fprintf(stderr, "PUSH EDI\n");
			break;
	}
	#endif

	/* Push eax-edi. */
	cpu_stack_push_doubleword(cpu_state, (&cpu_state->eax)[op_code & 0x7]);

	return true;
}

case 0x58 ... 0x5f: {
	#ifdef DEBUG_PRINT_INST
	switch(op_code & 0x7){
		case 0:
			fprintf(stderr, "POP EAX\n");
			break;
		case 1:
			fprintf(stderr, "POP ECX\n");
			break;
		case 2:
			fprintf(stderr, "POP EDX\n");
			break;
		case 3:
			fprintf(stderr, "POP EBX\n");
			break;
		case 4:
			fprintf(stderr, "POP ESP\n");
			break;
		case 5:
			fprintf(stderr, "POP EBP\n");
			break;
		case 6:
			fprintf(stderr, "POP ESI\n");
			break;
		case 7:
			fprintf(stderr, "POP EDI\n");
			break;
	}
	#endif

	/* Pop eax */
	(&cpu_state->eax)[op_code & 0x7] = cpu_stack_pop_doubleword(cpu_state);

	return true;
}

case 0xE8: {
	/* CALL rel32 */
	int32_t rel32 = cpu_consume_word_from_mem(cpu_state);
	cpu_stack_push_doubleword(cpu_state, cpu_state->eip);

	cpu_write_word_in_reg(&(cpu_state->eip), cpu_state->eip+rel32);

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "CALL rel32\n");
	#endif

	return true;
}

case 0x9A: {
	/* CALL ptr32
	 * Call far, absolute, address given in operand.
	 */
	uint32_t abs = cpu_consume_word_from_mem(cpu_state);
	cpu_stack_push_doubleword(cpu_state, cpu_state->eip);
	cpu_write_word_in_reg(&(cpu_state->eip), abs);

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "CALL ptr32\n");
	#endif

	return true;
}

case 0xC3: {
	/* Near return to calling procedure. */
	cpu_write_word_in_reg(&(cpu_state->eip), cpu_stack_pop_doubleword(cpu_state));

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "RET\n");
	#endif

	return true;
}

case 0xCB:{
	/* Far return to calling procedure.
	 * A return to a calling procedure located
	 * in a different segment
	 */

	/* No segments implemented yet */
	fprintf(stderr, "No segments implemented yet\n");

	break;
}

case 0xC2: {
	/* RET imm16 */

	/* Near return to calling procedure and pop
	 * imm16 bytes from stack.
	 */
	cpu_write_word_in_reg(&(cpu_state->eip), cpu_stack_pop_doubleword(cpu_state));

	uint16_t imm16;

	uint8_t byte0, byte1;

	byte0 = cpu_consume_byte_from_mem(cpu_state);
	byte1 = cpu_consume_byte_from_mem(cpu_state);

	imm16 = byte0;
	imm16 |= (byte1 << 8);

	cpu_write_word_in_reg(&(cpu_state->esp), cpu_state->esp+imm16);

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "RET imm16\n");
	#endif

	return true;
}

case 0xCA: {
	/*  Far return to calling procedure and pop imm16
	 *  bytes from stack.
	 */

	/* No segments implemented yet */
	fprintf(stderr, "No segments implemented yet\n");

	break;
}
