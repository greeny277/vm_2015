case 0: {
	/* Copy imm8 to r/m8. */
	uint8_t src = cpu_consume_byte_from_mem(cpu_state);
	if(s_op.regmem_type == MEMORY){
		cpu_write_byte_in_mem(cpu_state, src, s_op.regmem_mem);
	} else {
		cpu_write_byte_in_reg(s_op.regmem_reg, src, IS_HIGH(s_op.regmem));
	}

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "MOV rm8 imm8\n");
	#endif

	return true;
}
default:
	break;
