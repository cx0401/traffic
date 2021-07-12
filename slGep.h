#include<iostream>
#include<string>
#include<vector>
#include<time.h>
#include<GL/glut.h>
#include<windows.h>
#include"DE.h"
#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include "math.h"
#include "string.h"
TrafficSystem staticSys;
using namespace std;
#define H	10			//head lenFTh of the main program
#define T	(4 * (H - 1))		//tail lenFTh of the main program   (the maximum arities of all functions is 2)
#define FSIZE 4			//number of ADFs
#define FH	7			//head lenFTh of ADFs
#define FT		(FH + 1)	//tail lenFTh of ADFs
#define FNVARS	(FH+FT)	
#define NVARS	(H+T + FSIZE *(FH+FT))	//chromosome lenFTh

#define POPSIZE	50					//population size
#define MAX_TERMINAL_NUM	10		//maximun terminal number
int		L_terminal = 10000;			//start value of terminal symbol
int		F_par = 20000;			//start value of input symbol
int     F_2 = 5; //{and, sub, mul, div, >}
int     F_1 = 6; //{sin, cos, exp, log, abs, floor}
int		SFN = F_2 + F_1;		//{and, sub, mul, div, >, sin, cos, exp, log, abs, floor}
int		generation;					//number of generations
int		terminal_num = 8;			//current number of terminals
int		par_num = 4;			//current number of terminals
int     L_constant = L_terminal + terminal_num;         //start value of constant
int     constant_num = 1000;  
int		function_num = (SFN + FSIZE);			//15
int     max_function_num = function_num + 1;       //{and, sub, mul, div, >, sin, cos, exp, log, abs, floor, G0, G1, G2, G3, max}16
bool	variable_value[MAX_TERMINAL_NUM];					//input variable values
int		gene_type_flag[NVARS];								//the type of each bit in the chromosome
//========for stochastical analysis ===========================
#define MAXEVALS 1000000
#define MAXGENS	1
//============= nodes and tree for computing the fitness of individuals ==============================
#define		MAX_SIBLING	20				//the maximum sibling for each node
#define		LINK_LENFTH	(NVARS * 20)	//add enouFH to save all necessary node.
const int maxTime = 200;
//=============== parameters for symbolic regression problem ======================================================
int		function = 0;		//current problem index
int		job = 0;				//EA run index 
#define MAXINPUTS		1000	//maximum input-output pairs of each problem
#define	MAX_VARIABLES	10
#define MAXIMUM_ELEMENTS	100					//MAXIMUM_ELEMENTS > function_num && MAXIMUM_ELEMENTS > terminal_num
class SL_GEP {
public:
	typedef struct
	{
		int gene[NVARS];
		double f;
	}CHROMOSOME;
	CHROMOSOME population[POPSIZE + 1], newpopulation[POPSIZE];
	double	fbest;
	int bestGen;
	int		evals;
	struct LINK_COMP
	{
		int value;							// node label
		int sibling_num;
		struct LINK_COMP* siblings[MAX_SIBLING];
	};
	struct LINK_COMP* link_root, link_comp[LINK_LENFTH];		//the whole expression tree
	struct LINK_COMP* sub_root[FSIZE], sub_comp[FSIZE][FNVARS]; //the sub expression tree 


	int		input_num;
	double	current_value[MAXINPUTS];
	double	training_inputs[MAXINPUTS][MAX_VARIABLES];
	double	training_outputs[MAXINPUTS];
	int		training_cases;

	//for sub expression trees
	double sub_sibling_value[MAX_SIBLING][MAXINPUTS];
	double sub_current_value[MAXINPUTS];

