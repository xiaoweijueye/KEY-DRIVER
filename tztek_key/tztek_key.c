/*
 * tztek_key.c - for tztek GEAC90L(ali) key driver
 *
 * use two GPI pins as input and two GPO pins as output
 *
 * Copyright (c) 2015-2019, TZTEK CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/tztek_debug.h>
#include <linux/err.h>
#include <linux/kdev_t.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/wait.h>
#include <asm/uaccess.h>
#include <linux/of_gpio.h>
#include <asm/poll.h>

#define DRV_NAME "gpio_switch"
#define KEY1_PRESSED	1
#define KEY2_PRESSED	2

/*need to add to tztek_key.h*/
struct tztek_key_button{
	struct device_node *tztek_key_node;
	unsigned int tztek_key_major;
	struct class *tztek_cls;
	struct device *tztek_devs;
	unsigned int tztek_gpio_num_1;
	unsigned int tztek_gpio_num_2;
	unsigned int tztek_irq_1;
	unsigned int tztek_irq_2;
	struct timer_list tztek_key_timer_1;
	struct timer_list tztek_key_timer_2;
	int key_counter;
	wait_queue_head_t tztek_key_wq;
	int tztek_flag;
	struct fasync_struct *tztek_fa;
	int key_status;
};

static struct tztek_key_button *tztek_key_button_info;

static void tztek_key_1_timer_interrupt_handler(unsigned long v)
{
	if(gpio_get_value(tztek_key_button_info->tztek_gpio_num_1) == 1){
		tztek_key_button_info->key_status = KEY1_PRESSED;
	
		//while key is pressed and key status is get,register the signal to process
		kill_fasync(&tztek_key_button_info->tztek_fa,SIGIO,POLLIN);
 
 		wake_up_interruptible(&tztek_key_button_info->tztek_key_wq);
  		tztek_key_button_info->tztek_flag = 1; 	
	}
	return ;
}

static void tztek_key_2_timer_interrupt_handler(unsigned long v)
{
	if(gpio_get_value(tztek_key_button_info->tztek_gpio_num_2) == 1){
		tztek_key_button_info->key_status = KEY2_PRESSED; 

		//while key is pressed and key status is get,register the signal to process
        	kill_fasync(&tztek_key_button_info->tztek_fa,SIGIO,POLLIN);

 		wake_up_interruptible(&tztek_key_button_info->tztek_key_wq);
  		tztek_key_button_info->tztek_flag = 1; 	
	}
	return ;
}

static irqreturn_t tztek_key_1_top_interrupt_handler(int irq, void *dev_id)
{
	if(!timer_pending(&tztek_key_button_info->tztek_key_timer_1))
    		mod_timer(&tztek_key_button_info->tztek_key_timer_1, jiffies + 30 * HZ / 1000);

	return IRQ_WAKE_THREAD;
}

static irqreturn_t tztek_key_1_bottom_interrupt_handler(int irq, void *dev_id)
{	
	return IRQ_HANDLED;
}

static irqreturn_t tztek_key_2_top_interrupt_handler(int irq, void *dev_id)
{
	if(!timer_pending(&tztek_key_button_info->tztek_key_timer_2))
    		mod_timer(&tztek_key_button_info->tztek_key_timer_2, jiffies + 30 * HZ / 1000);
	return IRQ_WAKE_THREAD;
}

static irqreturn_t tztek_key_2_bottom_interrupt_handler(int irq, void *dev_id)
{
	return IRQ_HANDLED;
}

static int tztek_key_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t tztek_key_read(struct file *filp, char __user *ubuf, size_t size, loff_t *off)
{
	wait_event_interruptible(tztek_key_button_info->tztek_key_wq, tztek_key_button_info->tztek_flag != 0);
	if(copy_to_user(ubuf, (void *)&tztek_key_button_info->key_status, sizeof(tztek_key_button_info->key_status)))
		return -EFAULT;		
	
	tztek_key_button_info->tztek_flag = 0;
	return sizeof(tztek_key_button_info->key_status);
}

static int tztek_key_close(struct inode *inode, struct file *filp)
{
	return 0;
}

static int tztek_key_fasync(int fd, struct file *filp, int on)
{
	/*help register signal*/
	return fasync_helper(fd, filp, on, &tztek_key_button_info->tztek_fa);
}

static struct file_operations tztek_key_fops = {
	.owner = THIS_MODULE,
	.open = tztek_key_open,
	.read = tztek_key_read,
	.fasync = tztek_key_fasync,
	.release = tztek_key_close,
};

