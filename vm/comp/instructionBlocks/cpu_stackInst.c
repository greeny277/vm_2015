case 0x50 ... 0x57: {
	#ifdef DEBUG_PRINT_INST
	switch(op_code & 0x7){
		case 0:
			cpu_print_inst("PUSH EAX\n");
			break;
		case 1:
			cpu_print_inst("PUSH ECX\n");
			break;
		case 2:
			cpu_print_inst("PUSH EDX\n");
			break;
		case 3:
			cpu_print_inst("PUSH EBX\n");
			break;
		case 4:
			cpu_print_inst("PUSH ESP\n");
			break;
		case 5:
			cpu_print_inst("PUSH EBP\n");
			break;
		case 6:
			cpu_print_inst("PUSH ESI\n");
			break;
		case 7:
			cpu_print_inst("PUSH EDI\n");
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
			cpu_print_inst("POP EAX\n");
			break;
		case 1:
			cpu_print_inst("POP ECX\n");
			break;
		case 2:
			cpu_print_inst("POP EDX\n");
			break;
		case 3:
			cpu_print_inst("POP EBX\n");
			break;
		case 4:
			cpu_print_inst("POP ESP\n");
			break;
		case 5:
			cpu_print_inst("POP EBP\n");
			break;
		case 6:
			cpu_print_inst("POP ESI\n");
			break;
		case 7:
			cpu_print_inst("POP EDI\n");
			break;
	}
	#endif

	/* Pop eax */
	(&cpu_state->eax)[op_code & 0x7] = cpu_stack_pop_doubleword(cpu_state);

	return true;
}

case 0xE8: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("CALL rel32\n");
	#endif
	/* CALL rel32 */
	int32_t rel32 = cpu_consume_doubleword_from_mem(cpu_state);
	cpu_stack_push_doubleword(cpu_state, cpu_state->eip);

	cpu_write_doubleword_in_reg(&(cpu_state->eip), cpu_state->eip+rel32);


	return true;
}

case 0x9A: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("CALL ptr32\n");
	#endif
	/* CALL ptr32
	 * Call far, absolute, address given in operand.
	 */
	uint32_t abs = cpu_consume_doubleword_from_mem(cpu_state);
	cpu_stack_push_doubleword(cpu_state, cpu_state->eip);
	cpu_write_doubleword_in_reg(&(cpu_state->eip), abs);


	return true;
}

case 0xC3: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("RET\n");
	#endif
	/* Near return to calling procedure. */
	cpu_write_doubleword_in_reg(&(cpu_state->eip), cpu_stack_pop_doubleword(cpu_state));


	return true;
}

case 0xCB:{
	/* Far return to calling procedure.
	 * A return to a calling procedure located
	 * in a different segment
	 */

	/* No segments implemented yet */
	#ifdef DEBUG_PRINT_ERRORS
	fprintf(stderr, "No segments implemented yet\n");
	#endif

	break;
}

case 0xC2: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("RET imm16\n");
	#endif
	/* RET imm16 */

	/* Near return to calling procedure and pop
	 * imm16 bytes from stack.
	 */
	cpu_write_doubleword_in_reg(&(cpu_state->eip), cpu_stack_pop_doubleword(cpu_state));

	uint16_t imm16;

	uint8_t byte0, byte1;

	byte0 = cpu_consume_byte_from_mem(cpu_state);
	byte1 = cpu_consume_byte_from_mem(cpu_state);

	imm16 = byte0;
	imm16 |= (byte1 << 8);

	cpu_write_doubleword_in_reg(&(cpu_state->esp), cpu_state->esp+imm16);


	return true;
}

case 0xCA: {
	/*  Far return to calling procedure and pop imm16
	 *  bytes from stack.
	 */

	/* No segments implemented yet */
	#ifdef DEBUG_PRINT_ERRORS
	fprintf(stderr, "No segments implemented yet\n");
	#endif

	break;
}

case 0xCF: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("IRET\n");
	#endif
	/*IRET*/
	cpu_restore_state(cpu_state);


	return true;
}