	//return a uniform random nubmer within [a,b)
	double randval(double a, double b)
	{
		return a + (b - a) * rand() / (double)RAND_MAX;
	}
	//compute the sub-tree
	void compute_sub_rule(const struct LINK_COMP* node, int i)
	{
		int j, k;
		if (node->value >= F_par) {
			// If the node is an input then read data from the input vector, i.e., sub_sibling_value[...];
			sub_current_value[i] = sub_sibling_value[node->value - F_par][i];
		}
		else if (node->value >= L_constant) {
			sub_current_value[i] = node->value - L_constant;
		}
		else {
			// First compute the left child of the node.
			double t[4][MAXINPUTS];
			compute_sub_rule(node->siblings[0], i);

			t[0][i] = sub_current_value[i];
			//then compute the riFHt child of the node if the node contain riFHt child
			if (node->value < F_2) {  // note that the first 4 functions have 2 children
				compute_sub_rule(node->siblings[1], i);
				t[1][i] = sub_current_value[i];
			}
			switch (node->value) {
			case 0: //+ 			
				sub_current_value[i] = t[0][i] + t[1][i]; break;
			case 1: //-
				sub_current_value[i] = t[0][i] - t[1][i]; break;
			case 2: //*
				sub_current_value[i] = t[0][i] * t[1][i]; break;
			case 3: // /
			{ if (fabs(t[1][i]) < 1e-20) sub_current_value[i] = 0; else sub_current_value[i] = t[0][i] / t[1][i]; } break;
			case 4:// >
			{ sub_current_value[i] = (t[0][i] > t[1][i] ? 1 : 0); break; }
			case 5: // sin
			{  sub_current_value[i] = sin(t[0][i]); } break;
			case 6: // cos
			{  sub_current_value[i] = cos(t[0][i]); } break;
			case 7: // exp
			{  sub_current_value[i] = exp(t[0][i]); } break;
			case 8: // log
			{  sub_current_value[i] = log(t[0][i]); } break;
			case 9: // abs
			{  sub_current_value[i] = abs(t[0][i]); } break;
			case 10: // floor
			{  sub_current_value[i] = floor(t[0][i]); } break;
			default: printf("unknow function\n");
			}

		}
	}

	//Compute the entire solution tree.
	double compute_rule(const struct LINK_COMP* node, int i)
	{
		if (node->value >= L_constant) {
			current_value[i] = node->value - L_constant;
		}
		else if (node->value >= L_terminal) {
			current_value[i] = training_inputs[i][node->value - L_terminal];
		}
		else {
			double t[4][MAXINPUTS];
			compute_rule(node->siblings[0], i);
			t[0][i] = current_value[i];
			if (node->value < F_2) {
				compute_rule(node->siblings[1], i);
				t[1][i] = current_value[i];
			}
			if (node->value >= SFN) {
				for (int k = 1; k < 4; k++) {
					compute_rule(node->siblings[k], i);
					t[k][i] = current_value[i];
				}
			}
			switch (node->value) {
			case 0: //+ 			
				current_value[i] = t[0][i] + t[1][i]; break;
			case 1: //-
				current_value[i] = t[0][i] - t[1][i]; break;
			case 2: //*
				current_value[i] = t[0][i] * t[1][i]; break;
			case 3: // /
			{ if (fabs(t[1][i]) < 1e-20) current_value[i] = 0; else current_value[i] = t[0][i] / t[1][i]; } break;
			case 4:// >
			{ current_value[i] = (t[0][i] > t[1][i]) ? 1 : 0; break; }
			case 5: // sin
			{  current_value[i] = sin(t[0][i]); } break;
			case 6: // cos
			{  current_value[i] = cos(t[0][i]); } break;
			case 7: // exp
			{  current_value[i] = exp(t[0][i]); } break;
			case 8: // log
			{  current_value[i] = log(t[0][i]); } break;
			case 9: // abs
			{  current_value[i] = abs(t[0][i]); } break;
			case 10: // floor
			{  current_value[i] = floor(t[0][i]); } break;
			case 15: //max
			{
				int ans = 0;
				for (int k = 0; k < 4; k++) {
					if (t[ans][i] < t[k][i]) {
						ans = k;
					}
				}
				current_value[i] = ans;
			} break;
			default: //GI
			{
				for (int k = 0; k < 4; k++) {
					sub_sibling_value[k][i] = t[k][i];
				}
				compute_sub_rule(sub_root[node->value - SFN], i);
				current_value[i] = sub_current_value[i];
				break;
			}
			}
		}
		return current_value[i];
	}


