#ifndef __IMAPX820_CHARGER_H__
#define __IMAPX820_CHARGER_H__

#include <mach/hardware.h>

enum imap_charger_err{
	IMAP_CHG_ERR,
};

enum imap_charge_events{
	IMAP_CHG_INSERTED_EVENT,
	IMAP_CHG_ENUMERATED_EVENT,
	IMAP_CHG_REMOVED_EVENT,
	IMAP_CHG_DONE_EVENT,
	IMAP_BATT_START_RAPID_CHARGE,
	IMAP_BATT_RESUME_CHARGE,
	IMAP_BATT_TEMP_OUTOFRANGE,
	IMAP_BATT_TEMP_INRANGE,
	IMAP_BATT_INSERTED,
	IMAP_BATT_REMOVED,
	IMAP_BATT_STATE_CHANGE,
};

struct imap_charger_event{
	enum imap_charge_events event;
};

struct imap_charger_param{
	int chg_u_input_lim_ac;
	int chg_u_input_lim_usb;
	int chg_u_input_def_ac;
	int chg_u_input_def_usb;

	int chg_i_input_lim_ac;
	int chg_i_input_lim_usb;
	int chg_i_input_def_ac;
	int chg_i_input_def_usb;
	
	int chg_u_trickle;
	int chg_u_cc;
	int chg_u_cv;
	int chg_u_eoc;
	int chg_u_restart;

	int chg_i_trickle;
	int chg_i_cc;
	int chg_i_eoc;

	int chg_timer_wd;
	int chg_timer_trickle;
	int chg_timer_count;
	int chg_timer_soft;

	int chg_temp_batt_lim;
	int chg_temp_ic_lim;
	
	int chg_ic_version;
};

struct imap_charge_info{
	struct imap_charger_param charger_param;
	
	int chg_charger_state;
	int chg_charger_valid;
	enum imap_charger_err chg_charger_err;
	int chg_charger_type;
	int chg_charger_u_input;
	int chg_charger_i_input;
	int chg_charge_u;
	int chg_charge_i;
	int chg_charge_cap;
	int chg_batt_temp;
	int chg_charger_temp;

	int mon_charger_interval;
	struct delayed_work monitor_charger;

	struct imap_charger_event *event_queue;
	int tail;
	int head;
	spinlock_t queue_lock;
	int queue_count;
	struct work_struct queue_work;
	struct workqueue_struct *event_wq_thread;

	int chg_api_version;
};

#endif /* __IMAPX820_CHARGER_H__ */
