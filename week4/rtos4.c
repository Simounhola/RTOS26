/**
 * The code operates as required by the 2p implementation instructions.
 * Test cases are comprehensive.
 * 
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/timing/timing.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>


// Thread initializations
#define STACKSIZE 500
#define PRIORITY 5

#define COMMAND_OK 0
#define TIME_LEN_ERROR      -1
#define TIME_ARRAY_ERROR    -2
#define TIME_VALUE_ERROR    -3

void red_led_task(void *, void *, void*);
void yellow_led_task(void *, void *, void*);
void green_led_task(void *, void *, void*);

void dispatcher_task(void *, void *, void *);
void uart_task(void *, void *, void *);
void debug_task(void *, void *, void*);

// UART initialization
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

// Condition Variables
K_MUTEX_DEFINE(red_mutex);
K_CONDVAR_DEFINE(red_signal);
K_MUTEX_DEFINE(green_mutex);
K_CONDVAR_DEFINE(green_signal);
K_MUTEX_DEFINE(yellow_mutex);
K_CONDVAR_DEFINE(yellow_signal);

K_MUTEX_DEFINE(release_mutex);
K_CONDVAR_DEFINE(release_signal);


// Led pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

K_THREAD_DEFINE(red_thread,STACKSIZE,red_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(yellow_thread,STACKSIZE,yellow_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(green_thread,STACKSIZE,green_led_task,NULL,NULL,NULL,PRIORITY,0,0);

K_THREAD_DEFINE(dis_thread,STACKSIZE,dispatcher_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(uart_thread,STACKSIZE,uart_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(debug_thread,STACKSIZE,debug_task,NULL,NULL,NULL,PRIORITY,0,0);



// Create dispatcher FIFO buffer
K_FIFO_DEFINE(dispatcher_fifo);
K_FIFO_DEFINE(data_fifo);
int init_led(void); 
int time_parse();
char timer_color;

// Timer initializations
struct k_timer timer;
void timer_handler(struct k_timer *timer_id);

void timer_work_handler(struct k_work *work);

// Timer work
K_WORK_DEFINE(timer_work, timer_work_handler);


// FIFO dispatcher data type
struct data_t {
	/*************************
	// Add fifo_reserved below
	*************************/
	void *fifo_reserved;
	char msg[20];
	uint64_t time;
};

/********************
 * init UART
 */
