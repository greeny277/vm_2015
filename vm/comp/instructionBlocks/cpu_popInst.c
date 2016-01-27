
case 0x17: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("POP SS \n");
	#endif

	uint32_t val = cpu_stack_pop_doubleword(cpu_state);
	cpu_load_segment_register(cpu_state, STACK, val);
	return true;
}

case 0x1f: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("POP DS \n");
	#endif

	uint32_t val = cpu_stack_pop_doubleword(cpu_state);
	cpu_load_segment_register(cpu_state, DATA, val);

	return true;
}

case 0x07: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("POP ES \n");
	#endif

	uint32_t val = cpu_stack_pop_doubleword(cpu_state);
	cpu_load_segment_register(cpu_state, EXTRA, val);
	return true;
}


//case 0x8f /0 is a special case, not yet implemented
//case 0x0f 0xA1 is a special case
//case 0x0f 0xA9 is a special case
