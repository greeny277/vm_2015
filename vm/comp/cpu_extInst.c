case 0x82: {
	/* JB rel32
	 * Jump short if below (CF=1).
	 */
	int32_t offset = cpu_read_word_from_ram(cpu_state);

	if(cpu_get_carry_flag(cpu_state)){
		cpu_set_eip(cpu_state, cpu_state->eip + offset);
	}

	return true;
}

case 0x85: {
	/* JNE rel32
	 * Jump short if not equal (ZF=0).
	 */
	int32_t offset = cpu_read_word_from_ram(cpu_state);

	if(!cpu_get_zero_flag(cpu_state)){
		cpu_set_eip(cpu_state, cpu_state->eip + offset);
	}
	return true;
}
