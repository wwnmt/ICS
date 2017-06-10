#include "nemu.h"
#include <stdlib.h>
#include <elf.h>
#include "monitor/elf.h"
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

int get_reg_val(char* reg);

enum {
	NOTYPE = 256,NUM,NUM_16, EQ,lshift,rshift,ENQ,deref,neg,REG,str

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},				
	{"\\$[a-z]+", REG},
	{"\\+\\+",'p'},
	{"\\+" , '+'},                  
	{"0x[0-9a-f]+",NUM_16},
	{">=",'>'},
	{"<=",'<'},
	{"!=",ENQ},
	{"!",'!'},
	{"\\|\\|",'|'},
	{"-", '-'},
	{"\\*", '*'},
	{"/",'/'},
	{"[0-9]+", NUM},
	{"\\(", '('},
	{"\\)",')'},
	{"<<",lshift},
	{">>",rshift},
	{"&&",'&'},
	{"[a-zA-Z_]+",str},
	{"==", EQ}						
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
  */
void init_regex( ) {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
 		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
 	}
} 

typedef struct token {
	int type;
	char str[32];
}  Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

     	while(e[position] != '\0') {
		/* Try all rules one by one. */
     		for(i = 0; i < NR_REGEX; i ++) {
  	 		if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array ``tokens''. For certain 
				 * types of tokens, some extra actions should be performed.
 				 */

 				switch(rules[i].token_type) {
					case NOTYPE: break;
					case NUM: break;
					case '+': break;
					case '-': break;
					case '*': break;
					case '/': break;
					case '(': break;
					case ')': break;
					case '&': break;
					case '|': break;
					case '!': break;
					case lshift: break;
					case rshift: break;
					case '>': break;
					case '<': break;
					case ENQ: break;
					case EQ: break;
					case 'p': break;
					case 'm': break;
					case NUM_16: break;
					case REG: break;
					case str: break;
					default: break; //panic("please implement me");
     				}
				if(rules[i].token_type != NOTYPE){
					tokens[nr_token].type = rules[i].token_type;
					memcpy(tokens[nr_token].str,substr_start,substr_len);
				//	printf("|%s|\n",tokens[nr_token].str);
					nr_token++;
			 	} 
				break;
    			}
   		}
 
		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
  		}
  	}

	return true; 
 }   

uint32_t eval (int ,int );
bool check_parentheses(Token *,int ,int );
uint32_t dominant(Token *,int ,int );
uint32_t expr(char *e, bool *success) {
  	if(!make_token(e)) {
		*success = false;
		return 0;
 	}
	/* TODO: Insert codes to evaluate the expression. */
	int i;
	for(i=0; i < nr_token-1; i++)
	{
		if(tokens[i].type == '*' && (i == 0 || (tokens[i-1].type != NUM && tokens[i-1].type != NUM_16 && tokens[i-1].type != ')')))
	tokens[i].type = deref;

		if(tokens[i].type == '-' && (i == 0 || (tokens[i-1].type != NUM && tokens[i-1].type != NUM_16 && tokens[i-1].type != ')')))
	tokens[i].type = neg;
	}
	uint32_t num=eval(0,nr_token-1);
	
	for(i=0;i<32;i++)
	{
		memset(tokens[i].str,0,32);
	}
	return num;
 }

bool check_parentheses(Token *tokens,int p,int q)
{
 	bool flag=false;
	bool flag2=true;
	int num=0;
	int i;
	for(i=p;i<=q;i++)
	{
		if(flag2==false && num==0 && i!=q)
			return false;
		if(tokens[i].type=='(') 
			num--;
		if(tokens[i].type==')'){
			num++;
			if(i!=q) flag2=false;
		}
	}
 	if((tokens[p].type=='(') && (tokens[q].type==')') && num==0 ){
		flag=true;
	}
	return flag;

}

