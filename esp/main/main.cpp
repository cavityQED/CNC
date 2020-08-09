#include "axis.h"
#include "spi_client.h"

#include "freertos/queue.h"

#define DEVICE_SELECT	(gpio_num_t) 21
#define ZERO_INTERLOCK	(gpio_num_t) 32

static xQueueHandle evt_queue;
axis gen_axis;
SpiClient spi;

enum INTERRUPT_TYPE {
	GET_MESSAGE,
	ZERO_STOP
};

typedef struct {
	INTERRUPT_TYPE type;
} event_info_t;

void IRAM_ATTR msg_ready(void* arg) {
	event_info_t evt;
	evt.type = GET_MESSAGE;
	xQueueSendFromISR(evt_queue, &evt, NULL);
}

void IRAM_ATTR zero_interlock(void* arg) {
	event_info_t evt;
	evt.type = ZERO_STOP;
	xQueueSendFromISR(evt_queue, &evt, NULL);
}

void get_message() {
	std::vector<int> msg(MAX_TRANSACTION_LENGTH);
	std::cout << "Getting Message....\n";
	spi.get_message(msg);
	switch((AXIS_FUNCTION_CODE) msg[0]) {
		case SET_SPEED:
			gen_axis.set_speed_rpm(msg[1]);
			break;
		case SET_DIRECTION:
			gen_axis.set_direction((bool) msg[1]);
			break;
		case SET_STEPS_TO_MOVE:
			gen_axis.set_steps_to_move(msg[1]);
			break;
		case SET_JOG_SPEED_STEPS:
			gen_axis.set_jog_speed_steps(msg[1]);
			break;
		case SET_JOG_SPEED_MM:
			gen_axis.set_jog_speed_mm(msg[1]);
			break;
		case ENA_JOG_MODE:
			gen_axis.enable_jog_mode(true);
			break;
		case DIS_JOG_MODE:
			gen_axis.enable_jog_mode(false);
			break;
		case ENA_STEP_MODE:
			gen_axis.enable_step_mode(true);
			break;
		case DIS_STEP_MODE:
			gen_axis.enable_step_mode(false);
			break;
		case ENA_SYNC_MODE:
			gen_axis.enable_sync_mode(true);
			break;
		case DIS_SYNC_MODE:
			gen_axis.enable_sync_mode(false);
			break;
		case GET_POSITION:
			int steps;
			gen_axis.get_position_steps(steps);
			spi.set_sendbuffer_value(1, steps);
			break;
		case MOVE:
			gen_axis.move();
			break;
		case CIRCLE_MOVE:
			gen_axis.circle_move();
			break;
		case ZERO:
			gen_axis.zero();
			break;
		case RECEIVE:
			break;
		case SETUP_CIRCLE_MOVE:
			gen_axis.setup_circle_move();
			break;
		case SET_CIRCLE_OPS:
			gen_axis.set_circle_ops(msg[1]);
			break;
		case GET_CIRCLE_STEPS:
			spi.get_data(gen_axis.get_circle_steps());
			break;
		case GET_CIRCLE_TIMES:
			spi.get_data(gen_axis.get_circle_times());
			break;
		case GET_CIRCLE_DIRS:
			spi.get_data(gen_axis.get_circle_dirs());
			break;
		case PRINT_CIRCLE_INFO:
			gen_axis.print_circle_info();
			break;
		default:
			break;
		}
}

static void main_task(void* arg) {
	event_info_t evt;
	while(1) {	
		std::cout << "Main Task Waiting.....\n";
		xQueueReceive(evt_queue, &evt, portMAX_DELAY);
		switch(evt.type) {
			case GET_MESSAGE:
				get_message();
				break;
			case ZERO_STOP:
				gen_axis.zero_interlock_stop();
				break;
			default:
				break;
		}
	}
}
				
extern "C" void app_main(void) {
	evt_queue = xQueueCreate(10, sizeof(event_info_t));
	
	gpio_config_t io_conf;
	memset(&io_conf, 0, sizeof(io_conf));
	
	io_conf.intr_type		= GPIO_INTR_POSEDGE;
	io_conf.mode			= GPIO_MODE_INPUT;
	io_conf.pull_down_en	= GPIO_PULLDOWN_ENABLE;
	io_conf.pin_bit_mask	= (1ULL << ZERO_INTERLOCK) | (1 << DEVICE_SELECT);
	
	gpio_config(&io_conf);
	gpio_install_isr_service(0);
	gen_axis.setup_gpio();
	
	gpio_set_intr_type(ZERO_INTERLOCK, GPIO_INTR_POSEDGE);
	gpio_isr_handler_add(ZERO_INTERLOCK, zero_interlock, NULL);
	gpio_set_intr_type(DEVICE_SELECT, GPIO_INTR_POSEDGE);
	gpio_isr_handler_add(DEVICE_SELECT, msg_ready, NULL);
	
	gen_axis.set_spi(&spi);
	xTaskCreate(main_task, "main_task", 4096, NULL, 1, NULL);
}
