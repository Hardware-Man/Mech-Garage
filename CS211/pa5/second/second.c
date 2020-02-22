#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct varnode{
	char* id;
	int* bit_vals;
}Varnode;

typedef struct savenode{
	char save_operation[500];
	int valid;
	Savenode* next;
}Savenode;

Varnode* inputs;
Varnode* outputs;
Varnode* tempvars;

int input_size;
int tempvar_size;
int output_size;

void allocate_input(int);
void insert_tempvar(int,char*);
void allocate_output();
int* give_bit_vals(char*);
int* basic_gates(int,char*,char*, int);
void decode_op(int,int,int,char**,char**);
void multiplex_op(int,int,int,char**,char**,char*);
void print_input_ouput(int);
void free_input();
void free_tempvar();
void free_output();

void allocate_input(int m){
	inputs = malloc(input_size*sizeof(Varnode));
	for(int i = 0; i < input_size; i++){
		inputs[i].bit_vals = malloc(m*sizeof(int));
		int curr_bit = 0;
		int flip = 0;
		int temp = m>>(i+1);
		for(int j = 0; j < m; j++){
			if(j != 0 && j % temp == 0){
				flip^=1;
				if(flip == 1) curr_bit^=1;
			}
			inputs[i].bit_vals[j] = curr_bit;
		}
	}
}

void insert_tempvar(int m, char* var_id){
	if(tempvar_size == 0){
		tempvars = malloc(sizeof(Varnode));
		tempvars[0].id = malloc(strlen(var_id)+1);
		strcpy(tempvars[0].id, var_id);
		tempvars[0].bit_vals = malloc(m*sizeof(int));
		return;
	}
	tempvars = realloc(tempvars, (tempvar_size+1)*sizeof(Varnode));
	tempvars[tempvar_size].id = malloc(strlen(var_id)+1);
	strcpy(tempvars[tempvar_size].id, var_id);
	tempvars[tempvar_size].bit_vals = malloc(m*sizeof(int));
}

void allocate_output(){
	outputs = malloc(output_size*sizeof(Varnode));
}

int* give_bit_vals(char* test){
	int* res;
	if(test[0] >= 'A' && test[0] <= 'Z'){
		for(int i = 0; i < input_size; i++){
			if(strcmp(inputs[i].id, test) == 0){
				res = inputs[i].bit_vals;
				break;
			}
		}
	}
	else{
		for(int i = 0; i < tempvar_size; i++){
			if(strcmp(tempvars[i].id, test) == 0){
				res = tempvars[i].bit_vals;
				break;
			}
		}
	}
	return res;
}

