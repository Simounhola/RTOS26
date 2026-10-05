/**
 * almost working none of the buttons work correctly have to check every task one by one sleeps etc!!!
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <inttypes.h>
#include <zephyr/sys/util.h>



//button
#define BUTTON_0 DT_ALIAS(sw0)
#define BUTTON_1 DT_ALIAS(sw1)
#define BUTTON_2 DT_ALIAS(sw2)
#define BUTTON_3 DT_ALIAS(sw3)
#define BUTTON_4 DT_ALIAS(sw4)

static const struct gpio_dt_spec button_0 = GPIO_DT_SPEC_GET_OR(BUTTON_0, gpios, {0});
static const struct gpio_dt_spec button_1 = GPIO_DT_SPEC_GET_OR(BUTTON_1, gpios, {0});
static const struct gpio_dt_spec button_2 = GPIO_DT_SPEC_GET_OR(BUTTON_2, gpios, {0});
static const struct gpio_dt_spec button_3 = GPIO_DT_SPEC_GET_OR(BUTTON_3, gpios, {0});
static const struct gpio_dt_spec button_4 = GPIO_DT_SPEC_GET_OR(BUTTON_4, gpios, {0});

static struct gpio_callback button_0_data;
static struct gpio_callback button_1_data;
static struct gpio_callback button_2_data;
static struct gpio_callback button_3_data;
static struct gpio_callback button_4_data;



// Led pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

// Red led thread initialization
#define STACKSIZE 500
#define PRIORITY 5

int init_led(void); 
int led_state = 0;
int previous_state = 5;
int manual_red = 0;
int manual_yellow = 0;
int manual_green = 0;
int blinky_yellow = 0;

int init_buttons(void);

void red_led_task(void *, void *, void*);
void yellow_led_task(void *, void *, void*);
void green_led_task(void *, void *, void*);
void blinky(void*, void*, void*);

K_THREAD_DEFINE(red_thread,STACKSIZE,red_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(yellow_thread,STACKSIZE,yellow_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(green_thread,STACKSIZE,green_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(blinky_thread,STACKSIZE,blinky,NULL,NULL,NULL,PRIORITY,0,0);
/**
 * 		sw0 = &button_1_vol_dn;              
 *		sw1 = &button_2_vol_up; 
 *      sw2 = &button3;                    
 *		sw3 = &button4;        
 *		sw4 = &button5;
 */
void button_0_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	
	if (pins == 32)
	{
		if (blinky_yellow == 0) 
		{ 
		blinky_yellow = 1;


		
		printk("blinky on\n");
		}
		else 
		{
		blinky_yellow = 0;
		gpio_pin_set_dt(&red,0);
		gpio_pin_set_dt(&green,0);
		printk("continue!!!!!\n");
		}
	}
	if (pins == 64)
	{
		if (manual_green == 0) 
		{ 
		manual_green = 1;
		
		printk("green on\n");
		}
		else 
		{
		manual_green = 0;
		gpio_pin_set_dt(&green,0);
		printk("continue!!!\n");
		}
		
	}
	if (pins == 16)
	{
		if (manual_yellow == 0) 
		{ 
		manual_yellow = 1;
		
		printk("yellow on\n");
		}
		else 
		{
		manual_yellow = 0;
		gpio_pin_set_dt(&red,0);
		gpio_pin_set_dt(&green,0);
		printk("continue!!\n");
		}
		
	}
	if (pins == 8) //button 2 = vol+
	{
		if (manual_red == 0) 
		{ 
		manual_red = 1;
		
		printk("red on\n");
		}
		else 
		{
		manual_red = 0;
		gpio_pin_set_dt(&red,0);
		printk("continue!\n");
		}
		
	}

	if (pins == 4 )
	{

	
		if (led_state !=4)
	{
		previous_state = led_state;
		led_state = 4;
		printk("current state is --> %d\n\r", led_state);
	
	}
	else
	
	{
		led_state = previous_state;

		printk("current state is --> %d\n\r", led_state);
	}
	
	
}
}

// Main program
int main(void)
{
	init_led();

	int ret = init_buttons();
	if (ret < 0) 
	{
		return 0;
	}

	return 0;

}

