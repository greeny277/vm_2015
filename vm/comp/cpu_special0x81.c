case 7: {
	/*Compare imm32 with r/m32. */
	if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT)){
		uint32_t subtrahend =cpu_read_word_from_reg(s_op.reg);
		uint32_t minuend;
		if(s_op.regmem_type == MEMORY)
			minuend = cpu_peek_byte_from_mem(cpu_state, s_op.regmem_mem);
		else
			minuend = cpu_read_byte_from_reg(s_op.regmem_reg, s_op.regmem_type == REGISTER_HIGH);
		uint32_t result = minuend-subtrahend;

		cpu_set_eflag_arith(cpu_state, minuend, subtrahend, result,
				!EIGHT_BIT,SUBTRACTION);

		return true;
	}
	break;
}
default:
	break;
