// SPDX-License-Identifier: GPL-2.0
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <linux/slab.h>

#ifdef CONFIG_SECURITY_SELINUX
extern void kernel_set_selinux_enforcing(bool enforcing);
#endif

#ifdef CONFIG_SPOOF_LBL
bool femboy_selinux_enabled = false;
bool femboy_bl_enabled = false;
bool femboy_fs_enabled = false;
bool spoof_lbl_activated = false;

static int __init parse_femboy_selinux(char *str)
{
	int val;
	if (kstrtoint(str, 0, &val) == 0)
		femboy_selinux_enabled = !!val;
	else
		femboy_selinux_enabled = true;
	return 1;
}
__setup("femboy.selinux=", parse_femboy_selinux);

static int __init parse_femboy_bl(char *str)
{
	int val;
	if (kstrtoint(str, 0, &val) == 0)
		femboy_bl_enabled = !!val;
	else
		femboy_bl_enabled = true;
	return 1;
}
__setup("femboy.bl=", parse_femboy_bl);

static int __init parse_femboy_fs(char *str)
{
	int val;
	if (kstrtoint(str, 0, &val) == 0)
		femboy_fs_enabled = !!val;
	else
		femboy_fs_enabled = true;
	return 1;
}
__setup("femboy.fs=", parse_femboy_fs);

static ssize_t active_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", spoof_lbl_activated);
}

static ssize_t active_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int new_value;
	if (kstrtoint(buf, 10, &new_value))
		return -EINVAL;
	spoof_lbl_activated = !!new_value;
	pr_info("Femboy spoof LBL state changed to %d\n", spoof_lbl_activated);

#ifdef CONFIG_SECURITY_SELINUX
	kernel_set_selinux_enforcing(spoof_lbl_activated);
#endif

	return count;
}

static struct kobj_attribute active_attribute = __ATTR_RW(active);
#endif

static int cmdline_proc_show(struct seq_file *m, void *v)
{
#ifdef CONFIG_SPOOF_LBL
	char *c;
	char *s;
	bool spoof_bl = false;
	bool spoof_selinux = false;

	if (femboy_bl_enabled || (femboy_fs_enabled && spoof_lbl_activated)) {
		spoof_bl = true;
	}

	if (femboy_selinux_enabled || (femboy_fs_enabled && spoof_lbl_activated)) {
		spoof_selinux = true;
	}

	c = kstrdup(saved_command_line, GFP_KERNEL);
	if (!c) {
		seq_puts(m, saved_command_line);
		seq_putc(m, '\n');
		return 0;
	}

	if (spoof_bl) {
		while ((s = strstr(c, "androidboot.verifiedbootstate=orange")) != NULL) {
			memcpy(s + 30, "green ", 6);
		}

		while ((s = strstr(c, "androidboot.vbmeta.device_state=unlocked")) != NULL) {
			memcpy(s + 32, "locked  ", 8);
		}
	}

	if (spoof_selinux) {
		while ((s = strstr(c, "androidboot.selinux=permissive")) != NULL) {
			memcpy(s + 20, "enforcing ", 10);
		}
	}

	seq_printf(m, "%s\n", c);
	kfree(c);
	return 0;
#else
	seq_puts(m, saved_command_line);
	seq_putc(m, '\n');
	return 0;
#endif
}

static int __init proc_cmdline_init(void)
{
#ifdef CONFIG_SPOOF_LBL
	int retval;
	struct kobject *femboy_kobj;
#endif

	proc_create_single("cmdline", 0, NULL, cmdline_proc_show);

#ifdef CONFIG_SPOOF_LBL
	if (femboy_fs_enabled) {
		femboy_kobj = kobject_create_and_add("femboy", kernel_kobj);
		if (femboy_kobj) {
			retval = sysfs_create_file(femboy_kobj, &active_attribute.attr);
			if (retval)
				pr_err("femboy: failed to create active sysfs node\n");
		}
	} else {
#ifdef CONFIG_SECURITY_SELINUX
		if (femboy_selinux_enabled) {
			kernel_set_selinux_enforcing(true);
		}
#endif
	}
#endif

	return 0;
}
fs_initcall(proc_cmdline_init);
