#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

#define DRIVER_NAME             "crcmodule"
#define CRCMODULE_SYSFS_DIR     "crcmodule_sysfs"

// --- CRCMODULE MEMORY MAP ---
#define CRCMODULE_BASE_ADDR     0x00100000  // physical address on RISC-V virt board
#define CRCMODULE_SIZE          0x1000      // RTC starts at 0x00101000

// --- CRCMODULE REGISTERS ---
#define CRCMODULE_REG_IN        0x200       // data input, 1 byte
#define CRCMODULE_REG_STATE     0x208       // module state (event bits)
#define CRCMODULE_REG_RESULT    0x210       // CRC16 result
#define CRCMODULE_REG_CTRL      0x218       // control, 0x00 = calculate CRC

// --- QEMU EXIT ---
#define CRCMODULE_EXIT          0x3333      // shut down QEMU with error code
#define CRCMODULE_EXIT_CODE     0x7F        // QEMU exit code

static void __iomem *crcmodule_base; 		// start of I/O register module adderss 
static struct kobject *crcmodule_kobj;		// kobject handeler (sysfs)


/* START OF SYSFS ATTRIBUTES */

// _store & _show sufix are obligatory because of using macros _WO & _RO

// IN - write one byte to the module
static ssize_t in_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    u8 data;
    int ret;

    ret = kstrtou8(buf, 0, &data);
    if (ret < 0) {
        pr_err("crcmodule_in: invalid value, expected 0x00-0xFF\n");
        return ret;
    }

    iowrite8(data, crcmodule_base + CRCMODULE_REG_IN);
    return count;
}

// CTRL - send command to the module
static ssize_t ctrl_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    u8 data;
    int ret;

    ret = kstrtou8(buf, 0, &data);
    if (ret < 0) {
        pr_err("crcmodule_ctrl: invalid value, expected 0x00-0xFF\n");
        return ret;
    }

    iowrite8(data, crcmodule_base + CRCMODULE_REG_CTRL);
    return count;
}

// STATE - read module state
static ssize_t state_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    u8 state = ioread8(crcmodule_base + CRCMODULE_REG_STATE);

    return sysfs_emit(buf, "0x%02X\n", state);
}

// RESULT - read CRC result
static ssize_t result_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    u16 result = ioread16(crcmodule_base + CRCMODULE_REG_RESULT);

    return sysfs_emit(buf, "0x%04X\n", result);
}

static struct kobj_attribute in_attr = 		__ATTR_WO(in);      // IN file
static struct kobj_attribute ctrl_attr = 	__ATTR_WO(ctrl);    // CTRL file
static struct kobj_attribute state_attr = 	__ATTR_RO(state);   // STATE file
static struct kobj_attribute result_attr = 	__ATTR_RO(result);	// RESULT file

static struct attribute *crcmodule_attrs[] = {
    &in_attr.attr,
    &ctrl_attr.attr,
    &state_attr.attr,
    &result_attr.attr,
    NULL,
};

static const struct attribute_group crcmodule_attr_group = {
    .attrs = crcmodule_attrs,
};

/* END OF SYSFS ATTRIBUTES */


/* START OF DRIVER DESCRIPTION */

// Registration in the kernel
static int __init crcmodule_init(void)
{
    int ret;

    // map hardware registers into kernel virtual memory
    crcmodule_base = ioremap(CRCMODULE_BASE_ADDR, CRCMODULE_SIZE);
    if (!crcmodule_base) {
        pr_err("crcmodule: failed to map registers\n");
        return -ENOMEM;
    }

    // create sysfs folder kobj is the parent
    crcmodule_kobj = kobject_create_and_add(CRCMODULE_SYSFS_DIR, kernel_kobj);
    if (!crcmodule_kobj) {
        pr_err("crcmodule: failed to create sysfs directory\n");
        iounmap(crcmodule_base);
        return -ENOMEM;
    }

    // create sysfs files
    ret = sysfs_create_group(crcmodule_kobj, &crcmodule_attr_group);
    if (ret < 0) {
        pr_err("crcmodule: Failed to create sysfs files\n");
        kobject_put(crcmodule_kobj);
        iounmap(crcmodule_base);
        return ret;
    }

    pr_info("crcmodule: init successful\n");
    return 0;
}

// Unregistration from the kernel
static void __exit crcmodule_exit(void)
{
    // remove sysfs files first - hardware is no longer accesible
    kobject_put(crcmodule_kobj);

    pr_info("crcmodule: Exit successful\n");

    // shut down QEMU
    writel(CRCMODULE_EXIT | (CRCMODULE_EXIT_CODE << 16), crcmodule_base);

	// reached only if QEMU did not shut down 
    iounmap(crcmodule_base);
}

module_init(crcmodule_init);
module_exit(crcmodule_exit);

/* END OF DRIVER DESCRIPTION */


MODULE_DESCRIPTION("CrcModule CRC16-CDMA2000 sysfs driver");
MODULE_LICENSE("GPL");