static int tztek_key_probe(struct platform_device *pdev)
{
	int err;	

	/*alloc memory*/
	tztek_key_button_info = devm_kzalloc(&pdev->dev, sizeof(struct tztek_key_button), GFP_KERNEL);
	if(!tztek_key_button_info){
		TZTEK_ERROR("%s:unable to alloc data structure\n", __func__);
		err = -ENOMEM;
		goto err_devm_kzalloc;
	}

	memset(tztek_key_button_info, 0, sizeof(struct tztek_key_button));
	tztek_key_button_info->tztek_flag = 0;
	
	/*register chracter device*/	
	tztek_key_button_info->tztek_key_major = register_chrdev(0, DRV_NAME, &tztek_key_fops);
	if(tztek_key_button_info->tztek_key_major < 0){
		TZTEK_ERROR("%s:register chrdev error!\n", __func__);
		err = -ENOMEM;
		goto err_register_chrdev;
	}

	/*create class*/
	tztek_key_button_info->tztek_cls = class_create(THIS_MODULE, DRV_NAME);	
	if(IS_ERR(tztek_key_button_info->tztek_cls)){
		TZTEK_ERROR("%s:class create failed!\n", __func__);
		err = PTR_ERR(tztek_key_button_info->tztek_cls);
		goto err_class_create;
	}
	
	/*create two device node*/
	for(tztek_key_button_info->key_counter = 1; tztek_key_button_info->key_counter <= 2; tztek_key_button_info->key_counter ++){
		tztek_key_button_info->tztek_devs = device_create(tztek_key_button_info->tztek_cls, NULL,MKDEV(tztek_key_button_info->tztek_key_major, tztek_key_button_info->key_counter - 1), NULL, "gpio_switch_%d", tztek_key_button_info->key_counter);	
		if(IS_ERR(tztek_key_button_info->tztek_devs)){
			TZTEK_ERROR("%s:device create failed!\n", __func__);
			err = PTR_ERR(tztek_key_button_info->tztek_devs);
			goto err_device_create;
		}
	}

	/*get two gpio number from devicetree*/
	tztek_key_button_info->tztek_key_node = pdev->dev.of_node;
	
	tztek_key_button_info->tztek_gpio_num_1 = of_get_named_gpio(tztek_key_button_info->tztek_key_node, "gpio_switch_1", 0);		
	if(tztek_key_button_info->tztek_gpio_num_1 < 0){
		TZTEK_ERROR("%s:failed to get gpio number from dt!\n", __func__);
		err = -ENODEV;
		goto err_of_get_named_gpio;
	}

	tztek_key_button_info->tztek_gpio_num_2 = of_get_named_gpio(tztek_key_button_info->tztek_key_node, "gpio_switch_2", 0);
	if(tztek_key_button_info->tztek_gpio_num_2 < 0){
		TZTEK_ERROR("%s:failed to get gpio number from dt!\n", __func__);
		err = -ENODEV;
		goto err_of_get_named_gpio;
	}

	/*request gpio*/
	if(gpio_is_valid(tztek_key_button_info->tztek_gpio_num_1)){
		if(devm_gpio_request(&pdev->dev, tztek_key_button_info->tztek_gpio_num_1, "gpio_switch_1")){
			TZTEK_ERROR("%s:gpio request error!\n", __func__);
			err = -ENOMEM;
			goto err_gpio_request_1;
		}

		if(gpio_direction_input(tztek_key_button_info->tztek_gpio_num_1)){
			TZTEK_ERROR("%s:failed to set gpio direction!\n", __func__);
			err = -EINVAL;
			goto err_gpio_direction_input_1;
		}

		tztek_key_button_info->tztek_irq_1 = gpio_to_irq(tztek_key_button_info->tztek_gpio_num_1);
		if(tztek_key_button_info->tztek_irq_1 < 0){
			TZTEK_ERROR("%s:failed to get irq number!\n", __func__);
			err = -EINVAL;
			goto err_gpio_to_irq_1;
		}
	}

	if(gpio_is_valid(tztek_key_button_info->tztek_gpio_num_2)){
		if(devm_gpio_request(&pdev->dev, tztek_key_button_info->tztek_gpio_num_2, "gpio_switch_2")){
			TZTEK_ERROR("%s:gpio request error!\n", __func__);
			err = -ENOMEM;
			goto err_gpio_request_2;
		}
		
		if(gpio_direction_input(tztek_key_button_info->tztek_gpio_num_2)){
			TZTEK_ERROR("%s:failed to set gpio direction!\n", __func__);
			err = -EINVAL;
			goto err_gpio_direction_input_2;
		}

		tztek_key_button_info->tztek_irq_2 = gpio_to_irq(tztek_key_button_info->tztek_gpio_num_2);
		if(tztek_key_button_info->tztek_irq_2 < 0){
			TZTEK_ERROR("%s:failed to get irq number!\n", __func__);
			err = -EINVAL;
			goto err_gpio_to_irq_2;
		}
	}

	/*register interrupt*/
	if(request_threaded_irq(tztek_key_button_info->tztek_irq_1, tztek_key_1_top_interrupt_handler, tztek_key_1_bottom_interrupt_handler,
					IRQF_TRIGGER_RISING | IRQF_ONESHOT, "gpio_switch_1", NULL) < 0){
		TZTEK_ERROR("%s:failed to request irq!\n", __func__);
		err = -EINVAL;
		goto err_requested_irq_1;
	}	

	if(request_threaded_irq(tztek_key_button_info->tztek_irq_2, tztek_key_2_top_interrupt_handler, tztek_key_2_bottom_interrupt_handler,
					IRQF_TRIGGER_RISING | IRQF_ONESHOT, "gpio_switch_2", NULL) < 0){
		TZTEK_ERROR("%s:failed to request irq!\n", __func__);
		err = -EINVAL;
		goto err_requested_irq_2;
	}

	/*timer initial*/
	init_timer(&tztek_key_button_info->tztek_key_timer_1);
	tztek_key_button_info->tztek_key_timer_1.function = tztek_key_1_timer_interrupt_handler;
	add_timer(&tztek_key_button_info->tztek_key_timer_1);
	
	init_timer(&tztek_key_button_info->tztek_key_timer_2);
	tztek_key_button_info->tztek_key_timer_2.function = tztek_key_2_timer_interrupt_handler;
	add_timer(&tztek_key_button_info->tztek_key_timer_2);
	
	/*waitqueue initial*/
	init_waitqueue_head(&tztek_key_button_info->tztek_key_wq);

	return 0;

err_requested_irq_2:
	free_irq(tztek_key_button_info->tztek_irq_1, NULL);
err_requested_irq_1:
	
err_gpio_to_irq_2:
err_gpio_direction_input_2:
	devm_gpio_free(&pdev->dev, tztek_key_button_info->tztek_gpio_num_2);
err_gpio_request_2:
err_gpio_to_irq_1:
err_gpio_direction_input_1:
	devm_gpio_free(&pdev->dev, tztek_key_button_info->tztek_gpio_num_1);
err_gpio_request_1:
	
err_of_get_named_gpio:
	for(tztek_key_button_info->key_counter = 1; tztek_key_button_info->key_counter >= 0; tztek_key_button_info->key_counter--){
		device_destroy(tztek_key_button_info->tztek_cls, MKDEV(tztek_key_button_info->tztek_key_major, tztek_key_button_info->key_counter));
	}
err_device_create:
	class_destroy(tztek_key_button_info->tztek_cls);
err_class_create:
	unregister_chrdev(tztek_key_button_info->tztek_key_major, DRV_NAME);
err_register_chrdev:
	devm_kfree(&pdev->dev, tztek_key_button_info);	
err_devm_kzalloc:
	return err;
}