int* basic_gates(int g, char* a, char* b, int m){
	int* res = malloc(m*sizeof(int));
	int* a_bits;
	int a_bit;
	int* b_bits;
	int b_bit;
	int code = 0;
	if(a[0] == '0' || a[0] == '1'){
		a_bit = atoi(a);
	}
	else{
		a_bits = give_bit_vals(a);
		code++;
	}
	if(b[0] == '0' || b[0] == '1'){
		b_bit = atoi(b);
	}
	else{
		b_bits = give_bit_vals(b);
		code+=2;
	}
	switch(code)
	{
		case 0:
			for(int i = 0; i < m; i++){
				switch(g)
				{
					case 1:
						res[i] = a_bit & b_bit; 
						continue;
					case 2:
						res[i] = a_bit | b_bit;
						continue;
					case 3:
						res[i] = 1^(a_bit & b_bit);
						continue;
					case 4:
						res[i] = 1^(a_bit | b_bit);
						continue;
					case 5:
						res[i] = a_bit ^ b_bit;
						continue;
					case 6:
						res[i] = 1^(a_bit ^ b_bit);
						continue;
					default:
						continue;
				}
			}
			break;
		case 1:
			for(int i = 0; i < m; i++){
				switch(g)
				{
					case 1:
						res[i] = a_bits[i] & b_bit; 
						continue;
					case 2:
						res[i] = a_bits[i] | b_bit;
						continue;
					case 3:
						res[i] = 1^(a_bits[i] & b_bit);
						continue;
					case 4:
						res[i] = 1^(a_bits[i] | b_bit);
						continue;
					case 5:
						res[i] = a_bits[i] ^ b_bit;
						continue;
					case 6: 
						res[i] = 1^(a_bits[i] ^ b_bit);
						continue;
					default:
						continue;
				}
			}
			break;
		case 2:
			for(int i = 0; i < m; i++){
				switch(g)
				{
					case 1:
						res[i] = a_bit & b_bits[i]; 
						continue;
					case 2:
						res[i] = a_bit | b_bits[i];
						continue;
					case 3:
						res[i] = 1^(a_bit & b_bits[i]);
						continue;
					case 4:
						res[i] = 1^(a_bit | b_bits[i]);
						continue;
					case 5:
						res[i] = a_bit ^ b_bits[i];
						continue;
					case 6:
						res[i] = 1^(a_bit ^ b_bits[i]);
						continue;
					default:
						continue;
				}
			}
			break;
		case 3:
			for(int i = 0; i < m; i++){
				switch(g)
				{
					case 1:
						res[i] = a_bits[i] & b_bits[i]; 
						continue;
					case 2:
						res[i] = a_bits[i] | b_bits[i];
						continue;
					case 3:
						res[i] = 1^(a_bits[i] & b_bits[i]);
						continue;
					case 4:
						res[i] = 1^(a_bits[i] | b_bits[i]);
						continue;
					case 5:
						res[i] = a_bits[i] ^ b_bits[i];
						continue;
					case 6:
						res[i] = 1^(a_bits[i] ^ b_bits[i]);
						continue;
					default:
						continue;
				}
			}
			break;
		default:
			break;
	}

	return res;
}

void decode_op(int num_in, int dec_bit_size, int m, char** a_ids, char** b_ids){
	int* out_index = malloc(dec_bit_size*sizeof(int));
	for(int i = 0; i < dec_bit_size; i++){
		if(b_ids[i][0] >= 'a' && b_ids[i][0] <= 'z'){
			insert_tempvar(m, b_ids[i]);
			tempvar_size++;
			out_index[i] = tempvar_size-1;
		}
		else{
			for(int j = 0; j < output_size; j++){
				if(strcmp(b_ids[i], outputs[j].id) == 0){
					out_index[i] = j;
					break;
				}
			}
		}
	}

	int**a = malloc(num_in*sizeof(int*));
	for(int i = 0; i < num_in; i++){
		a[i] = give_bit_vals(a_ids[i]);
	}
	for(int i = 0; i < m; i++){
		int gray_seq = 0;
		for(int j = 0; j < num_in; j++){
			gray_seq+=(a[j][i]<<(num_in-j-1));
		}
		for(int j = 0; j < dec_bit_size; j++){
			int dec_bits = j^(j>>1);
			if(dec_bits == gray_seq){
				if(b_ids[j][0] >= 'a' && b_ids[j][0] <= 'z'){
					tempvars[out_index[j]].bit_vals[i] = 1;
				}
				else{
					outputs[out_index[j]].bit_vals[i] = 1;
				}
			}
			else{
				if(b_ids[j][0] >= 'a' && b_ids[j][0] <= 'z'){
					tempvars[out_index[j]].bit_vals[i] = 0;
				}
				else{
					outputs[out_index[j]].bit_vals[i] = 0;
				}
			}
		}
	}
	free(a);
	free(out_index);
}

