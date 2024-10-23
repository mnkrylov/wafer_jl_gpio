#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x92997ed8, "_printk" },
	{ 0xfe4f99b8, "cdev_init" },
	{ 0xece45940, "cdev_add" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x9d3f7918, "__class_create" },
	{ 0xcffd4ad4, "cdev_del" },
	{ 0x54655f9e, "device_create" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x7a61f9a8, "device_destroy" },
	{ 0xf0e6f1bd, "class_destroy" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x3b6c41ea, "kstrtouint" },
	{ 0xd92deb6b, "acpi_evaluate_object" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x37a0cba, "kfree" },
	{ 0xc2855f40, "module_layout" },
};

MODULE_INFO(depends, "");

