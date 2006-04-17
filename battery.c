/*
 * battery.c -- battery related functions
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

#ifdef OMNIBOOK_STANDALONE
#include "omnibook.h"
#else
#include <linux/omnibook.h>
#endif

#include "ec.h"

static int omnibook_ec_read16(u8 addr, u16 * data)
{
	int retval;
	u8 high;
	u8 low;
	u16 result;

	retval = omnibook_ec_read(addr, &low);
	if (retval)
		return retval;
	retval = omnibook_ec_read(addr + 0x01, &high);
	result = ((high << 8) + low);
	*data = result;
	return retval;
}

static int omnibook_battery_present(int num)
{
	int retval;
	int i;
	u8 bat;
	u8 mask = 0;
	
	/*
	 * XE3GF
	 * TSP10
	 * TSM30X
	 */
	if (omnibook_ectype & (XE3GF|TSP10|TSM30X) ) {

		if (num >= 2)
			return -EINVAL;
		if ((retval = omnibook_ec_read(XE3GF_BAL, &bat)))
			return retval;
		mask = XE3GF_BAL0_MASK;
		for (i = 0; i < num; i++)
			mask = mask << 1;
	/*
	 * XE3GC
	 * AMILOD
	 */
	} else if (omnibook_ectype & (XE3GC|AMILOD) ) {
		if (num >= 2)
			return -EINVAL;
		if ((retval = omnibook_ec_read(XE3GC_BAT, &bat)))
			return retval;
		mask = XE3GC_BAT0_MASK;
		for (i = 0; i < num; i++)
			mask = mask << 1;
	}
	return (bat & mask) ? 1 : 0;
}

/*
 * Get static battery information
 * All info have to be reread every time because battery sould be cahnged
 * when laptop is on AC power 
 * return values:
 *  < 0 - ERROR
 *    0 - OK
 *    1 - Battery is not present
 *    2 - Not supported
 */
static int omnibook_get_battery_info(int num,
				     struct omnibook_battery_info *battinfo)
{
	int retval;
	u32 offset;

	/*
	 * XE3GF
	 * TSP10
	 * TSM30X
	 */
	if (omnibook_ectype & (XE3GF|TSP10|TSM30X) ) {
		offset = 0x10;
		retval = omnibook_battery_present(num);
		if (retval < 0)
			return retval;
		if (retval) {
			if ((retval =
			     omnibook_ec_read(XE3GF_BTY0 + (offset * num),
				     &(*battinfo).type)))
				return retval;
			if ((retval =
			     omnibook_ec_read16(XE3GF_BSN0 + (offset * num),
				       &(*battinfo).sn)))
				return retval;
			if ((retval =
			     omnibook_ec_read16(XE3GF_BDV0 + (offset * num),
				       &(*battinfo).dv)))
				return retval;
			if ((retval =
			     omnibook_ec_read16(XE3GF_BDC0 + (offset * num),
				       &(*battinfo).dc)))
				return retval;

			(*battinfo).type =
			    ((*battinfo).type & XE3GF_BTY_MASK) ? 1 : 0;
		} else
			return 1;
	/*
	 * XE3GC
	 */
	} else if (omnibook_ectype & (XE3GC) ) {
		offset = 0x10;
		retval = omnibook_battery_present(num);
		if (retval < 0)
			return retval;
		if (retval) {
			if ((retval =
			     omnibook_ec_read16(XE3GC_BDV0 + (offset * num),
				       &(*battinfo).dv)))
				return retval;
			if ((retval =
			     omnibook_ec_read16(XE3GC_BDC0 + (offset * num),
				       &(*battinfo).dc)))
				return retval;
			if ((retval =
			     omnibook_ec_read(XE3GC_BTY0 + (offset * num),
				     &(*battinfo).type)))
				return retval;

			(*battinfo).type =
			    ((*battinfo).type & XE3GC_BTY_MASK) ? 1 : 0;
			(*battinfo).sn = 0;	/* Unknown */
		} else
			return 1;
	/*
	 * AMILOD
	 */
	} else if (omnibook_ectype & (AMILOD) ) {
		offset = 0x10;
		retval = omnibook_battery_present(num);
		if (retval < 0)
			return retval;
		if (retval) {
			if ((retval =
			     omnibook_ec_read16(AMILOD_BDV0 + (offset * num),
				       &(*battinfo).dv)))
				return retval;
			if ((retval =
			     omnibook_ec_read16(AMILOD_BDC0 + (offset * num),
				       &(*battinfo).dc)))
				return retval;
			if ((retval =
			     omnibook_ec_read(AMILOD_BTY0 + (offset * num),
				     &(*battinfo).type)))
				return retval;

			(*battinfo).type =
			    ((*battinfo).type & AMILOD_BTY_MASK) ? 1 : 0;
			(*battinfo).sn = 0;	/* Unknown */
		} else
			return 1;
	/*
	 * FIXME
	 * OB500
	 * OB510
	 */
	} else if (omnibook_ectype & (OB500|OB510) ) {
		switch (num) {
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		default:
			return -EINVAL;
		}
	/*
	 * OB6000
	 * OB6100
	 * XE4500
	 */
	} else if (omnibook_ectype & (OB6000|OB6100|XE4500) ) {
		switch (num) {
		case 0:
			break;
		case 1:
			break;
		default:
			return -EINVAL;
		}
	} else {
		return 2;
	}
	return 0;
}

