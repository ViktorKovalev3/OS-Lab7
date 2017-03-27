#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For file mode constants */
#include <mqueue.h>

using namespace std;

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

struct thread_arg{
    bool  thread_ended = 0; //isn't end
    mqd_t mq_desc;
};


static void * func1_thread(void *vp_arg){
    thread_arg* arg = (thread_arg*) vp_arg;
    char msg = '1';
    while(!(arg->thread_ended)){
        //writer
        if (
            mq_send(
                arg->mq_desc,
                &msg,
                sizeof(msg),
                1
                )
        ) handle_error("mq_send");
        sleep(1);
    }
    pthread_exit((void*) "Writer thread ended");
}

int main(void)
{
    printf("Start writing!\n");
    pthread_t thread1; thread_arg thread1_arg;

    //Message Queue section
    mq_attr mq_attributes;
    mq_attributes.mq_flags   = O_NONBLOCK;   /* Flags: 0 or O_NONBLOCK */
    mq_attributes.mq_maxmsg  = 10;           /* Max. # of messages on queue */
    mq_attributes.mq_msgsize = sizeof(char); /* Max. message size (bytes) */
    mq_attributes.mq_curmsgs = 0;            /* # of messages currently in queue */
    mqd_t mq_desc = mq_open("/MessageQueue",
                            O_CREAT | O_WRONLY,
                            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
                            &mq_attributes
                            );
    if (mq_desc == -1)
            handle_error("mq_open");

    //Thread section
    thread1_arg.mq_desc = mq_desc;
    if ( pthread_create( &thread1, NULL, func1_thread, &thread1_arg ) )
        handle_error("pthread_create");

    getchar();
    thread1_arg.thread_ended = 1;

    //Exit section
    char* exit_thread1_code;
    if ( pthread_join( thread1, (void**) &exit_thread1_code ) )
            return 1;
    printf("\n%s\n", exit_thread1_code);
    if (mq_close(mq_desc)) handle_error("mq_close");
    if (mq_unlink("/MessageQueue")) handle_error("mq_unlink");
    return EXIT_SUCCESS;
}
