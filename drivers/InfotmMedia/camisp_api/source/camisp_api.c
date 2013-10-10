/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file camisp_api.c
--
--  Description :
--		
--
--	Author:
--  	Leo Zhang   <leo.zhang@infotmic.com.cn>
--------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	leo@2012/04/18: first commit.
--
------------------------------------------------------------------------------*/

#include <InfotmMedia.h>
#include <IM_cameraapi.h>
#include <isp_pwl.h>
#include <camisp_api.h>


#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"CAMISP_I:"
#define WARNHEAD	"CAMISP_W:"
#define ERRHEAD		"CAMISP_E:"
#define TIPHEAD		"CAMISP_T:"

#define TIME_STATS	0

//
// control model.
//
#define CANCEL_STAT_TRIGGER			1
#define CANCEL_STAT_DONE			2

#define THREAD_STAT_INITED			1
#define THREAD_STAT_EXIT_TRIGGER	2
#define THREAD_STAT_EXIT_DONE		3

typedef struct{
	IM_INT32		index;	// the index of ctrlthrd.
	im_list_handle_t	cwlist;	// list of ctrl word which waiting process.
	isppwl_thread_t		thread;	// control thread.
	isppwl_signal_t		signal;
	isppwl_lock_t		lock;


	// about cancel control.
	IM_INT32		execCtrlWordId;	// the current executing ctrl word.
	IM_INT32		cancelStat;	// CANCEL_STAT_xxx.

	// thread exit.
	IM_INT32		threadStat;	// THREAD_STAT_xxx.
	IM_BOOL 		sigHasSet;//maybe also need.......
}control_thread_t;

typedef struct{
	ctrlmodel_config_t	config;
	control_thread_t	*ctrlthrd[CONTROL_THREAD_COUNT_MAX];
}ctrlmodel_t;

//
// stream model.
//
typedef struct{
	strmmodel_config_t	config;
}strmmodel_t;

//
// work model's event queue.
//
#define EVTQ_STAT_IDLE			0
#define EVTQ_STAT_WAITING		1
#define EVTQ_STAT_REQUEST_EXIT	2

typedef struct{
	im_list_handle_t	list;	// list of workmodel_event_t.
	isppwl_signal_t		signal;
	isppwl_lock_t		lock;

	IM_INT32		stat;	// EVTQ_STAT_xxx.
	IM_BOOL 		sigHasSet;
}event_queue_t;

//
// work model.
//
typedef struct{
	im_mempool_handle_t	mpl;
	ctrlmodel_t		ctrlmdl;
	strmmodel_t		strmmdl;
	event_queue_t		evtq;
	camapi_interface_t	camIntfs;

	IM_BOOL			camInited;
	IM_BOOL			ctrlmdlInited;
	IM_BOOL			evtqInited;
}workmodel_t;


static workmodel_t *gWorkModel = IM_NULL;


//
// event queue.
//
IM_RET evtqInit(void)
{
	event_queue_t *evtq = &gWorkModel->evtq;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if(isppwl_lock_init(&evtq->lock) != IM_RET_OK){
		IM_ERRMSG((IM_STR("isppwl_lock_init() failed")));
		goto Fail;
	}

	if(isppwl_sig_init(&evtq->signal, IM_FALSE/*IM_TRUE*/) != IM_RET_OK){
		IM_ERRMSG((IM_STR("isppwl_sig_init() failed")));
		goto Fail;
	}

	evtq->list = im_list_init(sizeof(workmodel_event_t), gWorkModel->mpl);
	if(evtq->list == IM_NULL){
		IM_ERRMSG((IM_STR("im_list_init() failed")));
		goto Fail;
	}

	evtq->stat = EVTQ_STAT_IDLE;
	evtq->sigHasSet = IM_FALSE;

	return IM_RET_OK;
Fail:
	if(evtq->list != IM_NULL){
		im_list_deinit(evtq->list);
		evtq->list = IM_NULL;
	}
	if(evtq->signal != IM_NULL){
		isppwl_sig_deinit(evtq->signal);
		evtq->signal = IM_NULL;
	}
	if(evtq->lock != IM_NULL){
		isppwl_lock_deinit(evtq->lock);	
		evtq->lock = IM_NULL;
	}

	return IM_RET_FAILED;
}

