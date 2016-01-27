case 0x6a: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("PUSH imm8 \n");
	#endif

	cpu_stack_push_byte(cpu_state, cpu_consume_byte_from_mem(cpu_state));
	return true;
}

case 0x68: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("PUSH imm32 \n");
	#endif

	cpu_stack_push_doubleword(cpu_state, cpu_consume_doubleword_from_mem(cpu_state));
	return true;
}

case 0x0e: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("PUSH CS \n");
	#endif

	cpu_stack_push_doubleword(cpu_state, cpu_state->cs.public_part);
	return true;
}

case 0x16: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("PUSH SS \n");
	#endif

	cpu_stack_push_doubleword(cpu_state, cpu_state->ss.public_part);
	return true;
}

case 0x1e: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("PUSH DS \n");
	#endif

	cpu_stack_push_doubleword(cpu_state, cpu_state->ds.public_part);
	return true;
}

case 0x06: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("PUSH ES \n");
	#endif

	cpu_stack_push_doubleword(cpu_state, cpu_state->es.public_part);
	return true;
}


//case 0xff /6 is a special case
//case 0x0f 0xA0 is a special case
//case 0x0f 0xA8 is a special case
