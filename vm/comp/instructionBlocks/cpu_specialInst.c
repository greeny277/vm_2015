case 0x0f: {
	/* The opcode is two bytes long */
	uint8_t op_code_2 = cpu_consume_byte_from_mem(cpu_state);
	switch(op_code_2){
		#include "cpu_twoBytes_0x0f.c"
	}
}

case 0x80: {
	/* Special case: Specific instruction decoded in Mod/RM byte */

	if(likely(cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT))){
		switch(s_op.reg_value){
			#include "cpu_special0x80.c"
		}
	}
	break;
}

case 0x81: {
	/* Special case: Specific instruction decoded in Mod/RM byte */

	if(likely(cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT))){
		switch(s_op.reg_value){
			#include "cpu_special0x81.c"
		}
	}
	break;
}

case 0x83: {
	/* Special case: Specific instruction decoded in Mod/RM byte */

	if(likely(cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT))){
		switch(s_op.reg_value){
			#include "cpu_special0x83.c"
		}
	}
	break;
}

case 0xC6: {
	/* Special case: Specific instruction decoded in Mod/RM byte */

	if(likely(cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT))){
		switch(s_op.reg_value){
			#include "cpu_special0xC6.c"
		}
	}
	break;
}

case 0xC7: {
	/* Special case: Specific instruction decoded in Mod/RM byte */

	if(likely(cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT))){
		switch(s_op.reg_value){
			#include "cpu_special0xC7.c"
		}
	}
	break;
}

case 0xF6: {
	/* Special case: Specific instruction decoded in Mod/RM byte */

	if(likely(cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT))){
		switch(s_op.reg_value){
			#include "cpu_special0xF6.c"
		}
	}
	break;
}

case 0xFF: {
	/* Special case: Specific instruction decoded in Mod/RM byte */

	if(likely(cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT))){
		switch(s_op.reg_value){
			#include "cpu_special0xFF.c"
		}
	}
	break;
}