IM_RET evtqDeinit(void)
{
	workmodel_event_t *evt;
	event_queue_t *evtq = &gWorkModel->evtq;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	// break evtqWait().
	isppwl_lock(evtq->lock);
	if(evtq->stat == EVTQ_STAT_WAITING){
		evtq->stat = EVTQ_STAT_REQUEST_EXIT;
		isppwl_sig_set(evtq->signal);
		evtq->sigHasSet = IM_TRUE;
		while(IM_TRUE){
			isppwl_unlock(evtq->lock);
			isppwl_msleep(500);
			isppwl_lock(evtq->lock);
			if(evtq->stat == EVTQ_STAT_IDLE){
				break;
			}
		}
	}
	isppwl_unlock(evtq->lock);

	// release resource.
	if(evtq->list != IM_NULL){
		isppwl_lock(evtq->lock);
		evt = (workmodel_event_t *)im_list_begin(evtq->list);
		while(evt != IM_NULL){
			evt = (workmodel_event_t *)im_list_erase(evtq->list, evt);
		}
		im_list_deinit(evtq->list);
		evtq->list = IM_NULL;
		isppwl_unlock(evtq->lock);
	}
	if(evtq->signal != IM_NULL){
		isppwl_sig_deinit(evtq->signal);
		evtq->signal = IM_NULL;
	}
	if(evtq->lock != IM_NULL){
		isppwl_lock_deinit(evtq->lock);	
		evtq->lock = IM_NULL;
	}

	return IM_RET_OK;
}

IM_RET evtqPut(workmodel_event_t *evt)
{
	event_queue_t *evtq = &gWorkModel->evtq;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	isppwl_lock(evtq->lock);
	im_list_put_back(evtq->list, (void *)evt);
	if(im_list_size(evtq->list) == 1){
		IM_INFOMSG((IM_STR("event signal set++")));
		isppwl_sig_set(evtq->signal);
		evtq->sigHasSet = IM_TRUE;
		IM_INFOMSG((IM_STR("event signal set--")));
	}
	isppwl_unlock(evtq->lock);

	return IM_RET_OK;
}

