#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
#include "cpu/reg.h"
#include "monitor/elf.h"
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);
uint32_t dram_read(hwaddr_t,size_t);
/* We use the ``readline'' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_si(char *args) {
	char *arg = strtok(NULL," ");
	int N, i;
	if(arg == NULL) {
		cpu_exec(1);
	}
	else {
		N = atol(arg);
		for(i = 0; i < N; i++){
			cpu_exec(1);
		}
	}
	return 0;
}

static int cmd_info(char *args) {
	char *arg = strtok(NULL," ");
	int i;
	switch(*arg)
	{
		case 'r':
			for(i = 0; i < 8; i++){
				printf("%s:\t0x%08x\n",regsl[i],cpu.gpr[i]._32);
				printf("\t%s:\t0x%04x\n",regsw[i],cpu.gpr[i]._32 & 0xffff);
				if(i < 4){
					printf("\t\t%s:\t0x%02x\n",regsb[i],cpu.gpr[i]._32 & 0xff);
					printf("\t\t%s:\t0x%02x\n",regsb[i+4],(cpu.gpr[i]._32 >> 0) & 0xff);
				}
			}
			break;
		case 'b':
			show_watchpoint();
	}
	return 0;
}

static int cmd_x(char *args) {
	int n, i, j;
	char expre[32];
	if(args == NULL){
		printf("please input argumengts\n");
		return 0;
	}
	if(sscanf(args,"%d %s",&n,expre) != 2){
		printf("arguments errors");
		return 0;
	}
	bool success = true;
	hwaddr_t address = expr(expre,&success);
	if(!success)
		return 0;
	for (i = 0;i < n;i++){
		printf("%x:\t",address);
		for (j = 0;j < 4;j++){
			uint8_t memory = dram_read(address,8);
			printf("%02x ",memory);
			address +=1;
		}
		printf("\n");
	}
	return 0;
}

static int cmd_p(char *args) 
{
	char *arg = strtok(NULL," ");
	bool success=true;
	uint32_t num = expr(arg,&success);
	printf("decimal:%d\thex:0X%08x\n",num,num);
	return 0;
}

static int cmd_w(char *args)
{
	char *arg = strtok(NULL," ");
	WP* wp = NULL;
	bool *success = (bool*)true;
	if (arg == NULL){
		printf("please into expression!\n");
	}
	else{
		wp = new_wp();
		wp->expr = (char*)malloc(sizeof(char)*10);
		strcpy(wp->expr,arg);
		printf("set watchpoint at %s\n",wp->expr);	
		wp->oldValue = expr(arg,success);
	}
	return 0;
}

static int cmd_d(char *args)
{
	char *arg = strtok(NULL," ");
	WP* wp = NULL;
	int n;
	if (arg == NULL){
		printf("please input right number!\n");
	}
	else{
		n = atoi(arg);
		if(strcmp("-all",arg) == 0){
			delete_all_watchpoint();
		}
		else if(n == 0){
			printf("empty input!\n");
		}
		else{
			wp = delete_watchpoint(n);
			free_wp(wp);
		}
	}
	return 0;
}

static int cmd_bt(char *args)
{
	backtrace();
	return 0;
}

void cache_debug(hwaddr_t addr);
static int cmd_cache(char *args)
{
	char *arg=strtok(NULL," ");
	if (arg == NULL){
		printf("please input a number!\n");
		return 0;
	}
	else {
		int addr = 0;
		bool success = true;
		addr=expr(arg,&success);
		if(success == false){
			return 0;
		}
		else{
			cache_debug(addr);
			return 0;
		}
	}
}
void page_debug(lnaddr_t);
lnaddr_t seg_translate(swaddr_t addr, uint8_t sreg);
static int cmd_page(char *args){
	if(args == NULL){
		printf("please input a number!\n");
		return 0;
	}
	char expre[32];
	sscanf(args,"%s",expre);
	swaddr_t addr=0;
	bool  success=true;
	addr=expr(expre,&success);
	if(!success)
		return 0;
	lnaddr_t lnaddr = seg_translate(addr,R_SS);
	printf("lnaddr = 0x%x\n",lnaddr);
	page_debug(lnaddr);
	return 0;
}
static int cmd_help(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "Let the program siingle-step instructions to suspend after N when N is not given,the default is 1", cmd_si},	
	{ "info", "print all registers", cmd_info},
	{ "x", "scan memory", cmd_x},
	{ "p", "count expression", cmd_p},
	{ "w", "watchpoint", cmd_w},
	{ "d", "n delete watchpoint -all delete all watchpoint", cmd_d},
	{ "bt", "printf stackfarme", cmd_bt},
	{ "cache", "test whether addr is in the cache", cmd_cache},
	{ "page", "page swaddr",cmd_page}
	/* TODO: Add more commands */
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
