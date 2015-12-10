/* $Id: sig_host_bus.h 10023 2014-10-16 09:56:44Z raimue $ */

#ifndef __SIG_HOST_BUS_H_INCLUDED
#define __SIG_HOST_BUS_H_INCLUDED

#include <inttypes.h>
#include <stdbool.h>


/** callback functions for host bus  */
struct sig_host_bus_funcs {
	/** callback for read byte.
	  * @param s object pointer of callee
	  * @param addr requested address
	  * @param valp should write value there
	  * @return true, if address is from callee
	  */
	bool (*readb)(void *s, uint32_t addr, uint8_t *valp);

	/** callback for write byte
	  * @param s object pointer of callee
	  * @param addr requested address
	  * @param val the value to write
	  * @return true, if port is from callee
	  */
	bool (*writeb)(void *s, uint32_t addr, uint8_t val);

	/** callback for writing a byte via io-devices
	  * @param s object pointer of callee
	  * @param addr requested address
	  * @param val the value to write
	  * @return true, if port is from callee
	  */
	bool (*outb)(void *s, uint32_t addr, uint8_t val);

	/** callback for reading a byte via io-devices
	  * @param s object pointer of callee
	  * @param addr requested address
	  * @param vap should write value there
	  * @return true, if port is from callee
	  */
	bool (*inb)(void *s, uint32_t addr, uint8_t *valp);

	/** callback for writing a byte to an io-device
	 *  from io-decoder
	 * @param s object pointer of callee
	 * @param addr requested address
	 * @param val the value to write
	 * @return true, if port is from callee
	 */
	bool (*write_to_io_dev)(void *s, uint32_t addr, uint8_t val);

	/** callback for reading a byte from an io-device
	  * @param s object pointer of callee
	  * @param addr requested address
	  * @param vap should write value there
	  * @return true, if port is from callee
	  */
	bool (*read_from_io_dev)(void *s, uint32_t addr, uint8_t *valp);
};

/** host bus structure */
struct sig_host_bus {
	/** number of registered members */
	unsigned int nmembers;
	struct {
		/** object of registered member */
		void *s;
		/** callback functions for registered member */
		const struct sig_host_bus_funcs *f;
	} members[8];
};

/** create sig_host_bus
  * @return sig_host_bus instance
  */
extern struct sig_host_bus *
sig_host_bus_create(void);

/** destroy a sig_host_bus instance
  * @param sig_host_bus instance
  */
extern void
sig_host_bus_destroy(struct sig_host_bus *bus);

/** connect a member to the host bus
  * @param bus bus object
  * @param s object pointer
  * @param f callback functions
  */
extern void
sig_host_bus_connect(
	struct sig_host_bus *bus,
	void *s,
	const struct sig_host_bus_funcs *f
);

/** write to the host bus
  * @param bus bus object
  * @param s sender object
  * @param addr memory address
  * @param val value to write
  */
extern void
sig_host_bus_writeb(
	const struct sig_host_bus *bus,
	void *s,
	uint32_t addr,
	uint8_t val
);

/** write to the host bus
  * @param bus bus object
  * @param s sender object
  * @param addr memory address
  * @return read value
  */
extern uint8_t
sig_host_bus_readb(const struct sig_host_bus *bus, void *s, uint32_t addr);

extern uint8_t
sig_host_bus_inb(const struct sig_host_bus *bus, void *s, uint32_t addr);

extern void
sig_host_bus_outb(const struct sig_host_bus *bus, void *s, uint32_t addr, uint8_t val);

extern uint8_t
sig_host_bus_read_io_dev(const struct sig_host_bus *bus, void *s, uint32_t addr);

extern void
sig_host_bus_write_io_dev(const struct sig_host_bus *bus, void *s, uint32_t addr, uint8_t val);

#endif /* __SIG_HOST_BUS_H_INCLUDED */
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
