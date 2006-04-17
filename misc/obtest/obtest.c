/*
 * Utility to test embedded controller and keyboard controller
 * can be found in HP OmniBooks and other laptops
 *
 * Original EC code comes from Ducrot Bruno <ducrot@poupinou.org>
 * Modified by Soós Péter <sp@osb.hu>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <errno.h>

#define EC_DATA		0x62
#define EC_SC		0x66
#define KBC_DATA	0x60
#define KBC_SC		0x64

/* Various bit mask for EC_SC (R) and KBC_SC*/
#define OBF		0x01
#define IBF		0x02

/* Commands for EC_SC (W) */
#define RD_EC		0x80
#define WR_EC		0x81

/* OmniBook command for KBC_SC (W) */
#define OB_KBC		0x59 

#define UDELAY		200
#define EC_UDELAY	100

#define my_cli() do {asm ("cli");} while (0)
#define my_sti() do {asm ("sti");} while (0)

typedef unsigned char u8;
typedef unsigned int u32;

static int ec_wait (u8 event)
{
	u8	status = 0;
	u32	i = UDELAY;

	switch (event) {
	case OBF:
		do {
			status = inb(EC_SC);
			if (status & OBF)
				return 0;
			usleep(EC_UDELAY);
		} while (--i>0);
		break;
	case IBF:
		do {
			status = inb(EC_SC);
			if (!(status & IBF))
				return 0;
			usleep(EC_UDELAY);
		} while (--i>0);
		break;
	default:
		return -EINVAL;
	}
	return -ETIME;
}

static int kbc_wait (u8 event)
{
	u8	status = 0;
	u32	i = UDELAY;

	switch (event) {
	case OBF:
		do {
			status = inb(KBC_SC);
			if (status & OBF)
				return 0;
			usleep(EC_UDELAY);
		} while (--i > 0);
		break;
	case IBF:
		do {
			status = inb(KBC_SC);
			if (!(status & IBF))
				return 0;
			usleep(EC_UDELAY);
		} while (--i > 0);
		break;
	default:
		return -EINVAL;
	}
	return -ETIME;
}

static int ec_read(u8 address, u8 *data)
{
	int result = 0;

	*data = 0;

	my_cli();

	result = ec_wait(IBF);
	if(result)
		return result;
	outb(RD_EC, EC_SC);

	result = ec_wait(IBF);
	if (result)
		return result;
	outb(address, EC_DATA);
	
	result = ec_wait(OBF);
	if (result)
		return result;
	*data = inb(EC_DATA);
	
	my_sti();

	return result;
}

static int ec_write(u8 address, u8 data)
{
	int result = 0;

	my_cli();

	result = ec_wait(IBF);
	if (result)
		return result;
	outb(WR_EC, EC_SC);

	result = ec_wait(IBF);
	if (result)
		return result;
	outb(address, EC_DATA);

	result = ec_wait(IBF);
	if (result)
		return result;
	outb(data, EC_DATA);

	my_sti();

	return result;
}

static int sysio_write(u32 address, u8 data)
{
	my_cli();
	
	outb(data, address);

	my_sti();

	return 0;
}

static int sysio_read(u32 address, u8 *data)
{
	my_cli();
	
	*data = inb(address);

	my_sti();

	return 0;
}

int kbc_command(u8 cmd, u8 data)
{
	int result = 0;

	my_cli();

	result = kbc_wait(IBF);
	if (result)
		return result;
	outb(OB_KBC, KBC_SC);

	result = kbc_wait(IBF);
	if (result)
		return result;
	outb(data, KBC_DATA);

	my_sti();

	return result;
}

char *itob(int a)
{
	static char buff[9];
	char *p;
	int i;

	if (a < 0 || a > 255)
		return NULL;

	for (p = buff, i = 0; i < 8; a <<= 1, i++)
		*p++ = (a & 0x80) ? '1' : '0';
	*p = 0;

	return buff;
}

long ntoi(const char *n)
{
	long a = 0;
	int i = 0;
	
	if (n[0] == '0') {
		if (n[1] == 'x' || n[1] == 'X') {
			i = 2;
			while (n[i]) {
				if (n[i] >= '0' && n[i] <= '9')
					a = 16 * a + (n[i] - '0');
				else if (n[i] >= 'a' && n[i] <= 'f')
					a = 16 * a + (n[i] - 'a' + 10);
				else if (n[i] >= 'A' && n[i] <= 'F')
					a = 16 * a + (n[i] - 'A' + 10);
				else
					return -EINVAL;
				i++;
			}
			if (i == 2)
				return -EINVAL;
		} else if (n[1] == 'b' || n[1] == 'B') {
			i = 2;
			while (n[i]) {
				if (n[i] == '0' || n[i] == '1')
					a = 2 * a + (n[i] - '0');
				else
					return -EINVAL;
				i++;
			}
			if (i == 2)
				return -EINVAL;
		} else if (n[1] >= '0' && n[1] <= '7') {
			i = 1;
			while (n[i]) {
				if (n[i] >= '0' && n[i] <= '7')
					a = 8 * a + (n[i] - '0');
				else
					return -EINVAL;
				i++;
			}
		}
	} else {
		while (n[i]) {
			if (n[i] >= '0' && n[i] <= '9')
				a = 10 * a + (n[i] - '0');
			else
				return -EINVAL;
			i++;
		}
	}
	return a;
}

