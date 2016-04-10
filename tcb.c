#include "general.h"
#include "tcb.h"

volatile uint16_t asynch_presc;
volatile task_list_t norm_task_list;
volatile asynch_task_list_t asynch_task_list;

void init() {
	norm_task_list.first = norm_task_list.last = NULL;
	asynch_task_list.first = asynch_task_list.last = NULL;
}

void add_task(void(*e)(void)) {
	task_t * task_new = norm_task_constr(e);
	/*	Current task indicator will be assigned in other
	 *	part of kernel (in executing block). 		*/
	if(!norm_task_list.first) {
		norm_task_list.first = norm_task_list.last = task_new;
		task_new->next = task_new->prev = NULL;
		return;
	}
	/* 	Adding new tasks on the end of task list, to firstly
	 *	execute tasks, which were added earlier				 		*/
	norm_task_list.last->next = task_new;
	task_new->prev = norm_task_list.last;
	task_new->next = NULL;
	norm_task_list.last = task_new;
}

void add_task_pre(bool(*pre)(void), void(*e)(void)) {
	//	Execute pre-procedure
	if(pre()) add_task(e);
}

void delete_task(task_t* task) {
	//Last task on the list
	if(norm_task_list.first == task && norm_task_list.last == task) {
		norm_task_list.first = norm_task_list.last = NULL;
		free(task);
		return;
	}
	//Deleting lastly added task on the list
	if(norm_task_list.first == task) {
		task->next->prev = NULL;
		norm_task_list.first = task->next;
		free(task);
		return;
	}
	//Deleting the oldest task on the list
	if(norm_task_list.last == task) {
		norm_task_list.last = task->prev;
		norm_task_list.last->next = NULL;
		free(task);
		return;
	}
	//The rest of cases
	task->prev->next = task->next;
	task->next->prev = task->prev;
	free(task);
	return;
}

void asynch_app_timer_init() {
	TCCR3B |= (1<<WGM32);				//	CTC mode
	TCCR3B |= (1<<CS32) | (1<<CS30);	//	Prescaler 1024. Timer clock frequency 7812,5 MHz
	asynch_presc = 1024;
}


/*	This implementation should be in *.asm. E.g. Adding new asynchronous
	task, which makes only incrementation, every millisecond in program, leads to
	executing of only 780 tasks per second.								*/	  
void add_asynch_task(void(*e)(void), uint16_t time, bool importance) {
	
	uint16_t TCNT_content;
	//	Read and remember TCNT content firstly to ensure the most possible consistency
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		TCNT_content = TCNT_U;
	}
	
	asynch_task_t* task_new = asynch_task_constr(e, time, importance);

	//	Empty asynchronous task list
	if(!asynch_task_list.first) {

		OCRA_U = (float)F_CPU / (float)asynch_presc * (float)task_new->time / 1000.0 + 0.5;

		asynch_task_list.first = asynch_task_list.last = task_new;

		task_new->next = NULL;
		
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			A_TCNT_CLR;
			A_INT_EN;	
		}

		return;
	}

	uint16_t OCR_content;
	uint32_t time_to_next_int;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {

		OCR_content = OCRA_U;

		time_to_next_int = (float)asynch_presc / (float)F_CPU *
			(float)(OCR_content - TCNT_content) * 1000.0 + 0.5;

		//	If new task should be executed before any task listed
		if(task_new->time < time_to_next_int) {

			OCRA_U = (float)F_CPU / (float)asynch_presc * (float)task_new->time / 1000.0 + 0.5;

			//	Inside every object the differential time between consequent tasks is stored
			asynch_task_list.first->time = time_to_next_int - task_new->time;

			task_new->next = asynch_task_list.first;
			asynch_task_list.first = task_new;

			A_TCNT_CLR;
			A_INT_EN;

			return;
		}

		asynch_task_t* asynch_temp = asynch_task_list.first;
		uint32_t sum_time = time_to_next_int;

		//	Adding object between first and last object on the list
		while(asynch_temp->next) {

			if(sum_time + asynch_temp->next->time > task_new->time) {
				task_new->time = task_new->time - sum_time;
				asynch_temp->next->time = asynch_temp->next->time - task_new->time;

				task_new->next = asynch_temp->next;
				asynch_temp->next = task_new;
				return;
			}

			sum_time += asynch_temp->next->time;
			asynch_temp = asynch_temp->next;
		}

		/*	If the new task should be executed as
			last regarding only tasks existing on task list	*/
		task_new->time = task_new->time - sum_time;
		asynch_task_list.last->next = task_new;
		asynch_task_list.last = task_new;
		task_new->next = NULL;
	}
}

asynch_task_t * throw_asynch_task() {
	
	asynch_task_t * temp_a_task;
	
	//	Just one element on the list
	if(asynch_task_list.first == asynch_task_list.last) {
		temp_a_task = asynch_task_list.first;
		asynch_task_list.first = NULL;
		asynch_task_list.last = NULL;
		return temp_a_task;
	}

	temp_a_task = asynch_task_list.first;
	asynch_task_list.first = temp_a_task->next;
	return temp_a_task;
}

//	Implementation assuming that interrupt can't be interrupted
ISR(TIMER3_COMPA_vect) {
	
	asynch_task_t * act_asynch_task = throw_asynch_task();
	
	//	If it is important to execute a task function just after the given time:
	if(act_asynch_task->important) {
		act_asynch_task->exec();
	} else {
		//	If it is important just to execute the task function after the given time:
		add_task(act_asynch_task->exec);
	}

	//	Delete asynchronous task from the task list
	free(act_asynch_task);

	//	Initialization of next interrupt on asynchronous task
	if(asynch_task_list.first) {

		//	If there will be a situation, when two tasks should be executed at the same time
		while(asynch_task_list.first->time == 0) {

			asynch_task_t * act_asynch_task = throw_asynch_task();

			//	If it is important to execute a task function just after the given time:
			if(act_asynch_task->important) {
				act_asynch_task->exec();
				} else {
				//	If it is important just to execute the task function after the given time:
				add_task(act_asynch_task->exec);
			}

			//	Delete asynchronous task from the task list
			free(act_asynch_task);

			if(!asynch_task_list.first) {
				A_INT_DIS;
				return;
			}
		}

		OCRA_U = (float)F_CPU / (float)asynch_presc * (float)asynch_task_list.first->time / 1000.0 + 0.5;
		A_INT_EN;

	} else {

		A_INT_DIS;

	}
}