/*
 * Get battery status
 * return values:
 *  < 0 - ERROR
 *    0 - OK
 *    1 - Battery is not present
 *    2 - Not supported
 */
int omnibook_get_battery_status(int num,
				       struct omnibook_battery_state *battstat)
{
	int retval;
	u8 status;
	u16 dc;
	int gauge;
	u8 offset;

	/*
	 * XE3GF
	 * TSP10
	 * TSM30X
	 */
	if (omnibook_ectype & (XE3GF|TSP10|TSM30X) ) {
		offset = 0x10;
		retval = omnibook_battery_present(num);
		if (retval < 0)
			return retval;
		if (retval) {
			if ((retval =
			     omnibook_ec_read(XE3GF_BST0 + (offset * num), &status)))
				return retval;
			if ((retval =
			     omnibook_ec_read16(XE3GF_BRC0 + (offset * num),
				       &(*battstat).rc)))
				return retval;
			if ((retval =
			     omnibook_ec_read16(XE3GF_BPV0 + (offset * num),
				       &(*battstat).pv)))
				return retval;
			if ((retval =
			     omnibook_ec_read16(XE3GF_BFC0 + (offset * num),
				       &(*battstat).lc)))
				return retval;
			if ((retval =
			     omnibook_ec_read(XE3GF_GAU0 + (offset * num),
				     &(*battstat).gauge)))
				return retval;

			if (status & XE3GF_BST_MASK_CRT)
				(*battstat).status = OMNIBOOK_BATTSTAT_CRITICAL;
			else if (status & XE3GF_BST_MASK_CHR)
				(*battstat).status = OMNIBOOK_BATTSTAT_CHARGING;
			else if (status & XE3GF_BST_MASK_DSC)
				(*battstat).status =
				    OMNIBOOK_BATTSTAT_DISCHARGING;
			else if (status &
				 (XE3GF_BST_MASK_CHR | XE3GF_BST_MASK_DSC))
				(*battstat).status = OMNIBOOK_BATTSTAT_UNKNOWN;
			else {
				(*battstat).status = OMNIBOOK_BATTSTAT_CHARGED;
			}
		} else
			return 1;
	/*
	 * XE3GC
	 */
	} else if (omnibook_ectype & (XE3GC) ) {
		offset = 0x10;
		retval = omnibook_battery_present(num);
		if (retval < 0)
			return retval;
		if (retval) {
			if ((retval =
			     omnibook_ec_read(XE3GC_BST0 + (offset * num), &status)))
				return retval;
			if ((retval =
			     omnibook_ec_read16(XE3GC_BRC0 + (offset * num),
				       &(*battstat).rc)))
				return retval;
			if ((retval =
			     omnibook_ec_read16(XE3GC_BPV0 + (offset * num),
				       &(*battstat).pv)))
				return retval;
			if ((retval =
			     omnibook_ec_read16(XE3GC_BDC0 + (offset * num), &dc)))
				return retval;

			if (status & XE3GC_BST_MASK_CRT)
				(*battstat).status = OMNIBOOK_BATTSTAT_CRITICAL;
			else if (status & XE3GC_BST_MASK_CHR)
				(*battstat).status = OMNIBOOK_BATTSTAT_CHARGING;
			else if (status & XE3GC_BST_MASK_DSC)
				(*battstat).status =
				    OMNIBOOK_BATTSTAT_DISCHARGING;
			else if (status &
				 (XE3GC_BST_MASK_CHR | XE3GC_BST_MASK_DSC))
				(*battstat).status = OMNIBOOK_BATTSTAT_UNKNOWN;
			else {
				(*battstat).status = OMNIBOOK_BATTSTAT_CHARGED;
			}
			gauge = ((*battstat).rc * 100) / dc;
			(*battstat).gauge = gauge;
			(*battstat).lc = 0;	/* Unknown */
		} else
			return 1;
	/*
	 * AMILOD
	 */
	} else if (omnibook_ectype & (AMILOD) ) {
		offset = 0x10;
		retval = omnibook_battery_present(num);
		if (retval < 0)
			return retval;
		if (retval) {
			if ((retval =
			     omnibook_ec_read(AMILOD_BST0 + (offset * num), &status)))
				return retval;
			if ((retval =
			     omnibook_ec_read16(AMILOD_BRC0 + (offset * num),
				       &(*battstat).rc)))
				return retval;
			if ((retval =
			     omnibook_ec_read16(AMILOD_BPV0 + (offset * num),
				       &(*battstat).pv)))
				return retval;
			if ((retval =
			     omnibook_ec_read16(AMILOD_BDC0 + (offset * num), &dc)))
				return retval;

			if (status & AMILOD_BST_MASK_CRT)
				(*battstat).status = OMNIBOOK_BATTSTAT_CRITICAL;
			else if (status & AMILOD_BST_MASK_CHR)
				(*battstat).status = OMNIBOOK_BATTSTAT_CHARGING;
			else if (status & AMILOD_BST_MASK_DSC)
				(*battstat).status =
				    OMNIBOOK_BATTSTAT_DISCHARGING;
			else if (status &
				 (AMILOD_BST_MASK_CHR | AMILOD_BST_MASK_DSC))
				(*battstat).status = OMNIBOOK_BATTSTAT_UNKNOWN;
			else {
				(*battstat).status = OMNIBOOK_BATTSTAT_CHARGED;
			}
			gauge = ((*battstat).rc * 100) / dc;
			(*battstat).gauge = gauge;
			(*battstat).lc = 0;	/* Unknown */
		} else
			return 1;
	/*
	 * OB500
	 * OB510
	 */
	} else if (omnibook_ectype & (OB500|OB510) ) {
		switch (num) {
		case 0:
			if ((retval = omnibook_ec_read(OB500_BT1S, &status)))
				return retval;
			if ((retval = omnibook_ec_read16(OB500_BT1C, &(*battstat).rc)))
				return retval;
			if ((retval = omnibook_ec_read16(OB500_BT1V, &(*battstat).pv)))
				return retval;
			break;
		case 1:
			if ((retval = omnibook_ec_read(OB500_BT2S, &status)))
				return retval;
			if ((retval = omnibook_ec_read16(OB500_BT2C, &(*battstat).rc)))
				return retval;
			if ((retval = omnibook_ec_read16(OB500_BT2V, &(*battstat).pv)))
				return retval;
			break;
		case 2:
			if ((retval = omnibook_ec_read(OB500_BT3S, &status)))
				return retval;
			if ((retval = omnibook_ec_read16(OB500_BT3C, &(*battstat).rc)))
				return retval;
			if ((retval = omnibook_ec_read16(OB500_BT3V, &(*battstat).pv)))
				return retval;
			break;
		default:
			return -EINVAL;
		}
		if (status & OB500_BST_MASK_CRT)
			(*battstat).status = OMNIBOOK_BATTSTAT_CRITICAL;
		else if (status & OB500_BST_MASK_CHR)
			(*battstat).status = OMNIBOOK_BATTSTAT_CHARGING;
		else if (status & OB500_BST_MASK_DSC)
			(*battstat).status = OMNIBOOK_BATTSTAT_DISCHARGING;
		else if (status & (OB500_BST_MASK_CHR | OB500_BST_MASK_DSC))
			(*battstat).status = OMNIBOOK_BATTSTAT_UNKNOWN;
		else {
			(*battstat).status = OMNIBOOK_BATTSTAT_CHARGED;
		}
	/*
	 * OB6000
	 * OB6100
	 * XE4500
	 */
	} else if (omnibook_ectype & (OB6000|OB6100|XE4500) ) {
		switch (num) {
		case 0:
			if ((retval = omnibook_ec_read(OB500_BT1S, &status)))
				return retval;
			if ((retval = omnibook_ec_read16(OB500_BT1C, &(*battstat).rc)))
				return retval;
			if ((retval = omnibook_ec_read16(OB500_BT1V, &(*battstat).pv)))
				return retval;
			break;
		case 1:
			if ((retval = omnibook_ec_read(OB500_BT3S, &status)))
				return retval;
			if ((retval = omnibook_ec_read16(OB500_BT3C, &(*battstat).rc)))
				return retval;
			if ((retval = omnibook_ec_read16(OB500_BT3V, &(*battstat).pv)))
				return retval;
			break;
		default:
			return -EINVAL;
		}
		if (status & OB500_BST_MASK_CRT)
			(*battstat).status = OMNIBOOK_BATTSTAT_CRITICAL;
		else if (status & OB500_BST_MASK_CHR)
			(*battstat).status = OMNIBOOK_BATTSTAT_CHARGING;
		else if (status & OB500_BST_MASK_DSC)
			(*battstat).status = OMNIBOOK_BATTSTAT_DISCHARGING;
		else if (status & (OB500_BST_MASK_CHR | OB500_BST_MASK_DSC))
			(*battstat).status = OMNIBOOK_BATTSTAT_UNKNOWN;
		else {
			(*battstat).status = OMNIBOOK_BATTSTAT_CHARGED;
		}
	} else {
		return 2;
	}
	return 0;
}

