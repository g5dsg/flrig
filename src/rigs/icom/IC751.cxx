// ----------------------------------------------------------------------------
// Copyright (C) 2014
//              David Freese, W1HKJ
//
// This file is part of flrig.
//
// flrig is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// flrig is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// aunsigned long int with this program.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#include "icom/IC751.h"

//=============================================================================
// IC-751
//
const char IC751name_[] = "IC-751";

static std::vector<std::string>IC751modes_;
static const char *vIC751modes_[] =
{ "LSB", "USB", "AM", "CW", "RTTY", "FM"};
const char IC751_mode_type[] = { 'L', 'U', 'U', 'L', 'L', 'U' };

static std::vector<std::string>IC751_widths;
static const char *vIC751_widths[] =
{ "NARR", "WIDE"};
static int IC751_bw_vals[] = {1,2, WVALS_LIMIT};

RIG_IC751::RIG_IC751() {
	name_ = IC751name_;
	modes_ = IC751modes_;
	_mode_type = IC751_mode_type;
	bandwidths_ = IC751_widths;
	bw_vals_ = IC751_bw_vals;

	serial_baudrate = BR9600;
	stopbits = 2;
	serial_retries = 2;
//	serial_write_delay = 0;
//	serial_post_write_delay = 0;
	serial_timeout = 50;
	serial_echo = true;
	serial_rtscts = false;
	serial_rtsplus = true;
	serial_dtrplus = true;
	serial_catptt = true;
	serial_rtsptt = false;
	serial_dtrptt = false;
	modeA = 1;
//	bwA = 0;

    has_mode_control = true;
    has_smeter = true;
    has_power_out = true;
    has_ptt_control = true;

	defaultCIV = 0x1C;
	adjustCIV(defaultCIV);

	precision = 10;
	ndigits = 7;

};

//=============================================================================

void RIG_IC751::initialize()
{
	VECTOR (IC751modes_, vIC751modes_);
	VECTOR (IC751_widths, vIC751_widths);

	modes_ = IC751modes_;
	_mode_type = IC751_mode_type;
	bandwidths_ = IC751_widths;
	bw_vals_ = IC751_bw_vals;
}

bool RIG_IC751::check ()
{
	std::string resp = pre_fm;
	resp += '\x03';
	cmd = pre_to;
	cmd += '\x03';
	cmd.append( post );
	bool ok = waitFOR(10, "check vfo");
	rig_trace(2, "check()", str2hex(replystr.c_str(), replystr.length()));
	return ok;
}

unsigned long long RIG_IC751::get_vfoA ()
{
	std::string cstr = "\x03";
	std::string resp = pre_fm;
	resp.append(cstr);
	cmd = pre_to;
	cmd.append(cstr);
	cmd.append( post );
	if (waitFOR(10, "get vfo A")) {
		size_t p = replystr.rfind(resp);
		if (p != std::string::npos) {
			if (replystr[p+5] == -1)
				A.freq = 0;
			else
				freqA = fm_bcd_be(replystr.substr(p+5), 8);
		}
	}
	return freqA;
}

void RIG_IC751::set_vfoA (unsigned long long freq)
{
	freqA = freq;
	cmd = pre_to;
	cmd += '\x05';
	cmd.append( to_bcd_be( freq, 8 ) );
	cmd.append( post );
	waitFB("set vfo A");
}
int RIG_IC751::get_smeter()
{
	cmd = pre_to;
	cmd.append("\x15\x02").append(post);
	std::string resp = pre_fm;
	resp.append("\x15\x02");
	if (waitFOR(9, "get smeter")) {
		size_t p = replystr.rfind(resp);
		if (p != std::string::npos)
			return (int)ceil(fm_bcd(replystr.substr(p+6), 3) * 100 / 255);
	}
	return -1;
}

// Tranceiver PTT on/off
void RIG_IC751::set_PTT_control(int val)
{
	cmd.assign(pre_to);
	cmd += '\x1c';
	cmd += '\x00';
	cmd += (val ? '\x01' : '\x00');
	cmd.append( post );
	waitFB("set PTT");
}

void RIG_IC751::set_modeA(int val)
{
	modeA = val;
	cmd = pre_to;
	cmd += "\x06";
	cmd += modeA;		   // set the mode byte
	cmd.append( post );
	waitFB("set mode");
}

int RIG_IC751::get_modeA()
{
	cmd = pre_to;
	cmd += '\x04';
	cmd.append(post);
	std::string resp = pre_fm;
	resp += '\x04';
	if (waitFOR(7, "get mode")) {
		size_t p = replystr.rfind(resp);
		if (p != std::string::npos) {
			if (replystr[p+5] == -1) { modeA = 0; }
			else {
				modeA = replystr[p+5];
			}
		}
	}
	return modeA;
}