	//Decode the chromosome, build the main expression tree, including sub-expression trees.
	void decode_gene(CHROMOSOME* p)
	{
		int op = -1, i = 0, k = 0, j;
		for (i = 0; i < NVARS; i++) {
			link_comp[i].value = p->gene[i];
			for (j = 0; j < MAX_SIBLING; j++)
				link_comp[i].siblings[j] = NULL;
		}

		op = -1, i = 1;
		link_root = &link_comp[0];
		if (link_root->value < max_function_num) {
			do {
				//find an op type item
				do { 
					op++; 
					if (op >= i)
						break; 
				} while (link_comp[op].value >= L_terminal);
				if (op >= i) break;
				//set its left and riFHt;
				if (link_comp[op].value < L_terminal) {
					if (i >= H + T) { break; }
					link_comp[op].siblings[0] = &link_comp[i];
					i++;
					if (link_comp[op].value < F_2) {
						if (i >= H + T)  break; 
						link_comp[op].siblings[1] = &link_comp[i];
						i++;
					}
					if (link_comp[op].value >= SFN) {
						if (i >= H + T)  break;
						for (k = 1; k < 4; k++)
							link_comp[op].siblings[k] = &link_comp[i++];
					}
				}
			} while (true);
			if (op < i && i >= H + T) {
				printf("\nERROR RULE111");
				getchar();
			}
		}
		else {
			//printf("terminate");
		}

		//build sub expression trees of the individual
		for (int g = 0; g < FSIZE; g++) {
			k = H + T + g * FNVARS;	// the starting position of the ADF.	
			for (i = 0; i < FNVARS; i++) {
				sub_comp[g][i].value = p->gene[k + i];
				for (j = 0; j < MAX_SIBLING; j++)
					sub_comp[g][i].siblings[j] = NULL;
			}
			op = -1, i = 1;
			sub_root[g] = &sub_comp[g][0];
			if (sub_root[g]->value < L_terminal) {  // note that F_par > L_terminal;
				do { //find an op type item
					do { op++; if (op >= i)break; } while (sub_comp[g][op].value >= L_terminal);
					if (op >= i) break;
					//set its left and riFHt;
					if (sub_comp[g][op].value < SFN) {
						if (i >= FH + FT - 1) { break; }
						sub_comp[g][op].siblings[0] = &sub_comp[g][i];
						i++;
						if (sub_comp[g][op].value < F_2) {
							sub_comp[g][op].siblings[1] = &sub_comp[g][i];
							i++;
						}
					}
				} while (true);
				if (op < i && i >= FH + FT - 1) { printf("SUB ERROR RULE111"); getchar(); }
			}
			else {
				//printf("SUB terminate");
			}
		}
	}
	//按照长度选择当前相位（贪心）
	void numToRuleTime(int t, int& nextRuleTime, double ans, int time[], int t_i) {
		int index = 0;
		//for (int i = 4; i < 4 + phaseNum; i++) {//根据堵车数量贪心决定
		for (int i = 0; i < phaseNum; i++) {//根据车流量决定
			if (training_inputs[t_i][i] > training_inputs[t_i][index]) {
				index = i;
			}
		}
		ans = int(ans);
		if (ans > 3 || ans < 0) {
			cout << "Error" << ans << endl;
		}
		int baseAddTime = 20;
		//ans = index; //采用贪心
		time[int(ans)] = t;//采用gep
		//time[rand() % 4] = t;//c采用随机
		//time[(t / baseAddTime) % 4] = t;//采用定时策略
		nextRuleTime = t + baseAddTime;
	}
	void getSignal(CHROMOSOME* p, TrafficSystem& s) {
		for (int i = 0; i < maxCross; i++) {
			if (s.cross[i].tag) {
				if (s.cross[i].nextRuleTime == s.currentTime) {
					//cout << i << endl;
					decode_gene(p);
					//四种相位下的车辆数量
					training_inputs[i][0] = s.road[s.cross[i].linkroad[1]].vichle[2].size() + s.road[s.cross[i].linkroad[3]].vichle[1].size();
					training_inputs[i][1] = s.road[s.cross[i].linkroad[1]].vichle[3].size() + s.road[s.cross[i].linkroad[3]].vichle[0].size();
					training_inputs[i][2] = s.road[s.cross[i].linkroad[0]].vichle[0].size() + s.road[s.cross[i].linkroad[2]].vichle[3].size();
					training_inputs[i][3] = s.road[s.cross[i].linkroad[0]].vichle[1].size() + s.road[s.cross[i].linkroad[2]].vichle[2].size();
					training_inputs[i][4] = s.road[s.cross[i].linkroad[1]].delay[2] + s.road[s.cross[i].linkroad[3]].delay[1];
					training_inputs[i][5] = s.road[s.cross[i].linkroad[1]].delay[3] + s.road[s.cross[i].linkroad[3]].delay[0];
					training_inputs[i][6] = s.road[s.cross[i].linkroad[0]].delay[0] + s.road[s.cross[i].linkroad[2]].delay[3];
					training_inputs[i][7] = s.road[s.cross[i].linkroad[0]].delay[1] + s.road[s.cross[i].linkroad[2]].delay[2];
					
					double ans = compute_rule(link_root, i);
					//for (int j = 0; j < 8; j++) {
					//	cout << training_inputs[i][j] << " ";
					//}
					//cout << endl;
					//cout << ans << endl;
					numToRuleTime(s.currentTime, s.cross[i].nextRuleTime, ans, s.cross[i].traficSignalTurnTime, i);
				}
				for (int j = 0; j < phaseNum; j++) {
					if (s.currentTime == s.cross[i].traficSignalTurnTime[j]) {
						s.cross[i].state_traffic = j;
						memcpy(s.cross[i].status, s.phase[s.cross[i].state_traffic], sizeof(s.cross[i].status));
						break;
					}
				}		
			}
		}
	}
	void objective(CHROMOSOME* p, TrafficSystem s)
	{
		p->f = 0;

		//decode_gene(p);
		//inOrderTree(link_comp);
		
		for (int tt = 0; tt < maxTime; tt++) {
			getSignal(p, s);
			s.appear_car();
			s.run();
			s.ans();
		}
		double v = s.ans();
		//	cout << s.ans()<< endl;
		p->f = v;
		if (v > fbest) {
			fbest = v;
			bestGen = generation;
		}
		evals++;

		
		//printf("\t%f\n", p->f);
	}
	//================================================================================
	//randomly set the value of the I-th bit of an individual, x points to this bit.
	//There are only four possibles: 0: the main head; 1: the main tail; 2: the sub head; 3: the sub tail;
	void rand_set_value(int I, int* x)
	{
		if (I == 0) {
			*x = function_num;
			return;
		}
		switch (gene_type_flag[I]) {
		case 0:
			if (randval(0, 1) < 1. / 3) *x = rand() % (function_num);		// note that function_num = SFN + FSIZE;
			else if (randval(0, 1) < 0.5) *x = SFN + rand() % (FSIZE);
			else if (randval(0, 1) < 0.9) *x = L_terminal + rand() % (terminal_num);
			else *x = L_constant + rand() % (constant_num);
			break;
		case 1: 
			if (randval(0, 1) < 0.9) *x = L_terminal + rand() % (terminal_num);
			else *x = L_constant + rand() % (constant_num);
			break;
		case 2: 
			if (rand() % 2 == 0)	*x = rand() % (SFN);
			 else if (randval(0, 1) < 0.9) *x = F_par + rand() % (par_num);
			 else *x = L_constant + rand() % (constant_num);
			break;
		case 3:
			if (randval(0, 1) < 0.9) *x = F_par + rand() % (par_num);
			else *x = L_constant + rand() % (constant_num);
			break;
		default: printf("fds");
		}
	}