static int omnibook_battery_read(char *buffer)
{
	char *statustr;
	char *typestr;
	int max = 0;
	int num = 0;
	int len = 0;
	int retval;
	int i;
	struct omnibook_battery_info battinfo;
	struct omnibook_battery_state battstat;
	/*
	 * XE3GF
	 * XE3GC
	 * 0B6000
	 * 0B6100
	 * XE4500
	 * AMILOD
	 * TSP10
	 */
	if (omnibook_ectype & (XE3GF|XE3GC|OB6000|OB6100|XE4500|AMILOD|TSP10) )
		max = 2;
	/*
	 * OB500
	 * 0B510
	 */
	else if (omnibook_ectype & (OB500|OB510) )
		max = 3;
	/*
	 * TSM30X
	 */
	else if (omnibook_ectype & (TSM30X) )
		max = 1;

	for (i = 0; i < max; i++) {
		retval = omnibook_get_battery_info(i, &battinfo);
		if (retval == 0) {
			num++;
			omnibook_get_battery_status(i, &battstat);
			typestr = (battinfo.type) ? "Li-Ion" : "NiMH";
			switch (battstat.status) {
			case OMNIBOOK_BATTSTAT_CHARGED:
				statustr = "charged";
				break;
			case OMNIBOOK_BATTSTAT_DISCHARGING:
				statustr = "discharging";
				break;
			case OMNIBOOK_BATTSTAT_CHARGING:
				statustr = "charging";
				break;
			case OMNIBOOK_BATTSTAT_CRITICAL:
				statustr = "critical";
				break;
			default:
				statustr = "unknown";
			}

			len +=
			    sprintf(buffer + len, "Battery:            %11d\n",
				    i);
			len +=
			    sprintf(buffer + len, "Type:               %11s\n",
				    typestr);
			if (battinfo.sn)
				len +=
				    sprintf(buffer + len,
					    "Serial Number:      %11d\n",
					    battinfo.sn);
			len +=
			    sprintf(buffer + len,
				    "Present Voltage:    %11d mV\n",
				    battstat.pv);
			len +=
			    sprintf(buffer + len,
				    "Design Voltage:     %11d mV\n",
				    battinfo.dv);
			len +=
			    sprintf(buffer + len,
				    "Remaining Capacity: %11d mAh\n",
				    battstat.rc);
			if (battstat.lc)
				len +=
				    sprintf(buffer + len,
					    "Last Full Capacity: %11d mAh\n",
					    battstat.lc);
			len +=
			    sprintf(buffer + len,
				    "Design Capacity:    %11d mAh\n",
				    battinfo.dc);
			len +=
			    sprintf(buffer + len,
				    "Gauge:              %11d %%\n",
				    battstat.gauge);
			len +=
			    sprintf(buffer + len, "Status:             %11s\n",
				    statustr);
			len += sprintf(buffer + len, "\n");
		}
	}
	if (num == 0)
		len += sprintf(buffer + len, "No battery present\n");

	return len;
}

struct omnibook_feature battery_feature  = {
	 .name = "battery",
	 .enabled = 1,
	 .read = omnibook_battery_read,
	 .ectypes = XE3GF|XE3GC|AMILOD|TSP10|TSM30X, /* FIXME: OB500|OB6000|OB6100|XE4500 */
};


module_param_named(battery, battery_feature.enabled, int, S_IRUGO);
MODULE_PARM_DESC(battery, "Use 0 to disable, 1 to enable battery status monitoring");
/* End of file */
