case 7: {
	/*Compare imm8 with r/m8. */
	uint8_t subtrahend = cpu_read_byte_from_reg(s_op.reg, IS_HIGH(s_op.reg));
	uint8_t minuend;
	if(s_op.regmem_type == MEMORY)
		minuend = cpu_peek_byte_from_mem(cpu_state, s_op.regmem_mem);
	else
		minuend = cpu_read_byte_from_reg(s_op.regmem_reg, IS_HIGH(s_op.regmem));
	uint8_t result = minuend-subtrahend;

	cpu_set_eflag_arith(cpu_state, minuend, subtrahend, result,
			EIGHT_BIT,SUBTRACTION);

	return true;
}
default:
	break;
