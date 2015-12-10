case 0xe4:{

#ifdef DEBUG_PRINT_INST
cpu_print_inst("IN AL imm8\n");
#endif
	/*IN AL, imm8*/
	uint8_t addr = cpu_consume_byte_from_mem(cpu_state);
	uint8_t val = sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr);
	cpu_write_byte_in_reg(&(cpu_state->eax), val, !HIGH_BYTE);

	return true;
}

case 0xe5:{

#ifdef DEBUG_PRINT_INST
cpu_print_inst("IN AL imm32\n");
#endif
	/*IN AL, imm32*/
	uint8_t addr = cpu_consume_byte_from_mem(cpu_state);
	uint32_t val =  (sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr+3) << 24) |
					(sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr+2) << 16) |
					(sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr+1) << 8) |
					sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr);

	cpu_write_doubleword_in_reg(&(cpu_state->eax), val);

	return true;
}

case 0xec: {

#ifdef DEBUG_PRINT_INST
cpu_print_inst("IN AL DX\n");
#endif
	/*IN AL,DX*/
	uint16_t addr = cpu_read_doubleword_from_reg(&(cpu_state->edx));
	uint8_t val = sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr);
	cpu_write_byte_in_reg(&(cpu_state->eax), val, !HIGH_BYTE);

	return true;
}

case 0xed:{

#ifdef DEBUG_PRINT_INST
cpu_print_inst("IN EAX DX\n");
#endif
	/*IN EAX, DX*/
	uint16_t addr = cpu_read_doubleword_from_reg(&(cpu_state->edx));
	uint32_t val =  (sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr+3) << 24) |
					(sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr+2) << 16) |
					(sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr+1) << 8) |
					sig_host_bus_read_io_dev(cpu_state->port_host, cpu_state, addr);

	cpu_write_doubleword_in_reg(&(cpu_state->eax), val);

	return true;
}

case 0xe6:{

#ifdef DEBUG_PRINT_INST
cpu_print_inst("OUT imm8 AL\n");
#endif
	/*OUT imm8,AL*/
	uint8_t addr = cpu_consume_byte_from_mem(cpu_state);
	uint8_t val = cpu_read_byte_from_reg(&(cpu_state->eax), !HIGH_BYTE);
	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr, val);

	return true;
}

case 0xe7:{

#ifdef DEBUG_PRINT_INST
cpu_print_inst("IN imm32 AL\n");
#endif
	/*OUT imm32, AL*/
	uint8_t addr = cpu_consume_byte_from_mem(cpu_state);
	uint32_t val = cpu_read_doubleword_from_reg(&(cpu_state->eax));

	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr+3, (val >> 24) & 0xFF);
	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr+2, (val >> 16) & 0xFF);
	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr+1, (val >>  8)  & 0xFF);
	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr,     val        & 0xFF);

	return true;
}

case 0xee:{

#ifdef DEBUG_PRINT_INST
cpu_print_inst("OUT DX AL\n");
#endif
	/*OUT DX,AL*/
	uint16_t addr = cpu_read_doubleword_from_reg(&(cpu_state->edx));
	uint8_t val = cpu_read_byte_from_reg(&(cpu_state->eax), !HIGH_BYTE);
	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr, val);

	return true;
}

case 0xef:{

#ifdef DEBUG_PRINT_INST
cpu_print_inst("OUT DX EAX\n");
#endif
	/*OUT DX,EAX*/
	uint16_t addr = cpu_read_doubleword_from_reg(&(cpu_state->edx));
	uint32_t val = cpu_read_doubleword_from_reg(&(cpu_state->eax));

	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr+3, (val >> 24) & 0xFF);
	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr+2, (val >> 16) & 0xFF);
	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr+1, (val >>  8)  & 0xFF);
	sig_host_bus_write_io_dev(cpu_state->port_host, cpu_state, addr,     val        & 0xFF);

	return true;
}
