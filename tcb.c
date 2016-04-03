#include "general.h"
#include "tcb.h"

uint16_t asynch_presc;

void add_task(void(*e)(void)) {
	task_t * task_new = norm_task_constr(e);
	/*	Current task indicator will be assigned in other
	 *	part of kernel (in executing block). 		*/
	if(!norm_task_list.first) {
		norm_task_list.first = norm_task_list.last = task_new;
		task_new->next = task_new->prev = NULL;
		return;
	}
	/* 	Adding new tasks on the end of tasklist, to firstly
	 *	execute tasks, which were added earlier				 		*/
	norm_task_list.last->next = task_new;
	task_new->prev = norm_task_list.last;
	task_new->next = NULL;
	norm_task_list.last = task_new;
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
	TCCR3B |= (1<<CS32) | (1<<CS30);	//	Prescaler 1024. Timer tick frequency 7812,5 MHz
	asynch_presc = 1024;
}

void add_asynch_task(void(*e)(void), uint16_t time, bool importance) {

	asynch_task_t* task_new = asynch_task_constr(e, time, importance);

	uint16_t OCR_content;

	//	Empty asynchronous task list
	if(!asynch_task_list.first) {

		OCRA_U = (float)F_CPU / (float)asynch_presc * (float)task_new->time / 1000.0 + 0.5;

		asynch_task_list.first = asynch_task_list.last = task_new;

		task_new->next = NULL;

		A_TCNT_CLR;
		A_INT_EN;

		return;
	}

	uint16_t TCNT_content;
	uint32_t time_to_next_int;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {

		TCNT_content = TCNT_U;
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
			last regarding only tasks existing on tasklist	*/
		task_new->time = task_new->time - sum_time;
		asynch_task_list.last->next = task_new;
		asynch_task_list.last = task_new;
		task_new->next = NULL;
	}
}

void delete_asynch_task() {
	//	Just one element on the list
	if(asynch_task_list.first == asynch_task_list.last) {
		asynch_task_list.first = NULL;
		free(asynch_task_list.last);
		asynch_task_list.last = NULL;
		return;
	}

	asynch_task_t * task_temp = asynch_task_list.first;
	asynch_task_list.first = task_temp->next;
	free(task_temp);
}

//	Implementation assumming that interrupt can't be interrupted
ISR(TIMER3_COMPA_vect) {

	//	If it is important to execute a task function just after the given time:
	if(asynch_task_list.first->important) {
		asynch_task_list.first->exec();
	} else {
		//	If it is important just to execute the task function after the given time:
		add_task(asynch_task_list.first->exec);
	}

	//	Delete asynchronous task from the tasklist
	delete_asynch_task();

	//	Initialization of next interrupt on asynchronous task
	if(asynch_task_list.first) {

		//	If there will be a situation, when two tasks should be executed at the same time
		while(asynch_task_list.first->time == 0) {

			//	If it is important to execute a task function just after the given time:
			if(asynch_task_list.first->important) {
				asynch_task_list.first->exec();
			} else {
				//	If it is important just to execute the task function after the given time:
				add_task(asynch_task_list.first->exec);
			}

			//	Delete asynchronous task from the tasklist
			delete_asynch_task();

			if(!asynch_task_list.first) {
				A_INT_DIS;
				return;
			}
		}

		OCRA_U = (float)F_CPU / (float)asynch_presc *
				(float)asynch_task_list.first->time / 1000.0 + 0.5;
		A_INT_EN;

	} else {

		A_INT_DIS;

	}
}

