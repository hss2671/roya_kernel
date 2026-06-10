obj-$(CONFIG_KASUMI) += kasumi.o

kasumi-y := \
	kasumi_main.o \
	core/kasumi_bootstrap.o \
	core/kasumi_runtime.o \
	core/kasumi_file_view.o \
	core/kasumi_root_detection.o \
	control/kasumi_ioctl.o \
	policy/kasumi_path_policy.o \
	overlay/kasumi_overlay.o \
	hooks/kasumi_proc_hooks.o \
	hooks/kasumi_proc_read_hooks.o \
	hooks/kasumi_vfs_hooks.o \
	hooks/kasumi_syscall_redirect.o \
	hooks/kasumi_ftrace_hooks.o \
	features/kasumi_uname.o \
	features/kasumi_sop_override.o \
	features/kasumi_dop_override.o \
	features/kasumi_xattr_sid_override.o \
	features/kasumi_iop_override.o \
	features/kasumi_fop_override.o \
	features/kasumi_fake_mountinfo.o \
	features/kasumi_fake_selinuxfs_access.o

kasumi-$(CONFIG_ARM64) += hooks/arm64/kasumi_patch_memory.o

ccflags-y += -gz=none
ccflags-y += -I$(srctree)/$(src)/include
ccflags-y += -I$(srctree)/$(src)/internal
ccflags-y += -I$(srctree)/$(src)/core
ccflags-y += -I$(srctree)/$(src)/control
ccflags-y += -I$(srctree)/$(src)/policy
ccflags-y += -I$(srctree)/$(src)/overlay
ccflags-y += -I$(srctree)/$(src)/hooks
ccflags-y += -I$(srctree)/$(src)/features
