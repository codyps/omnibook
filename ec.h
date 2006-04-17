/*
 * ec.h -- low level definitions to access Embedded Controller
 *         and Keyboard Controller and system I/O ports or memory
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
 */

extern int omnibook_ec_read(u8 addr, u8 *data);
extern int omnibook_ec_write(u8 addr, u8 data);
extern int omnibook_kbc_command(u8 cmd, u8 data);
extern int omnibook_io_read(u32 addr, u8 * data);
extern int omnibook_io_write(u32 addr, u8 data);
extern int omnibook_mem_read(u32 addr, u8 * data);
extern int omnibook_mem_write(u32 addr, u8 data);


/*
 *	Embedded controller adresses
 */

#define XE3GF_CHGM				0x90	/* , 16 bit */
#define XE3GF_CHGS				0x92	/* , 16 bit */
#define XE3GF_CHGC				0x94	/* Current charge of board, 16 bit */
#define XE3GF_CHGV				0x96	/* Current voltage, 16 bit */
#define XE3GF_CHGA				0x98	/* Current intensity, 16 bit */
#define XE3GF_BAL				0x9A	/* Battery present status */
#define XE3GF_STA1				0x9C	/* Various status bits */
#define XE3GF_CSPR				0xA1	/* Port replicator status, 1 bit */
#define XE3GF_ADP				0xA3	/* AC acapter status, 1 bit */
#define XE3GF_FOT				0xA5	/* Fan off temperature, 8 bit */
#define XE3GF_FSD1				0xA6	/* Fan on temperature, 8 bit */
#define XE3GF_FSD2				0xA7	/* Fan level 2 temperature, 8 bit */
#define XE3GF_FSD3				0xA8	/* Fan level 3 temperature, 8 bit */
#define XE3GF_FSD4				0xA9	/* Fan level 4 temperature, 8 bit */
#define XE3GF_FSD5				0xAA	/* Fan level 5 temperature, 8 bit */
#define XE3GF_FSD6				0xAB	/* Fan level 6 temperature, 8 bit */
#define XE3GF_FSD7				0xAC	/* Fan level 7 temperature, 8 bit */
#define XE3GF_FSRD				0xAD	/* Fan status, 8 bit */
#define XE3GF_CTMP				0xB0	/* CPU tempetature, 8 bit */
#define XE3GF_BRTS				0xB9	/* LCD brightness, 4 bit */
#define XE3GF_BTY0				0xC0	/* Battery 0 type, 1 bit */
#define XE3GF_BST0				0xC1	/* Battery 0 status, 3 bit */
#define XE3GF_BRC0				0xC2	/* Battery 0 remaining capacity, 16 bit */
#define XE3GF_BSN0				0xC4	/* Battery 0 serial number 16 bit */
#define XE3GF_BPV0				0xC6	/* Battery 0 present voltage, 16 bit */
#define XE3GF_BDV0				0xC8	/* Battery 0 design voltage 16 bit */
#define XE3GF_BDC0				0xCA	/* Battery 0 design capacity 16 bit */
#define XE3GF_BFC0				0xCC	/* Battery 0 last full capacity 16 bit */
#define XE3GF_GAU0				0xCE	/* Battery 0 gauge, 8 bit */
#define XE3GF_BTY1				0xD0	/* Battery 1 type, 1 bit */
#define XE3GF_BST1				0xD1	/* Battery 1 status, 3 bit */
#define XE3GF_BRC1				0xD2	/* Battery 1 remaining capacity, 16 bit */
#define XE3GF_BSN1				0xD4	/* Battery 1 serial number, 16 bit */
#define XE3GF_BPV1				0xD6	/* Battery 1 present voltage, 16 bit */
#define XE3GF_BDV1				0xD8	/* Battery 1 design voltage 16 bit */
#define XE3GF_BDC1				0xDA	/* Battery 1 design capacity 16 bit */
#define XE3GF_BFC1				0xDC	/* Battery 1 last full capacity 16 bit */
#define XE3GF_GAU1				0xDE	/* Battery 1 gauge, 8 bit */

/*
 * Bitmasks for sub byte values
 */

