case 0xF4: {
	#ifdef DEBUG_PRINT_INST
	fprintf(stderr, "HLT \n");
	#endif

	//If interrupts are disabled (cli was called), stop vm execution
	if(!cpu_get_interrupt_flag(cpu_state))
		return false;

	sigset_t mask;
	sigfillset(&mask);
	sigdelset(&mask, SIGIO);
	sigsuspend(&mask);
	return true;
}
