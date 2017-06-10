#include "common.h"
#include <stdlib.h>
#include <elf.h>
#include "monitor/elf.h"
#include "cpu/helper.h"
#include "cpu/reg.h"
char *exec_file = NULL;

static char *strtab = NULL;
static Elf32_Sym *symtab = NULL;
static int nr_symtab_entry;

typedef struct{
	swaddr_t prev_ebp;
	swaddr_t ret_addr;
	uint32_t args[4];
}PartOfStackFarme;

void load_elf_tables(int argc, char *argv[]) {
	int ret;
	Assert(argc == 2, "run NEMU with format 'nemu [program]'");
	exec_file = argv[1];

	FILE *fp = fopen(exec_file, "rb");
	Assert(fp, "Can not open '%s'", exec_file);

	uint8_t buf[sizeof(Elf32_Ehdr)];
	ret = fread(buf, sizeof(Elf32_Ehdr), 1, fp);
	assert(ret == 1);

	/* The first several bytes contain the ELF header. */
	Elf32_Ehdr *elf = (void *)buf;
	char magic[] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};

	/* Check ELF header */
	assert(memcmp(elf->e_ident, magic, 4) == 0);		// magic number
	assert(elf->e_ident[EI_CLASS] == ELFCLASS32);		// 32-bit architecture
	assert(elf->e_ident[EI_DATA] == ELFDATA2LSB);		// littel-endian
	assert(elf->e_ident[EI_VERSION] == EV_CURRENT);		// current version
	assert(elf->e_ident[EI_OSABI] == ELFOSABI_SYSV || 	// UNIX System V ABI
			elf->e_ident[EI_OSABI] == ELFOSABI_LINUX); 	// UNIX - GNU
	assert(elf->e_ident[EI_ABIVERSION] == 0);			// should be 0
	assert(elf->e_type == ET_EXEC);						// executable file
	assert(elf->e_machine == EM_386);					// Intel 80386 architecture
	assert(elf->e_version == EV_CURRENT);				// current version


	/* Load symbol table and string table for future use */

	/* Load section header table */
	uint32_t sh_size = elf->e_shentsize * elf->e_shnum;
	Elf32_Shdr *sh = malloc(sh_size);
	fseek(fp, elf->e_shoff, SEEK_SET);
	ret = fread(sh, sh_size, 1, fp);
	assert(ret == 1);

	/* Load section header string table */
	char *shstrtab = malloc(sh[elf->e_shstrndx].sh_size);
	fseek(fp, sh[elf->e_shstrndx].sh_offset, SEEK_SET);
	ret = fread(shstrtab, sh[elf->e_shstrndx].sh_size, 1, fp);
	assert(ret == 1);

	int i;
	for(i = 0; i < elf->e_shnum; i ++) {
		if(sh[i].sh_type == SHT_SYMTAB && 
				strcmp(shstrtab + sh[i].sh_name, ".symtab") == 0) {
			/* Load symbol table from exec_file */
			symtab = malloc(sh[i].sh_size);
			fseek(fp, sh[i].sh_offset, SEEK_SET);
			ret = fread(symtab, sh[i].sh_size, 1, fp);
			assert(ret == 1);
			nr_symtab_entry = sh[i].sh_size / sizeof(symtab[0]);
		}
		else if(sh[i].sh_type == SHT_STRTAB && 
				strcmp(shstrtab + sh[i].sh_name, ".strtab") == 0) {
			/* Load string table from exec_file */
			strtab = malloc(sh[i].sh_size);
			fseek(fp, sh[i].sh_offset, SEEK_SET);
			ret = fread(strtab, sh[i].sh_size, 1, fp);
			assert(ret == 1);
		}
	}

	free(sh);
	free(shstrtab);

	assert(strtab != NULL && symtab != NULL);

	fclose(fp);
}

uint32_t find_str(char *args)
{
	int i;
	for(i=0;i<nr_symtab_entry;i++){
		if(!strcmp(strtab+symtab[i].st_name,args)){
			return symtab[i].st_value;
		}
	}
	return 0;
}

int backtrace()
{
	int i, j;
	PartOfStackFarme stackfarme;
	if (cpu.eip == 0x100000){
		printf("No Stack.\n");
		return 0;
	}
	stackfarme.ret_addr = cpu.ebp;
	stackfarme.prev_ebp = swaddr_read(stackfarme.ret_addr,4,R_SS);
	printf("prev_ebp: 0x%x\n",stackfarme.prev_ebp);
	printf("ret_addr: 0x%x\n",stackfarme.ret_addr);
	for (i = 0;i < 4; i++){
		stackfarme.args[i] = swaddr_read(stackfarme.ret_addr+8+i*4,4,R_SS);
	}
	for (i = nr_symtab_entry-1; i>=0; i--){
		if (symtab[i].st_info == 0x12){
			if (stackfarme.ret_addr){
				printf("%s \n",strtab+symtab[i].st_name);
				printf("prev_ebp: 0x%x\n",stackfarme.prev_ebp);
				for (j = 0;j < 4;j++){
					printf("argument:%d 0x%x\t",j,stackfarme.args[j]);
				}
				printf("\n");
				stackfarme.ret_addr = stackfarme.prev_ebp;
				printf("ret_addr: 0x%x\n",stackfarme.ret_addr);
				stackfarme.prev_ebp = swaddr_read(stackfarme.ret_addr,4,3);
			}
		}
	}
	return 0;
}