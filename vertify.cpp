//#include <stdlib.h>
//#include<iostream>
//#include<string>
//#include<vector>
//#include<time.h>
//#include<GL/glut.h>
//#include<cmath>
//#include<windows.h>
//#include<time.h>
//#include"slGep.h"
//using namespace std;
//const int sample = 50;
//TrafficSystem s[sample];
//SL_GEP gep[sample];
//int state[cycle_time / phase_time][maxCross - maxE] = { 0 };
//void loadCar(int car_cross[maxE][MAXT][2], int carNum) {
//	//��ʼ���������
//	for (int i = 0; i < maxE; i++) {
//		for (int j = 0; j < MAXT; j++) {
//			car_cross[i][j][0] = -1;
//			car_cross[i][j][1] = -1;
//		}
//	}
//	char name[200];
//	sprintf(name, "car\\car%d.txt", carNum);
//	FILE* fp2 = fopen(name, "r");
//	int x, y, z, k;
//	for (int i = 0; i < maxE * maxCar; i++) {
//		fscanf(fp2, "%d %d %d", &x, &y, &z);
//		if (x < maxE && y < MAXT && z < maxE) {
//			car_cross[x][y][0] = x;
//			car_cross[x][y][1] = z;
//		}
//	}
//	fclose(fp2);
//}
//void generateSignal(TrafficSystem s) {
//	//��ַ�����̵�
//	DE compute;
//	int ans = compute.run(s);
//	memcpy(state, compute.indivual[ans].pos, sizeof(state));
//	//for (int i = 0; i < cycle_time / phase_time; i++) {
//	//	for (int j = 0; j < maxCross - maxE; j++) {
//	//		cout << state[i][j] << " ";
//	//	}
//	//	cout << endl;
//	//}
//	//cout << endl;
//}
//double myIdle(TrafficSystem s) {//����Ͳ��
//	for (int i = 0; i < maxTime; i++) {
//		if (s.currentTime % (cycle_time) == 0) {
//			generateSignal(s);
//		}
//		if (s.currentTime % phase_time == 0) {
//			s.traffic_signal(state[(s.currentTime / phase_time) % (cycle_time / phase_time)]);
//		}
//		s.appear_car();
//		s.car_coor();
//		s.run();
//	}
//	printf("%f\n", s.ans() * 20);
//	return s.Velocity * 20;
//}
//int main(int argc, char* argv[]) {
//	srand((int)time(NULL));
//	FILE* fp_ans = fopen("best.txt", "w");
//	//��¼���ֲ�ͬ�ĳ������ɽ��
//	//for (int i = 0; i < sample; i++) {
//	//	//generateCar(i);
//	//	int car_cross[maxE][MAXT][2];//����ԭ���Ŀ�ĵ�
//	//	loadCar(car_cross, i);
//	//	s[i].init(car_cross);
//	//	gep[i].SLGEP(s[i]);//gep���ӣ�ֻ��myidle1�õ�
//	//	fprintf(fp_ans, "%f\n", gep[i].fbest * 20.0);
//	//}
//	for (int i = 0; i < sample; i++) {
//		//generateCar(i);
//		int car_cross[maxE][MAXT][2];//����ԭ���Ŀ�ĵ�
//		loadCar(car_cross, i);
//		s[i].init(car_cross);
//		fprintf(fp_ans, "%f\n", myIdle(s[i]));
//	}
//}