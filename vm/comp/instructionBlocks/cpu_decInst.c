		case 0x48: {
			/* Decrement word register eax by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->eax));
			src--;
			cpu_write_word_in_reg(&(cpu_state->eax), src);

			cpu_set_overflow_sub(cpu_state, src+1, 1, src, !EIGHT_BIT);
			cpu_set_aux_flag_sub(cpu_state, src+1, 1, !EIGHT_BIT);
			cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
			cpu_set_zero_flag(cpu_state, src);

			#ifdef DEBUG_PRINT_INST
			fprintf(stderr, "DEC EAX \n");
			#endif

			return true;
		}

		case 0x49: {
			/* Decrement word register ecx by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->ecx));
			src--;
			cpu_write_word_in_reg(&(cpu_state->ecx), src);

			cpu_set_overflow_sub(cpu_state, src+1, 1, src, !EIGHT_BIT);
			cpu_set_aux_flag_sub(cpu_state, src+1, 1, !EIGHT_BIT);
			cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
			cpu_set_zero_flag(cpu_state, src);

			#ifdef DEBUG_PRINT_INST
			fprintf(stderr, "DEC ECX \n");
			#endif

			return true;
		}

		case 0x4A: {
			/* Decrement word register edx by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->edx));
			src--;
			cpu_write_word_in_reg(&(cpu_state->edx), src);

			cpu_set_overflow_sub(cpu_state, src+1, 1, src, !EIGHT_BIT);
			cpu_set_aux_flag_sub(cpu_state, src+1, 1, !EIGHT_BIT);
			cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
			cpu_set_zero_flag(cpu_state, src);

			#ifdef DEBUG_PRINT_INST
			fprintf(stderr, "DEC EDX \n");
			#endif

			return true;
		}

		case 0x4B: {
			/* Decrement word register ebx by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->ebx));
			src--;
			cpu_write_word_in_reg(&(cpu_state->ebx), src);

			cpu_set_overflow_sub(cpu_state, src+1, 1, src, !EIGHT_BIT);
			cpu_set_aux_flag_sub(cpu_state, src+1, 1, !EIGHT_BIT);
			cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
			cpu_set_zero_flag(cpu_state, src);

			#ifdef DEBUG_PRINT_INST
			fprintf(stderr, "DEC EBX \n");
			#endif

			return true;
		}

		case 0x4C: {
			/* Decrement word register esp by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->esp));
			src--;
			cpu_write_word_in_reg(&(cpu_state->esp), src);

			cpu_set_overflow_sub(cpu_state, src+1, 1, src, !EIGHT_BIT);
			cpu_set_aux_flag_sub(cpu_state, src+1, 1, !EIGHT_BIT);
			cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
			cpu_set_zero_flag(cpu_state, src);

			#ifdef DEBUG_PRINT_INST
			fprintf(stderr, "DEC ESP \n");
			#endif

			return true;
		}

		case 0x4D: {
			/* Decrement word register ebp by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->ebp));
			src--;
			cpu_write_word_in_reg(&(cpu_state->ebp), src);

			cpu_set_overflow_sub(cpu_state, src+1, 1, src, !EIGHT_BIT);
			cpu_set_aux_flag_sub(cpu_state, src+1, 1, !EIGHT_BIT);
			cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
			cpu_set_zero_flag(cpu_state, src);

			#ifdef DEBUG_PRINT_INST
			fprintf(stderr, "DEC EBX \n");
			#endif

			return true;
		}

		case 0x4E: {
			/* Decrement word register esi by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->esi));
			src--;
			cpu_write_word_in_reg(&(cpu_state->esi), src);

			cpu_set_overflow_sub(cpu_state, src+1, 1, src, !EIGHT_BIT);
			cpu_set_aux_flag_sub(cpu_state, src+1, 1, !EIGHT_BIT);
			cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
			cpu_set_zero_flag(cpu_state, src);

			#ifdef DEBUG_PRINT_INST
			fprintf(stderr, "DEC ESI \n");
			#endif

			return true;
		}

		case 0x4F: {
			/* Decrement word register edi by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->edi));
			src--;
			cpu_write_word_in_reg(&(cpu_state->edi), src);

			cpu_set_overflow_sub(cpu_state, src+1, 1, src, !EIGHT_BIT);
			cpu_set_aux_flag_sub(cpu_state, src+1, 1, !EIGHT_BIT);
			cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
			cpu_set_zero_flag(cpu_state, src);

			#ifdef DEBUG_PRINT_INST
			fprintf(stderr, "DEC EDI \n");
			#endif

			return true;
		}
