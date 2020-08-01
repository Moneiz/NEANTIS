#include <ctype.h>
#include <errno.h>
#include <signal.h>

#include <neantis/sched.h>
#include <neantis/tty.h>
#include <asm/segment.h>
#include <asm/system.h>

struct tty_struct tty_table[] = {
	{
		ICRNL,
		OPOST | ONLCR,
		0,
		ISIG | ICANON | ECHO | ECHOCTL | ECHOKE,
		0,
		INIT_C_CC,
		0,
		0,
		con_write,
		{0,0,0,0,""}, // read queue
		{0,0,0,0,""}, // write queue
		{0,0,0,0,""} // secondary queue
	},{
		0,
		0,
		B2400 | CS8,
		0,
		0,
		INIT_C_CC,
		0,
		0,
		rs_write,
		{0x3f8,0,0,0,""},
		{0x3f8,0,0,0,""},
		{0,0,0,0,""},
	},{
		0,
		0,
		B2400 | CS8,
		0,
		0,
		INIT_C_CC,
		0,
		0,
		rs_write,
		{0x2f8,0,0,0,""},
		{0x2f8,0,0,0,""},
		{0,0,0,0,""},
	}
};

void tty_init(void) {
	rs_init();
}

void chr_dev_init(void) {

}