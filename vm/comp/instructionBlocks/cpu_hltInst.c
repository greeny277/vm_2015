case 0xF4: {

	#ifdef DEBUG_PRINT_INST
	cpu_print_inst("HLT \n");
	#endif

	//If interrupts are disabled (cli was called), stop vm execution
	if(!cpu_get_interrupt_flag(cpu_state))
		return false;

	sigset_t mask;
	sigfillset(&mask);
	sigdelset(&mask, SIGIO);
	sigdelset(&mask, SIGINT);
	sigsuspend(&mask);
	return true;
}