IM_RET evtqWait(void)
{
	IM_RET ret = IM_RET_OK;
	event_queue_t *evtq = &gWorkModel->evtq;

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	isppwl_lock(evtq->lock);
	evtq->stat = EVTQ_STAT_WAITING;
	IM_INFOMSG((IM_STR("%s(), event num = %d"), IM_STR(_IM_FUNC_), (im_list_size(evtq->list))));
	//set timeout = 0.5 s
	//if(im_list_size(evtq->list) == 0){//目前这样会有bug， signal set有可能一直操作，到下次list size 真正为0 sig_wait 却能成功，导致后面get_event失败
	//if(im_list_size(evtq->list) <= 1){//目前这样也会有bug， 当list size为1 sig_wait却一直在等不往下往下执行， 而g2d不会出现这样的情况，因为他的wait和get object from list 在同一个函数中，当list size ＝ 1时，必然之前有且仅有一次sig_set调用， 所以sig_wait可以往下继续运行
	if((im_list_size(evtq->list) == 0) || (evtq->sigHasSet == IM_TRUE)){
		/*if(im_list_size(evtq->list) == 0){
			IM_TIPMSG((IM_STR("%s(), evq empty(sigSet=%d)"), IM_STR(_IM_FUNC_), evtq->sigHasSet));
		}*/
#if TIME_STATS
		struct timeval _s, _e;
		IM_INT64 time;
		do_gettimeofday(&_s);
		IM_TIPMSG((IM_STR(" signal wait begin time.sec=%ds,time.usec=%d++ \n"),_s.tv_sec, _s.tv_usec));
#endif
		ret = isppwl_sig_wait(evtq->signal, &evtq->lock, 500000/*us*/);
#if TIME_STATS
		do_gettimeofday(&_e);
		IM_TIPMSG((IM_STR(" signal wait end time.sec=%ds,time.usec=%d-- \n"),_e.tv_sec, _e.tv_usec));
		time = (_e.tv_sec*1000000+_e.tv_usec - (_s.tv_sec*1000000+_s.tv_usec))/1000;
		IM_TIPMSG((IM_STR(" signal wait time=%lldms-- \n"),time));
#endif
		evtq->sigHasSet = IM_FALSE;
		if(ret != IM_RET_OK)
		{
			if(ret == IM_RET_TIMEOUT)
			{
				IM_ERRMSG((IM_STR("wait event signal time out!")));
			}
			else
			{
				IM_ERRMSG((IM_STR("wait event signal error!")));
			}
		}
	}

	if(evtq->stat == EVTQ_STAT_REQUEST_EXIT){
		IM_INFOMSG((IM_STR("evtq request exit")));
		evtq->stat = EVTQ_STAT_IDLE;
		isppwl_unlock(evtq->lock);
		return IM_RET_FALSE;
	}

	evtq->stat = EVTQ_STAT_IDLE;
	isppwl_unlock(evtq->lock);
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET evtqGet(workmodel_event_t *evt)
{
	workmodel_event_t *tmp;
	event_queue_t *evtq = &gWorkModel->evtq;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	isppwl_lock(evtq->lock);
	if(im_list_size(evtq->list) == 0){
		IM_ERRMSG((IM_STR("evtq list is empty, it can not get event!")));
		isppwl_unlock(evtq->lock);
		return IM_RET_FAILED;
	}
	tmp = (workmodel_event_t *)im_list_begin(evtq->list);
	isppwl_memcpy((void *)evt, (void *)tmp, sizeof(workmodel_event_t));
	im_list_erase(evtq->list, tmp);
	if(im_list_size(evtq->list) == 0){
		IM_INFOMSG((IM_STR("evtq list is empty, signal reset!")));
		isppwl_sig_reset(evtq->signal);
	}
	isppwl_unlock(evtq->lock);

	return IM_RET_OK;
}

static IM_RET controlDone(ctrl_word_t *cw)
{
	workmodel_event_t evt;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	evt.type = WORKMODEL_EVENT_CONTROL_DONE;
	isppwl_memcpy((void *)&evt.contents.cw, (void *)cw, sizeof(ctrl_word_t));

	evtqPut(&evt);

	return IM_RET_OK;
}

static IM_RET frameDone(strm_frame_t *frm)
{
	workmodel_event_t evt;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	evt.type = WORKMODEL_EVENT_FRAME_DONE;
	isppwl_memcpy((void *)&evt.contents.frm, (void *)frm, sizeof(strm_frame_t));

	evtqPut(&evt);

	return IM_RET_OK;
}


//
// stream model.
//


//
// control model.
//
static IM_INT32 getCodeBoxIndex(IM_INT32 code)
{
	IM_INT32 index, i;
	IM_INFOMSG((IM_STR("%s(code=0x%x)"), IM_STR(_IM_FUNC_), code));

	for(index=0; index < gWorkModel->ctrlmdl.config.ctCnt; index++){
		for(i=0; i < gWorkModel->ctrlmdl.config.ctCodeBoxSize[index]; i++){
			if(gWorkModel->ctrlmdl.config.ctCodeBox[index][i] == code){
				IM_INFOMSG((IM_STR("index=%d"), index));
				break;
			}
		}
	}	

	return (index == gWorkModel->ctrlmdl.config.ctCnt) ? -1 : index;
}

static void ctrlthrdEntry(void *data)
{
	IM_RET ret;
	ctrl_word_t *pcw, cw;
	control_thread_t *ctrlthrd = (control_thread_t *)data;
	IM_INFOMSG((IM_STR("%s(index=%d)++"), IM_STR(_IM_FUNC_), ctrlthrd->index));

	isppwl_lock(ctrlthrd->lock);
	ctrlthrd->threadStat = THREAD_STAT_INITED;

	do{
		if((im_list_size(ctrlthrd->cwlist) == 0) || (ctrlthrd->sigHasSet == IM_TRUE)){  
			isppwl_sig_wait(ctrlthrd->signal, &ctrlthrd->lock, -1);
			ctrlthrd->sigHasSet = IM_FALSE;
		}

		// check if request exit.
		if(ctrlthrd->threadStat == THREAD_STAT_EXIT_TRIGGER){
			IM_INFOMSG((IM_STR("request work thread(%d) exit"), ctrlthrd->index));
			isppwl_unlock(ctrlthrd->lock);
			break;
		}

		// check there if has ctrl code to do.
		pcw = (ctrl_word_t *)im_list_begin(ctrlthrd->cwlist);
		if(pcw != IM_NULL){
			isppwl_memcpy((void *)&cw, (void *)pcw, sizeof(ctrl_word_t));
			ctrlthrd->execCtrlWordId = pcw->id;
			im_list_erase(ctrlthrd->cwlist, pcw);	// delete from fifo, and put to current exec unit.
			if(im_list_size(ctrlthrd->cwlist) == 0){
				isppwl_sig_reset(ctrlthrd->signal);
				ctrlthrd->sigHasSet = IM_FALSE;
			}

			isppwl_unlock(ctrlthrd->lock);	// unlock when exec().
			do{
				ret = gWorkModel->ctrlmdl.config.control_exec[ctrlthrd->index](&cw, IM_FALSE);
				isppwl_lock(ctrlthrd->lock);
				if(ret == IM_RET_RETRY){
					if(ctrlthrd->cancelStat == CANCEL_STAT_TRIGGER){
						IM_INFOMSG((IM_STR("request cancel the ctrl code 0x%x, id %d"), cw.code, cw.id));
						isppwl_unlock(ctrlthrd->lock);
						ret = gWorkModel->ctrlmdl.config.control_exec[ctrlthrd->index](&cw, IM_TRUE);
						cw.ret = IM_RET_FALSE;
						isppwl_lock(ctrlthrd->lock);
					}
				}
			}while(ret == IM_RET_RETRY);

			// here has already got lock.
			ctrlthrd->cancelStat = CANCEL_STAT_DONE;

			isppwl_unlock(ctrlthrd->lock);
			controlDone(&cw);
			isppwl_lock(ctrlthrd->lock);
		}
	}while(IM_TRUE);

	isppwl_lock(ctrlthrd->lock);
	ctrlthrd->threadStat = THREAD_STAT_EXIT_DONE;
	isppwl_unlock(ctrlthrd->lock);
	
	IM_INFOMSG((IM_STR("%s(index=%d)--"), IM_STR(_IM_FUNC_), ctrlthrd->index));
}

static IM_RET ctrlthrdInit(IM_INT32 index)
{
	control_thread_t *ctrlthrd = gWorkModel->ctrlmdl.ctrlthrd[index];
	IM_INFOMSG((IM_STR("%s(index=%d)"), IM_STR(_IM_FUNC_), index));

	if(isppwl_lock_init(&ctrlthrd->lock) != IM_RET_OK){
		IM_ERRMSG((IM_STR("isppwl_lock_init() failed")));
		goto Fail;
	}

	if(isppwl_sig_init(&ctrlthrd->signal, /*IM_TRUE*/IM_FALSE) != IM_RET_OK){
		IM_ERRMSG((IM_STR("isppwl_sig_init() failed")));
		goto Fail;
	}
	ctrlthrd->sigHasSet = IM_FALSE;

	ctrlthrd->cwlist = im_list_init(sizeof(ctrl_word_t), gWorkModel->mpl);
	if(ctrlthrd->cwlist == IM_NULL){
		IM_ERRMSG((IM_STR("im_list_init() failed")));
		goto Fail;
	}

	if(isppwl_thread_init(&ctrlthrd->thread, (isppwl_func_thread_entry_t)ctrlthrdEntry, (void *)ctrlthrd) != IM_RET_OK){
		IM_ERRMSG((IM_STR("isppwl_thread_init() failed")));
		goto Fail;
	}

	// wait the thread init ok.	
	while(IM_TRUE){
		isppwl_lock(ctrlthrd->lock);
		if(ctrlthrd->threadStat == THREAD_STAT_INITED){
			isppwl_unlock(ctrlthrd->lock);
			break;
		}
		isppwl_unlock(ctrlthrd->lock);
		isppwl_msleep(5);
	}

	return IM_RET_OK;
Fail:
	if(ctrlthrd->thread != IM_NULL){
		isppwl_thread_deinit(ctrlthrd->thread);
		ctrlthrd->thread = IM_NULL;
	}
	if(ctrlthrd->cwlist != IM_NULL){
		im_list_deinit(ctrlthrd->cwlist);
		ctrlthrd->cwlist = IM_NULL;
	}
	if(ctrlthrd->signal != IM_NULL){
		isppwl_sig_deinit(ctrlthrd->signal);
		ctrlthrd->signal = IM_NULL;
	}
	if(ctrlthrd->lock != IM_NULL){
		isppwl_lock_deinit(ctrlthrd->lock);	
		ctrlthrd->lock = IM_NULL;
	}

	return IM_RET_FAILED;
}

static IM_RET ctrlthrdDeinit(IM_INT32 index)
{
	ctrl_word_t *cw;
	control_thread_t *ctrlthrd = gWorkModel->ctrlmdl.ctrlthrd[index];
	IM_INFOMSG((IM_STR("%s(index=%d)"), IM_STR(_IM_FUNC_), index));

	if(ctrlthrd->thread != IM_NULL){
		isppwl_lock(ctrlthrd->lock);
		ctrlthrd->cancelStat = CANCEL_STAT_TRIGGER;
		ctrlthrd->threadStat = THREAD_STAT_EXIT_TRIGGER;
		isppwl_sig_set(ctrlthrd->signal);
		ctrlthrd->sigHasSet = IM_TRUE;
		
		// wait the thread exit ok.	
		while(IM_TRUE){
			isppwl_unlock(ctrlthrd->lock);
			isppwl_msleep(50);
			isppwl_lock(ctrlthrd->lock);
			if(ctrlthrd->threadStat == THREAD_STAT_EXIT_DONE){
				break;
			}
		}
		isppwl_unlock(ctrlthrd->lock);

		isppwl_thread_deinit(ctrlthrd->thread);
		ctrlthrd->thread = IM_NULL;
	}
	if(ctrlthrd->cwlist != IM_NULL){
		isppwl_lock(ctrlthrd->lock);
		cw = (ctrl_word_t *)im_list_begin(ctrlthrd->cwlist);
		while(cw != IM_NULL){	// clear all pending cw.
			cw->ret = IM_RET_FALSE;	// mark it's been canceled.
			controlDone(cw);
			//cw = (ctrl_word_t *)im_list_erase(ctrlthrd->cwlist);
			im_list_erase(ctrlthrd->cwlist, cw);
		}
		im_list_deinit(ctrlthrd->cwlist);
		ctrlthrd->cwlist = IM_NULL;
		isppwl_unlock(ctrlthrd->lock);
	}
	if(ctrlthrd->signal != IM_NULL){
		isppwl_sig_deinit(ctrlthrd->signal);
		ctrlthrd->signal = IM_NULL;
	}
	if(ctrlthrd->lock != IM_NULL){
		isppwl_lock_deinit(ctrlthrd->lock);	
		ctrlthrd->lock = IM_NULL;
	}

	return IM_RET_OK;
}

static IM_RET ctrlthrdPutCtrlWord(IM_INT32 index, ctrl_word_t *cw)
{
	control_thread_t *ctrlthrd;
	IM_INFOMSG((IM_STR("%s(index=%d)"), IM_STR(_IM_FUNC_), index));
	
	if(gWorkModel->ctrlmdl.ctrlthrd[index] == IM_NULL){
		gWorkModel->ctrlmdl.ctrlthrd[index] = (control_thread_t *)isppwl_malloc(sizeof(control_thread_t));
		if(gWorkModel->ctrlmdl.ctrlthrd[index] == IM_NULL){
			IM_ERRMSG((IM_STR("malloc(ctrlthrd) failed")));
			return IM_RET_FAILED;
		}
		ctrlthrd = gWorkModel->ctrlmdl.ctrlthrd[index];
		isppwl_memset((void *)ctrlthrd, 0, sizeof(control_thread_t));
		ctrlthrd->index = index;

		if(ctrlthrdInit(index) != IM_RET_OK){
			IM_ERRMSG((IM_STR("ctrlthrdInit() failed")));
			isppwl_free(gWorkModel->ctrlmdl.ctrlthrd[index]);
			gWorkModel->ctrlmdl.ctrlthrd[index] = IM_NULL;
			return IM_RET_FAILED;
		}
	}

	ctrlthrd = gWorkModel->ctrlmdl.ctrlthrd[index];
	isppwl_lock(ctrlthrd->lock);
	im_list_put_back(ctrlthrd->cwlist, (void *)cw);
	if(im_list_size(ctrlthrd->cwlist) == 1){
		isppwl_sig_set(ctrlthrd->signal);
		ctrlthrd->sigHasSet = IM_TRUE;
	}
	isppwl_unlock(ctrlthrd->lock);

	return IM_RET_OK;
}

static IM_RET ctrlthrdCancelCtrlWord(IM_INT32 index, IM_INT32 id)
{
	ctrl_word_t *cw;
	control_thread_t *ctrlthrd = gWorkModel->ctrlmdl.ctrlthrd[index];
	IM_INFOMSG((IM_STR("%s(index=%d, id=%d)"), IM_STR(_IM_FUNC_), index, id));
	IM_ASSERT(ctrlthrd != IM_NULL);

	isppwl_lock(ctrlthrd->lock);
	if(id == ctrlthrd->execCtrlWordId){
		ctrlthrd->cancelStat = CANCEL_STAT_TRIGGER;
		while(ctrlthrd->cancelStat != CANCEL_STAT_DONE){
			isppwl_unlock(ctrlthrd->lock);	// give up lock, let ctrlthrdEntry execute.
			
			isppwl_msleep(5);

			isppwl_lock(ctrlthrd->lock);	// test again.
		}
	}else{
		cw = (ctrl_word_t *)im_list_begin(ctrlthrd->cwlist);	
		while(cw != IM_NULL){
			if(cw->id == id){
				im_list_erase(ctrlthrd->cwlist, cw);
				if(im_list_size(ctrlthrd->cwlist) == 0){
					isppwl_sig_reset(ctrlthrd->signal);
					ctrlthrd->sigHasSet = IM_TRUE;
				}
				break;
			}
			cw = (ctrl_word_t *)im_list_next(ctrlthrd->cwlist);
		}
	}
	ctrlthrd->cancelStat = CANCEL_STAT_DONE;
	isppwl_unlock(ctrlthrd->lock);

	return IM_RET_OK;
}

static IM_RET ctrlmodelInit(ctrlmodel_config_t *config)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(config->ctCnt <= CONTROL_THREAD_COUNT_MAX);
	isppwl_memcpy((void *)&gWorkModel->ctrlmdl.config, (void *)config, sizeof(ctrlmodel_config_t));
	return IM_RET_OK;
}

