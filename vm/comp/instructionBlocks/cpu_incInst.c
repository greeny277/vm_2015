		case 0x40: {
			/* Increment word register eax by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->eax));
			src++;
			cpu_write_word_in_reg(&(cpu_state->eax), src);

			cpu_set_overflow_add(cpu_state, src-1, 1, src, !EIGHT_BIT);
			cpu_set_aux_flag_add(cpu_state, src+1, 1, src, !EIGHT_BIT);
			cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
			cpu_set_zero_flag(cpu_state, src);

			#ifdef DEBUG_PRINT_INST
			fprintf(stderr, "INC EAX \n");
			#endif

			return true;
		}

		case 0x41: {
			/* Increment word register ecx by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->ecx));
			src++;
			cpu_write_word_in_reg(&(cpu_state->ecx), src);

			cpu_set_overflow_add(cpu_state, src-1, 1, src, !EIGHT_BIT);
			cpu_set_aux_flag_add(cpu_state, src+1, 1, src, !EIGHT_BIT);
			cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
			cpu_set_zero_flag(cpu_state, src);

			#ifdef DEBUG_PRINT_INST
			fprintf(stderr, "INC ECX \n");
			#endif

			return true;
		}

		case 0x42: {
			/* Increment word register edx by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->edx));
			src++;
			cpu_write_word_in_reg(&(cpu_state->edx), src);

			cpu_set_overflow_add(cpu_state, src-1, 1, src, !EIGHT_BIT);
			cpu_set_aux_flag_add(cpu_state, src+1, 1, src, !EIGHT_BIT);
			cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
			cpu_set_zero_flag(cpu_state, src);

			#ifdef DEBUG_PRINT_INST
			fprintf(stderr, "INC EDX \n");
			#endif

			return true;
		}

		case 0x43: {
			/* Increment word register ebx by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->ebx));
			src++;
			cpu_write_word_in_reg(&(cpu_state->ebx), src);

			cpu_set_overflow_add(cpu_state, src-1, 1, src, !EIGHT_BIT);
			cpu_set_aux_flag_add(cpu_state, src+1, 1, src, !EIGHT_BIT);
			cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
			cpu_set_zero_flag(cpu_state, src);

			#ifdef DEBUG_PRINT_INST
			fprintf(stderr, "INC EBX \n");
			#endif

			return true;
		}

		case 0x44: {
			/* Increment word register esp by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->esp));
			src++;
			cpu_write_word_in_reg(&(cpu_state->esp), src);

			cpu_set_overflow_add(cpu_state, src-1, 1, src, !EIGHT_BIT);
			cpu_set_aux_flag_add(cpu_state, src+1, 1, src, !EIGHT_BIT);
			cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
			cpu_set_zero_flag(cpu_state, src);

			#ifdef DEBUG_PRINT_INST
			fprintf(stderr, "INC ESP \n");
			#endif

			return true;
		}

		case 0x45: {
			/* Increment word register ebp by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->ebp));
			src++;
			cpu_write_word_in_reg(&(cpu_state->ebp), src);

			cpu_set_overflow_add(cpu_state, src-1, 1, src, !EIGHT_BIT);
			cpu_set_aux_flag_add(cpu_state, src+1, 1, src, !EIGHT_BIT);
			cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
			cpu_set_zero_flag(cpu_state, src);

			#ifdef DEBUG_PRINT_INST
			fprintf(stderr, "INC EBP \n");
			#endif

			return true;
		}

		case 0x46: {
			/* Increment word register esi by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->esi));
			src++;
			cpu_write_word_in_reg(&(cpu_state->esi), src);

			cpu_set_overflow_add(cpu_state, src-1, 1, src, !EIGHT_BIT);
			cpu_set_aux_flag_add(cpu_state, src-1, 1, src, !EIGHT_BIT);
			cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
			cpu_set_zero_flag(cpu_state, src);

			#ifdef DEBUG_PRINT_INST
			fprintf(stderr, "INC ESI \n");
			#endif

			return true;
		}

		case 0x47: {
			/* Increment word register edi by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->edi));
			src++;
			cpu_write_word_in_reg(&(cpu_state->edi), src);

			cpu_set_overflow_add(cpu_state, src-1, 1, src, !EIGHT_BIT);
			cpu_set_aux_flag_add(cpu_state, src-1, 1, src, !EIGHT_BIT);
			cpu_set_sign_flag(cpu_state, src, !EIGHT_BIT);
			cpu_set_zero_flag(cpu_state, src);

			#ifdef DEBUG_PRINT_INST
			fprintf(stderr, "INC EDI \n");
			#endif

			return true;
		}
