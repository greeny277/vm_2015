case 0x82: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("JB rel32\n");
	#endif
	/* JB rel32
	 * Jump short if below (CF=1).
	 */


	cond = cpu_get_carry_flag(cpu_state);
	goto jmp32;
}

case 0x84: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("JE rel32\n");
	#endif
	/* JE rel32
	 * Jump short if equal (ZF=1).
	 */


	cond = cpu_get_zero_flag(cpu_state);
	goto jmp32;
}

case 0x85: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("JNE rel32\n");
	#endif
	/* JNE rel32
	 * Jump short if not equal (ZF=0).
	 */


	cond = !cpu_get_zero_flag(cpu_state);
	goto jmp32;
}

case 0x01: {
	if(likely(cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT))){
		switch(s_op.reg_value){
			#include "cpu_twoBytes_special0x01.c"
		}
	}
	break;
}

jmp32: {
	int32_t offset = cpu_consume_doubleword_from_mem(cpu_state);

	if(cond){
		cpu_set_eip(cpu_state, cpu_state->cs.base_addr+cpu_state->eip + offset);
	}

	return true;
}

case 0xa0: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("PUSH FS \n");
	#endif

	cpu_stack_push_doubleword(cpu_state, cpu_state->fs.public_part);
	return true;
}

case 0xa8: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("PUSH GS \n");
	#endif

	cpu_stack_push_doubleword(cpu_state, cpu_state->gs.public_part);
	return true;
}

case 0xa1: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("POP FS \n");
	#endif

	uint32_t val = cpu_stack_pop_doubleword(cpu_state);
	cpu_load_segment_register(cpu_state, GENERAL_PURPOSE1, val);
	return true;
}

case 0xa9: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("POP GS \n");
	#endif

	uint32_t val = cpu_stack_pop_doubleword(cpu_state);
	cpu_load_segment_register(cpu_state, GENERAL_PURPOSE2, val);
	return true;
}