#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/kernel.h>   /* printk() */
#include <linux/fs.h>       /* everything... */
#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/clk.h>


#include <asm/arch-s3c2410/regs-clock.h>
#include <asm/arch/regs-adc.h>
#include <asm/hardware.h>
#include <asm/semaphore.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/irq.h>
#include <asm/io.h>

#include "s3c2410-adc.h"
#include "s3c2410.h_chip.H"

 
#define DEVICE_NAME "adc"
static int adc_major = 0;
static void __iomem *base_addr;
struct clk *clk;


typedef struct //ad设备控制结构体
{
 struct semaphore lock;
 wait_queue_head_t wait;
 int channel;
 int prescale;
}ADC_DEV;
static ADC_DEV adcdev;
struct cdev adc_cdev;//设备cdev

static irqreturn_t adcdone_int_handler(int irq,void *dev_id,struct pt_regs *regs)
{
 wake_up(&adcdev.wait);
 return IRQ_HANDLED ;
}
/**********************************************************************/
/*ad设备写操作函数*/
static ssize_t adc_write(struct file *file, const char *buffer, size_t count, loff_t * ppos)
{

	int data;

	if(count!=sizeof(data)){
		//error input data size
		printk("the size of  input data must be %d\n", sizeof(data));
		return 0;
	}

	copy_from_user(&data, buffer, count);
	adcdev.channel=ADC_WRITE_GETCH(data);
	adcdev.prescale=ADC_WRITE_GETPRE(data);

	return count;
}

/**********************************************************************/
/*ad设备读操作函数*/
static ssize_t adc_read(struct file *filp, char *buffer, size_t count, loff_t *ppos)
{
		int ret = 0;
		
		if (down_interruptible(&adcdev.lock))
		return -ERESTARTSYS;
		clk_enable(clk); //时钟使能
		writel((1<<14) | PRSCVL(adcdev.prescale) | ADC_INPUT(adcdev.channel) | 0x01 | 0x01,base_addr+S3C2410_ADCCON);//初始化ADC设备
		interruptible_sleep_on(&adcdev.wait);//中断使之睡眠
		ret = readl(base_addr+S3C2410_ADCDAT0);//读取数据寄存器内容
		ret &= 0x3ff;
		copy_to_user(buffer, (char *)&ret, sizeof(ret));//把数据传到用户空间
		up(&adcdev.lock);
		
		return sizeof(ret);
}

/**********************************************************************/
/*ad设备打开函数*/
static int adc_open(struct inode *inode, struct file *filp)
{ 

		init_MUTEX(&adcdev.lock);//初始化互斥锁
		init_waitqueue_head(&(adcdev.wait));//初始化等待队列
		adcdev.channel=0;//设置通道号
		adcdev.prescale=255;//设置比例因子
		printk("adc open success!\n");
		return 0;
}

/**********************************************************************/
/*ad设备关闭函数*/
static int adc_release(struct inode *inode, struct file *filp)
{
 printk( "adc closed\n");
 return 0;
}


/**********************************************************************/
/*ad设备操作集*/
static struct file_operations adc_fops = {
 .owner =THIS_MODULE,
 .open = adc_open,
 .read = adc_read, 
 .write = adc_write,
 .release = adc_release,
};

/**********************************************************************/
/*ad驱动模块加载函数*/
int __init adc_init(void)
{
		int ret;
		int result;
		 //生成设备号
		dev_t devno = MKDEV(adc_major, 0);
		
		//基地址映射
		base_addr = ioremap(0x58000000, 0x20);
		if (base_addr == NULL) {
		      printk(KERN_ERR "Failed to remap register block\n");
		      return -ENOMEM;
		}
			
		ret = request_irq(IRQ_ADC, adcdone_int_handler, SA_INTERRUPT, DEVICE_NAME, NULL);//申请中断
		if (ret) 
		{
		return ret;
		}
		 /* normal ADC */
		 writel(0,base_addr+S3C2410_ADCTSC); //XP_PST(NOP_MODE);
		 /* Figure out our device number. */
		 //注册设备号
		 if (adc_major)
		  result = register_chrdev_region(devno, 1, "adc");
		 else 
		 	{
		  result = alloc_chrdev_region(&devno, 0, 1, "adc");//动态申请设备号
		  adc_major = MAJOR(devno);
		 	}
		 if (result < 0) //设备号申请失败  报错并返回函数
		 {
		  printk(KERN_WARNING "adc: unable to get major %d\n", adc_major);
		  free_irq(IRQ_ADC, NULL);//释放中断号
		  return result;
		 }
		 if (adc_major == 0)                          
		 adc_major = result;
		 //初始化设备cdev并注册设备
		 cdev_init(&adc_cdev,&adc_fops);
		 adc_cdev.owner=THIS_MODULE;
		 result=cdev_add(&adc_cdev,devno,1);
		if(result)
		{
			printk("注册设备失败");
			unregister_chrdev_region(MKDEV(adc_major,0),1);
			return result;
		}	

   	//获取时钟
       clk = clk_get(NULL, "adc");
       if (IS_ERR(clk) || clk == NULL) {
              printk(KERN_ERR "ADC clk_get err!!!!!!!!!!!!!\n");
       }
				 
		 
		 printk("adc device installed, with major %d\n", adc_major);
		 return 0;
}
void __exit adc_exit(void)
{
		free_irq(IRQ_ADC, NULL);//释放中断号
		cdev_del(&adc_cdev);//注销设备
		iounmap(base_addr);
		unregister_chrdev_region(MKDEV(adc_major, 0), 1);//释放设备号
		printk("adc device uninstalled\n");
}

module_init(adc_init);
module_exit(adc_exit);
MODULE_AUTHOR("Liang Baoqiang");
MODULE_LICENSE("Dual BSD/GPL");
