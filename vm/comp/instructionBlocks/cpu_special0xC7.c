case 0: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV rm32 imm32\n");
	#endif
	/* Copy imm32 to r/m32. */
	uint8_t src = cpu_consume_doubleword_from_mem(cpu_state);
	if(s_op.regmem_type == MEMORY){
		cpu_write_doubleword_in_mem(cpu_state, src, s_op.regmem_mem, DATA);
	} else {
		cpu_write_doubleword_in_reg(s_op.regmem_reg, src);
	}


	return true;
}
default:
	break;