	//===============================probability of components ============================================================ 
	double	FQ;										//in the main heads of population, the proportion of bits being function symbol

	double	function_freq[MAXIMUM_ELEMENTS];						//in the main parts of population, the frequency of each function symbol
	double	terminal_freq[MAXIMUM_ELEMENTS];						//in the main parts of population, the frequency of each terminal symbol
	double	constant_freq[MAXIMUM_ELEMENTS];						//in the main parts of population, the frequency of each terminal symbol
	double	terminal_probability[MAXIMUM_ELEMENTS];				//store the selection probability of each terminal
	double	function_probability[MAXIMUM_ELEMENTS];				//store the selection probability of each function
	double	constant_probability[MAXIMUM_ELEMENTS];				//store the selection probability of each function
	void update_probability()
	{
		double sum = 0;
		int i, j, k;
		//in the main head of population, the proportion of bits being function symbol
		FQ = 0;
		int	CC = 0;
		for (i = 0; i < POPSIZE; i++) {
			for (j = 0; j < H; j++) {
				if (population[i].gene[j] < L_terminal) FQ++;
				else if (population[i].gene[j] >= L_terminal) CC++;
			}
		}
		FQ = FQ / (double)(POPSIZE * H);

		bool print_flag = false;

		//now compute the frequency of each symbol in the main parts of the current population.
		for (i = 0; i < MAXIMUM_ELEMENTS; i++) {
			function_freq[i] = 1;	//initialize a very small value.
			terminal_freq[i] = 1;
			constant_freq[i] = 1;
		}

		for (i = 0; i < POPSIZE; i++) {
			for (j = 0; j < H + T; j++) {  //only consider main parts
				if (population[i].gene[j] < L_terminal) {

					function_freq[population[i].gene[j]]++;
				}
				else if(population[i].gene[j] < L_constant)
					terminal_freq[population[i].gene[j] - L_terminal] ++;
				else
					constant_freq[population[i].gene[j] - L_constant] ++;
			}
		}
		sum = 0;
		for (i = 0; i < function_num; i++) {
			sum += function_freq[i];
		}
		function_probability[0] = function_freq[0] / sum;
		for (i = 1; i < function_num; i++) {
			function_probability[i] = function_freq[i] / sum + function_probability[i - 1];
		}
		
		sum = 0;
		for (i = 0; i < terminal_num; i++) {
			sum += terminal_freq[i];
			terminal_probability[i] = terminal_freq[i];
		}
		terminal_probability[0] = terminal_probability[0] / sum;
		for (i = 1; i < terminal_num; i++) {
			terminal_probability[i] = terminal_probability[i] / sum + terminal_probability[i - 1];
		}

		sum = 0;
		for (i = 0; i < constant_num; i++) {
			sum += constant_freq[i];
			constant_probability[i] = constant_freq[i];
		}
		constant_probability[0] = constant_probability[0] / sum;
		for (i = 1; i < terminal_num; i++) {
			constant_probability[i] = constant_probability[i] / sum + constant_probability[i - 1];
		}
	}

