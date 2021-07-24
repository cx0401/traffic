#include <stdlib.h>
#include<iostream>
#include<string>
#include<vector>
#include<time.h>
#include<GL/glut.h>
#include<cmath>
#include<windows.h>
#include<time.h>
#include"slGep.h"
using namespace std;
TrafficSystem s;
SL_GEP gep;
int state[cycle_time / phase_time][maxCross - maxE] = { 0 };

void generateSignal() {
	//差分分配红绿灯
	DE compute;
	int ans = compute.run(s);
	memcpy(state, compute.indivual[ans].pos, sizeof(state));
	for (int i = 0; i < cycle_time / phase_time; i++) {
		for (int j = 0; j < maxCross - maxE; j++) {
			cout << state[i][j] << " ";
		}
		cout << endl;
	}
	cout << endl;
}
void myIdle1(void) {//贪心和gep
	gep.getSignal(gep.population + POPSIZE, s);
	s.appear_car();
	s.car_coor();
	s.run();
	s.display(0);
	Sleep(100);
}
void myIdle2(void) {//随机和差分
	if (s.currentTime % (cycle_time) == 0) {
		generateSignal();
	}
	else
		Sleep(0);
	if (s.currentTime % phase_time == 0) {
		s.traffic_signal(state[(s.currentTime / phase_time) % (cycle_time / phase_time)]);
	}
	s.appear_car();
	s.car_coor();
	s.run();
	s.display(0);
	Sleep(100);
}
void traffic(unsigned char key, int x, int y) {
	if (key == 'q') {
		glutIdleFunc(&myIdle1);
	}
	if (key == 'p') {
		glutIdleFunc(NULL);
	}
}
float nextTime(float rateParameter)
{
	float ans = (float)rand() / (RAND_MAX + 1);
	cout << ans << " ";
	return -logf(1 - ans) / rateParameter + 1;
}
void generateCar(int carNum) {
	//初试车辆出入口数组生成
	char name[200];
	sprintf(name, "car\\car%d.txt", carNum);
	FILE* fp1 = fopen(name, "w");
	float lamba = 0.1;
	int carAppearTime[maxE][maxCar];
	int carDes[maxE][maxCar];
	for (int i = 0; i < maxE; i++) {
		for (int j = 0; j < maxCar; j++) {
			int temp = nextTime(lamba);
			if (temp == 0)
				temp++;
			if (j)
				carAppearTime[i][j] = carAppearTime[i][j - 1] + temp;
			else
				carAppearTime[i][j] = temp;
			carDes[i][j] = rand() % maxE;
			while (carDes[i][j] == i) {
				carDes[i][j] = rand() % maxE;
			}
			fprintf(fp1, "%d %d %d\n", i, carAppearTime[i][j], carDes[i][j]);
		}
	}
	fclose(fp1);
}	
void loadCar(int car_cross[maxE][MAXT][2], int carNum){
	//初始化随机车辆
	for (int i = 0; i < maxE; i++) {
		for (int j = 0; j < MAXT; j++) {
			car_cross[i][j][0] = -1;
			car_cross[i][j][1] = -1;
		}
	}
	char name[200];
	sprintf(name, "car\\car%d.txt", carNum);
	FILE* fp2 = fopen(name, "r");
	int x, y, z, k;
	for (int i = 0; i < maxE * maxCar; i++) {
		fscanf(fp2, "%d %d %d", &x, &y, &z);
		if (x < maxE && y < MAXT && z < maxE) {
			car_cross[x][y][0] = x;
			car_cross[x][y][1] = z;
		}
	}
	fclose(fp2);
}
int main(int argc, char* argv[]){
	srand((int)time(NULL));
	int car_cross[maxE][MAXT][2];//车辆原点和目的点
	loadCar(car_cross, 3);
	s.init(car_cross);
	gep.SLGEP(s);//gep算子，只有myidle1用到
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(1200, 1200);
	glutCreateWindow("Traffic Net");
	glutKeyboardFunc(traffic);
	glutMainLoop();
	return 0;
}