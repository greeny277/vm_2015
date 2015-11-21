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
case 5: {
	/* Increment r/m word by 1 */
	if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT)){
		uint32_t src;
		if(s_op.regmem_type == MEMORY){
			src = cpu_read_word_from_mem(cpu_state, s_op.regmem_mem);
			src++;
			cpu_write_word_in_mem(cpu_state, src, s_op.regmem_mem);
		} else {
			src = cpu_read_word_from_reg(s_op.reg);
			src++;
			cpu_write_word_in_reg(s_op.regmem_reg, src);
		}
		return true;
	}
	break;
}

default:
	break;