static int tztek_key_remove(struct platform_device *pdev)
{
	del_timer(&tztek_key_button_info->tztek_key_timer_2);
	del_timer(&tztek_key_button_info->tztek_key_timer_1);
	free_irq(tztek_key_button_info->tztek_irq_2, NULL);
	free_irq(tztek_key_button_info->tztek_irq_1, NULL);
	devm_gpio_free(&pdev->dev, tztek_key_button_info->tztek_gpio_num_2);
	devm_gpio_free(&pdev->dev, tztek_key_button_info->tztek_gpio_num_1);	
	for(tztek_key_button_info->key_counter = 1; tztek_key_button_info->key_counter >= 0; tztek_key_button_info->key_counter--){
                device_destroy(tztek_key_button_info->tztek_cls, MKDEV(tztek_key_button_info->tztek_key_major, tztek_key_button_info->key_counter));
	}
	class_destroy(tztek_key_button_info->tztek_cls);
	unregister_chrdev(tztek_key_button_info->tztek_key_major, DRV_NAME);
	devm_kfree(&pdev->dev, tztek_key_button_info);
	
	return 0;
}

static struct of_device_id tztek_key_match_tbl[] = {
	{
		.compatible = "tztek,key_driver",
	},
	{}
};

static struct platform_driver tztek_key_pdrv = {
	.probe = tztek_key_probe,
	.remove = tztek_key_remove,
	.driver = {
		.name = DRV_NAME,
		.owner= THIS_MODULE,
		.of_match_table = of_match_ptr(tztek_key_match_tbl),
	},
};

module_platform_driver(tztek_key_pdrv);

MODULE_AUTHOR("chenwei <chenwei@tztek.com>");
MODULE_DESCRIPTION("ali key driver");
MODULE_LICENSE("GPL");