static IM_RET ctrlmodelDeinit(void)
{
	IM_INT32 index;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	for(index=0; index < gWorkModel->ctrlmdl.config.ctCnt; index++){
		if(gWorkModel->ctrlmdl.ctrlthrd[index] != IM_NULL){
			if(ctrlthrdDeinit(index) != IM_RET_OK){
				IM_ERRMSG((IM_STR("ctrlthrdDeinit() failed")));
			}
			isppwl_free((void *)gWorkModel->ctrlmdl.ctrlthrd[index]);
			gWorkModel->ctrlmdl.ctrlthrd[index] = IM_NULL;
		}
	}

	return IM_RET_OK;
}

static IM_RET ctrlmodelSendControl(ctrl_word_t *cw)
{
	IM_RET ret;
	IM_INT32 index = getCodeBoxIndex(cw->code);
	IM_INFOMSG((IM_STR("%s(code=0x%x, async=0x%x, id=%d, index=%d)"), IM_STR(_IM_FUNC_), cw->code, cw->async, cw->id, index));

	if(index == -1){
		IM_ERRMSG((IM_STR("Don't support code 0x%x"), cw->code));
		return IM_RET_FAILED;
	}

	if(cw->async == IM_TRUE){
		ret = ctrlthrdPutCtrlWord(index, cw);
	}else{
		do{
			ret = gWorkModel->ctrlmdl.config.control_exec[index](cw, IM_FALSE);
		}while(ret == IM_RET_RETRY);
	}
	return ret;
}

