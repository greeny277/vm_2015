case 0x82: {
	/* JB rel32
	 * Jump short if below (CF=1).
	 */

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "JB rel32\n");
	#endif

	cond = cpu_get_carry_flag(cpu_state);
	goto jmp32;
}

case 0x84: {
	/* JE rel32
	 * Jump short if equal (ZF=1).
	 */

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "JE rel32\n");
	#endif

	cond = cpu_get_zero_flag(cpu_state);
	goto jmp32;
}

case 0x85: {
	/* JNE rel32
	 * Jump short if not equal (ZF=0).
	 */

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "JNE rel32\n");
	#endif

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
	int32_t offset = cpu_consume_word_from_mem(cpu_state);

	if(cond){
		cpu_set_eip(cpu_state, cpu_state->eip + offset);
	}

	return true;
}
