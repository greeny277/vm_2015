case 0x88: {

		#ifdef DEBUG_PRINT_INST
		cpu_print_inst("MOV r8 rm8 \n");
		#endif
	/* Copy r8 to r/m8 */
	if(likely(cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT))){
		uint8_t src = cpu_read_byte_from_reg(s_op.reg, IS_HIGH(s_op.reg));
		if(s_op.regmem_type == MEMORY){
			cpu_write_byte_in_mem(cpu_state, src, s_op.regmem_mem, DATA);
		} else {
			cpu_write_byte_in_reg(s_op.regmem_reg, src, IS_HIGH(s_op.regmem));
		}


		return true;
	}
	break;
}
case 0x89: {

		#ifdef DEBUG_PRINT_INST
		cpu_print_inst("MOV r32 rm32 \n");
		#endif
	/* Copy r32 to r/m32 */
	if(likely(cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT))){
		uint32_t src = cpu_read_doubleword_from_reg(s_op.reg);
		if(s_op.regmem_type == MEMORY){
			cpu_write_doubleword_in_mem(cpu_state, src, s_op.regmem_mem, DATA);
		} else {
			cpu_write_doubleword_in_reg(s_op.regmem_reg, src);
		}


		return true;
	}
	break;
}

case 0x8A: {

		#ifdef DEBUG_PRINT_INST
		cpu_print_inst("MOV rm8 r8 \n");
		#endif
	/* Copy r/m8 to r8. */
	if(likely(cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT))){
		uint8_t src;
		if(s_op.regmem_type == MEMORY){
			src = cpu_read_byte_from_mem(cpu_state, s_op.regmem_mem, DATA);
		} else {
			src = cpu_read_byte_from_reg(s_op.regmem_reg, IS_HIGH(s_op.regmem));
		}
		cpu_write_byte_in_reg(s_op.reg, src, IS_HIGH(s_op.regmem));


		return true;
	}
	break;
}

case 0x8B: {

		#ifdef DEBUG_PRINT_INST
		cpu_print_inst("MOV rm32 r32 \n");
		#endif
	/* Copy r/m32 to r32. */
	if(likely(cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT))){
		uint32_t src;
		if(s_op.regmem_type == MEMORY){
			src = cpu_read_doubleword_from_mem(cpu_state, s_op.regmem_mem, DATA);
		} else {
			src = cpu_read_doubleword_from_reg(s_op.regmem_reg);
		}
		cpu_write_doubleword_in_reg(s_op.reg, src);


		return true;
	}
	break;
}

case 0x8C: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV rm32 sreg \n");
	#endif

	if(likely(cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT))){
		segment_register *cur_reg = (&(cpu_state->es))+ s_op.reg_value;

		if(s_op.regmem_type == MEMORY){
			cpu_write_doubleword_in_mem(cpu_state, cur_reg->public_part, s_op.regmem_mem, DATA);
		} else {
			cpu_write_doubleword_in_reg(s_op.regmem_reg, cur_reg->public_part);
		}
	}
}

case 0x8E: {
	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV sreg rm32 \n");
	#endif

	if(likely(cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT))){
		uint16_t value = 0;
		if(s_op.regmem_type == MEMORY){
			value = cpu_read_doubleword_from_mem(cpu_state, s_op.regmem_mem, DATA);
		} else {
			value = cpu_read_doubleword_from_reg(s_op.regmem_reg);
		}

		if(unlikely(s_op.reg_value == CODE)){
			return false;
			//TODO: eigentlich sollte man hier eine exception werfen (invalid opcode exception #UD)
		}

		cpu_load_segment_register(cpu_state, (cpu_segment) s_op.reg_value, value);
	}
}

case 0xA0: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV AL moffs8 \n");
	#endif
	/* Copy byte at (seg:offset) to AL */
	uint32_t src_addr = cpu_consume_doubleword_from_mem(cpu_state);
	uint8_t src_byte = cpu_read_byte_from_mem(cpu_state, src_addr, DATA);
	cpu_write_byte_in_reg(&(cpu_state->eax), src_byte, !HIGH_BYTE);


	return true;
}

case 0xA1: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV EAX moffs32 \n");
	#endif
	/* Copy word at (seg:offset) to EAX */
	uint32_t src_addr = cpu_consume_doubleword_from_mem(cpu_state);
	uint32_t src_doubleword = cpu_read_doubleword_from_mem(cpu_state, src_addr, DATA);
	cpu_write_doubleword_in_reg(&(cpu_state->eax), src_doubleword);


	return true;
}

case 0xA2: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV moffs8 AL\n");
	#endif
	/* Copy AL to (seg:offset) */
	uint32_t dest_addr = cpu_consume_doubleword_from_mem(cpu_state);

	uint8_t src_byte = cpu_read_byte_from_reg(&(cpu_state->eax), !HIGH_BYTE);

	cpu_write_byte_in_mem(cpu_state, src_byte, dest_addr, DATA);


	return true;
}

