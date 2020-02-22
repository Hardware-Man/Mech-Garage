#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct lines
{
	unsigned long int tag;
	int valid;
	unsigned long int time;
}Lines;

Lines** cache;
Lines** prefetch_cache;


Lines** allocate_cache(int,int);
int log_2(int);
int check_hit(int,int,unsigned long int,int);
int check_prefetch_hit(int,int,unsigned long int,int,int);
void update_cache(int,int,unsigned long int,int);
void update_prefetch_cache(int,int,unsigned long int,int);
void free_cache(int);
void free_prefetch_cache(int);

Lines** allocate_cache(int s_num, int a_num){
	Lines** temp = malloc(s_num*sizeof(Lines*));
	for(int i = 0; i < s_num; i++){
		temp[i] = malloc(a_num*sizeof(Lines));
		for(int j = 0; j < a_num; j++){
			temp[i][j].valid = 0;
			temp[i][j].tag = 0;
			temp[i][j].time = 0;
		}
	}
	return temp;
}

int log_2(int num){
	int n = 0;
	while(num != 1){
		num >>= 1;
		n++;
	}
	return n;
}

int check_hit(int num_lines, int s_bits, unsigned long int t_bits, int mode){
	unsigned long int time_update = 0;
	if(mode == 1){
		for(int i = 0; i < num_lines; i++){
			if(time_update == 0){
				time_update = cache[s_bits][i].time;
			}
			if(time_update < cache[s_bits][i].time){
				time_update = cache[s_bits][i].time;
			}
		}
	}	
	for(int i = 0; i < num_lines; i++){
		if(cache[s_bits][i].valid == 0){
			break;
		}		
		if(cache[s_bits][i].tag == t_bits){
			if(mode == 1)cache[s_bits][i].time = time_update+0x1;
			return 1;
		}
	}
	return 0;
}

int check_prefetch_hit(int num_lines, int s_bits, unsigned long int t_bits, int p_check, int mode){
	unsigned long int time_update = 0;
	if(p_check == 0 && mode == 1){
		for(int i = 0; i < num_lines; i++){
			if(time_update == 0){
				time_update = prefetch_cache[s_bits][i].time;
			}
			if(time_update < prefetch_cache[s_bits][i].time){
				time_update = prefetch_cache[s_bits][i].time;
			}
		}
	}	
	for(int i = 0; i < num_lines; i++){
		if(prefetch_cache[s_bits][i].valid == 0){
			break;
		}		
		if(prefetch_cache[s_bits][i].tag == t_bits){
			if(p_check == 0 && mode == 1) prefetch_cache[s_bits][i].time = time_update+0x1;
			return 1;
		}
	}
	return 0;
}

void update_cache(int num_lines, int s_bits, unsigned long int t_bits, int mode){
	unsigned long int first_time = 0;
	int first_time_index = 0;
	unsigned long int last_time = 0;
	for(int i = 0; i < num_lines; i++){
		if(cache[s_bits][i].valid == 0){
			cache[s_bits][i].tag = t_bits;
			cache[s_bits][i].valid = 1;
			if(mode == 0) cache[s_bits][i].time = i + 0x1;
			else cache[s_bits][i].time = last_time + 0x1;
			return;
		}
		if(first_time == 0){
			first_time = cache[s_bits][i].time;
			first_time_index = i;
		}
		if(first_time > cache[s_bits][i].time){
			first_time = cache[s_bits][i].time;
			first_time_index = i;
		}
		if(last_time == 0){
			last_time = cache[s_bits][i].time;
		}
		if(last_time < cache[s_bits][i].time){
			last_time = cache[s_bits][i].time;
		}
	}
	cache[s_bits][first_time_index].tag = t_bits;
	cache[s_bits][first_time_index].time = last_time+0x1;
	return;
}

void update_prefetch_cache(int num_lines, int s_bits, unsigned long int t_bits, int mode){
	unsigned long int first_time = 0;
	int first_time_index = 0;
	unsigned long int last_time = 0;
	for(int i = 0; i < num_lines; i++){
		if(prefetch_cache[s_bits][i].valid == 0){
			prefetch_cache[s_bits][i].tag = t_bits;
			prefetch_cache[s_bits][i].valid = 1;
			if(mode == 0) prefetch_cache[s_bits][i].time = i + 0x1;
			else prefetch_cache[s_bits][i].time = last_time + 0x1;
			return;
		}
		if(first_time == 0){
			first_time = prefetch_cache[s_bits][i].time;
			first_time_index = i;
		}
		if(first_time > prefetch_cache[s_bits][i].time){
			first_time = prefetch_cache[s_bits][i].time;
			first_time_index = i;
		}
		if(last_time == 0){
			last_time = prefetch_cache[s_bits][i].time;
		}
		if(last_time < prefetch_cache[s_bits][i].time){
			last_time = prefetch_cache[s_bits][i].time;
		}
	}
	prefetch_cache[s_bits][first_time_index].tag = t_bits;
	prefetch_cache[s_bits][first_time_index].time = last_time+0x1;
	return;
}