void multiplex_op(int num_in, int num_select, int m, char** select_ids, char** i_ids, char* o_id){
	int** selectors = malloc(num_select*sizeof(int*));
	for(int i = 0; i < num_select; i++){
		selectors[i] = give_bit_vals(select_ids[i]);
	}

	int* in_type = malloc(num_in*sizeof(int));
	int** in_bits = malloc(num_in*sizeof(int*));
	for(int i = 0; i < num_in; i++){
		if(i_ids[i][0] != '0' && i_ids[i][0] != '1'){
			in_type[i] = 2;
			in_bits[i] = give_bit_vals(i_ids[i]);
		}
		else if(i_ids[i][0] == 0){
			in_type[i] = 0;
		}
		else{
			in_type[i] = 1;
		}
	}

	int out_index;
	int t_or_o = 1;
	if(o_id[0] >= 'A' && o_id[0] <= 'Z') t_or_o = 0;
	if(t_or_o){
		insert_tempvar(m, o_id);
		tempvar_size++;
		out_index = tempvar_size-1;
	}
	else{
		for(int i = 0; i < output_size; i++){
			if(strcmp(o_id, outputs[i].id) == 0){
				out_index = i;
				break;
			}
		}
	}

	for(int i = 0; i < m; i++){
		int gray_seq = 0;
		for(int j = 0; j < num_select; j++){
			if(select_ids[j][0] == '0'){}
			else if(select_ids[j][0] == '1'){
				gray_seq+=(1<<(num_select-1-j));
			}
			else{
				gray_seq+=(selectors[j][i]<<(num_select-1-j));
			}
		}
		int in_index = 0;
		for(; gray_seq > 0; gray_seq = gray_seq>>1){
			in_index ^= gray_seq;
		}
		if(t_or_o){
			if(in_type[in_index] == 2){
				tempvars[out_index].bit_vals[i] = in_bits[in_index][i];
			}
			else if(in_type[in_index] == 0){
				tempvars[out_index].bit_vals[i] = 0;
			}
			else{
				tempvars[out_index].bit_vals[i] = 1; 
			}
		}
		else{
			if(in_type[in_index] == 2){
				outputs[out_index].bit_vals[i] = in_bits[in_index][i];
			}
			else if(in_type[in_index] == 0){
				outputs[out_index].bit_vals[i] = 0;
			}
			else{
				outputs[out_index].bit_vals[i] = 1; 
			}
		}
	}

	free(selectors);
	free(in_type);
	free(in_bits);
}

void print_input_ouput(int m){
	for(int i = 0; i < m; i++){
		for(int j = 0; j < input_size; j++){
			printf("%d ", inputs[j].bit_vals[i]);
		}
		for(int k = 0; k < output_size-1; k++){
			printf("%d ", outputs[k].bit_vals[i]);
		}
		printf("%d\n", outputs[output_size-1].bit_vals[i]);
	}
}

void free_input(){
	for(int i = 0; i < input_size; i++){
		free(inputs[i].bit_vals);
	}
	free(inputs);
}

void free_tempvar(){
	for(int i = 0; i < tempvar_size; i++){
		free(tempvars[i].bit_vals);
	}
	free(tempvars);
}

void free_output(){
	for(int i = 0; i < output_size; i++){
		free(outputs[i].bit_vals);
	}
	free(outputs);
}