	//choose a constant according to its frequence.
	int choose_a_constant()
	{
		int i, j;
		double p = randval(0, 1);
		for (i = 0; i < constant_num - 1; i++) {
			if (p < constant_probability[i])
				break;
		}
		return L_constant + i;
	}

	//choose a terminal according to its frequence.
	int choose_a_terminal()
	{
		int i, j;
		double p = randval(0, 1);
		for (i = 0; i < terminal_num - 1; i++) {
			if (p < terminal_probability[i])
				break;
		}
		return L_terminal + i;
	}

	//choose a function according to its frequence.
	int choose_a_function()
	{
		int i, j, k;
		double p = randval(0, 1);
		for (i = 0; i < function_num - 1; i++) {
			if (p < function_probability[i])
				break;
		}
		return i;
	}

	//bially set value of bits. 
	void biasly_set_value(int I, int* x)
	{
		if (I == 0) {
			*x = function_num;
			return;
		}
			
		//here we only consder the main parts, while the sub-gene part are also randomly setting, so as to import population diversity.
		switch (gene_type_flag[I]) {
		case 0:
			if (randval(0, 1) < FQ) *x = choose_a_function();
			else if(randval(0, 1) < 0.5) *x = choose_a_terminal();
			else *x = choose_a_constant();
			break;
		case 1: if (randval(0, 1) < 0.5) *x = choose_a_terminal();
			  else *x = choose_a_constant(); break;
		case 2:
			if (rand() % 2 == 0) *x = rand() % (SFN);
			else if (randval(0, 1) < 0.5) *x = F_par + rand() % (par_num);
			 else *x = L_constant + rand() % (constant_num);
			break;
		case 3: if (randval(0, 1) < 0.5) *x = F_par + rand() % (par_num);
			  else *x = L_constant + rand() % (constant_num); break;
		default: printf("fds");
		}
	}

