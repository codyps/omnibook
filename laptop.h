/*
 * laptop.h -- Various structures about supported hardware
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
 *
 * Written by Soós Péter <sp@osb.hu>, 2002-2004
 * Written by Mathieu Bérard <mathieu.berard@crans.org>, 2006
 */


#define HP_SIGNATURE	"Hewlett-Packard"

struct omnibook_models_t {
	/* DMI field matchers (table inputs) */
	char *sys_vendor;
	char *product_name;
	char *product_version;
	char *board_name;
	/* Table outputs */
	char *syslog_name;	/* Name which will appear in the syslog */
	int ectype;	/* Type of the embedded controller firmware, see omnibook.h and README */
};

static struct omnibook_models_t omnibook_models[] __initdata = {
  /* sys_vendor product_name                 product_version                   board_name syslog_name                  ectype */
  { NULL,       "HP OmniBook PC*",           "HP OmniBook XE3 GF*",            NULL,    NULL,                          XE3GF },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook XT1000*",            NULL,    NULL,                          XE3GF },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook XE2 DC*",            NULL,    NULL,                          XE2 },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook XE3 GC*",            NULL,    NULL,                          XE3GC },
  /* HP Pavilion N5430 */
  { NULL,       "HP OmniBook PC*",           "HP OmniBook XE3 GD*",            NULL,    NULL,                          XE3GC },
  /* HP Pavilion N5415 */
  { NULL,       "HP OmniBook PC*",           "HP OmniBook XE3 GE*",            NULL,    NULL,                          XE3GC },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook 500 FA*",            NULL,    NULL,                          OB500 },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook 510 FB*",            NULL,    NULL,                          OB510 },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook 4150*",              NULL,    NULL,                          OB4150 },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook 900 B*",             NULL,    NULL,                          OB4150 },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook 6000 EA*",           NULL,    NULL,                          OB6000 },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook 6100 EB*",           NULL,    NULL,                          OB6100 },
  /* HP OmniBook xe4100 */
  { NULL,       "HP OmniBook PC*",           "HP OmniBook xe4000*",            NULL,    NULL,                          XE4500 },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook xe4400*",            NULL,    NULL,                          XE4500 },
  { NULL,       "HP OmniBook PC*",           "HP OmniBook xe4500*",            NULL,    NULL,                          XE4500 },
  /* HP OmniBook vt6200 and xt6200 */
  { NULL,       "HP OmniBook PC*",           "HP OmniBook 6200 EG*",           NULL,    NULL,                          XE4500 },
  /* There are no model specific strings of some HP OmniBook XT1500 */
  { NULL,       "HP OmniBook PC*",           "HP OmniBook*",                   NULL,    NULL,                          XE3GF },
  /* HP Pavilion ze4125 */
  { NULL,       "HP NoteBook PC*",           "HP NoteBook ze4000*",            NULL,    NULL,                          XE4500 },
  /* There are no model specific strings of some HP Pavilion xt155 and some HP Pavilion ze4100 */
  { NULL,       "HP NoteBook PC*",           "HP NoteBook PC*",                NULL,    NULL,                          XE4500 },
  /* There are no model specific strings of some HP nx9000 */
  { NULL,       "HP Notebook PC*",           "HP Notebook PC*",                NULL,    NULL,                          XE4500 },
  /* HP Pavilion ZU1155 and ZU1175 */
  { NULL,       "HP Pavilion Notebook PC*",  "HP Pavilion ZU1000 FA*",         NULL,    NULL,                          OB500 },
  /* HP Pavilion N5290 */
  { NULL,       "HP Pavilion Notebook PC*",  "HP Pavilion Notebook XE3 GC*",   NULL,    NULL,                          XE3GC },
  /* HP Pavilion N5441 */
  { NULL,       "HP Pavilion Notebook PC*",  "HP Pavilion Notebook Model GD*", NULL,    NULL,                          XE3GC },
  /* HP Pavilion XH545 */
  { NULL,       "HP Pavilion Notebook PC*",  "HP Pavilion Notebook Model GE*", NULL,    NULL,                          XE3GC },
  /* HP Pavilion ZT1141 */
  { NULL,       "HP Pavilion Notebook PC*",  "HP Pavilion Notebook ZT1000*",   NULL,    NULL,                          XE3GF },
  /* There are no model specific strings of some HP Pavilion ZT1175 and ZT1195 notebooks */
  { NULL,       "HP Pavilion Notebook PC*",  "HP Pavilion Notebook*",          NULL,    NULL,                          XE3GF },
  { NULL,       "Pavilion ze4200*",          NULL,                             NULL,    "HP Pavilion ze4200 series",   XE4500 },
  { NULL,       "Pavilion ze4300*",          NULL,                             NULL,    "HP Pavilion ze4300 series",   XE4500 },
  { NULL,       "Pavilion ze8500*",          NULL,                             NULL,    "HP Pavilion ze8500 series",   XE4500 },
  /* Compaq nx9000 */
  { NULL,       "HP nx9000*",                NULL,                             NULL,    "HP Compaq nx9000",            XE4500 },
  /* Compaq nx9005 */
  { NULL,       "HP nx9005*",                NULL,                             NULL,    "HP Compaq nx9005",            XE4500 },
  /* Compaq nx9010 */
  { NULL,       "HP nx9010*",                NULL,                             NULL,    "HP Compaq nx9010",            XE4500 },
  { "TOSHIBA",  "S1000*",                    NULL,                             NULL,    "Toshiba Satellite 1000",      XE3GF },
  { "TOSHIBA",  "S1005*",                    NULL,                             NULL,    "Toshiba Satellite 1005",      XE3GF },
  { "TOSHIBA",  "S1110*",                    NULL,                             NULL,    "Toshiba Satellite 1110",      XE3GF },
  { "TOSHIBA",  "S1115*",                    NULL,                             NULL,    "Toshiba Satellite 1115",      XE3GF },
  { "TOSHIBA",  "S1900*",                    NULL,                             NULL,    "Toshiba Satellite 1900",      XE3GF },
  { "TOSHIBA",  "S1905*",                    NULL,                             NULL,    "Toshiba Satellite 1905",      XE3GF },
  { "TOSHIBA",  "S1950*",                    NULL,                             NULL,    "Toshiba Satellite 1950",      XE3GF },
  { "TOSHIBA",  "S1955*",                    NULL,                             NULL,    "Toshiba Satellite 1955",      XE3GF },
  { "TOSHIBA",  "S2430*",                    NULL,                             NULL,    "Toshiba Satellite 2430",      XE3GF },
  { "TOSHIBA",  "S2435*",                    NULL,                             NULL,    "Toshiba Satellite 2435",      XE3GF },
  { "TOSHIBA",  "S3000*",                    NULL,                             NULL,    "Toshiba Satellite 3000",      XE3GF },
  { "TOSHIBA",  "S3005*",                    NULL,                             NULL,    "Toshiba Satellite 3005",      XE3GF },
  { "TOSHIBA",  "Satellite 1000*",           NULL,                             NULL,    "Toshiba Satellite 1000",      XE3GF },
  { "TOSHIBA",  "Satellite 1005*",           NULL,                             NULL,    "Toshiba Satellite 1005",      XE3GF },
  { "TOSHIBA",  "Satellite 1110*",           NULL,                             NULL,    "Toshiba Satellite 1110",      XE3GF },
  { "TOSHIBA",  "Satellite 1115*",           NULL,                             NULL,    "Toshiba Satellite 1115",      XE3GF },
  { "TOSHIBA",  "Toshiba 1115*",             NULL,                             NULL,    "Toshiba Satellite 1115",      XE3GF },
  { "TOSHIBA",  "Satellite 1900*",           NULL,                             NULL,    "Toshiba Satellite 1900",      XE3GF },
  { "TOSHIBA",  "Satellite 1905*",           NULL,                             NULL,    "Toshiba Satellite 1905",      XE3GF },
  { "TOSHIBA",  "Satellite 1950*",           NULL,                             NULL,    "Toshiba Satellite 1950",      XE3GF },
  { "TOSHIBA",  "Satellite 1955*",           NULL,                             NULL,    "Toshiba Satellite 1955",      XE3GF },
  { "TOSHIBA",  "Satellite 2430*",           NULL,                             NULL,    "Toshiba Satellite 2430",      XE3GF },
  { "TOSHIBA",  "Satellite 2435*",           NULL,                             NULL,    "Toshiba Satellite 2435",      XE3GF },
  { "TOSHIBA",  "Satellite 3000*",           NULL,                             NULL,    "Toshiba Satellite 3000",      XE3GF },
  { "TOSHIBA",  "Satellite 3005*",           NULL,                             NULL,    "Toshiba Satellite 3005",      XE3GF },
  { "TOSHIBA",  "Satellite P10*",            NULL,                             NULL,    "Toshiba Satellite P10",       TSP10 },
  { "TOSHIBA",  "Satellite P15*",            NULL,                             NULL,    "Toshiba Satellite P15",       TSP10 },
  { "TOSHIBA",  "Satellite P20*",            NULL,                             NULL,    "Toshiba Satellite P20",       TSP10 },
  { "TOSHIBA",  "Satellite M30X*",           NULL,                             NULL,    "Toshiba Satellite M30X",      TSM30X },
  { "TOSHIBA",  "Satellite M35X*",           NULL,                             NULL,    "Toshiba Satellite M35X",      TSM30X },
  { "TOSHIBA",  "Satellite M70*",            NULL,                             NULL,    "Toshiba Satellite M70",       TSM30X },
  { "TOSHIBA",  "Satellite M40*",            NULL,                             NULL,    "Toshiba Satellite M40",       TSM40 },
  { "COMPAL",   NULL,                        NULL,                             "ACL00", "Compal ACL00",                XE3GF },
  { "COMPAL",   NULL,                        NULL,                             "ACL10", "Compal ACL10",                XE3GF },
  { "Acer",     "Aspire 1400 series*",       NULL,                             NULL,    "Acer Aspire 1400 series",     XE3GF },
  { "Acer",     "Aspire 1350*",              NULL,                             NULL,    "Acer Aspire 1350",            XE4500 },
  { "FUJITSU SIEMENS", "Amilo D-Series*",    NULL,                             NULL,    "Fujitsu-Siemens Amilo D series", AMILOD },
  /* This sentinel at the end catches all unsupported models */
  { NULL, NULL, NULL, NULL, NULL, NONE }
};

struct omnibook_tc_t {
	char *tc;
	int ectype;
};

/* HP technology codes */
static struct omnibook_tc_t omnibook_tc[] __initdata = {
	/* technology code      ectype */
	{"CI.", OB4150},
	{"CL.", OB4150},
	{"DC.", XE2},
	{"EA.", OB6000},
	{"EB.", OB6100},
	{"EG.", XE4500},
	{"FA.", OB500},
	{"FB.", OB510},
	{"GC.", XE3GC},
	{"GD.", XE3GC},
	{"GE.", XE3GC},
	{"GF.", XE3GF},
	{"IB.", XE3GF},
	{"IC.", XE3GF},
	{"ID.", XE3GF},
	{"KA.", XE4500},
	{"KB.", XE4500},
	{"KC.", XE4500},
	{"KD.", XE4500},
	{"KE.", XE4500},
	{"KE_KG.", XE4500},
	{"KF_KH.", XE4500},
	{NULL, NONE}
};
