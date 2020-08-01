/*C'est un peu osbolète attention*/
// voir http://www.roboard.com/Files/Reg/Serial_Port_Registers.pdf

#include <neantis/tty.h>
#include <neantis/sched.h>
#include <asm/system.h>
#include <asm/io.h>

#define WAKEUP_CHARS (TTY_BUF_SIZE/4)

extern void rs1_interrupt(void);
extern void rs2_interrupt(void);

static void init(int port) {
	outb(0x80, port + 3); // active DLAB
	outb(0x30, port); //vitesse de transmission = 48
	outb(0x00, port + 1); // MS par défaut
	outb(0x03, port + 3); // reset controle de ligne

	// inte = active l'interuption de sortie
	// rts = request to send
	// dtr = data terminal ready
	//active inte rts et dtr
	outb(0x0b, port + 4);
	outb(0x0d, port + 1); // active toutes les interupt
	(void)inb(port); // doit reset (?) d'après la spec
}

void rs_init(void) {
	set_intr_gate(0x24, rs1_interrupt);
	set_intr_gate(0x23, rs2_interrupt);
	init(tty_table[1].read_q.data);
	init(tty_table[2].read_q.data);
	outb(inb_p(0x21) & 0xE7, 0x21);
}