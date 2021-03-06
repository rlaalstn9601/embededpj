#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/slab.h> 
#include <linux/gpio.h>
#include <mach/platform.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/io.h>

#define	DOT_MAJOR	262
#define	DOT_NAME	"DOT_DRIVER"
#define GPIO_OUT1	10
#define GPIO_OUT2	20
#define	GPIO_SIZE	256
static int dot_usage = 0;
static void *dot_map;
volatile unsigned *dot;

static int dot_open(struct inode *minode, struct file *mfile){
	unsigned char index;

	if (dot_usage != 0)
		return -EBUSY;
	dot_usage = 1;

	// GPIO 2 ~ 18
	dot_map = ioremap(GPIO_BASE, GPIO_SIZE);
	if (!dot_map)
	{
		printk("error: mapping gpio memory");
		iounmap(dot_map);
		return -EBUSY;
	}

	int i=0;
	dot = (volatile unsigned int *)dot_map;
	for (i=0; index < 8; i++)
	{
		*(dot + 1) &= ~(0x07 << (3 * (GPIO_OUT1+i)));
		*(dot + 1) |= (0x01 << (3 * (GPIO_OUT1+i)));
		*(dot + 2) &= ~(0x07 << (3 * (GPIO_OUT2+i)));
		*(dot + 2) |= (0x01 << (3 * (GPIO_OUT2+i)));
	}
	return 0;
}

static int dot_release(struct inode *minode, struct file *mfile) {
	dot_usage = 0;
	if (dot)
		iounmap(dot);
	return 0;
}
static int dot_write(struct file *mfile, const char *gdata, size_t length, loff_t *off_what)
{
	char tmp_buf[9][9]={0};
	int result,i,j;
	result = copy_from_user(&tmp_buf, gdata, length);
	if (result < 0)
	{
		printk("Error: copy from user");
		return result;
	}
		

	printk("data from user: %x\n",tmp_buf[i][j]);
	*(dot+7)=(0xFF<<20);
	*(dot+7)=(0xFF<<10);
	*(dot+10)=(0x0<<10);
	msleep(5000);	

	*(dot+10)=(0xFF <<20);
	*(dot+10)=(0xFF <<10);

	
	return length;
}

static struct file_operations dot_fops =
{
	.owner		= THIS_MODULE,
	.open		= dot_open,
	.release	= dot_release,
	.write		= dot_write,
};

static int dot_init(void)
{
	int result;

	result = register_chrdev(DOT_MAJOR, DOT_NAME, &dot_fops);
	if (result < 0)
	{
		printk(KERN_WARNING "Can't get any major number\n");
		return result;
	}
	return 0;
}

static void dot_exit(void)
{
	unregister_chrdev(DOT_MAJOR, DOT_NAME);
	printk("DOT module removed.\n");
}

module_init(dot_init);
module_exit(dot_exit);

MODULE_LICENSE("GPL");