int main(int argc, char** argv){
	FILE* fp = fopen(argv[1], "rt");
	char curr_line[500];
	int bit_vals_size = 0;
	Savenode* save_order;
	int save_size = 0;
	int save_complete = 0;

	tempvar_size = 0;

	while(fgets(curr_line,500,fp) != NULL){
		char saver[500];
		strcpy(saver, curr_line);
		char* tok = strtok(curr_line, " \n");
		if(strcmp(tok,"INPUTVAR") == 0){
			tok = strtok(NULL," \n");
			input_size = atoi(tok);
			bit_vals_size = 1<<input_size;
			allocate_input(bit_vals_size);
			for(int i = 0; i < input_size; i++){
				tok = strtok(NULL, " \n");
				inputs[i].id = malloc(strlen(tok)+1);
				strcpy(inputs[i].id, tok);
			}
		}
		else if(strcmp(tok,"OUTPUTVAR") == 0){
			tok = strtok(NULL, " \n");
			output_size = atoi(tok);
			allocate_output();
			for(int i = 0; i < output_size; i++){
				tok = strtok(NULL, " \n");
				outputs[i].id = malloc(strlen(tok)+1);
				strcpy(outputs[i].id, tok);
			}
		}
		else{
			if(save_size == 0){
				save_order = malloc(sizeof(Savenode));
				strcpy(save_order[0].save_operation, saver);
				save_size++;
			}
			else{
				save_order = realloc(save_order, (save_size+1)*sizeof(Savenode));
				strcpy(save_order[save_size].save_operation, saver);
				save_size++;
			}
		}
	}

	//Allocate space here, since INPUTVARS may not come before OUTPUTVARS
	for(int i = 0; i < output_size; i++){
		outputs[i].bit_vals = malloc(bit_vals_size*sizeof(int));
	}

	while(1){
		for(int i = 0; i < save_size; i++){
			if(save_order[i].valid == 1) continue;
			char saver[500];
			strcpy(saver, save_order[i].save_operation);
			char* tok = strtok(save_order[i].save_operation, " \n");
			if(strcmp(tok,"NOT") == 0){
				tok = strtok(NULL, " \n");
				if(tok[0] == '0' || tok[0] == '1'){
					int a_int = atoi(tok);
					tok = strtok(NULL, " \n");
					if(tok[0] >= 'a' && tok[0] <= 'z'){
						insert_tempvar(bit_vals_size, tok);
						for(int i = 0; i < bit_vals_size; i++) tempvars[tempvar_size].bit_vals[i] = 1^a_int;
						tempvar_size++;
					}
					else{
						for(int i = 0; i < output_size; i++){
							if(strcmp(tok, outputs[i].id) == 0){
								for(int j = 0; j < bit_vals_size; j++) outputs[i].bit_vals[j] = 1^a_int;
								break;
							}
						}
					}
					continue;
				}
				int* a = give_bit_vals(tok);

				if(a == NULL){
					strcpy(save_order[i].save_operation,saver);
					continue;
				}

				tok = strtok(NULL, " \n");
				if(tok[0] >= 'a' && tok[0] <= 'z'){
					insert_tempvar(bit_vals_size, tok);
					for(int i = 0; i < bit_vals_size; i++) tempvars[tempvar_size].bit_vals[i] = 1^a[i];
					tempvar_size++;
				}
				else{
					for(int i = 0; i < output_size; i++){
						if(strcmp(tok, outputs[i].id) == 0){
							for(int j = 0; j < bit_vals_size; j++) outputs[i].bit_vals[j] = 1^a[j];
							break;
						}
					}
				}
				save_order[i].valid = 1;
				save_complete++;
			}
			else if((strcmp(tok,"AND") == 0) || (strcmp(tok,"OR") == 0) || (strcmp(tok,"NAND") == 0) || (strcmp(tok,"NOR") == 0) || (strcmp(tok,"XOR") == 0) || (strcmp(tok,"XNOR") == 0)){
				int gate;
				if(strcmp(tok,"AND") == 0) gate = 1;
				else if(strcmp(tok,"OR") == 0) gate = 2;
				else if(strcmp(tok,"NAND") == 0) gate = 3;
				else if(strcmp(tok,"NOR") == 0) gate = 4;
				else if(strcmp(tok,"XOR") == 0) gate = 5;
				else gate = 6;
				char* a1;
				char* a2;
				tok = strtok(NULL, " \n");
				if(tok[0] != '0' && tok[0] != '1' && give_bit_vals(tok) == NULL){
					strcpy(save_order[i].save_operation,saver);
					continue;
				}
				a1 = malloc(strlen(tok)+1);
				strcpy(a1, tok);
				tok = strtok(NULL, " \n");
				if(tok[0] != '0' && tok[0] != '1' && give_bit_vals(tok) == NULL){
					strcpy(save_order[i].save_operation,saver);
					continue;
				}
				a2 = malloc(strlen(tok)+1);
				strcpy(a2, tok);

				tok = strtok(NULL, " \n");
				if(tok[0] >= 'a' && tok[0] <= 'z'){
					insert_tempvar(bit_vals_size, tok);
					tempvars[tempvar_size].bit_vals = basic_gates(gate, a1, a2, bit_vals_size);
					tempvar_size++;
				}
				else{
					for(int i = 0; i < output_size; i++){
						if(strcmp(tok, outputs[i].id) == 0){
							outputs[i].bit_vals = basic_gates(gate, a1, a2, bit_vals_size);
							break;
						}
					}
				}
				free(a1);
				free(a2);
				save_order[i].valid = 1;
				save_complete++;
			}
			else if(strcmp(tok,"DECODER") == 0){
				tok = strtok(NULL, " \n");
				int n_i = atoi(tok);
				int n_o = 1<<n_i;
				char** i_lines = malloc(n_i*sizeof(char*));
				char** o_lines = malloc(n_o*sizeof(char*));

				int breakout = 0;
				for(int i = 0; i < n_i; i++){
					tok = strtok(NULL, " \n");
					if(tok[0] != '0' && tok[0] != '1' && give_bit_vals(tok) == NULL){
						strcpy(save_order[i].save_operation,saver);
						breakout = 1;
						break;
					}
					i_lines[i] = malloc(strlen(tok)+1);
					strcpy(i_lines[i], tok);
				}

				if(breakout) continue;

				for(int i = 0; i < n_o; i++){
					tok = strtok(NULL, " \n");
					o_lines[i] = malloc(strlen(tok)+1);
					strcpy(o_lines[i], tok);
				}
				decode_op(n_i, n_o, bit_vals_size, i_lines, o_lines);
				for(int i = 0; i < n_o; i++){
					if(i < n_i) free(i_lines[i]);
					free(o_lines[i]);
				}
				free(i_lines);
				free(o_lines);
				save_order[i].valid = 1;
				save_complete++;
			}
			else{
				tok = strtok(NULL, " \n");
				int n_i = atoi(tok);
				char** i_lines = malloc(n_i*sizeof(char*));

				int breakout = 0;
				for(int i = 0; i < n_i; i++){
					tok = strtok(NULL, " \n");
					if(tok[0] != '0' && tok[0] != '1' && give_bit_vals(tok) == NULL){
						strcpy(save_order[i].save_operation,saver);
						breakout = 1;
						break;
					}
					i_lines[i] = malloc(strlen(tok)+1);
					strcpy(i_lines[i], tok);
				}

				if(breakout) continue;

				int n_s = 0;
				for(int i = n_i; i > 1; i = i>>1){
					n_s++;
				}
				char** s_lines = malloc(n_s*sizeof(char*));
				for(int i = 0; i < n_s; i++){
					tok = strtok(NULL, " \n");
					if(tok[0] != '0' && tok[0] != '1' && give_bit_vals(tok) == NULL){
						strcpy(save_order[i].save_operation,saver);
						breakout = 1;
						break;
					}
					s_lines[i] = malloc(strlen(tok)+1);
					strcpy(s_lines[i], tok);
				}

				if(breakout) continue;

				char* o_line;
				tok = strtok(NULL, " \n");
				o_line = malloc(strlen(tok)+1);
				strcpy(o_line, tok);
				multiplex_op(n_i, n_s, bit_vals_size, s_lines, i_lines, o_line);
				for(int i = 0; i < n_i; i++){
					if(i < n_s) free(s_lines[i]);
					free(i_lines[i]);
				}
				free(i_lines);
				free(s_lines);
				free(o_line);
				save_order[i].valid = 1;
				save_complete++;
			}			
		}
		if(save_complete == save_size){
			break;
		}
	}

	print_input_ouput(bit_vals_size);

	free_input();
	free_tempvar();
	free_output();
	free(save_order);
	fclose(fp);
	return(0);
}