	void initialize(TrafficSystem s)
	{
		int i, j, k;
		int ibest = 0;
		evals = 0;
		fbest = 0;
		//firstly set the type of each bit.
		for (i = 0; i < NVARS; i++) {
			if (i < H)  gene_type_flag[i] = 0;
			else if (i < H + T)  gene_type_flag[i] = 1;
			else {
				j = i - H - T;
				if (j % (FH + FT) < FH) gene_type_flag[i] = 2;
				else gene_type_flag[i] = 3;
			}
		}
		const int preNum = 13;
		int start0[NVARS] = {
				function_num, SFN,SFN + 1,SFN + 2,SFN + 3,//H
				L_terminal, L_terminal + 1,L_terminal + 2,L_terminal + 3,//G0(x0,x1,x2,x3)
				L_terminal, L_terminal + 1,L_terminal + 2,L_terminal + 3,//G0(x0,x1,x2,x3)
				L_terminal, L_terminal + 1,L_terminal + 2,L_terminal + 3,//G0(x0,x1,x2,x3)
				L_terminal, L_terminal + 1,L_terminal + 2,L_terminal + 3,//G0(x0,x1,x2,x3)
				L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,
				L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,//21有效
				F_par,    F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,//G0 content
				F_par + 1,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,//G1 content
				F_par + 2,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,//G2 content
				F_par + 3,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,//G3 content
		};//贪心车流量
		int start1[NVARS] = {
				function_num, SFN,SFN + 1,SFN + 2,SFN + 3,//H
				L_terminal + 4, L_terminal + 5,L_terminal + 6,L_terminal + 7,//G0(x0,x1,x2,x3)
				L_terminal + 4, L_terminal + 5,L_terminal + 6,L_terminal + 7,//G0(x0,x1,x2,x3)
				L_terminal + 4, L_terminal + 5,L_terminal + 6,L_terminal + 7,//G0(x0,x1,x2,x3)
				L_terminal + 4, L_terminal + 5,L_terminal + 6,L_terminal + 7,//G0(x0,x1,x2,x3)
				L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,
				L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,//21有效
				F_par,    F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,//G0 content
				F_par + 1,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,//G1 content
				F_par + 2,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,//G2 content
				F_par + 3,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,//G3 content
		};//贪心堵车数量
		int start2[NVARS] = {
				function_num, SFN,SFN + 1,SFN + 2,SFN + 3,//H
				L_terminal,     L_terminal + 4,L_terminal + 4,L_terminal + 6,//G0(x0,x4,x2,x3)
				L_terminal + 1, L_terminal + 5,L_terminal + 5,L_terminal + 7,//G1(x1,x5,x2,x3)
				L_terminal + 2, L_terminal + 6,L_terminal + 6,L_terminal + 7,//G2(x2,x6,x2,x3)
				L_terminal + 3, L_terminal + 7,L_terminal + 6,L_terminal + 7,//G3(x3,x7,x2,x3)
				L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,
				L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,L_terminal,//21有效
				0, 2,F_par,L_constant + 1,F_par + 1,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,//G0 content
				0, 2,F_par,L_constant + 1,F_par + 1,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,//G1 content
				0, 2,F_par,L_constant + 1,F_par + 1,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,//G2 content
				0, 2,F_par,L_constant + 1,F_par + 1,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,F_par,//G3 content
		};//综合相加
		int start3[NVARS] = { 15, 12, 13, 1, 10, 10002, 13, 10003, 0, 10001, 10003, 10002, 10005, 10002, 10005, 10003, 10007, 10002, 10001, 10004, 10002, 10016, 10016, 10001, 10002, 10002, 10000, 10016, 10016, 10005, 10000, 10003, 10016, 10016, 10003, 10001, 10016, 10002, 10016, 10016, 10003, 10001, 10016, 10001, 10006, 10016, 10735, 20000, 20000, 20001, 20002, 20002, 20000, 10336, 20001, 10826, 20003, 20002, 20003, 10902, 10277, 20001, 8, 8, 10412, 20001, 9, 10815, 10663, 10413, 10099, 20003, 10962, 20000, 10512, 20002, 20003, 10557, 10961, 20003, 20003, 10426, 10583, 20003, 20000, 10568, 10913, 10397, 20000, 10268, 20002, 10844, 10, 20003, 9, 6, 20003, 20001, 20003, 10358, 20001, 10753, 10529, 10228, 10355, 10577 };//自测函数
		for (i = 0; i < POPSIZE; i++) {
			for (k = 0; k < NVARS; k++) {
				rand_set_value(k, &population[i].gene[k]);
				//设置一些初始特别解
				//if (i == 0) {
				//	if (k < NVARS)
				//		population[i].gene[k] = start0[k];
				//}
				//if (i == 1) {
				//	if (k < NVARS)
				//		population[i].gene[k] = start1[k];
				//}
				//if (i == 2) {
				//	if (k < NVARS)
				//		population[i].gene[k] = start2[k];
				//}
				if (i == 3) {
					if (k < NVARS)
						population[i].gene[k] = start3[k];
				}
			}
			decode_gene(population + i);
			//inOrderTree(link_comp);

			objective(&population[i], s);
			if (population[i].f > population[ibest].f) ibest = i;
			
			//printf("\n");
			//inOrderTree(sub_comp[0]);
			//printf("\n");
			//inOrderTree(sub_comp[1]);
			//printf("\n");
			//inOrderTree(sub_comp[2]);
			//printf("\n");
			//inOrderTree(sub_comp[3]);
			//printf("\t%f\n", population[i].f);
		}
		population[POPSIZE] = population[ibest];
	}

