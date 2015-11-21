case 0xE8: {
	/* CALL rel32 */
	int32_t rel32 = cpu_consume_word_from_mem(cpu_state);
	cpu_stack_push_doubleword(cpu_state, cpu_state->eip);

	cpu_write_word_in_reg(&(cpu_state->eip), cpu_state->eip+rel32);

	return true;
}

case 0x9A: {
	/* CALL ptr32
	 * Call far, absolute, address given in operand.
	 */
	uint32_t abs = cpu_consume_word_from_mem(cpu_state);
	cpu_stack_push_doubleword(cpu_state, cpu_state->eip);
	cpu_write_word_in_reg(&(cpu_state->eip), abs);

	return true;
}

case 0xC3: {
	/* Near return to calling procedure. */
	cpu_write_word_in_reg(&(cpu_state->eip), cpu_stack_pop_doubleword(cpu_state));

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