int cmp[11][11]=
{
	{1,1,1,1,1,1,1,1,1,1,0},
	{0,1,1,1,1,1,1,1,1,1,0},
	{0,0,0,1,1,1,1,1,1,1,0},
	{0,0,0,0,1,1,1,1,1,1,0},
	{0,0,0,0,0,1,1,1,1,1,0},
	{0,0,0,0,0,0,1,1,1,1,0},
	{0,0,0,0,0,0,0,1,1,1,0},
	{0,0,0,0,0,0,0,0,1,1,0},
	{0,0,0,0,0,0,0,0,0,1,0},
	{0,0,0,0,0,0,0,0,0,0,0},
	{1,1,1,1,1,1,1,1,1,1,0},
}; 

int GetNum(int c){
	switch(c)
   	{
		case neg: return 0; break;

		case deref: return 1; break;

		case '!': return 2; break;
		case 'p': return 2; break;
		case 'm': return 2; break;
		
		case '*': return 3; break;
		case '/': return 3; break;
		 
		case '+': return 4; break;
		case '-': return 4; break;
		
		case lshift: return 5; break;
		case rshift: return 5; break;
		
		case '>': return 6; break;
		case '<': return 6; break;
		
		case ENQ: return 7; break;
		case EQ: return 7; break;
		
		case '&': return 8; break;
		
		case '|': return 9; break;
		
		case '(': return 10; break;
 		case ')': return 10; break;
		default: assert(0);
 	}
 }

uint32_t dominant(Token *token,int p,int q){
	int op=q;
	int i;
	int deep=0;
	if(tokens[q].type==')')
		deep++;
	for(i=q-1;i>=p;i--)
   	{
		if(tokens[i].type==NUM||tokens[i].type==NUM_16||tokens[i].type==str)
			continue;
		if(tokens[op].type==NUM||tokens[op].type==NUM_16||tokens[op].type==str)
			op=i;
		if(tokens[i].type==')')
			deep++;
		if(tokens[i].type=='(')
			deep--;
		if(deep==0 && (tokens[i].type!=NUM) && op!=i)
  		{
			if(cmp[GetNum(tokens[op].type)][GetNum(tokens[i].type)]==1)
				op=i;
		}
 	}
	return op;
  }

uint32_t eval(int p,int q)
{ 
	uint32_t result = 0;
	if(p > q){
		printf("Error!\n");
	}
	else if(p==q){
		if(tokens[p].type==REG)
		{
			return get_reg_val(tokens[p].str+1);
		}
		if(tokens[p].type==str){
			return find_str(tokens[p].str);
		}
		if(tokens[p].type==NUM){
			result = atoi(tokens[p].str);}
		else 
			result = strtol(tokens[p].str,NULL,16);
	}
	else if(check_parentheses(tokens,p,q)==true){
		return eval(p+1,q-1);
	}
 	else{
 		int op;
		op = dominant(tokens,p,q);
		uint32_t result1,result2;
		if(op==p||op==q)
		{
			switch(tokens[op].type){
				case neg: return 0-eval(op+1,q);
				case deref:return result1=swaddr_read(eval(op+1,q),4,R_DS);
				case '!': return !eval(op+1,q);
				case 'p': return eval(op+1,q)+1;
				case 'm': return eval(op+1,q)-1;
			}
 		}
 		result1 = eval(p,op-1);
		result2 = eval(op+1,q);
 		switch(tokens[op].type){
			case '+': return result1+result2; break;
			case '-': return result1-result2; break;
			case '*': return result1*result2; break;
			case '/': return result1/result2; break;
			case lshift: return result1 << result2; break;
			case rshift: return result1 >> result2; break;
			case '&': return result1 & result2;
			case '|': return result1 || result2;
			case '>': return result1 >= result2;
			case '<': return result1 <= result2;
			case ENQ: return result1 != result2;
			case EQ:  return result1 == result2;
			default: assert(0); 
	 	}

 	}
 	return result;
}

int get_reg_val(char *reg)
{
	int i;
	for(i=0;i<8;i++)
	{
		if(strcmp(regsl[i],reg) == 0){
			return cpu.gpr[i]._32;
		}
		if(strcmp(regsw[i],reg) == 0){
			return cpu.gpr[i]._16;
		}
		if(strcmp(regsb[i],reg) == 0){
			return reg_b(i);
		}
	}

	if(strcmp(reg,"eip") == 0){
		return cpu.eip;
	}
	return 0;
}
