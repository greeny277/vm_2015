#include "pic.h"

static bool
pic_int_ack(void * _pic_state, uint8_t int_num){
	return false;
}

static bool
pic_write_to_io_dev(void *_pic_state, uint32_t addr, uint8_t *valp){
	pic_state *pic_state = (struct pic_state*) _pic_state;
	return false;
}

static bool
pic_read_from_io_dev(void *_pic_state, uint32_t addr, uint8_t val){
	pic_state *pic_state = (struct pic_state*) _pic_state;
	return false;
}


void *
pic_create(struct sig_host_bus *port_host, struct sig_boolean *pic_to_cpu_bool)
{
	pic_state *pic_state;
	static const struct sig_host_bus_funcs hf = {
		.read_from_io_dev = pic_read_from_io_dev,
		.write_to_io_dev = pic_write_to_io_dev,
		.int_ack = pic_int_ack
	};

	static const struct sig_boolean_funcs bf = {
		.set = pic_set
	};

	pic_state = malloc(sizeof(struct pic_state));
	assert(pic_state != NULL);

	pic_state->port_host = port_host;
	pic_state->pic_to_cpu_bool = pic_to_cpu_bool;

	sig_host_bus_connect(port_host, pic_state, &hf);
	sig_boolean_connect(pic_to_cpu_bool, pic_state, &bf);

	return pic_state;
}

void
pic_destroy(void *_pic_state)
{
	free(_pic_state);
}
