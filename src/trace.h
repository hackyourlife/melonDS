#ifndef TRACE_H
#define TRACE_H

#include <string>
#include <cstdio>

class ARM;
class ARMv5;

#include "types.h"

struct CPUState {
	u32	R[16];
	u32	CPSR;
};

class Trace {
private:
	FILE*	trc;
	CPUState last9 = {};
	u64	step = 0;

public:
	Trace(std::string filename);
	~Trace();

	void open(std::string filename);
	void step9(const ARMv5* cpu);
	void irq(const ARM* cpu);

	void read8(const u32 addr, const u8 val);
	void read16(const u32 addr, const u16 val);
	void read32(const u32 addr, const u32 val);
	void write8(const u32 addr, const u8 val);
	void write16(const u32 addr, const u16 val);
	void write32(const u32 addr, const u32 val);

	void dump(const u32 addr, const u32 size, const u8* data);
};

#endif
