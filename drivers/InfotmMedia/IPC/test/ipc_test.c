/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
**      
** Revision History: 
** ----------------- 
** v1.0.1	leo@2012/04/28: first commit.
**
*****************************************************************************/ 

#include <InfotmMedia.h>
#include <IM_ipcapi.h>


#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"IPCTST_I:"
#define WARNHEAD	"IPCTST_W:"
#define ERRHEAD		"IPCTST_E:"
#define TIPHEAD		"IPCTST_T:"

#define PRINTMSG(str)	printf("IPCTEST:"); printf str; printf("\r\n")

#define TB_ARGP_KEY_FUNCTION		0x1000
#define TB_ARGP_KEY_KEYSTR		0x1001
#define TB_ARGP_KEY_ROLE		0x1002
#define TB_ARGP_KEY_COUNT		0x1003

#define ROLE_OWNER	0
#define ROLE_SLAVE	1

static argp_key_table_entry_t gArgKeyTable[] = 
{
	{TB_ARGP_KEY_FUNCTION, (IM_TCHAR)'f', IM_STR("func"), IM_STR("[sync][shm][pipe]. default [unknwon].")},
	{TB_ARGP_KEY_KEYSTR, (IM_TCHAR)'k', IM_STR("keystr"), IM_STR("key-string. default [unknwon].")},
	{TB_ARGP_KEY_ROLE, (IM_TCHAR)'r', IM_STR("role"), IM_STR("[owner][slave]. default [unknown].")},
	{TB_ARGP_KEY_COUNT, (IM_TCHAR)'c', IM_STR("count"), IM_STR("session count. default [1].")},
};

typedef struct{
	IM_INT32	func;	// IPC_TYPE_xxx.
	IM_TCHAR	keystr[IPC_KEYSTR_MAX_LEN - 2];
	IM_INT32	role;	// ROLE_xxx.
	IM_INT32	count;
}ipc_tb_config_t;

static ipc_tb_config_t gTbConfig;

static void menu_help()
{
	IM_TCHAR optstr[256] = {0};
	IM_INT32 i;

	PRINTMSG((IM_STR("#############################################################")));
	for(i=0; i<sizeof(gArgKeyTable) / sizeof(gArgKeyTable[0]); i++){
		memset((void *)optstr, 0, sizeof(optstr));
		if(gArgKeyTable[i].shortOpt != 0){
			sprintf(optstr, "-%c", gArgKeyTable[i].shortOpt);// migrate has problem???
			//PRINTMSG((IM_STR("-%c "), gArgKeyTable[i].shortOpt));
		}
		if(gArgKeyTable[i].longOpt != IM_NULL){
			sprintf(optstr, "%s\t--%s", optstr, gArgKeyTable[i].longOpt);// migrate has problem???
			//PRINTMSG((IM_STR("--%s"), gArgKeyTable[i].longOpt));
		}
		PRINTMSG((IM_STR("%s: %s"), optstr, gArgKeyTable[i].desc));
	}
	PRINTMSG((IM_STR("#############################################################")));
}

static argp_key_table_entry_t * get_keytable_entry(IM_INT32 key)
{
	IM_INT32 i;
	for(i=0; i<sizeof(gArgKeyTable) / sizeof(gArgKeyTable[0]); i++){
		if(key == gArgKeyTable[i].key){
			return &gArgKeyTable[i];
		}
	}

	return IM_NULL;
}

static void tb_config_init(IM_INT32 argc, IM_TCHAR *argv[])
{
	IM_TCHAR *funcString = IM_NULL;

	// init default config.
	gTbConfig.func = -1;
	gTbConfig.keystr[0] = (IM_TCHAR)'\0';
	gTbConfig.role = -1;
	gTbConfig.count = 1;

	//
	if(argp_get_key_string(get_keytable_entry(TB_ARGP_KEY_FUNCTION), &funcString, argc, argv) == IM_RET_OK){
		if(im_oswl_strcmp(funcString, IM_STR("sync")) == 0){
			gTbConfig.func = IPC_TYPE_SYNC;
		}else if(im_oswl_strcmp(funcString, IM_STR("shm")) == 0){
			gTbConfig.func = IPC_TYPE_SHM;
		}else if(im_oswl_strcmp(funcString, IM_STR("pipe")) == 0){
			gTbConfig.func = IPC_TYPE_PIPE;
		}
	}
	if(gTbConfig.func == -1){
		menu_help();
		return;
	}

	if(argp_get_key_string(get_keytable_entry(TB_ARGP_KEY_KEYSTR), &funcString, argc, argv) == IM_RET_OK){
		im_oswl_strcpy(gTbConfig.keystr, funcString);
	}
	if(gTbConfig.keystr[0] == (IM_TCHAR)'\0'){
		menu_help();
		return;
	}

	if(argp_get_key_string(get_keytable_entry(TB_ARGP_KEY_ROLE), &funcString, argc, argv) == IM_RET_OK){
		if(im_oswl_strcmp(funcString, IM_STR("owner")) == 0){
			gTbConfig.role = ROLE_OWNER;
		}else if(im_oswl_strcmp(funcString, IM_STR("slave")) == 0){
			gTbConfig.role = ROLE_SLAVE;
		}
	}
	if(gTbConfig.role == -1){
		menu_help();
		return;
	}
	
	argp_get_key_int(get_keytable_entry(TB_ARGP_KEY_COUNT), &gTbConfig.count, argc, argv);
	
	//
	IM_INFOMSG((IM_STR("func=%d"), gTbConfig.func));
	IM_INFOMSG((IM_STR("keystr=%s"), gTbConfig.keystr));
	IM_INFOMSG((IM_STR("role=%d"), gTbConfig.role));
	IM_INFOMSG((IM_STR("count=%d"), gTbConfig.count));
}

