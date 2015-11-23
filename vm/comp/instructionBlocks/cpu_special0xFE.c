case 0: {
	/* Increment r/m byte by 1 */
	uint8_t src;
	if(s_op.regmem_type == MEMORY){
		src = cpu_read_byte_from_mem(cpu_state, s_op.regmem_mem);
		src++;
		cpu_write_byte_in_mem(cpu_state, src, s_op.regmem_mem);
	} else {
		src = cpu_read_byte_from_reg(s_op.reg, s_op.regmem_type == REGISTER_HIGH);
		src++;
		cpu_write_byte_in_reg(s_op.regmem_reg, src ,IS_HIGH(s_op.regmem));
	}
	return true;
}

case 1: {
	/* Decrement r/m byte by 1 */
	uint8_t src;
	if(s_op.regmem_type == MEMORY){
		src = cpu_read_byte_from_mem(cpu_state, s_op.regmem_mem);
		src--;
		cpu_write_byte_in_mem(cpu_state, src, s_op.regmem_mem);
	} else {
		src = cpu_read_byte_from_reg(s_op.reg, s_op.regmem_type == REGISTER_HIGH);
		src--;
		cpu_write_byte_in_reg(s_op.regmem_reg, src ,IS_HIGH(s_op.regmem));
	}
	return true;
}


dafault:
	break;
