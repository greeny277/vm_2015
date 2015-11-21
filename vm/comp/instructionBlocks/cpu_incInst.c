		case 0x40: {
			/* Increment doubleword register eax by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->eax));
			src++;
			cpu_write_word_in_reg(&(cpu_state->eax), src);
			return true;
		}
		
		case 0x41: {
			/* Increment doubleword register ecx by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->ecx));
			src++;
			cpu_write_word_in_reg(&(cpu_state->ecx), src);
			return true;
		}

		case 0x42: {
			/* Increment doubleword register edx by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->edx));
			src++;
			cpu_write_word_in_reg(&(cpu_state->edx), src);
			return true;
		}

		case 0x43: {
			/* Increment doubleword register ebx by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->ebx));
			src++;
			cpu_write_word_in_reg(&(cpu_state->ebx), src);
			return true;
		}

		case 0x44: {
			/* Increment doubleword register esp by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->esp));
			src++;
			cpu_write_word_in_reg(&(cpu_state->esp), src);
			return true;
		}

		case 0x45: {
			/* Increment doubleword register ebp by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->ebp));
			src++;
			cpu_write_word_in_reg(&(cpu_state->ebp), src);
			return true;
		}

		case 0x46: {
			/* Increment doubleword register esi by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->esi));
			src++;
			cpu_write_word_in_reg(&(cpu_state->esi), src);
			return true;
		}

		case 0x47: {
			/* Increment doubleword register edi by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->edi));
			src++;
			cpu_write_word_in_reg(&(cpu_state->edi), src);
			return true;
		}
