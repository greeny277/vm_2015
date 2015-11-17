
case 0xEB: {
	/* Jump short relative 8*/
	cpu_set_eip(cpu_state, eip_old+cpu_read_byte_from_ram(cpu_state));
	return true;
}
case 0xE9: {
	/* Jump near, relative 32*/
	cpu_set_eip(cpu_state, eip_old+cpu_read_word_from_ram(cpu_state));
	return true;
}
case 0xFF:
	/* jump near, absolute indirect, address given in r/m32. */
	if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT)){
		cpu_set_eip(cpu_state, eip_old + (*s_op.regmem_reg));
		return true;
	}
	break;