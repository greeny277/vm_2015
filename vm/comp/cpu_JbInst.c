case 0x72: {
	/* JB rel8
	 * Jump short if below (CF=1).
	 */
	int8_t offset = cpu_read_byte_from_ram(cpu_state);

	if(cpu_get_carry_flag(cpu_state)){
		cpu_set_eip(cpu_state, eip_old + offset);
	}

	return true;
}

case 0x82: {
	/* JB rel32
	 * Jump short if below (CF=1).
	 */
	int32_t offset = cpu_read_word_from_ram(cpu_state);

	if(cpu_get_carry_flag(cpu_state)){
		cpu_set_eip(cpu_state, eip_old + offset);
	}

	return true;
}