#define XE3GF_SHDD_MASK				0x40	/* External display status */
#define XE3GF_CSPR_MASK				0x01	/* Port replicator status */
#define XE3GF_ADP_MASK				0x20	/* AC acapter status */
#define XE3GF_BAL0_MASK				0x01	/* Battery 0 present */
#define XE3GF_BAL1_MASK				0x02	/* Battery 1 present */
#define XE3GF_BMF_MASK				0x70	/* Model code */
#define XE3GF_BTY_MASK				0x80	/* Type: Ni-MH or Li-Ion */
#define XE3GF_BST_MASK_DSC			0x01	/* Discarging */
#define XE3GF_BST_MASK_CHR			0x02	/* Charging */
#define XE3GF_BST_MASK_CRT			0x04	/* Critical */
#define XE3GF_FSRD_MASK_S1			0x01	/* Fan level 1 */
#define XE3GF_FSRD_MASK_S2			0x02	/* Fan level 2 */
#define XE3GF_FSRD_MASK_S3			0x04	/* Fan level 3 */
#define XE3GF_FSRD_MASK_S4			0x08	/* Fan level 4 */
#define XE3GF_FSRD_MASK_S5			0x10	/* Fan level 5 */
#define XE3GF_FSRD_MASK_S6			0x20	/* Fan level 6 */
#define XE3GF_FSRD_MASK_S7			0x40	/* Fan level 7 */
#define XE3GF_BRTS_MASK				0x0F	/* LCD brightness */

#define XE3GF_FAN_ON_MASK			0x02	/* Fan on */

/*
 * OmniBook XE3 GC values
 */

#define XE3GC_CTMP				0x28	/* CPU tempetature, 8 bit */
#define XE3GC_STA1				0x30	/* Various status bits */
#define XE3GC_Q0A				0x31	/* Various status bits */
#define XE3GC_CCUR				0x38	/* Current charge of board, 16 bit ? */
#define XE3GC_CVOL				0x3A	/* Current voltage, 16 bit ? */
#define XE3GC_CARM				0x3C	/* Current intensity, 16 bit ? */
#define XE3GC_BAT				0x3E	/* Battery present status */
#define XE3GC_BST0				0x40	/* Battery 0 status, 3 bit */
#define XE3GC_BPR0				0x41	/* Battery 0 present rate, 16 bit ? */
#define XE3GC_BRC0				0x43	/* Battery 0 remaining capacity, 16 bit */
#define XE3GC_BPV0				0x45	/* Battery 0 present voltage, 16 bit */
#define XE3GC_BDV0				0x47	/* Battery 0 design voltage 16 bit */
#define XE3GC_BDC0				0x49	/* Battery 0 design capacity 16 bit */
#define XE3GC_BTY0				0x4A	/* Battery 0 type, 1 bit ? */
#define XE3GC_BTP0				0x4B	/* Battery 0 ?, 1 bit */
#define XE3GC_BSN0				0x4C	/* Battery 0 serial number, 8 bit ? */
#define XE3GC_BMF0				0x4D	/* Battery 0 ?,8 bit */
#define XE3GC_BST1				0x50	/* Battery 1 status, 3 bit */
#define XE3GC_BPR1				0x51	/* Battery 1 present rate, 16 bit ? */
#define XE3GC_BRC1				0x53	/* Battery 1 remaining capacity, 16 bit */
#define XE3GC_BPV1				0x55	/* Battery 1 present voltage, 16 bit */
#define XE3GC_BDV1				0x57	/* Battery 1 design voltage 16 bit */
#define XE3GC_BDC1				0x59	/* Battery 1 design capacity 16 bit */
#define XE3GC_BTY1				0x5A	/* Battery 1 type, 1 bit ? */
#define XE3GC_BTP1				0x5B	/* Battery 1 ?, 1 bit */
#define XE3GC_BSN1				0x5C	/* Battery 1 serial number, 8 bit ? */
#define XE3GC_BMF1				0x5D	/* Battery 1 ?,8 bit */
#define XE3GC_STA2				0x61	/* Various status bits */
#define XE3GC_BTVL				0x6A	/* LCD brightness, 4 bit */

/*
 * Bitmasks for sub byte values
 */

#define XE3GC_ADP_MASK				0x40	/* AC acapter status */
#define XE3GC_BAT0_MASK				0x01	/* Battery 0 present */
#define XE3GC_BAT1_MASK				0x02	/* Battery 1 present */
#define XE3GC_BTY_MASK				0x01	/* Type: Ni-MH or Li-Ion */
#define XE3GC_BST_MASK_DSC			0x01	/* Discarging */
#define XE3GC_BST_MASK_CHR			0x02	/* Charging */
#define XE3GC_BST_MASK_CRT			0x04	/* Critical */
#define XE3GC_CRTI_MASK				0x04	/* External display status */
#define XE3GC_SLPB_MASK				0x01	/* Sleep button pressed */
#define XE3GC_F5_MASK				0x02	/* Fn-F5 - LCD/CRT switch pressed */
#define XE3GC_VOLD_MASK				0x04	/* Fn-down arrow or Volume down pressed */
#define XE3GC_VOLU_MASK				0x08	/* Fn-up arrow or Volume up pressed */
#define XE3GC_MUTE_MASK				0x10	/* Fn+F7 - Volume mute pressed */
#define XE3GC_CNTR_MASK				0x20	/* Fn+F3/Fn+F4 - Contrast up or down pressed */
#define XE3GC_BRGT_MASK				0x40	/* Fn+F1/Fn+F2 - Brightness up or down pressed */
#define XE3GC_BTVL_MASK				0x0F	/* LCD brightness */

