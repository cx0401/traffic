#include<cstdlib>
#include<stdio.h>
#include<vector>
#include<ctime>
#include<string>
#include"TrafficSystem.h"
using namespace std;
const int nPop = 10;
const int G = 100;
const int nVar = 7;
const float pro = 1.0;
const float F = 0.8;
const float upper = 3;
const float lower = 0;
class DE {
public:
	DE() {}
	struct node {
		float fitness;
		int pos[cycle_time / phase_time][nVar];
	}indivual[nPop];
	float func(int pos[cycle_time / phase_time][nVar], TrafficSystem s) {
		s.test(pos);
		return 20 * s.ans();
	}
	void mutate(TrafficSystem s) {
		node m[nPop];
		for (int i = 0; i < nPop; i++) {
			for (int k = 0; k < cycle_time / phase_time; k++) {
				for (int j = 0; j < nVar; j++) {
					float p = float(rand()) / RAND_MAX;
					if (p < pro) {
						int rand1 = rand() % nPop;
						int rand2 = rand() % nPop;
						int rand3 = rand() % nPop;

						//int rand1 = rand() % (nPop - 1);
						//int rand2 = rand() % (nPop - 2);
						//int rand3 = rand() % (nPop - 3);
						//while (rand1 == i)rand1++;
						//while (rand2 == i || rand2 == rand1)rand2++;
						//while (rand3 == i || rand3 == rand2 || rand3 == rand1)rand3++;

						m[i].pos[k][j] = indivual[rand1].pos[k][j] + F * (indivual[rand2].pos[k][j] - indivual[rand3].pos[k][j]);
						if (m[i].pos[k][j] > upper)
							m[i].pos[k][j] = upper;
						if (m[i].pos[k][j] < lower)
							m[i].pos[k][j] = lower;
					}
					else
						m[i].pos[k][j] = indivual[i].pos[k][j];
				}
			}
			
			m[i].fitness = func(m[i].pos,s);
		}
		for (int i = 0; i < nPop; i++) {
			if (m[i].fitness > indivual[i].fitness) {
				memcpy(indivual[i].pos, m[i].pos, sizeof(m[i].pos));
				indivual[i].fitness = m[i].fitness;
			}
		}
	}
	void intial(TrafficSystem s) {
		for (int i = 0; i < nPop; i++) {
			for (int k = 0; k < cycle_time / phase_time; k++) {
				for (int j = 0; j < nVar; j++) {
					indivual[i].pos[k][j] = (float(rand()) / RAND_MAX) * (upper - lower) + lower;
				}
			}
			indivual[i].fitness = func(indivual[i].pos,s);
		}
	}
	int run(TrafficSystem s) {
		srand(unsigned(time(NULL)));
		intial(s);
		int ans = 0;
		int index = 0;
		float best = 0;
		for (int i = 0; i < G; i++) {
//			printf("第%d代\n: ", i);
			for (int j = 0; j < nPop; j++) {
				//for (int z = 0; z < 4; z++) {
				//	for (int k = 0; k < nVar; k++) {
				//		printf("%d ", indivual[j].pos[z][k]);
				//	}
				//}
				//printf("%lf\n", indivual[j].fitness);
				if (indivual[j].fitness > best) {
					ans = i;
					index = j;
					best = indivual[j].fitness;
				}
			}
	//		printf(" %d\n",index);
			mutate(s);
		}
//		printf("最优解的代数是 %d 适应度是 %f 下标为%d\n", ans, best, index);
		return index;
	}
	~DE() {}
};