#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x59253af, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xd7dcc513, __VMLINUX_SYMBOL_STR(platform_driver_unregister) },
	{ 0x6a79c969, __VMLINUX_SYMBOL_STR(__platform_driver_register) },
	{ 0x65345022, __VMLINUX_SYMBOL_STR(__wake_up) },
	{ 0x72f426cc, __VMLINUX_SYMBOL_STR(kill_fasync) },
	{ 0xb2396b63, __VMLINUX_SYMBOL_STR(gpiod_get_raw_value) },
	{ 0xab40cca9, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0x85825898, __VMLINUX_SYMBOL_STR(add_timer) },
	{ 0x8fdf772a, __VMLINUX_SYMBOL_STR(init_timer_key) },
	{ 0xd6b8e852, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0x5394f626, __VMLINUX_SYMBOL_STR(gpiod_to_irq) },
	{ 0xb0b40e2d, __VMLINUX_SYMBOL_STR(gpiod_direction_input) },
	{ 0xf1ce739e, __VMLINUX_SYMBOL_STR(gpio_to_desc) },
	{ 0xeffb8fcd, __VMLINUX_SYMBOL_STR(devm_gpio_request) },
	{ 0x504f19b2, __VMLINUX_SYMBOL_STR(of_get_named_gpio_flags) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xdfc05c53, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0xb2ce072e, __VMLINUX_SYMBOL_STR(__class_create) },
	{ 0x64d31de2, __VMLINUX_SYMBOL_STR(__register_chrdev) },
	{ 0xdcb764ad, __VMLINUX_SYMBOL_STR(memset) },
	{ 0xf4e13e7b, __VMLINUX_SYMBOL_STR(devm_kmalloc) },
	{ 0x9c5bc552, __VMLINUX_SYMBOL_STR(finish_wait) },
	{ 0xcb128141, __VMLINUX_SYMBOL_STR(prepare_to_wait_event) },
	{ 0x1000e51, __VMLINUX_SYMBOL_STR(schedule) },
	{ 0x622598b1, __VMLINUX_SYMBOL_STR(init_wait_entry) },
	{ 0xb35dea8f, __VMLINUX_SYMBOL_STR(__arch_copy_to_user) },
	{ 0xdcb02ba3, __VMLINUX_SYMBOL_STR(fasync_helper) },
	{ 0x526c3a6c, __VMLINUX_SYMBOL_STR(jiffies) },
	{ 0xfa5bcf35, __VMLINUX_SYMBOL_STR(mod_timer) },
	{ 0xa3f9d513, __VMLINUX_SYMBOL_STR(devm_kfree) },
	{ 0x6bc3fbc0, __VMLINUX_SYMBOL_STR(__unregister_chrdev) },
	{ 0x5702c523, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0x631e6e03, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0xcbb0990e, __VMLINUX_SYMBOL_STR(devm_gpio_free) },
	{ 0xf20dabd8, __VMLINUX_SYMBOL_STR(free_irq) },
	{ 0xb8eec98b, __VMLINUX_SYMBOL_STR(del_timer) },
	{ 0x1fdc7df2, __VMLINUX_SYMBOL_STR(_mcount) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