void usage(char *argv)
{
	fprintf(stderr, "usage: `%s -r addr'       to read from the address of EC\n", argv);
	fprintf(stderr, "       `%s -c addr'       to read EC continuosly\n", argv);
	fprintf(stderr, "       `%s -w addr value' to write value to address of EC\n", argv);
	fprintf(stderr, "       `%s -k cmd'        to send OmniBook commad to KBC\n", argv);
	fprintf(stderr, "       `%s -i addr'       to read from an I/O address\n", argv);
	fprintf(stderr, "       `%s -s addr'       to read from an I/O address continuosly\n", argv);
	fprintf(stderr, "       `%s -x addr value' to write value to an I/O address\n", argv);
	exit(1);
}

int main(int argc, char *argv[])
{
	u8 data;
	u32 a;
	u8 cmd;
	int result;
	const char *p;
	const char *q;
	const char *sw;

	if ((argc < 3) || (argc > 4)) {
		usage(argv[0]);
	}

	sw = argv[1];
	if (sw[0] != '-')
		usage(argv[0]);

	switch (sw[1]) {
		case 'r':
		case 'c':
		case 'k':
		case 'i':
		case 's':
			if (argc == 3)	
				p = argv[2];
			else
				usage(argv[0]);
			break;
		case 'w':
		case 'x':
			if (argc == 4) {
				p = argv[2];
				q = argv[3];
			} else
				usage(argv[0]);
			break;
		default:
			usage(argv[0]);
	}

	if (iopl(3)) {
		perror("iopl");
		exit (1);
	}

	a = ntoi(p);
	data = ntoi(q);

	switch (sw[1]) {
		case 'w':
			result = ec_write(a,data);
			if (result)
				return result;
			printf("Writing to EC at address 0x%.2x (%.3d) :  0x%02x %s (%d)\n", a, a, data, itob(data), data);
			result = ec_read(a, &data);
			if (result)
				return result;
			printf("Reading EC from address 0x%.2x (%.3d) :  0x%02x %s (%d)\n", a, a, data, itob(data), data);
			usleep(1000000);
			result = ec_read(a, &data);
			if (result)
				return result;
			printf("Reading EC from address 0x%.2x (%.3d) :  0x%02x %s (%d)\n", a, a, data, itob(data), data);
			break;
		case 'x':
			result = sysio_write(a,data);
			if (result)
				return result;
			printf("Writing to address 0x%.2x (%.3d) :  0x%02x %s (%d)\n", a, a, data, itob(data), data);
			result = sysio_read(a, &data);
			if (result)
				return result;
			printf("Reading from address 0x%.2x (%.3d) :  0x%02x %s (%d)\n", a, a, data, itob(data), data);
			usleep(1000000);
			result = sysio_read(a, &data);
			if (result)
				return result;
			printf("Reading from address 0x%.2x (%.3d) :  0x%02x %s (%d)\n", a, a, data, itob(data), data);
			break;
		case 'r':
			result = ec_read(a, &data);
			if (result)
				return result;
			printf("Reading EC from address 0x%.2x (%.3d) :  0x%02x %s (%d)\n", a, a, data, itob(data), data);
			break;
		case 'c':
			do {
				result = ec_read(a, &data);
				printf("Reading EC from address 0x%.2x (%.3d) :  0x%02x %s (%d)\n", a, a, data, itob(data), data);
			} while (1);
			break;
		case 'i':
			result = sysio_read(a, &data);
			if (result)
				return result;
			printf("Reading from address 0x%.4x (%.5d) :  0x%02x %s (%d)\n", a, a, data, itob(data), data);
			break;
		case 's':
			do {
				result = sysio_read(a, &data);
				printf("Reading from address 0x%.4x (%.5d) :  0x%02x %s (%d)\n", a, a, data, itob(data), data);
				usleep(50000);
			} while (1);
			break;
		case 'k':
			cmd = OB_KBC;
			result = kbc_command(cmd, a);
			if (result)
				return result;
			printf("Writing to KBC 0x%.2x command with 0x%.2x parameter.\n", cmd, a);
			break;
	}
	
	return result;
}

/* End of file */
