//  g++ -o test_cnd test_cnd.cpp  -lpthread
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define LOG_D(fmt, ...) printf("[%s  %d]: LOG "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define LOG_F(fmt, ...) printf("[%s  %d]: ERR "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);

typedef enum {
    FOTA_NULL = 0,
    FOTA_CHECK,
    FOTA_DOWNLOAD,
    FOTA_UPLOAD_LOG_1,
    FOTA_UPGRADE,
    FOTA_UPLOAD_LOG_2
} FOTA_OPERATION_TYPE;

FOTA_OPERATION_TYPE g_fota_flag = FOTA_NULL;
pthread_mutex_t g_fota_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_fota_cond = PTHREAD_COND_INITIALIZER;
// char g_fota_data[HTTP_BUF_LEN] = {0};

void sleepdown_fota_thread()
{
    if(FOTA_NULL == g_fota_flag) {
        LOG_D("fota thread is alread sleep");
        return;
    }
    pthread_mutex_lock(&g_fota_mut);
    LOG_D("sleepdown fota thread");
    g_fota_flag = FOTA_NULL;
    pthread_mutex_unlock(&g_fota_mut);
}

void* thread_fota_proc(void* param)
{
    pthread_detach(pthread_self());
    while(1) {
        pthread_mutex_lock(&g_fota_mut);
        while(FOTA_NULL == g_fota_flag) {
            LOG_D("fota thread suspend");
            pthread_cond_wait(&g_fota_cond, &g_fota_mut);
        }
        pthread_mutex_unlock(&g_fota_mut);
        LOG_D("fota thread run");

        if((FOTA_CHECK == g_fota_flag) || (FOTA_UPLOAD_LOG_1 == g_fota_flag) || (FOTA_UPLOAD_LOG_2 == g_fota_flag)) {
            LOG_D("send_req_to_fota_server()");
        }
        else if(FOTA_DOWNLOAD == g_fota_flag) {
            LOG_D("fota_download_file()");
        }
        sleepdown_fota_thread();
    }
    return NULL;
}

void wakeup_fota_thread(FOTA_OPERATION_TYPE flag)
{
    if(FOTA_NULL != g_fota_flag) {
        LOG_D("fota thread is alread running");
        return;
    }

    if((FOTA_CHECK != flag) && (FOTA_DOWNLOAD != flag) && (FOTA_UPLOAD_LOG_1 != flag) && (FOTA_UPLOAD_LOG_2 != flag)) {
        LOG_D("wakeup thread while operation is not recognizated");
        return;
    }

    pthread_mutex_lock(&g_fota_mut);
    LOG_D("wake up fota thread");
    
    if(FOTA_CHECK == flag) {
        LOG_D("clear_fota_devices() and create_fota_check_req_data()");
    }
    else if((FOTA_UPLOAD_LOG_1 == flag) || (FOTA_UPLOAD_LOG_2 == flag)) {
    	LOG_D("=======start UPLOAD LOG======\n");
        LOG_D("create_fota_log_req_data()");
    	LOG_D("=======end UPLOAD LOG======\n");
    }
    g_fota_flag = flag;
    pthread_cond_signal(&g_fota_cond);
    pthread_mutex_unlock(&g_fota_mut);
}

int main()
{
    int rc = 0;
    pthread_t thread_fota;

    rc = pthread_create(&thread_fota, NULL, thread_fota_proc, NULL);
    if(0 !=  rc) {
        LOG_F("fota check thread create fails");
        return rc;
    }
    sleep(1);
    LOG_D("wake up fota thread in main");
    wakeup_fota_thread(FOTA_CHECK);
    return 0;
}


