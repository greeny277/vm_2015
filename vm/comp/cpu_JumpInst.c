case 0x72: {
	/* JB rel8
	 * Jump short if below (CF=1).
	 */
	int8_t offset = cpu_read_byte_from_ram(cpu_state);

	if(cpu_get_carry_flag(cpu_state)){
		cpu_set_eip(cpu_state, cpu_state->eip + offset);
	}

	return true;
}


case 0x75: {
	/* JNE rel8
	 * Jump short if not equal (ZF=0).
	 */
	int8_t offset = cpu_read_byte_from_ram(cpu_state);

	if(!cpu_get_zero_flag(cpu_state)){
		cpu_set_eip(cpu_state, cpu_state->eip + offset);
	}
	return true;
}
