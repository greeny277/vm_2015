case 0: {
	/* Copy imm8 to r/m8. */
	if(!cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT)){
		uint8_t src = cpu_read_byte_from_mem(cpu_state);
		if(s_op.regmem_type == MEMORY){
			cpu_write_byte_in_mem(cpu_state, src, s_op.regmem_mem);
		} else {
			cpu_write_byte_in_reg(src, s_op.regmem_reg, s_op.regmem_type == REGISTER_HIGH);
		}
		return true;
	}
	break;
}
default:
	break;
