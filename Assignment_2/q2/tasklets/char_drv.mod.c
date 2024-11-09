/*********************************************************************
 * Advanced OS Assignment 2
 * File: module.mod.c
 * 
 * Purpose:
 *     Auto-generated kernel module build file containing symbol 
 *     versioning information, module metadata, and build configuration.
 *     This file is created by the kernel build system during module
 *     compilation.
 * 
 * Author: Parth Thakkar
 * Date: 8/11/24
 * 
 * Copyright (c) 2024 Parth Thakkar
 * All rights reserved.
 *********************************************************************/
/* Core kernel module support */
#include <linux/module.h>

/* Build system and version verification headers */
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>     /* Build-time salt for module authentication */
#include <linux/elfnote-lto.h>    /* Link Time Optimization (LTO) metadata */
#include <linux/export-internal.h> /* Internal symbol export definitions */
#include <linux/vermagic.h>       /* Kernel version magic number checking */
#include <linux/compiler.h>       /* Compiler-specific annotations */


/* ORC unwinder support for stack trace generation */
#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>       /* ORC unwinder metadata header */
ORC_HEADER;                       /* Insert ORC unwinder metadata */
#endif

/* Build-time metadata generation */
BUILD_SALT;                       /* Generate unique build identifier */
BUILD_LTO_INFO;                   /* Include LTO build information */

/* Module metadata information */
MODULE_INFO(vermagic, VERMAGIC_STRING);  /* Kernel version compatibility info */
MODULE_INFO(name, KBUILD_MODNAME);       /* Module name */


/**
 * Module structure definition
 * Contains core information about the kernel module including init/exit
 * functions and module name
 */
__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
    .name = KBUILD_MODNAME,           /* Module name from build system */
    .init = init_module,              /* Module initialization function */
#ifdef CONFIG_MODULE_UNLOAD
    .exit = cleanup_module,           /* Module cleanup function (if unload enabled) */
#endif
    .arch = MODULE_ARCH_INIT,         /* Architecture-specific initialization */
};

/* Retpoline mitigation for Spectre V2 vulnerability */
#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");          /* Indicate retpoline compilation */
#endif



/**
 * Symbol version table
 * Maps kernel symbols used by this module to their CRC checksums
 * Format: 16-byte records containing size, CRC, and symbol name
 */
static const char ____versions[]
__used __section("__versions") =
	"\x10\x00\x00\x00\xe6\x6e\xab\xbc"
	"sscanf\0\0"
	"\x10\x00\x00\x00\xfd\xf9\x3f\x3c"
	"sprintf\0"
	"\x1c\x00\x00\x00\x2b\x2f\xec\xe3"
	"alloc_chrdev_region\0"
	"\x14\x00\x00\x00\x24\x87\x65\x5c"
	"cdev_init\0\0\0"
	"\x14\x00\x00\x00\x95\x76\xdc\xa2"
	"cdev_add\0\0\0\0"
	"\x18\x00\x00\x00\x73\xe0\x38\x00"
	"class_create\0\0\0\0"
	"\x18\x00\x00\x00\xf9\x31\xa1\x23"
	"device_create\0\0\0"
	"\x14\x00\x00\x00\xcd\x6b\xa8\xc7"
	"kernel_kobj\0"
	"\x20\x00\x00\x00\x01\xd2\xee\xd6"
	"kobject_create_and_add\0\0"
	"\x20\x00\x00\x00\x34\x86\xf4\xdf"
	"sysfs_create_file_ns\0\0\0\0"
	"\x20\x00\x00\x00\x8e\x83\xd5\x92"
	"request_threaded_irq\0\0\0\0"
	"\x14\x00\x00\x00\xb1\xd2\x8d\xd5"
	"cdev_del\0\0\0\0"
	"\x24\x00\x00\x00\x33\xb3\x91\x60"
	"unregister_chrdev_region\0\0\0\0"
	"\x18\x00\x00\x00\xf2\x79\x84\x2f"
	"device_destroy\0\0"
	"\x18\x00\x00\x00\xe3\xe5\x01\x99"
	"class_destroy\0\0\0"
	"\x14\x00\x00\x00\xb0\xd8\xae\x22"
	"kobject_put\0"
	"\x20\x00\x00\x00\xb6\x1a\xd4\xbd"
	"sysfs_remove_file_ns\0\0\0\0"
	"\x14\x00\x00\x00\x3b\x4a\x51\xc1"
	"free_irq\0\0\0\0"
	"\x1c\x00\x00\x00\xac\xb8\x2a\x9d"
	"__tasklet_schedule\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x10\x00\x00\x00\x7e\x3a\x2c\x12"
	"_printk\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x18\x00\x00\x00\x3a\x0a\xd8\xfc"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";


/* Module dependency information (none in this case) */
MODULE_INFO(depends, "");

/* Source version identifier for module */
MODULE_INFO(srcversion, "0225EA18BE0702D67924FDD");
