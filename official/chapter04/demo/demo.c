#ifndef __KERNEL__
#define __KERNEL__
#endif
#ifndef MODULE
#define MODULE
#endif
#include <linux/config.h>
#include <linux/module.h>//模块相关
#include <linux/kernel.h>//内核相关
#include <linux/fs.h>//file_operations
#include <linux/types.h>//ssize_t定义文件
#include <linux/init.h>//__init和__exit相关
#include <linux/errno.h>
#include <linux/cdev.h>

#include <asm/uaccess.h>//copy_to_user()和copy_from_user()在此定义
#include <asm/system.h>

/*相关宏定义*/
#define DEVICE_NAME		"demo"//设备名称
#define demo_MAJOR 88//主设备号
#define demo_MINOR 0//此设备号
#define ERROR -1
static int MAX_BUF_LEN=1024;//数值的最大值
static int WRI_LENGTH=0;

/*结构体的定义*/

static int demo_major=demo_MAJOR;
struct demo_dev
{
	struct cdev cdev;
	char drv_buf[1024];
};
struct demo_dev * demo_devp;

/*************************************************************************************/
/*demo设备文件打开*/
int demo_open(struct inode * inode,struct file *filp)
{
	filp->private_data=demo_devp;
	sprintf(demo_devp->drv_buf,"device open sucess!\n");
	printk("device open sucess!\n");
	return 0;
}

/*************************************************************************************/
/*demo设备文件关闭*/
int demo_release(struct inode * inode,struct file *filp)
{
	return 0;
}

/*************************************************************************************/
/*逆序排列缓冲区数据*/
static void do_write(char * drv_buf)
{
	
	int i;
	int len = WRI_LENGTH;
	char tmp;
	for(i = 0; i < len; i++,len--){
		tmp = drv_buf[len-1];
		drv_buf[len-1] = drv_buf[i];
		drv_buf[i] = tmp;
	}
}
/*************************************************************************************/
static ssize_t  demo_write(struct file *filp,const char *buffer, size_t count,loff_t *ppos)
{ 
	struct demo_dev* dev=filp->private_data;
	if(count > MAX_BUF_LEN)count = MAX_BUF_LEN;
	copy_from_user(dev->drv_buf, buffer, count);
	WRI_LENGTH = count;
	printk("user write data to driver\n");
	do_write(dev->drv_buf);	
	return count;
}
/*************************************************************************************/
static ssize_t  demo_read(struct file *filp, char *buffer, size_t count, loff_t *ppos)
{
	struct demo_dev* dev=filp->private_data;
	if(count > MAX_BUF_LEN)
		count=MAX_BUF_LEN;
	copy_to_user(buffer, dev->drv_buf,count);
	printk("user read data from driver\n");
	return count;
}
/*************************************************************************************/
static int demo_ioctl(struct inode *inode, struct file *file, 
                 unsigned int cmd, unsigned long arg)
{
	printk("ioctl runing\n");
	switch(cmd){
		case 1:printk("runing command 1 \n");break;
		case 2:printk("runing command 2 \n");break;
		default:
			printk("error cmd number\n");break;
	}
	return 0;
}

/*************************************************************************************/
/*demo的模块加载函数*/

static const struct file_operations demo_fops=
{
	.owner=THIS_MODULE,
	.read=demo_read,
	.write=demo_write,
	.ioctl=demo_ioctl,
	.open=demo_open,
	.release=demo_release,	
};

/*************************************************************************************/
/*demo的模块加载函数*/
int __init demo_init(void)
{
	int result;
	int err;
	dev_t devno=MKDEV(demo_MAJOR,demo_MINOR);

        if(demo_MAJOR)
        {
          result=register_chrdev_region(devno,1,"demo");
        }
        else
        {
          result=alloc_chrdev_region(&devno,0,1,"demo");
        	demo_major=MAJOR(devno);
				}
        if(result<0)
        {
                return result;
        }

	demo_devp=kmalloc(sizeof(struct demo_dev),GFP_KERNEL);
	
	if(!demo_devp)
	{
			printk("空间申请失败\n");
			return ERROR;
	}
	printk("start demo init!\n");
	cdev_init(&demo_devp->cdev,&demo_fops);
	demo_devp->cdev.owner=THIS_MODULE;
	err=cdev_add(&demo_devp->cdev,devno,1);
	if(err)
	{
		printk("注册设备失败");
		unregister_chrdev_region(MKDEV(demo_MAJOR,demo_MINOR),1);
		return err;
	}
	
	printk("demo init seccess !\n");
	return 0;

}

/*************************************************************************************/
/*demo的模块卸载函数*/
void __exit demo_exit(void)
{
	cdev_del(&demo_devp->cdev);
	kfree(demo_devp);
	unregister_chrdev_region(MKDEV(demo_MAJOR,demo_MINOR),1);
}

MODULE_AUTHOR("Liang Baoqiang");
MODULE_LICENSE("Dual BSD/GPL");

module_init(demo_init);
module_exit(demo_exit);