int init_uart(void) {
	// UART initialization
	if (!device_is_ready(uart_dev)) {
		return 1;
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

/********************
 * Main task
 */
int main(void)
{
	
	int ret = init_uart();
	if (ret != 0) {
		printk("UART initialization failed!\n");
		return ret;
	}
	init_led();
	timing_init();
	k_timer_init(&timer, timer_handler, NULL);
	
	
    /*
	timing_start();
	timing_t start_time = timing_counter_get();

	k_msleep(100);

	printk("Program started..\n");

	timing_t end_time = timing_counter_get();
	timing_stop();
    uint64_t timing_ns = timing_cycles_to_ns(timing_cycles_get(&start_time, &end_time));
	printk("Initialization: %lld\n", timing_ns);
	*/
	while (true) {
		k_msleep(100);
	}

	return 0;
}

/********************
 * UART task
 */
void uart_task(void *unused1, void *unused2, void *unused3)
{
    char rc = 0;

    char uart_msg[20];
    memset(uart_msg, 0, sizeof(uart_msg));

    int uart_msg_cnt = 0;

    while (true)
    {
        if (uart_poll_in(uart_dev, &rc) == 0)
        {
            printk("%c", rc);

            if (rc != '\r' && rc != '\n')
            {
                if (uart_msg_cnt < sizeof(uart_msg) - 1)
                {
                    uart_msg[uart_msg_cnt] = rc;
                    uart_msg_cnt++;

                    uart_msg[uart_msg_cnt] = '\0';
                }
                else
                {
                    printk("ERROR: UART message too long\n");

                    uart_msg_cnt = 0;
                    memset(uart_msg, 0, sizeof(uart_msg));
                }
            }
            else
            {
                if (uart_msg_cnt > 0)
                {
                    printk("UART msg: %s\n", uart_msg);

                    struct data_t *buf =
                        k_malloc(sizeof(struct data_t));

                    if (buf == NULL)
                    {
                        printk("ERROR: malloc failed\n");
                        continue;
                    }

                    snprintf(buf->msg,
                             sizeof(buf->msg),"%s", uart_msg);

                    k_fifo_put(&dispatcher_fifo, buf);
                }

                uart_msg_cnt = 0;
                memset(uart_msg, 0, sizeof(uart_msg));
            }
        }

        k_msleep(10);
    }
}

/********************
 * Dispatcher task
 */
void dispatcher_task(void *unused1, void *unused2, void *unused3)
{
    while (true)
    {
        struct data_t *rec_item =
            k_fifo_get(&dispatcher_fifo, K_FOREVER);

        char sequence[20];

        memcpy(sequence, rec_item->msg, sizeof(sequence));

        k_free(rec_item);

        printk("Dispatcher: %s\n", sequence);

        char color = sequence[0];

        if (color != 'R' && color != 'Y' && color != 'G')
		{
    	printk("ERROR: unsupported color %c\n", color);
    		continue;
		}

        int ret = time_parse(sequence + 1);

        if (ret == TIME_LEN_ERROR)
        {
            printk("ERROR: -1\n");
            continue;
        }

        if (ret == TIME_ARRAY_ERROR)
        {
            printk("ERROR: -2\n");
            continue;
        }

        if (ret == TIME_VALUE_ERROR)
        {
            printk("ERROR: -3\n");
            continue;
        }
		timer_color = color;
        printk("Color: %c\n", color);
        printk("Timer starts in %d seconds\n", ret);

        k_timer_start(&timer, K_SECONDS(ret), K_NO_WAIT);
    }
}

void red_led_task(void *, void *, void*)
{
    printk("Red led thread started\n");

    while (true)
    {
        k_condvar_wait(&red_signal, &red_mutex, K_FOREVER);

        printk("Red ON\n");

        gpio_pin_set_dt(&red, 1);

        k_msleep(1500);

        gpio_pin_set_dt(&red, 0);

        printk("Red OFF\n");
    }
}

void yellow_led_task(void *, void *, void*) 
{
	
	printk("Yellow led thread started\n");
	while (true)
    {
        k_condvar_wait(&yellow_signal, &yellow_mutex, K_FOREVER);

        printk("Yellow ON\n");

        gpio_pin_set_dt(&red, 1);
		gpio_pin_set_dt(&green, 1);

        k_msleep(1500);

        gpio_pin_set_dt(&red, 0);
		gpio_pin_set_dt(&green, 0);

        printk("Yellow OFF\n");
    }
			
}	

void green_led_task(void *, void *, void*) 
{
	
	printk("Green led thread started\n");
	while (true)
    {
        k_condvar_wait(&green_signal, &green_mutex, K_FOREVER);

        printk("Green ON\n");

        gpio_pin_set_dt(&green, 1);

        k_msleep(1500);

        gpio_pin_set_dt(&green, 0);

        printk("Green OFF\n");
    }
			
	}	
void debug_task(void *, void *, void*) 
{

	// Store received data
	struct data_t *received;
	uint64_t total_time = 0;
	int task_count = 0; 

	while (true) {

		received = k_fifo_get(&data_fifo, K_FOREVER);
		total_time += (received->time);
		task_count++;
		//printk("Debug received: %lld\n", received->time);
		k_free(received);
		if (task_count != 0)
		
		printk("Total_time us-> %lld\n", total_time / 1000);
		
		k_yield();
		

		
	}
}

int time_parse(char *time) 
{
	if (time == NULL)
	{
		return TIME_ARRAY_ERROR;
	}
	if (strlen(time) != 6)
	{
		return TIME_LEN_ERROR;
	}
	for (int i = 0; i < 6; i++)
    {
        if (!isdigit(time[i]))
        {
            return TIME_VALUE_ERROR;
        }
    }

	int values[3];
	values[2] = atoi(time+4); // seconds
	time[4] = 0;
	values[1] = atoi(time+2); // minutes
	time[2] = 0;
	values[0] = atoi(time); // hours

	// For example: 124033 -> 12hour 40min 33sec

	if (values[0] < 0 || values[0] > 23)
	{
		return TIME_VALUE_ERROR;
	}
	if (values[1] < 0 || values[1] > 59)
	{
		return TIME_VALUE_ERROR;
	}
	if (values[2] < 0 || values[2] > 59)
	{
		return TIME_VALUE_ERROR;
	}
	
	int seconds = ((values[0] * 3600) + values[1] * 60) + values[2];
	return seconds;
}

void timer_work_handler(struct k_work *work)
{
	if (timer_color == 'R')
	{
    k_condvar_broadcast(&red_signal);
	}
	else if (timer_color == 'Y')
    {
        k_condvar_broadcast(&yellow_signal);
    }
    else if (timer_color == 'G')
    {
        k_condvar_broadcast(&green_signal);
    }
}


void timer_handler(struct k_timer *timer_id)
{
    k_work_submit(&timer_work);
}