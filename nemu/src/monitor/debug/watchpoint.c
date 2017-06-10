#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include<stdlib.h>
#define NR_WP 32

static WP wp_list[NR_WP];
static WP *head, *free_;
void init_wp_list() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_list[i].NO = i;
		wp_list[i].expr = NULL;
		wp_list[i].newValue = 0;
		wp_list[i].oldValue = 0;
		wp_list[i].next = &wp_list[i + 1];
	}
	wp_list[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_list;
}

WP* new_wp()
{
	WP* wp;
	if (free_ != NULL){
		
		wp = free_;
		if (free_->next == NULL){
			free_ = NULL;
		}
		else{
			free_ = free_->next;
		}
		if (head == NULL){
			head = wp;
			wp->next = NULL;
		}
		else{
			wp->next = head;
			head = wp;
		}
	}
	else{
		assert(0);
	}
	return wp;
}

void free_wp(WP *wp)
{
	if(wp == head){
		head = head->next;
	}
	else{
		WP* pre = head;
		while(pre->next != wp){
			pre = pre->next;
		}
		pre->next = wp->next;
		wp->next = NULL;
		pre = NULL;
		free(pre);
	}
	wp = NULL;
	free(wp);	
}

bool show_state()
{
	WP* a;
	a = head;
	bool *success = (bool *)true;
	bool state = false;
	while(a != NULL){
		a->newValue = expr(a->expr,success);
		if (a->oldValue != a->newValue){
			state = true;
			printf("here is a watchpoint!\n");
			printf("No.%d watchpoint :%s\n",a->NO,a->expr);
			printf("old value is :%d\n",a->oldValue);
			printf("old value is :%d\n",a->newValue);
		}
	}
	return state;
}

void show_watchpoint()
{
	WP* a;
	a = head;
	if (a == NULL){
		printf("None of watchpoints are used\n");
	}
	while(a !=NULL){
		printf("No.%d watchpoint: %s 's value is 0x%08x\n",a->NO+1,a->expr,a->oldValue);
		a = a->next;
	}
}

WP* delete_watchpoint(int n)
{
	WP* wp;
	wp = &wp_list[n-1];
	return wp;
}

void delete_all_watchpoint()
{
	WP* wp1;
    WP*	wp2;
	wp1 = head;
	while(wp1 != NULL){
		wp2 = wp1;
		wp1 = wp1->next;
		free_wp(wp2);
	}
	init_wp_list();
	printf("all watchpoints are clear!\n");
}


/* TODO: Implement the functionality of watchpoint */
