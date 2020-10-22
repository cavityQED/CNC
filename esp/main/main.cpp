#include "axis.h"
#include "spi_client.h"

#include "freertos/queue.h"

#define DEVICE_SELECT	(gpio_num_t) 21
#define ZERO_INTERLOCK	(gpio_num_t) 32

static xQueueHandle evt_queue;
static std::vector<int> msg;
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
	std::cout << "Getting Message\n";
	spi.set_sendbuffer_value(1, gen_axis.get_position_steps());
	spi.set_sendbuffer_value(2, gen_axis.in_motion());
	spi.get_message(msg);
	spi.printFunction((AXIS_FUNCTION_CODE)msg[0]);
	switch((AXIS_FUNCTION_CODE) msg[0]) {
		case SET_FEED_RATE:
			gen_axis.set_feed_rate(msg[1]);
			break;
		case SET_DIRECTION:
			gen_axis.set_direction((bool) msg[1]);
			break;
		case SET_STEP_TIME:
			gen_axis.set_step_time(msg[1]);
			break;
		case SET_STEPS_TO_MOVE:
			gen_axis.set_steps_to_move(msg[1]);
			break;
		case SET_JOG_STEPS:
			gen_axis.set_jog_steps(msg[1]);
			break;
		case SET_BACKLASH:
			gen_axis.set_backlash(msg[1]);
			break;
		case SET_X_AXIS:
			gen_axis.set_x_axis((bool)msg[1]);
			break;
		case SET_STEPS_PER_MM:
			gen_axis.set_steps_per_mm(msg[1]);
			break;
		case SET_MAX_STEPS:
			gen_axis.set_max_travel_steps(msg[1]);
			break;
		case ENA_JOG_MODE:
			gen_axis.enable_jog_mode((bool)msg[1]);
			break;
		case ENA_LINE_MODE:
			gen_axis.enable_line_mode((bool)msg[1]);
			break;
		case ENA_CURV_MODE:
			gen_axis.enable_curv_mode((bool)msg[1]);
			break;
		case ENA_SYNC_MODE:
			gen_axis.enable_sync_mode((bool)msg[1]);
			break;
		case ENA_JOG_CONTINUOUS:
			gen_axis.enable_continuous_jog((bool)msg[1]);
			break;
		case ENA_TRAVEL_LIMITS:
			gen_axis.enable_travel_limits((bool)msg[1]);
			break;
		case MOVE:
			gen_axis.move();
			break;
		case FIND_ZERO:
			gen_axis.find_zero();
			break;
		case STOP:
			gen_axis.stop();
			break;
		case RECEIVE:
			break;
		case SETUP_CURVE:
			gen_axis.setup_curve(msg);
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
				gen_axis.stop_zero_interlock();
				break;
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
