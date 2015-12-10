case 0: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV rm8 imm8\n");
	#endif
	/* Copy imm8 to r/m8. */
	uint8_t src = cpu_consume_byte_from_mem(cpu_state);
	if(s_op.regmem_type == MEMORY){
		cpu_write_byte_in_mem(cpu_state, src, s_op.regmem_mem);
	} else {
		cpu_write_byte_in_reg(s_op.regmem_reg, src, IS_HIGH(s_op.regmem));
	}


	return true;
}
default:
	break;
