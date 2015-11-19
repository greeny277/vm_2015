case 0: {
	/* Copy imm32 to r/m32. */
	if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT)){
		uint8_t src = cpu_read_word_from_mem(cpu_state);
		if(s_op.regmem_type == MEMORY){
			cpu_write_word_in_mem(cpu_state, src, s_op.regmem_mem);
		} else {
			cpu_write_word_in_reg(s_op.regmem_reg, src);
		}
		return true;
	}
	break;
}
default:
	break;