	void production(TrafficSystem s)
	{
		int i, j, k, r1, r2, r3, r4, r5;
		double CR, F;
		double change_vector[NVARS];
		update_probability();
		for (i = 0; i < POPSIZE; i++) {
			newpopulation[i] = population[i];
			F = randval(0, 1);
			CR = randval(0, 1);
			do { r1 = rand() % (POPSIZE); } while (r1 == i);
			do { r2 = rand() % POPSIZE; } while (r2 == r1 || r2 == i);
			k = rand() % NVARS;
			for (j = 1; j < NVARS; j++) {//注意这里修改了初始为1，max不变
				if (randval(0, 1) < CR || k == j) {
					double dd1 = 0;
					if (((int)population[POPSIZE].gene[j]) != ((int)population[i].gene[j])) dd1 = 1;
					double dd2 = 0;
					if (((int)population[r1].gene[j]) != ((int)population[r2].gene[j])) dd2 = 1;
					change_vector[j] = F * dd1 + F * dd2 - (F * dd1 * F * dd2);
					if (randval(0, 1) < change_vector[j]) {
						biasly_set_value(j, &newpopulation[i].gene[j]);
					}
					else {
						newpopulation[i].gene[j] = population[i].gene[j];
					}
				}
				else {
					change_vector[j] = 0;
					newpopulation[i].gene[j] = population[i].gene[j];
				}
			}
			//decode_gene(newpopulation + i);
			//inOrderTree(link_comp);
			//printf("\n");
			objective(&newpopulation[i], s);
			if (newpopulation[i].f > population[i].f) {
				population[i] = newpopulation[i];
				if (population[i].f > population[POPSIZE].f) {
					population[POPSIZE] = population[i];
				}
			}
			
		}
	}