/*
 * Emulated scancodes
 */

#define XE3GC_VOLD_SCAN				0x2E	/* Volume down button scancode */
#define XE3GC_VOLU_SCAN				0x30	/* Volume up button scancode */
#define XE3GC_MUTE_SCAN				0x20	/* Volume up button scancode */

/*
 * Fujitsu Amilo D values
 */

#define AMILOD_TMP				0x28	/* CPU tempetature, 8 bit */
#define AMILOD_STA1				0x30	/* Various status bits */
#define AMILOD_BAT				0x3E	/* Battery present status */
#define AMILOD_BDC0				0x40	/* Battery 0 design capacity 16 bit */
#define AMILOD_BDV0				0x42	/* Battery 0 design voltage 16 bit */
#define AMILOD_BTY0				0x44	/* Battery 0 type, 1 bit ? */
#define AMILOD_BST0				0x45	/* Battery 0 status, 3 bit */
#define AMILOD_BPR0				0x46	/* Battery 0 present rate, 16 bit ? */
#define AMILOD_BRC0				0x48	/* Battery 0 remaining capacity, 16 bit */
#define AMILOD_BPV0				0x4A	/* Battery 0 present voltage, 16 bit */
#define AMILOD_BTP0				0x4C	/* Battery 0 ?, 1 bit */
#define AMILOD_BDC1				0x50	/* Battery 1 design capacity 16 bit */
#define AMILOD_BDV1				0x52	/* Battery 1 design voltage 16 bit */
#define AMILOD_BTY1				0x54	/* Battery 1 type, 1 bit ? */
#define AMILOD_BST1				0x55	/* Battery 1 status, 3 bit */
#define AMILOD_BPR1				0x56	/* Battery 1 present rate, 16 bit ? */
#define AMILOD_BRC1				0x58	/* Battery 1 remaining capacity, 16 bit */
#define AMILOD_BPV1				0x5A	/* Battery 1 present voltage, 16 bit */
#define AMILOD_BTP1				0x5C	/* Battery 1 ?, 1 bit */
#define AMILOD_CBRG				0x6F	/* LCD brightness, 4 bit */

/*
 * Bitmasks for sub byte values
 */

#define AMILOD_ADP_MASK				0x40	/* AC acapter status */
#define AMILOD_BAT0_MASK			0x01	/* Battery 0 present */
#define AMILOD_BAT1_MASK			0x02	/* Battery 1 present */
#define AMILOD_BTY_MASK				0x01	/* Type: Ni-MH or Li-Ion */
#define AMILOD_BST_MASK_DSC			0x01	/* Discarging */
#define AMILOD_BST_MASK_CHR			0x02	/* Charging */
#define AMILOD_BST_MASK_CRT			0x04	/* Critical */
#define AMILOD_CBRG_MASK			0x0F	/* LCD brightness */

/*
 * OmniBook 500, 510, 6000, 6100, XE2 values
 */

#define OB500_STA1				0x44	/* Various status bits */
#define OB500_STA2				0x50	/* Various status bits */
#define OB500_CTMP				0x55	/* CPU tempetature, 8 bit */
#define OB500_BT1I				0x58	/* Battery 1 ? 16 bit */
#define OB500_BT1C				0x5A	/* Battery 1 remaining capacity 16 bit ? */
#define OB500_BT1V				0x5C	/* Battery 1 present voltage 16 bit ? */
#define OB500_BT1S				0x5E	/* Battery 1 status 3 bit ? */
#define OB500_BT2I				0x6A	/* Battery 2 ? 16 bit */
#define OB500_BT2C				0x6C	/* Battery 2 remaining capacity 16 bit ? */
#define OB500_BT2V				0x6E	/* Battery 2 present voltage 16 bit ? */
#define OB500_BT2S				0x70	/* Battery 2 status 3 bit ? */
#define OB500_BT3I				0x5F	/* Battery 3 ? 16 bit */
#define OB500_BT3C				0x61	/* Battery 3 remaining capacity 16 bit ? */
#define OB500_BT3V				0x63	/* Battery 3 present voltage 16 bit ? */
#define OB500_BT3S				0x65	/* Battery 3 status 3 bit ? */