void free_cache(int s_num){
	for(int i = 0; i < s_num; i++){
		free(cache[i]);
	}
	free(cache);
}

void free_prefetch_cache(int s_num){
	for(int i = 0; i < s_num; i++){
		free(prefetch_cache[i]);
	}
	free(prefetch_cache);
}

int main(int argc, char** argv){
	if(argc != 7){
		printf("error\n");
		exit(0);
	}

	int cache_size = atoi(argv[1]);
	int block_size = atoi(argv[2]);
	if(cache_size <= 0 || block_size <= 0 || block_size > cache_size || (block_size&(block_size-1)) != 0 || (cache_size&(cache_size-1)) != 0){
		printf("error\n");
		exit(0);
	}

	char* policy = argv[3];
	if(strcmp(policy,"fifo") != 0 && strcmp(policy,"lru") != 0){
		printf("error\n");
		exit(0);
	}

	int sets = 0;
	volatile int lines_per_set = 0;
	if(strcmp(argv[4],"direct") == 0){
		sets = cache_size/block_size;
		lines_per_set = 1;
	}
	else if(strcmp(argv[4],"assoc") == 0){
		sets = 1;
		lines_per_set = cache_size/block_size;
	}
	else if(strncmp(argv[4],"assoc:",sizeof(char)*6) == 0){
		int n;
		sscanf(argv[4], "assoc:%d", &n);
		if(n <= 0 || (block_size*n) > cache_size || (n&(n-1)) != 0){
			printf("error\n");
			exit(0);
		}
		sets = cache_size/(block_size*n);
		lines_per_set = n;
	}
	else{
		printf("error\n");
		exit(0);
	}

	int prefetch_size = atoi(argv[5]);
	if(prefetch_size < 0){
		printf("error\n");
		exit(0);
	}

	FILE* fp = fopen(argv[6], "r");
	if(fp == NULL){
		printf("error\n");
		exit(0);
	}

	cache = allocate_cache(sets,lines_per_set);
	prefetch_cache = allocate_cache(sets,lines_per_set);
	int set_log = log_2(sets);
	int block_size_log = log_2(block_size);
	int mask = (1<<set_log)-1;

	int reads = 0;
	int writes = 0;
	int hits = 0;
	int misses = 0;
	int prefetch_reads = 0;
	int prefetch_writes = 0;
	int prefetch_hits = 0;
	int prefetch_misses = 0;

	int mode_switch = 0;

	if(strcmp(policy,"lru") == 0){
		mode_switch = 1;
	}

	char action;
	unsigned long long int address;
	fscanf(fp,"%c %llx\n",&action,&address);
	while(action != '#'){
		int set_bits = (address>>block_size_log)&mask;
		unsigned long int tag_bits = (address>>block_size_log)>>set_log;
		if(check_hit(lines_per_set, set_bits, tag_bits, mode_switch) == 0){
			update_cache(lines_per_set, set_bits, tag_bits, mode_switch);
			if(action == 'R'){
				reads++;
				misses++;
			}
			if(action == 'W'){
				reads++;
				writes++;
				misses++;
			}
		}
		else{
			if(action == 'R'){
				hits++;
			}
			if(action == 'W'){
				writes++;
				hits++;
			}
		}
		if(check_prefetch_hit(lines_per_set, set_bits, tag_bits, 0, mode_switch) == 0){
			update_prefetch_cache(lines_per_set, set_bits, tag_bits, mode_switch);
			if(action == 'R'){
				prefetch_reads++;
				prefetch_misses++;
			}
			if(action == 'W'){
				prefetch_reads++;
				prefetch_writes++;
				prefetch_misses++;
			}
			unsigned long long int prefetch_address = address;
			for(int i = 0; i < prefetch_size; i++){
				prefetch_address = prefetch_address + block_size;
				set_bits = (prefetch_address>>block_size_log)&mask;
				tag_bits = (prefetch_address>>block_size_log)>>set_log;
				if(check_prefetch_hit(lines_per_set, set_bits, tag_bits, 1, mode_switch) == 0){
					update_prefetch_cache(lines_per_set, set_bits, tag_bits, mode_switch);
					prefetch_reads++;
				}
			}
		}
		else{
			if(action == 'R'){
				prefetch_hits++;
			}
			if(action == 'W'){
				prefetch_writes++;
				prefetch_hits++;
			}
		}
		fscanf(fp,"%c %llx\n",&action,&address);
	}

	printf("no-prefetch\n");
	printf("Memory reads: %d\n", reads);
	printf("Memory writes: %d\n", writes);
	printf("Cache hits: %d\n", hits);
	printf("Cache misses: %d\n", misses);
	printf("with-prefetch\n");
	printf("Memory reads: %d\n", prefetch_reads);
	printf("Memory writes: %d\n", prefetch_writes);
	printf("Cache hits: %d\n", prefetch_hits);
	printf("Cache misses: %d\n", prefetch_misses);
	fclose(fp);
	free_cache(sets);
	free_prefetch_cache(sets);

	return(0);
}