case 0xE8: {
	/* CALL rel32 */
	int32_t rel32 = cpu_read_word_from_mem(cpu_state);
	cpu_stack_push_doubleword(cpu_state, cpu_state->eip);

	cpu_write_word_in_reg(cpu_state->eip+rel32, &(cpu_state->eip));

	return true;
}

case 0x9A: {
	/* CALL ptr32
	 * Call far, absolute, address given in operand.
	 */
	int32_t abs = cpu_read_word_from_mem(cpu_state);
	cpu_stack_push_doubleword(cpu_state, cpu_state->eip);
	cpu_write_word_in_reg(abs, &(cpu_state->eip));

	return true;
}
