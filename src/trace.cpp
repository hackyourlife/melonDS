#include <cstdint>
#include <cstdlib>
#include "trace.h"
#include "ARM.h"

#define	MASK_CPSR	1

#define	TYPE_STEP7	0
#define	TYPE_STEP9	1
#define	TYPE_READ_8	2
#define	TYPE_READ_16	3
#define	TYPE_READ_32	4
#define	TYPE_WRITE_8	5
#define	TYPE_WRITE_16	6
#define	TYPE_WRITE_32	7
#define	TYPE_DUMP	8
#define	TYPE_IRQ	9

struct STEP9 {
	u8	type;
	u8	mask;
	u16	rmask;
	u32	icode;
	u64	cycle;
	u32	data[17];
};

Trace::Trace()
{
	trc = NULL;
}

Trace::Trace(std::string filename)
{
	open(filename);
}

void Trace::open(std::string filename)
{
	if(trc)
		fclose(trc);

	// open file
	trc = fopen(filename.c_str(), "wb");
	if(!trc)
		abort();

	// write header
	char header[] = { 'X', 'T', 'R', 'C', 0, 40 };
	fwrite(header, sizeof(header), 1, trc);
	fflush(trc);
}

Trace::~Trace()
{
	if(trc) {
		fflush(trc);
		fclose(trc);
	}
}

void Trace::step9(const ARMv5* cpu)
{
	if(!trc)
		return;

	int wr = 0;
	STEP9 step = {};
	step.type = TYPE_STEP9;
	step.cycle = this->step++;
	step.icode = cpu->NextInstr[0];

	if(cpu->CPSR != last9.CPSR) {
		step.mask |= MASK_CPSR;
		last9.CPSR = cpu->CPSR;
		step.data[wr++] = cpu->CPSR;
	}

	for(int i = 0; i < 16; i++) {
		if(cpu->R[i] != last9.R[i]) {
			step.rmask |= (1 << i);
			last9.R[i] = cpu->R[i];
			step.data[wr++] = cpu->R[i];
		}
	}

	fwrite(&step, 1, 16 + wr * 4, trc);
}

void Trace::irq(const ARM* cpu)
{
	if(!trc)
		return;

	u8 type = TYPE_IRQ;
	fwrite(&type, 1, 1, trc);
}

void Trace::read8(const u32 addr, const u8 value)
{
	if(!trc)
		return;

	u8 tmp[2] = { TYPE_READ_8, value };
	fwrite(tmp, 2, 1, trc);
	fwrite(&addr, 4, 1, trc);
}

void Trace::write8(const u32 addr, const u8 value)
{
	if(!trc)
		return;

	u8 tmp[2] = { TYPE_WRITE_8, value };
	fwrite(tmp, 2, 1, trc);
	fwrite(&addr, 4, 1, trc);
}

void Trace::read16(const u32 addr, const u16 value)
{
	if(!trc)
		return;

	u8 tmp[3] = { TYPE_READ_16, (u8) value, (u8) (value >> 8) };
	fwrite(tmp, 3, 1, trc);
	fwrite(&addr, 4, 1, trc);
}

void Trace::write16(const u32 addr, const u16 value)
{
	if(!trc)
		return;

	u8 tmp[3] = { TYPE_WRITE_16, (u8) value, (u8) (value >> 8) };
	fwrite(tmp, 3, 1, trc);
	fwrite(&addr, 4, 1, trc);
}

void Trace::read32(const u32 addr, const u32 value)
{
	if(!trc)
		return;

	u8 type = TYPE_READ_32;
	u32 tmp[2] = { value, addr };
	fwrite(&type, 1, 1, trc);
	fwrite(tmp, 8, 1, trc);
}

void Trace::write32(const u32 addr, const u32 value)
{
	if(!trc)
		return;

	u8 type = TYPE_WRITE_32;
	u32 tmp[2] = { value, addr };
	fwrite(&type, 1, 1, trc);
	fwrite(tmp, 8, 1, trc);
}

void Trace::dump(const u32 addr, const u32 size, const u8* data)
{
	if(!trc)
		return;

	u8 type = TYPE_DUMP;
	u32 tmp[2] = { addr, size };
	fwrite(&type, 1, 1, trc);
	fwrite(tmp, 4, 2, trc);
	fwrite(data, size, 1, trc);
}
