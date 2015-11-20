case 2: {
	/*  CALL r/m32 */
	
}
case 3: {
	
}
case 4: {
	/* jump near, absolute indirect, address given in r/m32 (r). */
	if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT)){
		cpu_set_eip(cpu_state, cpu_read_word_from_reg(s_op.regmem_reg));
		return true;
	}
	break;
}
default:
	break;