static IM_INT32 get_random_between(IM_INT32 min, IM_INT32 max)
{
	IM_INT32 val;
	do{
		val = rand();
		if((val < min) || (val > max)){
			val %= (max - min + 1);
			val += min;
		}
	}while((val < min) || (val > max));
	return val;
}

static IM_RET sync_test(void)
{
	IM_RET ret = IM_RET_FAILED;
	IM_INT32 cnt = 0;
	IPCSYNC_HANDLE hdl_p = IM_NULL;
	IPCSYNC_HANDLE hdl_v = IM_NULL;
	IM_TCHAR keystr[IPC_KEYSTR_MAX_LEN + 1];

	im_oswl_strcpy(keystr, gTbConfig.keystr);
	im_oswl_strcat(keystr, IM_STR("_p"));
	if(ipcsync_init(&hdl_p, keystr) != IM_RET_OK){
		IM_ERRMSG((IM_STR("ipcsync_init(p) failed")));
		goto Fail;
	}

	im_oswl_strcpy(keystr, gTbConfig.keystr);
	im_oswl_strcat(keystr, IM_STR("_v"));
	if(ipcsync_init(&hdl_v, keystr) != IM_RET_OK){
		IM_ERRMSG((IM_STR("ipcsync_init(v) failed")));
		goto Fail;
	}

	while(cnt++ < gTbConfig.count){
		if(gTbConfig.role == ROLE_OWNER){
			PRINTMSG((IM_STR("owner: start sync_set@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcsync_set(hdl_p) != IM_RET_OK){
				IM_ERRMSG((IM_STR("owner: ipcsync_set() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("owner: end sync_set@%d"), im_oswl_GetSystemTimeMs()));

			im_oswl_sleep(get_random_between(1, 1000));

			PRINTMSG((IM_STR("owner: start sync_wait@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcsync_wait(hdl_v, -1) != IM_RET_OK){
				IM_ERRMSG((IM_STR("owner: ipcsync_wait() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("owner: end sync_wait@%d"), im_oswl_GetSystemTimeMs()));
		}else{
			PRINTMSG((IM_STR("slave: start sync_wait@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcsync_wait(hdl_p, -1) != IM_RET_OK){
				IM_ERRMSG((IM_STR("slave: ipcsync_wait() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("slave: end sync_wait@%d"), im_oswl_GetSystemTimeMs()));

			im_oswl_sleep(get_random_between(1, 1000));

			PRINTMSG((IM_STR("slave: start sync_set@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcsync_set(hdl_v) != IM_RET_OK){
				IM_ERRMSG((IM_STR("slave: ipcsync_set() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("slave: end sync_set@%d"), im_oswl_GetSystemTimeMs()));
		}
	}

	ret = IM_RET_OK;
Fail:
	if(hdl_p){
		ipcsync_deinit(hdl_p);
	}
	if(hdl_v){
		ipcsync_deinit(hdl_v);
	}
	return ret;	
}

static IM_RET pipe_test(void)
{
	IM_RET ret = IM_RET_FAILED;
	IM_INT32 cnt = 0;
	IPCPIPE_HANDLE hdl_p = IM_NULL;
	IPCPIPE_HANDLE hdl_v = IM_NULL;
	IM_TCHAR keystr[IPC_KEYSTR_MAX_LEN + 1];
	IM_TCHAR msg[128];

	im_oswl_strcpy(keystr, gTbConfig.keystr);
	im_oswl_strcat(keystr, IM_STR("_p"));
	if(ipcpipe_init(&hdl_p, keystr, (gTbConfig.role==ROLE_OWNER)?IPC_PIPE_USAGE_WRITE:IPC_PIPE_USAGE_READ, sizeof(msg)) != IM_RET_OK){
		IM_ERRMSG((IM_STR("ipcpipe_init(p) failed")));
		goto Fail;
	}

	im_oswl_strcpy(keystr, gTbConfig.keystr);
	im_oswl_strcat(keystr, IM_STR("_v"));
	if(ipcpipe_init(&hdl_v, keystr, (gTbConfig.role==ROLE_OWNER)?IPC_PIPE_USAGE_READ:IPC_PIPE_USAGE_WRITE, sizeof(msg)) != IM_RET_OK){
		IM_ERRMSG((IM_STR("ipcpipe_init(v) failed")));
		goto Fail;
	}

	while(cnt++ < gTbConfig.count){
		if(gTbConfig.role == ROLE_OWNER){
			sprintf(msg, IM_STR("owner's msg %d"), cnt);
			PRINTMSG((IM_STR("owner: start pipe_write@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcpipe_write(hdl_p, (void *)msg, sizeof(msg)) != IM_RET_OK){
				IM_ERRMSG((IM_STR("owner: ipcpipe_write() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("owner: end pipe_write@%d"), im_oswl_GetSystemTimeMs()));

			im_oswl_sleep(get_random_between(1, 1000));

			PRINTMSG((IM_STR("owner: start pipe_read@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcpipe_read(hdl_v, (void *)msg, sizeof(msg), -1) != IM_RET_OK){
				IM_ERRMSG((IM_STR("owner: ipcpipe_read() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("owner: end pipe_read@%d, msg is [%s]"), im_oswl_GetSystemTimeMs(), msg));
		}else{
			PRINTMSG((IM_STR("slave: start pipe_read@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcpipe_read(hdl_p, (void *)msg, sizeof(msg), -1) != IM_RET_OK){
				IM_ERRMSG((IM_STR("slave: ipcpipe_read() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("slave: end pipe_read@%d, msg is [%s]"), im_oswl_GetSystemTimeMs(), msg));

			im_oswl_sleep(get_random_between(1, 1000));

			sprintf(msg, IM_STR("slave's msg %d"), cnt);
			PRINTMSG((IM_STR("slave: start pipe_write@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcpipe_write(hdl_v, (void *)msg, sizeof(msg)) != IM_RET_OK){
				IM_ERRMSG((IM_STR("slave: ipcpipe_write() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("slave: end pipe_write@%d"), im_oswl_GetSystemTimeMs()));
		}
	}

	ret = IM_RET_OK;
Fail:
	if(hdl_p){
		ipcsync_deinit(hdl_p);
	}
	if(hdl_v){
		ipcsync_deinit(hdl_v);
	}
	return ret;	

}

static IM_RET shm_test(void)
{
	IM_RET ret = IM_RET_FAILED;
	IM_INT32 cnt = 0;
	IPCSYNC_HANDLE hdl_sync_p = IM_NULL;
	IPCSYNC_HANDLE hdl_sync_v = IM_NULL;
	IPCSHM_HANDLE hdl_shm = IM_NULL;
	IM_TCHAR *buffer;
	IM_TCHAR keystr[IPC_KEYSTR_MAX_LEN + 1];

	if(ipcshm_init(&hdl_shm, gTbConfig.keystr, 16*1024) != IM_RET_OK){
		IM_ERRMSG((IM_STR("ipcshm_init() failed")));
		goto Fail;
	}

	im_oswl_strcpy(keystr, gTbConfig.keystr);
	im_oswl_strcat(keystr, IM_STR("_p"));
	if(ipcsync_init(&hdl_sync_p, keystr) != IM_RET_OK){
		IM_ERRMSG((IM_STR("ipcsync_init() failed")));
		goto Fail;
	}
	
	im_oswl_strcpy(keystr, gTbConfig.keystr);
	im_oswl_strcat(keystr, IM_STR("_v"));
	if(ipcsync_init(&hdl_sync_v, keystr) != IM_RET_OK){
		IM_ERRMSG((IM_STR("ipcsync_init() failed")));
		goto Fail;
	}

	while(cnt++ < gTbConfig.count){
		if(gTbConfig.role == ROLE_OWNER){
			//
			PRINTMSG((IM_STR("owner: start shm_lock@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcshm_lock(hdl_shm, (void **)&buffer, -1) != IM_RET_OK){
				IM_ERRMSG((IM_STR("owner: ipcshm_lock() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("owner: end shm_lock@%d"), im_oswl_GetSystemTimeMs()));

			sprintf(buffer, "owner's msg %d", cnt);

			PRINTMSG((IM_STR("owner: start shm_unlock@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcshm_unlock(hdl_shm) != IM_RET_OK){
				IM_ERRMSG((IM_STR("owner: ipcshm_unlock() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("owner: end shm_unlock@%d"), im_oswl_GetSystemTimeMs()));

			PRINTMSG((IM_STR("owner: start sync_set@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcsync_set(hdl_sync_p) != IM_RET_OK){
				IM_ERRMSG((IM_STR("owner: ipcsync_set() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("owner: end sync_set@%d"), im_oswl_GetSystemTimeMs()));

			//
			im_oswl_sleep(get_random_between(1, 1000));

			//
			PRINTMSG((IM_STR("owner: start sync_wait@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcsync_wait(hdl_sync_v, -1) != IM_RET_OK){
				IM_ERRMSG((IM_STR("owner: ipcsync_wait() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("owner: end sync_wait@%d"), im_oswl_GetSystemTimeMs()));

			PRINTMSG((IM_STR("owner: start shm_lock@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcshm_lock(hdl_shm, (void **)&buffer, -1) != IM_RET_OK){
				IM_ERRMSG((IM_STR("owner: ipcshm_lock() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("owner: end shm_lock@%d, msg is [%s]"), im_oswl_GetSystemTimeMs(), buffer));

			PRINTMSG((IM_STR("owner: start shm_unlock@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcshm_unlock(hdl_shm) != IM_RET_OK){
				IM_ERRMSG((IM_STR("owner: ipcshm_unlock() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("owner: end shm_unlock@%d"), im_oswl_GetSystemTimeMs()));
		}else{
			PRINTMSG((IM_STR("slave: start sync_wait@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcsync_wait(hdl_sync_p, -1) != IM_RET_OK){
				IM_ERRMSG((IM_STR("slave: ipcsync_wait() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("slave: end sync_wait@%d"), im_oswl_GetSystemTimeMs()));

			PRINTMSG((IM_STR("slave: start shm_lock@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcshm_lock(hdl_shm, (void **)&buffer, -1) != IM_RET_OK){
				IM_ERRMSG((IM_STR("slave: ipcshm_lock() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("slave: end shm_lock@%d, msg is [%s]"), im_oswl_GetSystemTimeMs(), buffer));

			PRINTMSG((IM_STR("slave: start shm_unlock@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcshm_unlock(hdl_shm) != IM_RET_OK){
				IM_ERRMSG((IM_STR("slave: ipcshm_unlock() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("slave: end shm_unlock@%d"), im_oswl_GetSystemTimeMs()));

			//
			im_oswl_sleep(get_random_between(1, 1000));

			//
			PRINTMSG((IM_STR("slave: start shm_lock@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcshm_lock(hdl_shm, (void **)&buffer, -1) != IM_RET_OK){
				IM_ERRMSG((IM_STR("slave: ipcshm_lock() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("slave: end shm_lock@%d"), im_oswl_GetSystemTimeMs()));

			sprintf(buffer, "slave's msg %d", cnt);

			PRINTMSG((IM_STR("slave: start shm_unlock@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcshm_unlock(hdl_shm) != IM_RET_OK){
				IM_ERRMSG((IM_STR("slave: ipcshm_unlock() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("slave: end shm_unlock@%d"), im_oswl_GetSystemTimeMs()));

			PRINTMSG((IM_STR("slave: start sync_set@%d"), im_oswl_GetSystemTimeMs()));
			if(ipcsync_set(hdl_sync_v) != IM_RET_OK){
				IM_ERRMSG((IM_STR("slave: ipcsync_set() failed")));
				goto Fail;
			}
			PRINTMSG((IM_STR("slave: end sync_set@%d"), im_oswl_GetSystemTimeMs()));
		}
	}

	ret = IM_RET_OK;
Fail:
	if(hdl_shm){
		ipcshm_deinit(hdl_shm);
	}
	if(hdl_sync_p){
		ipcsync_deinit(hdl_sync_p);
	}
	if(hdl_sync_v){
		ipcsync_deinit(hdl_sync_v);
	}
	return ret;	

}

IM_INT32 main(IM_INT32 argc, IM_TCHAR *argv[])
{
	IM_TCHAR verString[IM_VERSION_STRING_LEN_MAX + 1];

	//
	if(argc < 3){
		menu_help();
		return 0;
	}

	//
	tb_config_init(argc, argv);

	//
	PRINTMSG((IM_STR("#######################++IPC TEST++##########################")));

	ipc_version(verString);
	IM_INFOMSG((IM_STR("InfotmMediaIPC version: %s"), verString));

	srand(im_oswl_GetSystemTimeMs());

	//
	if(gTbConfig.func == IPC_TYPE_SYNC){
		IM_JIF(sync_test());
	}else if(gTbConfig.func == IPC_TYPE_PIPE){
		IM_JIF(pipe_test());
	}else if(gTbConfig.func == IPC_TYPE_SHM){
		IM_JIF(shm_test());
	}
	PRINTMSG((IM_STR("#######################--IPC TEST-- OK##########################")));
	return 0;
Fail:
	PRINTMSG((IM_STR("#######################--IPC TEST-- FAIL##########################")));
	return -1;
}


