#ifndef __IMAPX820_BATTERY_A_H__
#define __IMAPX820_BATTERY_A_H__

#define BATT_CURVE_NUM_MAX 15

enum imap_batt_version_e{
	BATT_VER_0 = 0,
	BATT_VER_1,
};

enum imap_batt_charger_state_e{
	BATT_CHARGER_OFF = 0,
	BATT_CHARGER_ON,
};

struct imap_battery_param{
	int batt_vol_max_design;
	int batt_vol_min_design;
	int batt_cap_low_design;

	int batt_ntc_design;
	int batt_temp_max_design;
	int batt_temp_min_design;
	
	int batt_technology;
	
	enum imap_batt_version_e batt_version;
};

enum batt_api_version_e {
	BATT_API_0 = 0,
	BATT_API_1,
};

struct imap_current_level{
	char name[20];
	int level;
	int currents;
};

struct battery_curve{
	int cap;
	int vol;
};

struct batt_curve_tbl{
	int level;
	struct battery_curve batt_curve[BATT_CURVE_NUM_MAX];
};

struct battery_curve discharge_curve[] = {
	[0] = {100, 4200},
	[1] = {90, 3962},
	[2] = {80, 3867},
	[3] = {70, 3788},
	[4] = {60, 3727},
	[5] = {50, 3684},
	[6] = {40, 3659},
	[7] = {30, 3638},
	[8] = {20, 3594},
	[9] = {10, 3484},
	[10] = {5, 3400},
	[11] = {0, 3300},
};

struct imap_columb_param{
};

struct imap_battery_info{
	struct imap_battery_param *batt_param;
	struct imap_columb_param *columb_param;

	struct power_supply *imap_psy_ac;
	struct power_supply *imap_psy_usb;
	struct power_supply *imap_psy_batt;

	int mon_battery_interval;
	struct semaphore mon_sem;
	struct task_struct *monitor_battery;
	
	int cap_timer_cnt_max;
	int cap_timer_cnt_min;
	int cap_timer_cnt_design;
	int cap_cnt;
	
	int batt_update_flag;
	int batt_init_flag;
	int batt_low_flag;

	int batt_state;
	int batt_health;
	int batt_valid;
	int batt_voltage;
	int batt_current;
	int batt_capacity;
	int batt_old_capacity;
	int batt_temperature;

	void (*get_current_high)(char *, int);
	int (*send_low_cap)(void);
	int (*calculate_capacity)(int);
	
	struct imap_current_level *current_level;
	int current_level_num;
	int current_state;

	int (*display_capacity)(int);

	//event info
	struct imap_charger_event *event_queue;
	int tail;
	int head;
	spinlock_t queue_lock;
	int queue_count;

	//work queue
	struct work_struct queue_work;
	struct workqueue_struct *event_wq_thread;

	enum imap_batt_charger_state_e charger_state;

	int batt_api_version;
};

#endif /* __IMAPX820_BATTERY_A_H__ */
