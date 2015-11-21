case 0x50: {
	/* Push eax. */
	cpu_stack_push_doubleword(cpu_state, cpu_state->eax);
	return true;
}

case 0x51: {
	/* Push ecx. */
	cpu_stack_push_doubleword(cpu_state, cpu_state->ecx);
	return true;
}

case 0x52: {
	/* Push edx. */
	cpu_stack_push_doubleword(cpu_state, cpu_state->edx);
	return true;
}

case 0x53: {
	/* Push ebx. */
	cpu_stack_push_doubleword(cpu_state, cpu_state->ebx);
	return true;
}

case 0x54: {
	/* Push esp. */
	cpu_stack_push_doubleword(cpu_state, cpu_state->esp);
	return true;
}

case 0x55: {
	/* Push ebp. */
	cpu_stack_push_doubleword(cpu_state, cpu_state->ebp);
	return true;
}

case 0x56: {
	/* Push esi. */
	cpu_stack_push_doubleword(cpu_state, cpu_state->esi);
	return true;
}

case 0x57: {
	/* Push edi. */
	cpu_stack_push_doubleword(cpu_state, cpu_state->edi);
	return true;
}

case 0x58: {
	/* Pop eax */
	cpu_state->eax = cpu_stack_pop_doubleword(cpu_state);
	return true;
}

case 0x59: {
	/* Pop ecx */
	cpu_state->ecx = cpu_stack_pop_doubleword(cpu_state);
	return true;
}

case 0x5a: {
	/* Pop edx */
	cpu_state->edx = cpu_stack_pop_doubleword(cpu_state);
	return true;
}

case 0x5b: {
	/* Pop ebx */
	cpu_state->ebx = cpu_stack_pop_doubleword(cpu_state);
	return true;
}

case 0x5c: {
	/* Pop esp */
	cpu_state->esp = cpu_stack_pop_doubleword(cpu_state);
	return true;
}

case 0x5d: {
	/* Pop ebp */
	cpu_state->ebp = cpu_stack_pop_doubleword(cpu_state);
	return true;
}

case 0x5e: {
	/* Pop esi */
	cpu_state->esi = cpu_stack_pop_doubleword(cpu_state);
	return true;
}

case 0x5f: {
	/* Pop edi */
	cpu_state->edi = cpu_stack_pop_doubleword(cpu_state);
	return true;
}

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
