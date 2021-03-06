/*
 * Memory.c -- implementation of some memory classes.
 */

#include <iostream>
#include <fstream>
#include "goodies.hh"
#include "List.hh"
#include "simulation.hh"
#include "Module.hh"
#include "Bus.hh"
#include "Memory.hh"

extern int get_byte(std::istream&);

Ram::Ram(Ulong size, Uint read_delay, Uint write_delay, const char* init_file) {
	int bit_num = find_ms1b(size);
	if ((size < 4) || ((1<<bit_num) != size))
		fatal("Memory: size must be a power of 2 and at least 4 bytes\n");

	// Allocate the contents array and set up the address mask.
	contents   =  new Uchar[size];
	addr_mask  =  ((1<<bit_num) - 1) & ~(sizeof(Word) - 1);

	// Record the read and write delays.
	this->read_delay  = read_delay;
	this->write_delay = write_delay;

	// Read in the contents of the initialization file, if there was one.
	if (!init_file)
		return;
	std::ifstream input(init_file);
	if (!input) {
		err("Memory: cannot open initialization file \"%s\"\n", init_file);
		return;
	}
	unsigned bytes_read = 0;
	for (;;) {
		int byte = get_byte(input);
		if (byte == -1)
			break;
		if (bytes_read >= size) {
			err("Memory: initialization file is too large\n");
			break;
		}
		contents[bytes_read++]  =  (Uchar)byte;
	}
}

void Ram::read(Read_write_info& info, Access_status& status) {
	Address addr = info.addr;
	Word*   buff = info.buff;
	std::cout << "Read of " << addr << " will complete in " << read_delay << " cycles\n";
	for (int i=info.count; i--; ) {
		*buff++ = *(Word*) (contents + (addr & addr_mask));
		addr += sizeof(Word);
	}
	status.set(read_delay, OK);
}

void Ram::write(Read_write_info& info, Access_status& status) {
	Address addr = info.addr;
	Word*   buff = info.buff;
	std::cout << "Write of " << addr << " will complete in " << read_delay << " cycles";
	for (int i=info.count; i--; ) {
		*(Word*)(contents + (addr & addr_mask)) = *buff++;
		addr += sizeof(Word);
	}
	status.set(write_delay, OK);
}

void Ram::write_part(Write_part_info& info, Access_status& status) {
	Ulong	index = info.addr & addr_mask;
	Word*	word = (Word*)(contents+index);
	std::cout << "Write (part) of " << info.addr << " will complete in " << read_delay << " cycles";
	*word = (*word & ~info.mask) | (info.word & info.mask);
	status.set(write_delay, OK);
}

#pragma argsused
void Rom::write(Read_write_info& info, Access_status& status) {
	status.set(1, FAULT);
}

#pragma argsused
void Rom::write_part(Write_part_info& info, Access_status& status) {
	status.set(1, FAULT);
}
