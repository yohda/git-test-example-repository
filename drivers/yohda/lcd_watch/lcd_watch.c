#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/kernel.h>

#define LCD_WATCH_NAME "LCD_WATCH"             
#define LCD_WATCH_ADDR  0x27              

static struct i2c_adapter *lcd_watch_adapter = NULL; 
static struct i2c_client  *yohda_i2c_client = NULL;  

/*
** This function writes the data into the I2C client
**
**  Arguments:
**      buff -> buffer to be sent
**      len  -> Length of the data
**   
*/
static int lcd_watch_write(unsigned char *buf, unsigned int len)
{
    /*
    ** Sending Start condition, Slave address with R/W bit, 
    ** ACK/NACK and Stop condtions will be handled internally.
    */ 
    int ret = i2c_master_send(yohda_i2c_client, buf, len);
    
    return ret;
}

/*
** This function reads one byte of the data from the I2C client
**
**  Arguments:
**      out_buff -> buffer wherer the data to be copied
**      len      -> Length of the data to be read
** 
*/
static int lcd_watch_read(unsigned char *out_buf, unsigned int len)
{
    /*
    ** Sending Start condition, Slave address with R/W bit, 
    ** ACK/NACK and Stop condtions will be handled internally.
    */ 
    int ret = i2c_master_recv(yohda_i2c_client, out_buf, len);
    
    return ret;
}

static int lcd_watch_probe(struct i2c_client *client,
                         const struct i2c_device_id *id)
{
    int err = -1,i;
	pr_info("YOHA Probed!!!\n");
	unsigned char buf[4] = { 0x00,};
	
	lcd_watch_write(0x03, 1);
	
	err = lcd_watch_read(buf, 4);
	if(err < -1)
	{
		pr_err("YOHDA LCD WATCH I2C Error\n");
		return -1;
	}
	
	for(i=0;i<4;i++)
		pr_info("YOHDA buf[%x]:0x%x\n",i,buf[i]);
		
    
    return 0;
}

static int lcd_watch_remove(struct i2c_client *client)
{   
    
    pr_info("YOHDA Removed!!!\n");
    return 0;
}

static const struct i2c_device_id lcd_watch_id[] = {
        { LCD_WATCH_NAME, 0 },
        { }
};
MODULE_DEVICE_TABLE(i2c, lcd_watch_id);

static struct i2c_driver lcd_watch_driver = {
        .driver = {
            .name   = LCD_WATCH_NAME,
            .owner  = THIS_MODULE,
        },
        .probe          = lcd_watch_probe,
        .remove         = lcd_watch_remove,
        .id_table       = lcd_watch_id,
};

static struct i2c_board_info yohda_lcd_watch_device = {
        I2C_BOARD_INFO(LCD_WATCH_NAME, LCD_WATCH_ADDR)
    };

static int __init yohda_lcd_watch_init(void)
{
    int ret = -1;
    lcd_watch_adapter     = i2c_get_adapter(1); // for rpi3-b, if you enalbe a i2c functionality with using raspi-config, by defauly you can check i2c-`1` in /dev/i2c-1 or /sys/bus/i2c/devices/i2c-1
    
    if( lcd_watch_adapter != NULL )
    {
        yohda_i2c_client = i2c_new_device(lcd_watch_adapter, &yohda_lcd_watch_device);
        
        if( yohda_i2c_client != NULL )
        {
            i2c_add_driver(&lcd_watch_driver);
            ret = 0;
        }
        
        i2c_put_adapter(lcd_watch_adapter);
    }
    
    pr_info("Driver Added!!!\n");
    return ret;
}

static void __exit yohda_lcd_watch_exit(void)
{
    i2c_unregister_device(yohda_i2c_client);
    i2c_del_driver(&lcd_watch_driver);
    pr_info("Driver Removed!!!\n");
}

module_init(yohda_lcd_watch_init);
module_exit(yohda_lcd_watch_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yohan Yoon <dbsdy1235@gmail.com>");
MODULE_DESCRIPTION("Simple LCD Watch");