case 0xA3: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV moffs32 EAX\n");
	#endif
	/* Copy EAX to (seg:offset) */

	uint32_t dest_addr = cpu_consume_doubleword_from_mem(cpu_state);
	uint32_t src_doubleword = cpu_read_doubleword_from_reg(&(cpu_state->eax));

	cpu_write_doubleword_in_mem(cpu_state, src_doubleword, dest_addr, DATA);


	return true;
}

case 0xB0: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV AL imm8\n");
	#endif
	/* Copy imm8 to AL */
	uint8_t src;
	src = cpu_consume_byte_from_mem(cpu_state);
	cpu_write_byte_in_reg(&(cpu_state->eax), src, !HIGH_BYTE);


	return true;
}

case 0xB1: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV CL imm8\n");
	#endif
	/* Copy imm8 to CL */
	uint8_t src;
	src = cpu_consume_byte_from_mem(cpu_state);
	cpu_write_byte_in_reg(&(cpu_state->ecx), src, !HIGH_BYTE);


	return true;
}

case 0xB2: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV DL imm8\n");
	#endif
	/* Copy imm8 to DL */
	uint8_t src;
	src = cpu_consume_byte_from_mem(cpu_state);
	cpu_write_byte_in_reg(&(cpu_state->edx), src, !HIGH_BYTE);


	return true;
}

case 0xB3: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV BL imm8\n");
	#endif
	/* Copy imm8 to BL */
	uint8_t src;
	src = cpu_consume_byte_from_mem(cpu_state);
	cpu_write_byte_in_reg(&(cpu_state->ebx), src, !HIGH_BYTE);


	return true;
}

case 0xB4: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV AH imm8\n");
	#endif
	/* Copy imm8 to AH */
	uint8_t src;
	src = cpu_consume_byte_from_mem(cpu_state);
	cpu_write_byte_in_reg(&(cpu_state->eax), src, HIGH_BYTE);


	return true;
}

case 0xB5: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV CH imm8\n");
	#endif
	/* Copy imm8 to CH */
	uint8_t src;
	src = cpu_consume_byte_from_mem(cpu_state);
	cpu_write_byte_in_reg(&(cpu_state->ecx), src, HIGH_BYTE);


	return true;
}

case 0xB6: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV DH imm8\n");
	#endif
	/* Copy imm8 to DH */
	uint8_t src;
	src = cpu_consume_byte_from_mem(cpu_state);
	cpu_write_byte_in_reg(&(cpu_state->edx), src, HIGH_BYTE);


	return true;
}

case 0xB7: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV BH imm8\n");
	#endif
	/* Copy imm8 to BH */
	uint8_t src;
	src = cpu_consume_byte_from_mem(cpu_state);
	cpu_write_byte_in_reg(&(cpu_state->ebx), src, HIGH_BYTE);


	return true;
}

case 0xB8: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV EAX imm32\n");
	#endif
	/* Copy imm32 to EAX */
	uint32_t src;
	src = cpu_consume_doubleword_from_mem(cpu_state);
	cpu_write_doubleword_in_reg(&(cpu_state->eax), src);


	return true;
}

case 0xB9: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV ECX imm32\n");
	#endif
	/* Copy imm32 to ECX */
	uint32_t src;
	src = cpu_consume_doubleword_from_mem(cpu_state);
	cpu_write_doubleword_in_reg(&(cpu_state->ecx), src);


	return true;
}

case 0xBA: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV EDX imm32\n");
	#endif
	/* Copy imm32 to EDX */
	uint32_t src;
	src = cpu_consume_doubleword_from_mem(cpu_state);
	cpu_write_doubleword_in_reg(&(cpu_state->edx), src);


	return true;
}

case 0xBB: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV EBX imm32\n");
	#endif
	/* Copy imm32 to EBX */
	uint32_t src;
	src = cpu_consume_doubleword_from_mem(cpu_state);
	cpu_write_doubleword_in_reg(&(cpu_state->ebx), src);


	return true;
}

case 0xBC: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV ESP imm32\n");
	#endif
	/* Copy imm32 to ESP */
	uint32_t src;
	src = cpu_consume_doubleword_from_mem(cpu_state);
	cpu_write_doubleword_in_reg(&(cpu_state->esp), src);


	return true;
}

case 0xBD: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV EBP imm32\n");
	#endif
	/* Copy imm32 to EBP */
	uint32_t src;
	src = cpu_consume_doubleword_from_mem(cpu_state);
	cpu_write_doubleword_in_reg(&(cpu_state->ebp), src);


	return true;
}

case 0xBE: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV ESI imm32\n");
	#endif
	/* Copy imm32 to ESI */
	uint32_t src;
	src = cpu_consume_doubleword_from_mem(cpu_state);
	cpu_write_doubleword_in_reg(&(cpu_state->esi), src);


	return true;
}
case 0xBF: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("MOV EDI imm32\n");
	#endif
	/* Copy imm32 to EDI */
	uint32_t src;
	src = cpu_consume_doubleword_from_mem(cpu_state);
	cpu_write_doubleword_in_reg(&(cpu_state->edi), src);


	return true;
}
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
