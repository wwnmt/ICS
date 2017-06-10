#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;

	/* TODO: Add more members if necessary */
	char *expr;
	uint32_t oldValue;
	uint32_t newValue;

} WP;

void show_watchpoint();
WP* new_wp();
void free_wp(WP*);
WP* delete_watchpoint(int);
void delete_all_watchpoint();
#endif
