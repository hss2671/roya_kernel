/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0 */
/*
 * Kasumi - module metadata and the thin entrypoint that forwards into bootstrap.
 *
 * License: Author's work under Apache-2.0; when used as a kernel module
 * (or linked with the Linux kernel), GPL-2.0 applies for kernel compatibility.
 *
 * Author: Anatdx
 */
#include <linux/module.h>
#include <linux/version.h>

#include "kasumi_bootstrap.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anatdx");
MODULE_DESCRIPTION("Kasumi kernel module");
#ifndef KASUMI_VERSION
#define KASUMI_VERSION "0.1.0-dev"
#endif
MODULE_VERSION(KASUMI_VERSION);
MODULE_SOFTDEP("pre: kernelsu");
#ifdef MODULE_IMPORT_NS
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 13, 0)
MODULE_IMPORT_NS("VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver");
#else
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
#endif
#endif

#ifndef CONFIG_MODULES
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static struct proc_dir_entry *kasumi_modules_proc_entry;

static int kasumi_modules_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "kasumi_lkm 16384 0 - Live 0x0000000000000000\n");
	return 0;
}

static void kasumi_create_modules_proc(void)
{
	kasumi_modules_proc_entry = proc_create_single("modules", 0444, NULL, kasumi_modules_proc_show);
	if (!kasumi_modules_proc_entry)
		pr_err("Kasumi: failed to create virtual /proc/modules\n");
	else
		pr_info("Kasumi: virtual /proc/modules created\n");
}

static void kasumi_remove_modules_proc(void)
{
	if (kasumi_modules_proc_entry) {
		proc_remove(kasumi_modules_proc_entry);
		kasumi_modules_proc_entry = NULL;
	}
}
#else
static inline void kasumi_create_modules_proc(void) {}
static inline void kasumi_remove_modules_proc(void) {}
#endif

static int __init kasumi_lkm_init(void)
{
	int ret = kasumi_bootstrap_init();

	if (ret == 0)
		kasumi_create_modules_proc();
	return ret;
}

static void __exit kasumi_lkm_exit(void)
{
	kasumi_remove_modules_proc();
	kasumi_bootstrap_exit();
}

module_init(kasumi_lkm_init);
module_exit(kasumi_lkm_exit);