static IM_RET ctrlmodelCancelControl(IM_INT32 code, IM_INT32 id)
{
	IM_INT32 index = getCodeBoxIndex(code);
	IM_INFOMSG((IM_STR("%s(code=0x%x, id=%d, index=%d)"), IM_STR(_IM_FUNC_), code, id, index));

	if(index == -1){
		IM_ERRMSG((IM_STR("Don't support code 0x%x"), code));
		return IM_RET_FAILED;
	}

	return ctrlthrdCancelCtrlWord(index, id);
}


//
// camisp api.
//
IM_RET camisp_init(camdev_config_t *config)
{
	workmodel_config_t wmconfig;
	IM_INFOMSG((IM_STR("%s(name=%s, facing=%d)"), IM_STR(_IM_FUNC_), config->camsenName, config->facing));

	// create gWorkModel instance.
	IM_ASSERT(gWorkModel == IM_NULL);
	gWorkModel = (workmodel_t *)isppwl_malloc(sizeof(workmodel_t));
	if(gWorkModel == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(gWorkModel) failed")));
		return IM_RET_FAILED;
	}
	isppwl_memset((void *)gWorkModel, 0, sizeof(workmodel_t));

	gWorkModel->mpl = im_mpool_init((func_mempool_malloc_t)isppwl_malloc, (func_mempool_free_t)isppwl_free);
	if(gWorkModel->mpl == IM_NULL){
		IM_ERRMSG((IM_STR("im_mpool_init() failed")));
		goto Fail;
	}

	IM_ASSERT(workmodel_register_camapi(&gWorkModel->camIntfs) == IM_RET_OK);
	//
	wmconfig.smcfg.frame_done = frameDone;
	if(gWorkModel->camIntfs.camapi_init(config, &wmconfig) != IM_RET_OK){
		IM_ERRMSG((IM_STR("camapi_init() failed")));
		goto Fail;
	}
	gWorkModel->camInited = IM_TRUE;

	if(ctrlmodelInit(&wmconfig.cmcfg) != IM_RET_OK){
		IM_ERRMSG((IM_STR("ctrlmodelInit() failed")));
		goto Fail;
	}
	gWorkModel->ctrlmdlInited = IM_TRUE;

	if(evtqInit() != IM_RET_OK){
		IM_ERRMSG((IM_STR("evtqInit() failed")));
		goto Fail;
	}
	gWorkModel->evtqInited = IM_TRUE;

	return IM_RET_OK;
Fail:
	if(gWorkModel->evtqInited == IM_TRUE){
		evtqDeinit();
		gWorkModel->evtqInited = IM_FALSE;
	}
	if(gWorkModel->ctrlmdlInited == IM_TRUE){
		ctrlmodelDeinit();
		gWorkModel->ctrlmdlInited = IM_FALSE;
	}
	if(gWorkModel->camInited == IM_TRUE){
		gWorkModel->camIntfs.camapi_deinit();
		gWorkModel->camInited = IM_FALSE;
	}
	if(gWorkModel->mpl != IM_NULL){
		im_mpool_deinit(gWorkModel->mpl);
		gWorkModel->mpl = IM_NULL;
	}
	isppwl_free((void *)gWorkModel);
	gWorkModel = IM_NULL;

	return IM_RET_FAILED;
}

