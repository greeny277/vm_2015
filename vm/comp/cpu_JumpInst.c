case 0x72: {
	/* JB rel8
	 * Jump short if below (CF=1).
	 */
	int8_t offset = cpu_read_byte_from_mem(cpu_state);

	if(cpu_get_carry_flag(cpu_state)){
		cpu_set_eip(cpu_state, cpu_state->eip + offset);
	}

	return true;
}

case 0x74: {

}


case 0x75: {
	/* JNE rel8
	 * Jump short if not equal (ZF=0).
	 */
	int8_t offset = cpu_read_byte_from_mem(cpu_state);

	if(!cpu_get_zero_flag(cpu_state)){
		cpu_set_eip(cpu_state, cpu_state->eip + offset);
	}
	return true;
}


/*
 * Unconditional jumps
 */
case 0xEB: {
	/* Jump short relative 8*/
	cpu_set_eip(cpu_state, cpu_state->eip + (int8_t) cpu_read_byte_from_mem(cpu_state));
	return true;
}
case 0xE9: {
	/* Jump near, relative 32*/
	cpu_set_eip(cpu_state, cpu_state->eip + (int32_t) cpu_read_word_from_mem(cpu_state));
	return true;
}
// case 0xFF: is a special case -> see cpu_special0xFF.c