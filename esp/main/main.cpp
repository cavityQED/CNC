#include "axis.h"
#include "spi_client.h"

#include "freertos/queue.h"

#define DEVICE_SELECT	(gpio_num_t) 25
#define ZERO_INTERLOCK	(gpio_num_t) 22

static xQueueHandle evt_queue;
static std::vector<int> msg;
axis gen_axis;
SpiClient spi;

enum INTERRUPT_TYPE
{
	GET_MESSAGE,
	ZERO_STOP
};

typedef struct
{
	INTERRUPT_TYPE type;
} event_info_t;

void IRAM_ATTR msg_ready(void* arg)
{
	event_info_t evt;
	evt.type = GET_MESSAGE;
	xQueueSendFromISR(evt_queue, &evt, NULL);
}

void IRAM_ATTR zero_interlock(void* arg)
{
	gpio_intr_disable(ZERO_INTERLOCK);
	event_info_t evt;
	evt.type = ZERO_STOP;
	xQueueSendFromISR(evt_queue, &evt, NULL);
}

void get_message()
{
	std::cout << "Getting Message\n";
	spi.set_sendbuffer_value(1, gen_axis.step_position());
	spi.set_sendbuffer_value(2, gen_axis.in_motion());
	spi.get_message(msg);
	spi.printFunction((AXIS_FUNCTION_CODE)msg[0]);
	switch((AXIS_FUNCTION_CODE) msg[0]) {

		case SET_AXIS:
		{
			gen_axis.set_axis((AXIS)msg[1]);
			break;
		}
		
		case SET_STEPS_PER_MM:
		{
			gen_axis.set_spmm(msg[1]);
			break;
		}
		
		case SET_MAX_STEPS:
		{
			gen_axis.set_max_steps(msg[1]);
			break;
		}
		
		case SET_DIRECTION:
		{
			gen_axis.set_direction((bool)msg[1]);
			break;
		}

		case SET_ACCELERATION:
		{
			gen_axis.set_accel(msg[1]);
			break;
		}

		case SET_INITIAL_PERIOD:
		{
			gen_axis.set_init_period(msg[1]);
			break;
		}

		case SET_JOG_STEPS:
		{
			gen_axis.set_jog_steps(msg[1]);
			break;
		}

		case SET_JOG_SPEED:
		{
			gen_axis.set_jog_speed(msg[1]);
			break;
		}

		case SET_RAPID_SPEED:
		{
			gen_axis.set_rapid_speed(msg[1]);
			break;
		}

		case ENA_JOG_MODE:
		{
			gen_axis.enable_jog_mode((bool)msg[1]);
			break;
		}

		case ENA_LINE_MODE:
		{
			gen_axis.enable_line_mode((bool)msg[1]);
			break;
		}

		case ENA_CURV_MODE:
		{
			gen_axis.enable_curv_mode((bool)msg[1]);
			break;
		}

		case ENA_SYNC_MODE:
		{
			gen_axis.enable_sync_mode((bool)msg[1]);
			break;
		}

		case VECTOR_MOVE:
		{
			gen_axis.enable_sync_mode(true);
			gen_axis.enable_line_mode(true);

			gen_axis.vector_move(	{msg[1], msg[2], msg[3]}, msg[4]	);
			break;
		}

		case CIRCLE_MOVE:
		{
			gen_axis.enable_sync_mode(true);
			gen_axis.enable_curv_mode(true);
			gen_axis.circle_move(	{msg[1], msg[2], 0},	//start position
									{msg[3], msg[4], 0},	//end position
									msg[5],					//feed rate (microseconds)
									msg[6],					//radius
									(bool)msg[7]);				//direction (CW or CCW)
			break;
		}

		case JOG_MOVE:
		{
			gen_axis.jog_move((bool)msg[1]);
			break;
		}

		case STOP:
		{
			break;
		}

		case RECEIVE:
		{
			gen_axis.print_info();
			break;
		}

		case FIND_ZERO:
		{
			int final_pulse = gen_axis.spmm() * gen_axis.max_mm();
			gen_axis.scalar_move(final_pulse, false, 250);
		}

		default:
			break;
		}
}

static void main_task(void* arg) {
	event_info_t evt;
	while(1)
	{	
		std::cout << "\nMain Task Waiting.....\n";
		xQueueReceive(evt_queue, &evt, portMAX_DELAY);
		
		switch(evt.type)
		{
			case GET_MESSAGE:
			{
				get_message();
				break;
			}

			case ZERO_STOP:
			{
				std::cout << "LIMIT SWITCH HIT\n";
				int steps = 3*gen_axis.spmm();
				gen_axis.reset_timers();
				gen_axis.set_motion(false);
				gen_axis.set_position(-steps);
				gen_axis.scalar_move(steps, true);
				gen_axis.set_homed(true);
				gpio_intr_enable(ZERO_INTERLOCK);
				break;
			}

			default:
				break;
		}
	}
}
				
extern "C" void app_main(void) {
	evt_queue = xQueueCreate(10, sizeof(event_info_t));
	msg.resize(MAX_TRANSACTION_LENGTH);
	
	gpio_config_t io_conf;
	memset(&io_conf, 0, sizeof(io_conf));

	io_conf.intr_type		= GPIO_INTR_POSEDGE;
	io_conf.mode			= GPIO_MODE_INPUT;
	io_conf.pull_down_en	= GPIO_PULLDOWN_ENABLE;
	io_conf.pin_bit_mask	= (1 << ZERO_INTERLOCK) | (1 << DEVICE_SELECT);

	gpio_config(&io_conf);
	gpio_install_isr_service(0);
	
	gpio_set_intr_type(ZERO_INTERLOCK, GPIO_INTR_POSEDGE);
	gpio_isr_handler_add(ZERO_INTERLOCK, zero_interlock, NULL);
	gpio_set_intr_type(DEVICE_SELECT, GPIO_INTR_POSEDGE);
	gpio_isr_handler_add(DEVICE_SELECT, msg_ready, NULL);
		
	gen_axis.setup_gpio();
	gen_axis.setup_timers();
	gen_axis.set_spi(&spi);

	xTaskCreate(main_task, "main_task", 4096, NULL, 1, NULL);
}