#define OB6000_STA1				0x77	/* Various status bits */

#define XE2_STA1				0x50	/* Various status bits */

/*
 * Bitmasks for sub byte values
 */

#define OB500_LIDS_MASK				0x01	/* LID status */
#define OB500_CRTS_MASK				0x20	/* External display status */
#define OB500_SLPS_MASK				0x40	/* Sleep button status */
#define OB500_DCKS_MASK				0x80	/* Docking status */
#define OB500_ADP_MASK				0x02	/* AC acapter status */
#define OB500_BST_MASK_DSC			0x01	/* Discarging */
#define OB500_BST_MASK_CHR			0x02	/* Charging */
#define OB500_BST_MASK_CRT			0x04	/* Critical */

#define OB6000_FAN_MASK				0x10	/* Fan status */

#define XE2_ADP_MASK				0x02	/* AC acapter status */

/*
 * OmniBook 4150
 */

#define OB4150_TMP				0x28	/* CPU tempetature, 8 bit */
#define OB4150_STA1				0x2E	/* Various status bits */
#define OB4150_STA2				0x2F	/* Various status bits */
#define OB4150_ADP				0x30	/* AC acapter status, 1 bit */
#define OB4150_DCID				0x2C	/* Port replicator */

/*
 * Bitmasks for sub byte values
 */

#define OB4150_FAN_MASK				0x01	/* Fan status */
#define OB4150_ADP_MASK				0x40	/* AC acapter status */
#define OB4150_CRST_MASK			0x20	/* External display status */

/*
 *	Keyboard controller command for some laptop functions
 */

#define OMNIBOOK_KBC_CONTROL_CMD		0x59

/*
 *	Keyboard controller command parameters for functions available via kbc
 */

#define OMNIBOOK_KBC_CMD_ONETOUCH_ENABLE	0x90	/* Enables OneTouch buttons */
#define OMNIBOOK_KBC_CMD_ONETOUCH_DISABLE	0x91	/* Disables OneTouch buttons */
#define OMNIBOOK_KBC_CMD_TOUCHPAD_ENABLE	0xAA	/* Enables touchpad */
#define OMNIBOOK_KBC_CMD_TOUCHPAD_DISABLE	0xA9	/* Disables touchpad */
#define OMNIBOOK_KBC_CMD_LCD_ON			0xE1	/* Turns LCD display on */
#define OMNIBOOK_KBC_CMD_LCD_OFF		0xE2	/* Turns LCD display off */
#define OMNIBOOK_KBC_CMD_AC_POWER_ENABLE	0xC2	/* Enable AC power */
#define OMNIBOOK_KBC_CMD_AC_POWER_DISABLE	0xC1	/* Disable AC power */

/*
 * Other I/O ports
 */

#define ACL00_AC_STAT				0x11B9	/* AC adapter status on ACL00 */
#define ACL00_AC_MASK				0x04	/* Bitmask for AC adapter status on ACL00 */
#define TOSH3K_AC_STAT				0x102D	/* AC adapter status on Toshiba 3000 */
#define TOSH3K_AC_MASK				0x08	/* Bitmask for AC adapter status on Toshiba 3000 */
#define XE3GF_AC_STAT				0x11B9	/* AC adapter status on XE3 GF */
#define XE3GF_AC_MASK				0x04	/* Bitmask for AC adapter status on XE3 GF */
#define XE3GF_LID_STAT				0x11AD	/* LID switch status on XE3 GF */
#define XE3GF_LID_MASK				0x20	/* Bitmask for LID switch status on XE3 GF */
#define XE3GC_SMIC				0xFE00

#define OB500_GPO1				0x8034	/* Fan control */
#define OB510_GPO2				0x11B9	/* LCD backlight */
#define OB510_GPIO				0x118F	/* Fan control */

#define OB500_FAN_ON_MASK			0x0A	/* Turn fan on with zero bits */
#define OB500_FAN_OFF_MASK			0x08	/* Fan status/off */
#define OB500_BKLT_MASK				0x40	/* LCD backlight */
#define OB510_FAN_ON_MASK			0x18	/* Turn fan on with zero bits */
#define OB510_FAN_OFF_MASK			0x10	/* Turn fan on */
#define OB510_BKLT_MASK				0x01	/* LCD backlight */

#define XE2_FAN_MASK				0x02	/* Turn fan on with zero bit */

/*
 * Memory adresses
 */

#define XE3GC_BCMD				0xFFFFEBC

/* End of file */
