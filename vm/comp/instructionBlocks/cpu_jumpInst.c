case 0x72: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("JB rel8 \n");
	#endif
	/* JB rel8
	 * Jump short if below (CF=1).
	 */


	cond = cpu_get_carry_flag(cpu_state);
	goto jmp8;
}

case 0x74: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("JE rel8 \n");
	#endif
	/* JE rel8
	 * Jump short if equal (ZF=1).
	 */


	cond = cpu_get_zero_flag(cpu_state);
	goto jmp8;
}


case 0x75: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("JNE rel8 \n");
	#endif
	/* JNE rel8
	 * Jump short if not equal (ZF=0).
	 */


	cond = !cpu_get_zero_flag(cpu_state);
	goto jmp8;

}

jmp8: {
	int8_t offset = cpu_consume_byte_from_mem(cpu_state);
	if(cond){
		cpu_set_eip(cpu_state, cpu_state->eip + offset);
	}
	return true;
}

//case 0x0f 0x84 JE rel 32: This is a two-byte opcode, see cpu_extInst.c
//case 0x0f 0x85 JNE rel 32: This is a two-byte opcode, see cpu_extInst.c

/*
 * Unconditional jumps
 */
case 0xEA: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("JMP ptr16:32 \n");
	#endif
	/* Jump far, absolute 32*/
	uint32_t abs = cpu_consume_word_from_mem(cpu_state);
	cpu_set_eip(cpu_state, abs);


	return true;
}
case 0xEB: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("JMP rel8 \n");
	#endif
	/* Jump short relative 8*/
	int8_t rel8 = cpu_consume_byte_from_mem(cpu_state);
	cpu_set_eip(cpu_state, cpu_state->eip + rel8);


	return true;
}
case 0xE9: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("JMP rel32 \n");
	#endif
	/* Jump near, relative 32*/
	int32_t rel32 = cpu_consume_word_from_mem(cpu_state);
	cpu_set_eip(cpu_state, cpu_state->eip + rel32);


	return true;
}
// case 0xFF: is a special case -> see cpu_special0xFF.c
