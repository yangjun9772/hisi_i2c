/*-------------------------------------------------------------------------
    
 
    Owner: George Yang
-------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <sys/times.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

static unsigned int open_cnt = 0;

static struct i2c_board_info hi_info= {
	I2C_BOARD_INFO("hi_test", 0x41),
};
static struct i2c_client *client;

unsigned int device_addr = 0x41;
unsigned char reg_addr_len = 2;

int test_driver_read(int register_address, unsigned char *buf, int size)
{
	int ret = 0;
	struct i2c_client * p_client = client;
	
	ret = hi_i2c_dma_read(p_client, (unsigned int)buf, register_address, reg_addr_len, size);

	return ret;
}

static ssize_t test_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	unsigned char buf[4] = {0};
	int reg_addr = 0, ret = 0;
	
	printk("<test_i2c read> \n");
	if (copy_from_user(buf, buff, count)){
		return -EFAULT;
	}
	reg_addr = (buf[1]<<8) | buf[0];
	printk("reg_addr 0x%02x 0x%02x \n", buf[0], buf[1]);
	ret = test_driver_read(reg_addr, buf, count);
	if (ret < 0){
		return -1;
	}
	
	if (copy_to_user(buff, buf, count)){
		return -EFAULT;
	}
	
	return 0;
}

static int test_open(struct inode* inode , struct file *file)
{
	printk("<test_i2c open> \n");
	if (0 == open_cnt++){
		return 0;
	}
	return -1;
}

static int test_close(struct inode* inode , struct file *file)
{
	printk("<test_i2c close> \n");
	open_cnt--;
	return 0;
}

static int test_driver_init(void)
{
	struct i2c_adapter * i2c_adap;
	
	i2c_adap = i2c_get_adapter(2);
	client = i2c_new_device(i2c_adap , &hi_info);
	i2c_put_adapter(i2c_adap);
	
	return 0;
}

static void test_driver_exit(void)
{
	i2c_unregister_device(client);
}

static struct file_operations test_fops =
{
	.owner = THIS_MODULE,
	.open = test_open,
	.release = test_close,
	.read = test_read
};

static int __init test_init(void)
{
	int ret = 0;
	
	ret = test_driver_init();
	printk("load test i2c ok...\n");
	return ret;
}

static void __exit test_exit(void)
{
	test_exit();
	printk("rmmod test i2c ....\n");
}

module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("TESTg");



