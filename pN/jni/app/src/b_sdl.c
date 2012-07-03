#if 0
#include <pthread.h>

pthread_mutex_t* b_CreateMutex(void)
{
	pthread_mutexattr_t attr;
	pthread_mutex_t *id;

	id = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	if (!id) return NULL;    

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

	if (pthread_mutex_init(id, &attr) != 0) {
		return NULL;
	}

	return id;
}

void b_DestroyMutex(pthread_mutex_t* mutex)
{
	if (mutex) {
		pthread_mutex_destroy(mutex);
		free(mutex);
	}
}

int b_LockMutex(pthread_mutex_t* mutex)
{
	int retval;

	if (mutex == NULL) {
		return -1;
	}

	retval = 0;
	if (pthread_mutex_lock(mutex) < 0) {
		retval = -1;
	}
	return retval;
}

int b_UnlockMutex(pthread_mutex_t * mutex)
{
	int retval;

	if (mutex == NULL) {
		return -1;
	}

	retval = 0;
	if (pthread_mutex_unlock(mutex) < 0) {
		retval = -1;
	}

	return retval;
}

/* Create a condition variable */
pthread_cond_t *b_CreateCond(void)
{
	pthread_cond_t *cond;

	cond = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));

	if (!cond) return NULL;

	if (pthread_cond_init(cond, NULL) < 0) {
		free(cond);
		cond = NULL;
	}
	return (cond);
}

/* Destroy a condition variable */
void b_DestroyCond(pthread_cond_t* cond)
{
	if (cond) {
		pthread_cond_destroy(cond);
		free(cond);
	}
}

/* Restart one of the threads that are waiting on the condition variable */
int b_CondSignal(pthread_cond_t* cond)
{
	int retval;

	if (!cond) {
		return -1;
	}

	retval = 0;
	if (pthread_cond_signal(cond) != 0) {
		retval = -1;
	}
	return retval;
}

/* Wait on the condition variable, unlocking the provided mutex.
   The mutex must be locked before entering this function!
 */
int b_CondWait(pthread_cond_t* cond, pthread_mutex_t* mutex)
{
	int retval;

	if (!cond) {
		return -1;
	}

	retval = 0;
	if (pthread_cond_wait(cond, mutex) != 0) {
		retval = -1;
	}
	return retval;
}

/* List of signals to mask in the subthreads */
static const int sig_list[] = {
	SIGHUP, SIGINT, SIGQUIT, SIGPIPE, SIGALRM, SIGTERM, SIGCHLD, SIGWINCH,
	SIGVTALRM, SIGPROF, 0
};

pthread_t *b_CreateThread(void *(* fn) (void *), void *data)
{
	pthread_attr_t type;
	pthread_t *handle;

	handle = malloc(sizeof(pthread_t));
	if (!handle) return NULL;

	/* Set the thread attributes */
	if (pthread_attr_init(&type) != 0) {
		return NULL;
	}
	pthread_attr_setdetachstate(&type, PTHREAD_CREATE_JOINABLE);

	/* Create the thread and go! */
	if (pthread_create(handle, &type, fn, data) != 0) {
		return NULL;
	}

	return handle;
}

void SetupThread()
{
	int i;
	sigset_t mask;

	/* Mask asynchronous signals for this thread */
	sigemptyset(&mask);
	for (i = 0; sig_list[i]; ++i) {
		sigaddset(&mask, sig_list[i]);
	}
	pthread_sigmask(SIG_BLOCK, &mask, 0);
	//actual code logic

	pthread_exit((void *) 0);
}

void b_WaitThread(pthread_t *handle)
{
	pthread_join(*handle, 0);
}
#endif
