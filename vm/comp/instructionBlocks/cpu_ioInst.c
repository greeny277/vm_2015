case 0xe4:{
	/*IN AL, imm8*/
	uint8_t addr = cpu_consume_byte_from_mem(cpu_state);
	uint8_t val = sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr);
	cpu_write_byte_in_reg(&(cpu_state->eax), val, !HIGH_BYTE);

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "IN AL imm8\n");
	#endif
	return true;
}

case 0xe5:{
	/*IN AL, imm32*/
	uint8_t addr = cpu_consume_byte_from_mem(cpu_state);
	uint32_t val =  (sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr+3) << 24) |
					(sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr+2) << 16) |
					(sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr+1) << 8) |
					sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr);

	cpu_write_word_in_reg(&(cpu_state->eax), val);

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "IN AL imm32\n");
	#endif
	return true;
}

case 0xec: {
	/*IN AL,DX*/
	uint16_t addr = cpu_read_word_from_reg(&(cpu_state->edx));
	uint8_t val = sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr);
	cpu_write_byte_in_reg(&(cpu_state->eax), val, !HIGH_BYTE);

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "IN AL DX\n");
	#endif
	return true;
}

case 0xed:{
	/*IN EAX, DX*/
	uint16_t addr = cpu_read_word_from_reg(&(cpu_state->edx));
	uint32_t val =  (sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr+3) << 24) |
					(sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr+2) << 16) |
					(sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr+1) << 8) |
					sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr);

	cpu_write_word_in_reg(&(cpu_state->eax), val);

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "IN EAX DX\n");
	#endif
	return true;
}

case 0xe6:{
	/*OUT imm8,AL*/
	uint8_t addr = cpu_consume_byte_from_mem(cpu_state);
	uint8_t val = cpu_read_byte_from_reg(&(cpu_state->eax), !HIGH_BYTE);
	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr, val);

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "OUT imm8 AL\n");
	#endif
	return true;
}

case 0xe7:{
	/*OUT imm32, AL*/
	uint8_t addr = cpu_consume_byte_from_mem(cpu_state);
	uint32_t val = cpu_read_word_from_reg(&(cpu_state->eax));

	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr+3, (val >> 24) & 0xFF);
	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr+2, (val >> 16) & 0xFF);
	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr+1, (val >>  8)  & 0xFF);
	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr,     val        & 0xFF);

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "IN imm32 AL\n");
	#endif
	return true;
}

case 0xee:{
	/*OUT DX,AL*/
	uint16_t addr = cpu_read_word_from_reg(&(cpu_state->edx));
	uint8_t val = cpu_read_byte_from_reg(&(cpu_state->eax), !HIGH_BYTE);
	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr, val);

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "OUT DX AL\n");
	#endif
	return true;
}

case 0xef:{
	/*OUT DX,EAX*/
	uint16_t addr = cpu_read_word_from_reg(&(cpu_state->edx));
	uint32_t val = cpu_read_word_from_reg(&(cpu_state->eax));

	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr+3, (val >> 24) & 0xFF);
	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr+2, (val >> 16) & 0xFF);
	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr+1, (val >>  8)  & 0xFF);
	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr,     val        & 0xFF);

	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "OUT DX EAX\n");
	#endif
	return true;
}