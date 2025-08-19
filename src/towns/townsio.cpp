/* LICENSE>>
Copyright 2020 Soji Yamakawa (CaptainYS, http://www.ysflight.com)

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

<< LICENSE */
#include <iostream>
#include "townsio.h"
#include "outside_world.h"



/* virtual */ void FMTownsCommon::IOWriteByte(unsigned int ioport,unsigned int data)
{
	switch(ioport)
	{
	case TOWNSIO_POWER_CONTROL: // 0x0022
		if(0!=(data&0x40))
		{
			var.powerOff=true;
		}
		if(0!=(data&1))
		{
			state.resetReason=RESET_REASON_SOFTWARE;
			Reset();
		}
		// Since Towns HG, bit 8 is CRT power OFF.  Tsugaru is not taking it into account
		// at this time.
		break;
	case TOWNSIO_RESET_REASON:
		if(0!=(data&0x40))
		{
			var.powerOff=true;
		}
		if(0!=(data&0x01))
		{
			ScheduleDeviceCallBack(*this,state.townsTime+1);
			state.resetReason=RESET_REASON_SOFTWARE;
		}
		break;
	case TOWNSIO_SERIAL_ROM_CTRL://=        0x32,
		if((0x20&data)==0x00 && // Chip-Select==0 (Active Low)
		   (0x80&state.lastSerialROMCommand)!=0 && 
		   (0x80&data)==0)
		{
			state.serialROMBitCount=0;
		}
		else if((0xA0&data)==0x00 && // ID RESET==0, Chip-Select==0 (Active Low)
		        (0x40&state.lastSerialROMCommand)==0 && (0x40&data)!=0)
		{
			state.serialROMBitCount=(state.serialROMBitCount+1)&255;
		}
		state.lastSerialROMCommand=data;
		break;

	case TOWNS_QUICK_DEBUG_BREAK: //        0xEA,  // Writing to this I/O port will break the VM.
		debugger.ExternalBreak("Break Request from VM");
		break;
	case TOWNS_QUICK_DEBUG_STATE: //        0xEB,  // Writing to this I/O port will show the VM state, not break.
		PrintStatus();
		break;
	case TOWNSIO_HOST_CONSOLE:
		{
			char c=data;
			std::cout << c;
		}
		break;

	case TOWNSIO_VM_HOST_IF_CMD_STATUS:
		ProcessVMToHostCommand(data,var.nVM2HostParam,var.VM2HostParam);
		var.nVM2HostParam=0;
		break;
	case TOWNSIO_VM_HOST_IF_DATA:
		if(var.nVM2HostParam<Variable::VM2HOST_PARAM_QUEUE_LENGTH)
		{
			var.VM2HostParam[var.nVM2HostParam++]=data;
		}
		break;
	case TOWNSIO_TIMER_1US_WAIT:
		state.townsTime+=1000;
		break;

	case TOWNSIO_ELEVOL_1_DATA: //           0x4E0, // [2] pp.18, pp.174
		state.eleVol[0][state.eleVolChLatch[0]].vol=(data&0x3f);
		UpdateEleVol(0);
		break;
	case TOWNSIO_ELEVOL_1_COM: //            0x4E1, // [2] pp.18, pp.174
		state.eleVolChLatch[0]=data&3;
		state.eleVol[0][state.eleVolChLatch[0]].EN=(0!=(data&4));
		state.eleVol[0][state.eleVolChLatch[0]].C0=(0!=(data&8));
		state.eleVol[0][state.eleVolChLatch[0]].C32=(0!=(data&16));
		UpdateEleVol(0);
		break;
	case TOWNSIO_ELEVOL_2_DATA: //           0x4E2, // [2] pp.18, pp.174
		state.eleVol[1][state.eleVolChLatch[1]].vol=(data&0x3f);
		UpdateEleVol(1);
		break;
	case TOWNSIO_ELEVOL_2_COM: //            0x4E3, // [2] pp.18, pp.174
		state.eleVolChLatch[1]=data&3;
		state.eleVol[1][state.eleVolChLatch[1]].EN=(0!=(data&4));
		state.eleVol[1][state.eleVolChLatch[1]].C0=(0!=(data&8));
		state.eleVol[1][state.eleVolChLatch[1]].C32=(0!=(data&16));
		UpdateEleVol(1);
		break;

	case TOWNSIO_MAINRAM_WAIT_1STGEN: //     0x5E0,
		state.mainRAMWait=data;
		AdjustMachineSpeedForMemoryWait();
		break;
	case TOWNSIO_MAINRAM_WAIT: //            0x5E2,
		state.mainRAMWait=data;
		AdjustMachineSpeedForMemoryWait();
		break;
	case TOWNSIO_VRAMWAIT: //                0x5E6,
		state.VRAMWait=data;
		AdjustMachineSpeedForMemoryWait();
		break;
	case TOWNSIO_FASTMODE: //                0x5EC, // [2] pp.794
		if(0!=(data&1))
		{
			state.mainRAMWait=0;
			state.VRAMWait=0;
		}
		else
		{
			state.mainRAMWait=6;
			state.VRAMWait=6;
		}
		AdjustMachineSpeedForMemoryWait();
		break;
	}
}
/* virtual */ void FMTownsCommon::IOWriteWord(unsigned int ioport,unsigned int data)
{
	Device::IOWriteWord(ioport,data);
}
/* virtual */ void FMTownsCommon::IOWriteDword(unsigned int ioport,unsigned int data)
{
	Device::IOWriteDword(ioport,data);
}
/* virtual */ unsigned int FMTownsCommon::IOReadByte(unsigned int ioport)
{

	switch(ioport)
	{
	case TOWNSIO_POWER_CONTROL: // 0x0022
		// Since Towns HG, bit 8 is CRT power OFF.  Tsugaru is not taking it into account
		// at this time.
		// FM Towns Technical Databook pp.806 tells bit 0 to bit 6 will be undefined when read.
		// However, Windows 95 reads from 0x0022 and OR with 0x80 to turn off monitor.
		// Therefore, I assume bit 0 to 6 should return 0 when read.
		// Tsugaru is not taking CRT Power Off into account, therefore it returns zero.
		return 0;
	case TOWNSIO_RESET_REASON://      0x20,
		{
			auto ret=state.resetReason;
			state.resetReason&=(~3);
			return ret;
		}
		break;
	case TOWNSIO_CPU_MISC3: // 0x24
		if(TOWNSTYPE_2_UG<=townsType)
		{
			return 0x07;
		}
		return 0xFF;
	case TOWNSIO_MACHINE_ID_LOW://         0x30
		return MachineID()&0xFF;
	case TOWNSIO_MACHINE_ID_HIGH://=        0x31,
		return (MachineID()>>8)&0xFF;
	case TOWNSIO_FREERUN_TIMER_LOW ://0x26,
		return ( (state.townsTime/1000)<<var.freeRunTimerShift)&0xff;
	case TOWNSIO_FREERUN_TIMER_HIGH://0x28,
		return (((state.townsTime/1000)<<var.freeRunTimerShift)>>8)&0xff;


	case TOWNSIO_SERIAL_ROM_CTRL://=        0x32,
		{
			unsigned int data=(state.lastSerialROMCommand&0xC0);
			unsigned int index=TownsPhysicalMemory::SERIAL_ROM_LENGTH-1-(state.serialROMBitCount>>3);
			unsigned int bit=(1<<(state.serialROMBitCount&7));
			if(0!=(physMem.serialROM[index]&bit))
			{
				data|=1;
			}
			return data;
		}
		break;

	case TOWNSIO_MAINRAM_WAIT_1STGEN: //     0x5E0,
		return state.mainRAMWait;
		break;

	case TOWNSIO_MAINRAM_WAIT: //            0x5E2,
		return state.mainRAMWait;
		break;

	case TOWNSIO_VRAMWAIT: //                0x5E6,
		return state.VRAMWait;
		break;

	case TOWNSIO_FASTMODE:
		if(TOWNSTYPE_2_CX<=townsType)
		{
			if(true==FASTModeLamp())
			{
				return 0x01;
			}
			else
			{
				return 0x00;
			}
		}
		else
		{
			return 0xFF;
		}
		break;


	case TOWNSIO_FMR_RESOLUTION: // 0x400
		// Bit0 should always be 0.
		// In FM-R, Bit3 is labeled as "160P".  Meaning unknown.
		return 0xFE;

	case TOWNSIO_TIMER_1US_WAIT:
		// Supposed to be 1us wait when written.  But, mouse BIOS is often reading from this register.
		state.townsTime+=1000;
		break;

	case TOWNSIO_ELEVOL_1_DATA: //           0x4E0, // [2] pp.18, pp.174
		return state.eleVol[0][state.eleVolChLatch[0]].vol;
	case TOWNSIO_ELEVOL_1_COM: //            0x4E1, // [2] pp.18, pp.174
		{
			unsigned int data=0;
			data|=state.eleVolChLatch[0];
			data|=(state.eleVol[0][state.eleVolChLatch[0]].EN ? 4 : 0);
			data|=(state.eleVol[0][state.eleVolChLatch[0]].C0 ? 8 : 0);
			data|=(state.eleVol[0][state.eleVolChLatch[0]].C32 ? 16 : 0);
			return data;
		}
		break;
	case TOWNSIO_ELEVOL_2_DATA: //           0x4E2, // [2] pp.18, pp.174
		return state.eleVol[1][state.eleVolChLatch[1]].vol;
		break;
	case TOWNSIO_ELEVOL_2_COM: //            0x4E3, // [2] pp.18, pp.174
		{
			unsigned int data=0;
			data|=state.eleVolChLatch[1];
			data|=(state.eleVol[1][state.eleVolChLatch[1]].EN ? 4 : 0);
			data|=(state.eleVol[1][state.eleVolChLatch[1]].C0 ? 8 : 0);
			data|=(state.eleVol[1][state.eleVolChLatch[1]].C32 ? 16 : 0);
			return data;
		}
		break;
	}
	return 0xff;
}
/* virtual */ unsigned int FMTownsCommon::IOReadWord(unsigned int ioport)
{
	switch(ioport)
	{
	case TOWNSIO_FREERUN_TIMER:// 0x26
		return ((state.townsTime/1000)<<var.freeRunTimerShift)&0xffff;
	}
	return Device::IOReadWord(ioport);
}
/* virtual */ unsigned int FMTownsCommon::IOReadDword(unsigned int ioport)
{
	return Device::IOReadWord(ioport);
}

void FMTownsCommon::UpdateEleVol(int eleVol)
{
	if(TOWNS_ELEVOL_FOR_CD==eleVol &&
	  (TOWNS_ELEVOL_CD_LEFT==state.eleVolChLatch[TOWNS_ELEVOL_FOR_CD] || TOWNS_ELEVOL_CD_RIGHT==state.eleVolChLatch[TOWNS_ELEVOL_FOR_CD]))
	{
		cdrom.var.CDEleVolUpdate=true;
	}
}

/* virtual */ void FMTownsCommon::RunScheduledTask(unsigned long long int townsTime)
{
	CPU().Reset();
}
void FMTownsCommon::AdjustMachineSpeedForMemoryWait(void)
{
	if(true==FASTModeLamp())
	{
		state.currentFreq=state.fastModeFreq;
		sprite.state.transferTime=TownsSprite::SPRITE_ONE_TRANSFER_TIME_FASTMODE;
	}
	else
	{
		state.currentFreq=var.slowModeFreq;
		sprite.state.transferTime=TownsSprite::SPRITE_ONE_TRANSFER_TIME;
	}
}
