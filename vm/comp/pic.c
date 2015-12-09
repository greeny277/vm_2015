#include "pic.h"

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

static bool pic_write_to_io_dev(void *_pic_state, uint32_t addr, uint8_t val);
static bool pic_read_from_io_dev(void *_pic_state, uint32_t addr, uint8_t *valp);

static bool pic_handle_OCW_Byte(pic_state *pic_state, uint8_t ocw_byte);
static bool pic_handle_ICW_byte(pic_state *pic_state, uint8_t icw_byte);
static bool pic_handle_interrupt(pic_state *pic_state, uint8_t int_num);

/**
 * Callback for writing to the pic from the CPU via the iodecoder using OUTB.
 * This is used for programming the PIC
 *
 * @param _pic_state the pic instance
 * @param addr the address in the IO address space
 * @param val  the value to write
 */
static bool
pic_write_to_io_dev(void *_pic_state, uint32_t addr, uint8_t val){
	pic_state *pic_state = (struct pic_state*) _pic_state;

	if(pic_state->cur_icw_byte_no > 4){
		pic_handle_OCW_Byte(pic_state, val);
	}else{
		pic_handle_ICW_byte(pic_state, val);
	}
	return false;
}

/**
 * Callback for reading from the pic in the CPU via the iodecoder using INB.
 * We abuse this for reading the interrupt number
 *
 * @param _pic_state the pic instance
 * @param addr the address in the IO address space
 * @param valp the pointer to write the value to
 * @return true
 */
static bool
pic_read_from_io_dev(void *_pic_state, uint32_t addr, uint8_t *valp){
	pic_state *pic_state = (struct pic_state*) _pic_state;

	//todo: reraise, if irr != 0?

	//in case no irr bit is set, return 7
	uint8_t i=0;
	while(i<8){
		if(pic_state->irr & (1 << i))
			break;
		i++;
	}
	*valp = pic_state->interrupt_vector_byte_base + i;

	return true;
}

/**
 * Check for erros in the Initialization Command Word and set our state accordingly
 *
 * @param _pic_state the pic instance
 * @param icw_byte one ICW byte
 * @return false in error case else true
 */
static bool pic_handle_ICW_byte(pic_state *pic_state, uint8_t icw_byte){
	switch(pic_state->cur_icw_byte_no){
		case 1:
			if(unlikely((icw_byte & (1 << IC4)) == 0)){
				fprintf(stderr, "^IC4 in ICW1 implies MCS-80/85 and normal EOI mode, which is unsupported!");
				return false;
			}
			if(unlikely((icw_byte & (1 << SNGL)) == 0)){
				fprintf(stderr, "^SNGL in ICW1 sets cascade mode, which is unsupported!");
				return false;
			}
			if(unlikely((icw_byte & (1 << LTIM)) == 0)){
				fprintf(stderr, "^LTIM in ICW1 sets level triggered mode, which is unsupported!");
				return false;
			}
			pic_state->cur_icw_byte_no++;
			break;
		case 2:
			pic_state->interrupt_vector_byte_base = icw_byte >> T3;
			pic_state->cur_icw_byte_no++;
			pic_state->cur_icw_byte_no++; //SNGL 1 in ICW1 is 'hardcoded' in our PIC => skip ICW3
			break;
		case 4:
			if(unlikely((icw_byte & (1 << uPM)) == 0)){
				fprintf(stderr, "^uPM in ICW4 sets MCS-80/85, which is unsupported!");
				return false;
			}
			if(unlikely((icw_byte & (1 << AEOI)) == 0)){
				fprintf(stderr, "^SNGL in ICW4 sets normal EOI mode, which is unsupported!");
				return false;
			}
			//TODO: Sollten wir BUF und MS auch pruefen?
			if(unlikely((icw_byte & (1 << SFNM)) == 1)){
				fprintf(stderr, "SFNM in ICW4 sets special fully nested mode, which is unsupported!");
				return false;
			}
			pic_state->cur_icw_byte_no++;
			break;
		default:
			fprintf(stderr, "Invalid number of ICW bytes read");
			return false;
	}
	return true;
}

/**
 * Check for erros in the Operation Command Word and set our state accordingly
 *
 * @param _pic_state the pic instance
 * @param ocw_byte one ICW byte
 * @return false in error case else true
 */
static bool pic_handle_OCW_Byte(pic_state *pic_state, uint8_t ocw_byte){
	if(pic_state->cur_ocw_byte_no == 1)
		pic_state->interrupt_mask = ocw_byte;

	//ignore ocw2 and ocw3 for now (they control modes we don't care about)

	pic_state->cur_ocw_byte_no++;
	if(pic_state->cur_ocw_byte_no == 4)
		pic_state->cur_ocw_byte_no = 1;

	return true;
}

/**
 * Handles an interrupt by saving an entry in irr
 *
 * @param _pic_state the pic instance
 * @param int_num the interrupt number to handle
 */

static bool pic_handle_interrupt(pic_state *pic_state, uint8_t int_num){
	pic_state->irr |= 1<<int_num;

	if(!(pic_state->interrupt_mask & (1 << int_num))){
		//this interrupt is masked and should not be handled
		return false;
	}

	cpu_interrupt(pic_state->cpu);

	return true;
}

bool pic_connect(pic_state *pic_instance, uint8_t int_no, pic_connection **conn){
	if(pic_instance->connected_lines & (1<<int_no))
		return false;

	if(int_no > 7)
		return false;

	pic_instance->connected_lines |= (1<<int_no);

	*conn = pic_instance->connections + int_no;
	(*conn)->pic_instance = pic_instance;
	(*conn)->int_no = int_no;

	return true;
}

bool inline pic_interrupt(pic_connection *conn){
	if(conn->int_no > 7)
		return false;
	return pic_handle_interrupt(conn->pic_instance, conn->int_no);
}

/**
 * The PICs' constructor
 * @param port_host the Host bus to which this pic is connected
 * @param cpu_instance the cpu to which this pic is connected
 */
void *
pic_create(struct sig_host_bus *port_host, cpu_state *cpu_instance)
{
	pic_state *pic_state;
	static const struct sig_host_bus_funcs hf = {
		.read_from_io_dev = pic_read_from_io_dev,
		.write_to_io_dev = pic_write_to_io_dev
	};

	pic_state = malloc(sizeof(struct _pic_state));
	assert(pic_state != NULL);

	pic_state->port_host = port_host;
	pic_state->cpu = cpu_instance;

	pic_state->cur_icw_byte_no = 1;
	pic_state->cur_ocw_byte_no = 1;
	pic_state->irr = 0;

	pic_state->connected_lines = 0;

	sig_host_bus_connect(port_host, pic_state, &hf);

	return pic_state;
}

/**
 * The PICs' destructor
 */
void
pic_destroy(void *_pic_state)
{
	free(_pic_state);
}