// Initialize leds
int  init_led() {

	// Led pin initialization
	int ret_red = gpio_pin_configure_dt(&red, GPIO_OUTPUT_ACTIVE);
	if (ret_red < 0) 
	{
		printk("Error: Led0 configure failed\n");		
		return ret_red;
	}
	// set led off
	gpio_pin_set_dt(&red,0);
    printk("Led0 initialized ok\n");

    int ret_green = gpio_pin_configure_dt(&green, GPIO_OUTPUT_ACTIVE);
	if (ret_green < 0) 
	{
		printk("Error: Led2 configure failed\n");		
		return ret_green;
	}
	// set led off
	gpio_pin_set_dt(&green,0);
    printk("Led1 initialized ok\n");

	
	return 0;
}

int init_buttons() {

	//button_0
	int ret;
	if (!gpio_is_ready_dt(&button_0)) {
		printk("Error: button 0 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_0, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_0, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_0_data, button_0_handler, BIT(button_0.pin));
	gpio_add_callback(button_0.port, &button_0_data);
	printk("Set up button 0 ok\n");

	// button_1
	if (!gpio_is_ready_dt(&button_1)) {
		printk("Error: button 1 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_1, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_1, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_1_data, button_0_handler, BIT(button_1.pin));
	gpio_add_callback(button_1.port, &button_1_data);
	printk("Set up button 1 ok\n");

	//button_2
	if (!gpio_is_ready_dt(&button_2)) {
		printk("Error: button 2 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_2, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_2, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_2_data, button_0_handler, BIT(button_2.pin));
	gpio_add_callback(button_2.port, &button_2_data);
	printk("Set up button 2 ok\n");

	//button_3
	if (!gpio_is_ready_dt(&button_3)) {
		printk("Error: button 3 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_3, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_3, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_3_data, button_0_handler, BIT(button_3.pin));
	gpio_add_callback(button_3.port, &button_3_data);
	printk("Set up button 3 ok\n");

	//button_4

	if (!gpio_is_ready_dt(&button_4)) {
		printk("Error: button 4 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_4, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_4, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_4_data, button_0_handler, BIT(button_4.pin));
	gpio_add_callback(button_4.port, &button_4_data);
	printk("Set up button 4 ok\n");
	
	return 0;
}

// Task to handle red led
void red_led_task(void *, void *, void*) 
{
	
	printk("Red led thread started\n");
	while (true) 
	{
		if (blinky_yellow == 0)
		{
		if (manual_red == 1)
			gpio_pin_set_dt(&red,1);
			else
		{

		if (led_state == 0) 
		{
			
			k_msleep(1000);
			gpio_pin_set_dt(&red,1);
			printk("Red on\n");
			k_msleep(1000);
			if (led_state != 4)
			led_state = 1;
			gpio_pin_set_dt(&red,0);
			printk("Red off\n");
		}
			k_msleep(1000);
	}	

	}
}
}


void yellow_led_task(void *, void *, void*) 
{
	
	printk("Yellow led thread started\n");
	while (true) 
	{
		if (blinky_yellow == 0)
		{

		if (manual_yellow == 1)
		{
			gpio_pin_set_dt(&green,1);
			gpio_pin_set_dt(&red,1);
		}
			else
		{
	
		if (led_state == 1) 
		{
		k_msleep(1000);
		gpio_pin_set_dt(&green,1);
		gpio_pin_set_dt(&red,1);
		printk("Yellow on\n");
		k_msleep(1000);
		if (led_state != 4)
		led_state = 2;
		gpio_pin_set_dt(&green,0);
		gpio_pin_set_dt(&red,0);
		printk("Yellow off\n");
		}
		k_msleep(1000);
		
		}
	}
}
}
void green_led_task(void *, void *, void*) 
{
	
	printk("Green led thread started\n");
	while (true) 
	{
		if (blinky_yellow == 0){

		
		
		if (manual_green == 1)
			gpio_pin_set_dt(&green,1);
			else
			{
	
		if (led_state == 2) 
		{
		gpio_pin_set_dt(&green,1);
		printk("Green on\n");
		k_msleep(1000);
		if (led_state != 4)
		led_state = 0;
		gpio_pin_set_dt(&green,0);
		printk("Green off\n");
		}
		k_msleep(1000);
	}
}
}
}
void blinky(void*, void*, void*)
{
	printk("blinky thread has started\n");
	while (true)
	{
		if (blinky_yellow == 1)
		{
		gpio_pin_toggle_dt(&red);
		gpio_pin_toggle_dt(&green);
		k_msleep(100);

	}
	else
	{
		k_msleep(100);
		

	}

	
}
}




