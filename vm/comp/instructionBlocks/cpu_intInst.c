case 0xCC : {
	cpu_handle_interrupt_vector(cpu_state, 3);
	return true;
}

case 0xCD : {
	cpu_handle_interrupt_vector(cpu_state, cpu_consume_byte_from_mem(cpu_state));
	return true;
}

case 0xCE : {
	if(cpu_get_overflow_flag(cpu_state)){
		cpu_handle_interrupt(cpu_state, 4);
	}
	return true;
}