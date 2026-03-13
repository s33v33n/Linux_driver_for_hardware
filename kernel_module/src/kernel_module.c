#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <asm/errno.h>
#include <asm/io.h>

#include <linux/sysfs.h> // sysfs library
#include <linux/kobject.h> // create entries in sysfs
#include <linux/string.h>
#include <linux/init.h>
#include <linux/printk.h>
//#include <linux/kstrtox.h>

MODULE_INFO(intree, "Y");
MODULE_LICENSE("GPL");

#define SYKT_GPIO_BASE_ADDR  (0x00100000)
#define SYKT_GPIO_SIZE       (0x8000)
#define SYKT_EXIT            (0x3333)
#define SYKT_EXIT_CODE       (0x7F)

#define SYKT_IN_REG 	     (0x200)
#define SYKT_CTRL_REG 	     (0x218)
#define SYKT_STATE_REG 	     (0x208)
#define SYKT_RESULT_REG 	 (0x210)
//#define SYKT_TEST_REG 	 (0x250)


#define MY_SYSFSDRV_BUFFER_LEN PAGE_SIZE  //dla arch. zgodnych z i386: 4096B

char my_array[MY_SYSFSDRV_BUFFER_LEN]="Oto SYSFS!\n";
void __iomem *baseptr;

static ssize_t dsczmi_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count){	// IN
	u8 data;
    	if (kstrtou8(buffer, 0, &data))
        	return -EINVAL;

    	iowrite8(data, baseptr + SYKT_IN_REG);

    	//pr_info("Called buffer: %s\n", buffer); //dla testow

	return count;
}

static ssize_t dtczmi_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count){	// CTRL
	u8 data;
    	if (kstrtou8(buffer, 0, &data))
        	return -EINVAL;

    	iowrite8(data, baseptr + SYKT_CTRL_REG);


    	return count;
}

static ssize_t dcczmi_show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer){	// STATE
	u8 data = ioread8(baseptr + SYKT_STATE_REG);
	return sprintf(buffer, "0x%02X\n", data);

	//pr_info("Random info for test\n"); // dla testow
        //return sprintf(buffer, "0xF0F0\n");
}

static ssize_t drczmi_show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer){	// RESULT
	u16 data = ioread16(baseptr + SYKT_RESULT_REG);
    	return sprintf(buffer, "0x%04X\n", data);
}


/*static ssize_t test_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count){	// TEST - in
	u32 data;
	if (kstrtou32(buffer, 0, &data))
		return -EINVAL;

	iowrite32(data, baseptr + SYKT_CTRL_REG);

	return count;
}

static ssize_t test_show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer){	// TEST - out 
	u8 data = ioread32(baseptr + SYKT_STATE_REG);
	return sprintf(buffer, "0x%02X\n", data);
}*/


static struct kobject *sykt; 					// plik my_sysfs
static struct kobj_attribute dsczmi_attr = __ATTR_WO(dsczmi);	// plik IN
static struct kobj_attribute dtczmi_attr = __ATTR_WO(dtczmi);	// plik CTRL
static struct kobj_attribute dcczmi_attr = __ATTR_RO(dcczmi);	// plik STATE
static struct kobj_attribute drczmi_attr = __ATTR_RO(drczmi);	// plik RESULT

//static struct kobj_attribute test_attr = __ATTR_RW(test);	// plik testowy


int my_init_module(void){
	printk(KERN_INFO "Init my module.\n");
	baseptr=ioremap(SYKT_GPIO_BASE_ADDR, SYKT_GPIO_SIZE);

	//tworzymy nowy katalog w sysfs, kernel_kobj – rodzic (deklarowany w sysfs.h)
	sykt=kobject_create_and_add("my_sysfs", kernel_kobj);
	if(!sykt){
		printk("my_sysfs - Error creating /sys/kernel/my_sysfs\n");
        return -ENOMEM;
	}

	//tworzymy nowy plik w katalogu my_sysfs dla operacji IN
	if(sysfs_create_file(sykt, &dsczmi_attr.attr)){
            	printk("dsczmi file - Error creating /sys/kernel/my_sysfs/dsczmi\n");
            	kobject_put(sykt);
        return -ENOMEM;
    }

	//tworzymy nowy plik w katalogu my_sysfs dla operacji CTRL
	if(sysfs_create_file(sykt, &dtczmi_attr.attr)){
            	printk("dtczmi file - Error creating /sys/kernel/my_sysfs/dtczmi\n");
            	kobject_put(sykt);
        return -ENOMEM;
    }

	//tworzymy nowy plik w katalogu my_sysfs dla operacji STATE
	if(sysfs_create_file(sykt, &dcczmi_attr.attr)){
            	printk("dcczmi file - Error creating /sys/kernel/my_sysfs/dcczmi\n");
            	kobject_put(sykt);
        return -ENOMEM;
    }

	//tworzymy nowy plik w katalogu my_sysfs dla operacji RESULT
	if(sysfs_create_file(sykt, &drczmi_attr.attr)){
            	printk("drczmi file - Error creating /sys/kernel/my_sysfs/drczmi\n");
            	kobject_put(sykt);
        return -ENOMEM;
    }

	//tworzymy plik testowy w katalogu my_sysfs 
	/*if(sysfs_create_file(sykt, &test_attr.attr)){
            	printk("test file - Error creating /sys/kernel/my_sysfs/test\n");
            	kobject_put(sykt);
        return -ENOMEM;
    }*/

	return 0;
}

void my_cleanup_module(void){
	printk(KERN_INFO "Cleanup my module.\n");
	writel(SYKT_EXIT | ((SYKT_EXIT_CODE)<<16), baseptr);
	iounmap(baseptr);
	kobject_put(sykt); // wywolanie gdy usuwany jest modul jadra 
}

module_init(my_init_module)
module_exit(my_cleanup_module)