IM_RET camisp_deinit(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if(gWorkModel == IM_NULL){
		return IM_RET_OK;
	}

	// first stop camera.
	if(gWorkModel->camInited == IM_TRUE){
		gWorkModel->camIntfs.camapi_deinit();
		gWorkModel->camInited = IM_FALSE;
	}

	if(gWorkModel->evtqInited == IM_TRUE){
		evtqDeinit();
		gWorkModel->evtqInited = IM_FALSE;
	}
	if(gWorkModel->ctrlmdlInited == IM_TRUE){
		ctrlmodelDeinit();
		gWorkModel->ctrlmdlInited = IM_FALSE;
	}
	if(gWorkModel->mpl != IM_NULL){
		im_mpool_deinit(gWorkModel->mpl);
		gWorkModel->mpl = IM_NULL;
	}
	isppwl_free((void *)gWorkModel);
	gWorkModel = IM_NULL;

	return IM_RET_OK;
}

IM_RET camisp_ioctl(IM_INT32 cmd, void *p)
{
	IM_RET ret;
	IM_INFOMSG((IM_STR("%s(cmd=0x%x)"), IM_STR(_IM_FUNC_), cmd));

	if(cmd == CAMISP_IOCTL_CMD_SEND_CTR){
		ret = ctrlmodelSendControl((ctrl_word_t *)p);
	}else if(cmd == CAMISP_IOCTL_CMD_CANCEL_CTR){
		ret = ctrlmodelCancelControl(((ioctl_ds_cancel_control *)p)->code, ((ioctl_ds_cancel_control *)p)->id);
	}else{
		ret = gWorkModel->camIntfs.camapi_ioctl(cmd, p);
	}

	return ret;
}

IM_RET camisp_suspend(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(gWorkModel->camIntfs.camapi_suspend == IM_NULL){
		return IM_RET_OK;
	}
	return gWorkModel->camIntfs.camapi_suspend();
}

IM_RET camisp_resume(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(gWorkModel->camIntfs.camapi_resume == IM_NULL){
		return IM_RET_OK;
	}
	return gWorkModel->camIntfs.camapi_resume();
}

IM_RET camisp_wait_event(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return evtqWait();
}

IM_RET camisp_get_event(workmodel_event_t *evt)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return evtqGet(evt);
}


