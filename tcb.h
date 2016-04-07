#ifndef _TCB_H
#define _TCB_H


//	Used timer registers
#define TCNT_U TCNT3
#define OCRA_U OCR3A

//	Interrupt on overflow enable/disable
#define A_INT_EN (TIMSK3 |= (1<<TOIE3))
#define A_INT_DIS (TIMSK3 &= ~(1<<TOIE3))

//	Clearing TCNT register
#define A_TCNT_CLR	(TCNT3 = 0)

/*	Task list record for doubly listed list of task
	with function indicator to function to execute	*/
typedef struct task {
	void(*exec)(void);
	struct task *next;
	struct task *prev;
}task_t;

/*	Asynchrounous task type with time variable, where time after which
 * 	the task will be executed counting from last asynchronous task execution is stored and
 * 	boolean variable, which indicates if the task code should be executed inside interrupt
 * 	(can't be interrupted - so is important) or should be just added to normal task list
 * 	(can't be executed before the interrupt).
 * 	Singly linked list implemented											*/
typedef struct asynch_task {
	struct asynch_task *next;
	void(*exec)(void);
	uint16_t time;
	bool important;
}asynch_task_t;

/* Task list for normal tasks	*/
typedef struct task_list {
	task_t * first;
	task_t * current;
	task_t * last;
}task_list_t;


/*	Should implement configuration file where asynchronous task
 * 	handling option will be added				*/
/* 	Task list for asynchronous tasks	*/
typedef struct a_task_list {
	asynch_task_t * first;
	asynch_task_t * last;
}asynch_task_list_t;

//	Global task lists
extern volatile task_list_t norm_task_list;
extern volatile asynch_task_list_t asynch_task_list;

//	Timer - for asynchronous operation - prescaler
extern uint16_t asynch_presc;

//	Initialization of objects
inline task_t* norm_task_constr(void(*e)(void)) {
	task_t * task_new = (task_t*) malloc (sizeof(task_t));
	task_new->exec = e;
	return task_new;
}

inline asynch_task_t* asynch_task_constr(void(*e)(void), uint16_t t, bool i) {
	asynch_task_t *a = (asynch_task_t*) malloc (sizeof(asynch_task_t));
	a->exec = e;
	a->important = i;
	a->time = t;
	return a;
}

//	First create a task to add it to the list
void add_task(void(*e)(void));

//	Task to delete should be indicated by argument
void delete_task(task_t* task);

//	Time to execute should be provided in miliseconds
void add_asynch_task(void(*e)(void), uint16_t time, bool importance);

/*	The method will look different for different AVRs.
	Timer3 of ATmega1284P is used in this application.	*/
void asynch_app_timer_init();

//	Always first task from the list deleted
void delete_asynch_task();


#endif
