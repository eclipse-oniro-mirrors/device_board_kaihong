/*
 * Expose some of the kernel scheduler routines
 *
 * Copyright (C) 2022 Broadcom.
 *
 *      Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 *
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions of
 * the license of that module.  An independent module is a module which is not
 * derived from this software.  The special exception does not apply to any
 * modifications of the software.
 *
 *      Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_linux_sched.c 815919 2019-04-22 09:06:50Z $
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <typedefs.h>
#include <linuxver.h>

int setScheduler(struct task_struct *p, int policy, struct sched_param *param)
{
	int rc = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0))
	sched_set_fifo_low(p);
#else
	rc = sched_setscheduler(p, policy, param);
#endif
	return rc;
}

int get_scheduler_policy(struct task_struct *p)
{
	int rc = SCHED_NORMAL;
	rc = p->policy;
	return rc;
}