	void inOrderTree(LINK_COMP* x) {
		if (x == NULL) {
			return;
		}
			
		if (x->value < max_function_num) {
			switch (x->value) {
			case 0: printf("("); inOrderTree(x->siblings[0]); printf("+"); inOrderTree(x->siblings[1]); printf(")"); break;
			case 1: printf("("); inOrderTree(x->siblings[0]); printf("-"); inOrderTree(x->siblings[1]); printf(")"); break;
			case 2: printf("("); inOrderTree(x->siblings[0]); printf("*"); inOrderTree(x->siblings[1]); printf(")"); break;
			case 3: printf("("); inOrderTree(x->siblings[0]); printf("/"); inOrderTree(x->siblings[1]); printf(")"); break;
			case 4: printf("("); inOrderTree(x->siblings[0]); printf(">"); inOrderTree(x->siblings[1]); printf(")"); break;
			case 5: printf("sin("); inOrderTree(x->siblings[0]);  printf(")"); break;
			case 6: printf("cos("); inOrderTree(x->siblings[0]);  printf(")"); break;
			case 7: printf("exp("); inOrderTree(x->siblings[0]);  printf(")"); break;
			case 8: printf("log("); inOrderTree(x->siblings[0]);  printf(")"); break;
			case 9: printf("abs("); inOrderTree(x->siblings[0]);  printf(")"); break;
			case 10: printf("floor("); inOrderTree(x->siblings[0]);  printf(")"); break;
			case 11: printf("G0("); inOrderTree(x->siblings[0]); printf(","); inOrderTree(x->siblings[1]);  printf(","); inOrderTree(x->siblings[2]);  printf(","); inOrderTree(x->siblings[3]); printf(")"); break;
			case 12: printf("G1("); inOrderTree(x->siblings[0]); printf(","); inOrderTree(x->siblings[1]);  printf(","); inOrderTree(x->siblings[2]);  printf(","); inOrderTree(x->siblings[3]); printf(")"); break;
			case 13: printf("G2("); inOrderTree(x->siblings[0]); printf(","); inOrderTree(x->siblings[1]);  printf(","); inOrderTree(x->siblings[2]);  printf(","); inOrderTree(x->siblings[3]); printf(")"); break;
			case 14: printf("G3("); inOrderTree(x->siblings[0]); printf(","); inOrderTree(x->siblings[1]);  printf(","); inOrderTree(x->siblings[2]);  printf(","); inOrderTree(x->siblings[3]); printf(")"); break;
			case 15: printf("max("); inOrderTree(x->siblings[0]); printf(","); inOrderTree(x->siblings[1]);  printf(","); inOrderTree(x->siblings[2]);  printf(","); inOrderTree(x->siblings[3]); printf(")"); break;
			}
		}
		else if (x->value >= F_par) {
			printf("y%d", x->value - F_par);
		}
		else if (x->value >= L_constant) {
			printf("%d", x->value - L_constant);
		}
		else if (x->value >= L_terminal) {
			printf("x%d", x->value - L_terminal);
		}
	}
	void SLGEP(TrafficSystem s)
	{
		initialize(s);
		generation = 0;
		while (generation < MAXGENS) {
			production(s);
			if (population[POPSIZE].f < 1e-5) {
				break;
			}
			if (generation % 10 == 0) {
				printf("\n%d\t%g\t%d\n", generation, fbest * 20, bestGen);
				
				printf("{");
				for (int i = 0; i < NVARS; i++) {
					if(i)
						printf(", ");
					printf("%d", population[POPSIZE].gene[i]);
				}
				printf("}\n");
				decode_gene(population + POPSIZE);
				inOrderTree(link_comp);
				printf("\n");
				inOrderTree(sub_comp[0]);
				printf("\n");
				inOrderTree(sub_comp[1]);
				printf("\n");
				inOrderTree(sub_comp[2]);
				printf("\n");
				inOrderTree(sub_comp[3]);
				printf("\n");
			}
			if (generation == MAXGENS - 1) {
				//printf("\n%d\t%g\t%d\n", generation, fbest * 20, bestGen);

				//printf("{");
				//for (int i = 0; i < NVARS; i++) {
				//	if (i)
				//		printf(", ");
				//	printf("%d", population[POPSIZE].gene[i]);
				//}
				//printf("}\n");
				//decode_gene(population + POPSIZE);
				//inOrderTree(link_comp);
				//printf("\n");
				//inOrderTree(sub_comp[0]);
				//printf("\n");
				//inOrderTree(sub_comp[1]);
				//printf("\n");
				//inOrderTree(sub_comp[2]);
				//printf("\n");
				//inOrderTree(sub_comp[3]);
				//printf("\n");
				printf("%g\n", fbest * 20);
				
			}
			generation++;
		}
		//printf("\n");
	}
};

