#include <stdlib.h>
#include <stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<time.h>
#include<GL/glut.h>
#include<cmath>
#include<windows.h>
#include <sstream>
using namespace std;
#define MAXT 1000
const int phase_time = 10;
const int cycle_time = 40;
const int maxR = 21;
const int maxCar = 1000;
const int maxCross = 21;//��·������
const int maxE = 14;//��ڳ�������
const float grid = 0.02;
const float car_length = 0.003;
const int max_length = 200;
const int max_integer = 0xfffffff;
const int maxPhaseTime = 40;
const int phaseNum = 4;
const int phaseRank = 24;
class TrafficSystem {
public:
	int phase[4][4][4] = {
					   0,0,0,0,  0,0,1,1,  0,0,0,0,  1,1,0,0, //��λ0
					   0,0,0,0,  1,0,0,0,  0,0,0,0,  0,0,1,0,//��λ1
					   0,0,0,1,  0,0,0,0,  0,1,0,0,  0,0,0,0,//��λ2
					   0,1,1,0,  0,0,0,0,  1,0,0,1,  0,0,0,0//��λ3
	};
	int color[4][12] = { 0,0,0, 1,1,0, 0,0,0, 0,1,1,
						 0,0,0, 0,0,1, 0,0,0, 1,0,0,
						 0,0,1, 0,0,0, 1,0,0, 0,0,0,
						 1,1,0, 0,0,0, 0,1,1, 0,0,0 };
	int linkroad_map[4][4] = { -1,1,1,0,
								3,-1,2,2,
								2,3,-1,2,
								1,1,0,-1 };
	const GLfloat R = 0.005;
	const GLfloat Pi = 3.1415926536;
	const int max_velocity = 3;
	int currentTime = 0;//��ǰʱ��
	int C = 0;
	int Pass = 0;
	bool tag = true;
	int cont = 0;
	float Distance = 0;
	float Velocity = 0;
	int c_map[maxCross][maxCross] = { 0 };//��¼·����·��֮��ĵ�·id
	int r_map[maxR][maxR] = { 0 };//��¼��·���·֮���ת��id������Ӧ���ߵĵ�·���
	int car_cross[maxE][MAXT][2];//����ԭ���Ŀ�ĵ�
	struct Cross {
		int status[4][4];//���̵�״̬�ж�
		int id;
		bool tag;//��·Ϊ0��ʮ��·��Ϊ1
		int state_traffic;
		int linkroad[4];//02��Ӧ���ϣ�31��Ӧ����
		vector<int>linkcross;//·�ڶ�Ӧ��˳��
		float pos[2];//·��λ��
		int nextRuleTime;//�´θ��¹����ʱ��
		int traficSignalTurnTime[4];//������λ����ʱ��
	}cross[maxCross];
	struct Road {
		int len;
		int id;
		int seg[4][max_length];//��¼��·���Ƿ��г������У����Ϊ1
		int inter[2];//������Ϊ0���ϻ�Ϊ1
		vector<int> vichle[4];// �ӱ����ϻ�����Ϊ0����֮Ϊ1
		int delay[4];//�³����
		float start_pos[2][2];
		float end_pos[2][2];
		int direction;//����Ϊ0���ϱ�Ϊ1
	}road[maxR];
	struct Car {
		int id;
		int velocity;
		int pos_road;
		int pos_seg;
		int direction;//�����򶫴ӱ�����Ϊ1����֮Ϊ0
		bool inNet;
		int distance;
		int appear_time;
		int leave_time;
		int turn;//�¸�ת��
		int source;
		int destination;
		float coor[2];
		int pathcross[maxCross];//��һ��·��
		int pathroad[maxCross];//��һ����·
	}car[maxCar];
	void selectFont(int size, int charset, const char* face)
	{
		HFONT hFont = CreateFontA(size, 0, 0, 0, FW_MEDIUM, 0, 0, 0,
			charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, face);
		HFONT hOldFont = (HFONT)SelectObject(wglGetCurrentDC(), hFont);
		DeleteObject(hOldFont);
	}
	void drawCNString(const char* str)
	{
		int len, i;
		wchar_t* wstring;
		HDC hDC = wglGetCurrentDC();
		GLuint list = glGenLists(1);
		len = 0;
		for (i = 0; str[i] != '\0'; ++i)
		{
			if (IsDBCSLeadByte(str[i]))
				++i;
			++len;
		}
		wstring = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, -1, wstring, len);
		wstring[len] = L'\0';
		for (i = 0; i < len; ++i)
		{
			wglUseFontBitmapsW(hDC, wstring[i], 1, list);
			glCallList(list);
		}
		free(wstring);
		glDeleteLists(list, 1);
	}
	void drawword()
	{
		selectFont(32, GB2312_CHARSET, "����");
		char Time[25];
		_itoa_s(currentTime, Time, 10);
		char v[25];
		char s[100];   
		float x = ans();
//		cout << x << endl;
		sprintf_s(v, "%.2f", 20 * x);
		strcat_s(v, "km/h");
		glRasterPos2f(-0.2f, 0.2f);
		drawCNString("Time��");
		glRasterPos2f(0, 0.2f);
		drawCNString(Time);

		glRasterPos2f(-0.2f, -0.1f);
		drawCNString("V��");
		glRasterPos2f(0, -0.1);
		drawCNString(v);
		if (currentTime == 201) {
			cout << "pause";
		}
	}
	void drawcross() {
		float t[12][2] = { -0.5,1, 0,1, 0.5,1,//��
							1,0.5,1,0,1,-0.5,//��
						-0.5,-1,0,-1,0.5,-1,//��
						-1,0.5,-1,0,-1,-0.5 };//��
		for (int i = 0; i < maxCross; i++) {
			if (cross[i].tag) {
				for (int k = 0; k < 12; k++) {
					if (color[cross[i].state_traffic][k])
						glColor3f(0.0f, 1.0f, 0.0f);
					else
						glColor3f(1.0f, 0.0f, 0.0f);
					glBegin(GL_POLYGON);
					for (int j = 0; j < 20; ++j) {
						glVertex2f(R * cos(2 * Pi / 20 * j) + GLfloat(cross[i].pos[0] + t[k][0] * grid), R * sin(2 * Pi / 20 * j) + GLfloat(cross[i].pos[1] + t[k][1] * grid));
					}
					glEnd();
					glColor3f(1.0f, 1.0f, 1.0f);
				}
			}
		}
	}
	void drawroad() {
		for (int i = 0; i < maxR; i++) {
			for (int j = 0; j < 2; j++) {
				glBegin(GL_LINES);
				glVertex2f(road[i].start_pos[j][0], road[i].start_pos[j][1]);
				glVertex2f(road[i].end_pos[j][0], road[i].end_pos[j][1]);
				glEnd();
			}
			glEnable(GL_LINE_STIPPLE);
			glLineWidth(0.1);
			glLineStipple(1, 0x0F0F);
			glBegin(GL_LINES);
			glVertex2f((road[i].start_pos[0][0] + road[i].start_pos[1][0]) / 2, (road[i].start_pos[0][1] + road[i].start_pos[1][1]) / 2);
			glVertex2f((road[i].end_pos[0][0] + road[i].end_pos[1][0]) / 2, (road[i].end_pos[0][1] + road[i].end_pos[1][1]) / 2);
			glEnd();
			glDisable(GL_LINE_STIPPLE);
		}
	}
	void drawcar() {
		int n = 20;
		for (int i = 0; i < C; i++) {
			if (car[i].inNet) {
				GLfloat car_color[4][3] = {
					1.0f, 1.0f, 0.0f,
					1.0f, 0.0f, 1.0f,
					0.0f, 1.0f, 1.0f,
					0.0f, 0.0f, 1.0f,
				};
				glColor3f(car_color[i % 4][0], car_color[i % 4][1], car_color[i % 4][2]);
				glColor3f(1.0f, 1.0f, 0.0f);
				glBegin(GL_POLYGON);
				glVertex2f(GLfloat(car[i].coor[0]) - car_length, GLfloat(car[i].coor[1]) + car_length);
				glVertex2f(GLfloat(car[i].coor[0]) + car_length, GLfloat(car[i].coor[1]) + car_length);
				glVertex2f(GLfloat(car[i].coor[0]) + car_length, GLfloat(car[i].coor[1]) - car_length);
				glVertex2f(GLfloat(car[i].coor[0]) - car_length, GLfloat(car[i].coor[1]) - car_length);
				//for (int j = 0; j < n; ++j) {
				//	glVertex2f(R * cos(2 * Pi / n * j) + GLfloat(car[i].coor[0]), R * sin(2 * Pi / n * j) + GLfloat(car[i].coor[1]));
				//}
				glEnd();
				glColor3f(1.0f, 1.0f, 1.0f);
			}
		}
	}
	void display(int x)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glRasterPos2f(-0.5f, 0.6f);
		drawroad();
		drawcar();
		drawcross();
		drawword();
		glFlush();
	}
	void init(int carCross[maxE][MAXT][2]) {
		float c_pos[maxCross][2] = { -1,0.35,-0.7,0.9,-0.3,0.8,0.2,0.6,0.8,0.7,1,0.35,0.2,0,0.8,-0.3,
									-0.9,-0.4,-0.7,-0.8,-0.3,-0.8,0.6,-0.8,0.8,-0.4,0.6,0.1,
									-0.7,0.35,-0.3,0.35,0.2,0.35,0.8,0.35,
									-0.7,-0.4,-0.3,-0.4,0.6,-0.4 };
		for (int i = 0; i < maxCross; i++) {
			cross[i].pos[0] = c_pos[i][0];
			cross[i].pos[1] = c_pos[i][1];
			cross[i].nextRuleTime = 0;
		}
		for (int i = 0; i < maxCross; i++) {
			for (int j = 0; j < maxCross; j++) {
				c_map[i][j] = -1;
			}
		}
		int cont = 0;
		c_map[0][14] = 0; c_map[1][14] = 1; c_map[2][15] = 2; c_map[3][16] = 3; c_map[4][17] = 4;
		c_map[14][15] = 5; c_map[15][16] = 6; c_map[16][17] = 7; c_map[5][17] = 8;
		c_map[14][18] = 9; c_map[15][19] = 10; c_map[6][16] = 11; c_map[7][17] = 12;
		c_map[8][18] = 13; c_map[18][19] = 14; c_map[19][20] = 15; c_map[13][20] = 16;
		c_map[12][20] = 17; c_map[9][18] = 18; c_map[10][19] = 19; c_map[11][20] = 20;
		//��ÿ����·�������ӵ�ʮ��·��
		for (int i = 0; i < maxCross; i++) {
			for (int j = 0; j < maxCross; j++) {
				if (c_map[i][j] >= 0) {
					cross[i].linkcross.push_back(j);
					cross[j].linkcross.push_back(i);
					if (cross[i].pos[0] == cross[j].pos[0]) {
						road[c_map[i][j]].direction = 1;//�ϱ�����
						if (cross[i].pos[1] > cross[j].pos[1]) {
							road[c_map[i][j]].inter[0] = i;
							road[c_map[i][j]].inter[1] = j;
						}
						else {
							road[c_map[i][j]].inter[0] = j;
							road[c_map[i][j]].inter[1] = i;
						}
						//����ÿ����·����ʼλ�ú���ֹλ��
						road[c_map[i][j]].start_pos[0][0] = cross[road[c_map[i][j]].inter[0]].pos[0] - grid;
						road[c_map[i][j]].start_pos[1][0] = cross[road[c_map[i][j]].inter[0]].pos[0] + grid;
						road[c_map[i][j]].start_pos[0][1] = cross[road[c_map[i][j]].inter[0]].pos[1] - grid;
						road[c_map[i][j]].start_pos[1][1] = cross[road[c_map[i][j]].inter[0]].pos[1] - grid;
						road[c_map[i][j]].end_pos[0][0] = cross[road[c_map[i][j]].inter[1]].pos[0] - grid;
						road[c_map[i][j]].end_pos[1][0] = cross[road[c_map[i][j]].inter[1]].pos[0] + grid;
						road[c_map[i][j]].end_pos[0][1] = cross[road[c_map[i][j]].inter[1]].pos[1] + grid;
						road[c_map[i][j]].end_pos[1][1] = cross[road[c_map[i][j]].inter[1]].pos[1] + grid;
					}
					else {
						road[c_map[i][j]].direction = 0;//��������
						if (cross[i].pos[0] < cross[j].pos[0]) {
							road[c_map[i][j]].inter[0] = i;
							road[c_map[i][j]].inter[1] = j;
						}
						else {
							road[c_map[i][j]].inter[0] = j;
							road[c_map[i][j]].inter[1] = i;
						}
						//����ÿ����·����ʼλ�ú���ֹλ��
						road[c_map[i][j]].start_pos[0][1] = cross[road[c_map[i][j]].inter[0]].pos[1] - grid;
						road[c_map[i][j]].start_pos[1][1] = cross[road[c_map[i][j]].inter[0]].pos[1] + grid;
						road[c_map[i][j]].start_pos[0][0] = cross[road[c_map[i][j]].inter[0]].pos[0] + grid;
						road[c_map[i][j]].start_pos[1][0] = cross[road[c_map[i][j]].inter[0]].pos[0] + grid;
						road[c_map[i][j]].end_pos[0][1] = cross[road[c_map[i][j]].inter[1]].pos[1] - grid;
						road[c_map[i][j]].end_pos[1][1] = cross[road[c_map[i][j]].inter[1]].pos[1] + grid;
						road[c_map[i][j]].end_pos[0][0] = cross[road[c_map[i][j]].inter[1]].pos[0] - grid;
						road[c_map[i][j]].end_pos[1][0] = cross[road[c_map[i][j]].inter[1]].pos[0] - grid;
					}
				}
			}
		}
		for (int i = 0; i < maxCross; i++) {
			for (int j = 0; j < maxCross; j++) {
				if (c_map[i][j] >= 0)
					c_map[j][i] = c_map[i][j];
			}
		}
		//��ÿ��·�ڼ������ӵĵ�·
		for (int i = 0; i < maxCross; i++) {
			if (cross[i].linkcross.size() == 4) {//ʮ��·��
				cross[i].tag = 1;
				for (int j = 0; j < 4; j++) {
					if (cross[cross[i].linkcross[j]].pos[0] - cross[i].pos[0] == 0) {
						if (cross[cross[i].linkcross[j]].pos[1] - cross[i].pos[1] > 0)
							cross[i].linkroad[0] = c_map[i][cross[i].linkcross[j]];
						else
							cross[i].linkroad[2] = c_map[i][cross[i].linkcross[j]];
					}
					else {
						if (cross[cross[i].linkcross[j]].pos[0] - cross[i].pos[0] < 0)
							cross[i].linkroad[3] = c_map[i][cross[i].linkcross[j]];
						else
							cross[i].linkroad[1] = c_map[i][cross[i].linkcross[j]];
					}
				}
			}
			//��ʮ��·��
			else {
				cross[i].tag = 0;
				cross[i].linkroad[0] = c_map[i][cross[i].linkcross[0]];
			}
		}
		for (int i = 0; i < maxR; i++) {
			road[i].len = abs(road[i].end_pos[0][road[i].direction] - road[i].start_pos[0][road[i].direction]) / (grid / 2);
			//			cout << i << " " << road[i].start_pos[0][0] << " " << road[i].end_pos[0][0] << " " << road[i].len << endl;
			road[i].id = i;
			for (int j = 0; j < road[i].len; j++) {
				for (int k = 0; k < 4; k++) {
					road[i].seg[k][j] = 0;
				}
			}
		}
		//Ԥ���·֮��ת��֮ǰ������ʻ��·�����ϱ��������0123����������ϵ���0123
		
		for (int i = 0; i < maxR; i++) {
			for(int j = 0; j < maxR; j++) {
				r_map[i][j] = -1;
			}
		}
		for (int i = 0; i < maxCross; i++) {
			if (cross[i].tag == 1) {
				for (int j = 0; j < 4; j++) {
					for (int k = 0; k < 4; k++) {
						r_map[cross[i].linkroad[j]][cross[i].linkroad[k]] = linkroad_map[j][k];
					}
				}
			}
		}
		//��ʼ���������
		for (int i = 0; i < maxE; i++) {
			for (int j = 0; j < MAXT; j++) {
				car_cross[i][j][0] = carCross[i][j][0];
				car_cross[i][j][1] = carCross[i][j][1];
			}
		}
	}
	void find_path(int s, int d, Car* car) {
		for (int i = 0; i < maxCross; i++) {
			car->pathroad[i] = -1;
		}
		int post_cross[maxCross];//��s����ĳ·�ڵ�ǰһ��·��
		float dis[maxCross][maxCross];
		for (int i = 0; i < maxCross; i++) {
			post_cross[i] = s;
			for (int j = 0; j < maxCross; j++) {
				if (c_map[i][j] == -1)
					dis[i][j] = FLT_MAX;
				else {
					dis[i][j] = road[c_map[i][j]].len;
				}	
			}
		}
		int tag[maxCross] = { 0 };
		for (int i = 0; i < maxCross; i++) {
			float temp_len = FLT_MAX;
			int temp_index = 0;
			for (int j = 0; j < maxCross; j++) {
				if (tag[j] == 0 && dis[s][j] < temp_len) {
					temp_len = dis[s][j];
					temp_index = j;
				}
			}
			tag[temp_index] = 1;
			for (int j = 0; j < maxCross; j++) {
				if (dis[s][j] > dis[s][temp_index] + dis[temp_index][j]) {
					dis[s][j] = dis[s][temp_index] + dis[temp_index][j];
					post_cross[j] = temp_index;
				}
			}
		}
		while (d != s) {
			int temp = post_cross[d];
			car->pathcross[temp] = d;
			car->pathroad[temp] = c_map[temp][d];
			d = temp;
		}
	}
	void appear_car() {
		for (int i = 0; i < maxE; i++) {
			if (car_cross[i][currentTime][0] != -1) {
				car[C].source = car_cross[i][currentTime][0];
				car[C].destination = car_cross[i][currentTime][1];
				find_path(car[C].source, car[C].destination, car + C);//�ҵ�������ʻ��·
				int entrance_road = cross[car_cross[i][currentTime][0]].linkroad[0];
				car[C].pos_road = entrance_road;
				car[C].direction = r_map[car[C].pos_road][car[C].pathroad[car[C].pathcross[car[C].source]]];//�жϳ���ʻ�����Լ�·��
				car[C].pos_seg = road[entrance_road].len - 1;
				if (car[C].direction <= 1) {
					car[C].pos_seg = 0;
				}
				car[C].distance = 0;
				car[C].appear_time = currentTime;
				car[C].id = C;
				car[C].turn = -1;
				car[C].inNet = true;
				road[entrance_road].seg[car[C].direction][car[C].pos_seg] = 1;
				road[entrance_road].vichle[car[C].direction].push_back(C);
				//			printf("the car%d appear in the cross%d\n", C, org_cross);
							//		drawcar(road[entrance_road].start_pos[0][0], road[entrance_road].start_pos[0][1]);
				C++;
			}
		}
	}
	void traffic_signal(int s[]) {
		int temp = 0;
		for (int i = 0; i < maxCross; i++) {
			if (cross[i].tag) {
				//�źŵƺ��̵�״̬�ж�
				cross[i].state_traffic = s[temp++];
				memcpy(cross[i].status, phase[cross[i].state_traffic], sizeof(cross[i].status));
			}
		}
	}
	void run() {
		for (int i = 0; i < maxCross; i++) {
			if (cross[i].tag == 0) {//��·����
				int curr_road = cross[i].linkroad[0];
				int direction = 0;
				if (road[curr_road].inter[0] == i) {
					direction = 2;
				}
				for (int temp_direction = 0; temp_direction < 2; temp_direction++) {
					direction += temp_direction;
					for (int j = 0; j < road[curr_road].vichle[direction].size(); j++) {
						int curr_car = road[curr_road].vichle[direction][j];
						int curr_pos = car[curr_car].pos_seg;
						int k = 0;
						road[curr_road].seg[direction][curr_pos] = 0;
						if (direction >= 2) {
							while (curr_pos + k - 1 >= 0 && road[curr_road].seg[direction][curr_pos + k - 1] == 0 && abs(k) < max_velocity) {
								k--;
							}
						}
						else {
							while (curr_pos + k + 1 < road[curr_road].len && road[curr_road].seg[direction][curr_pos + k + 1] == 0 && abs(k) < max_velocity) {
								k++;
							}
						}
						car[curr_car].pos_seg = curr_pos + k;

						if ((car[curr_car].pos_seg >= road[curr_road].len - 1 || car[curr_car].pos_seg <= 0) && abs(k) < max_velocity) {
							//			if(abs(k)<max_velocity){
							car[curr_car].inNet = false;
							car[curr_car].leave_time = currentTime;
							Pass++;
							//						printf("the car%d has been passed the traffic network in the cross%d and road%d\n", curr_car, i, curr_road);
							road[curr_road].vichle[direction].erase(road[curr_road].vichle[direction].begin());
							j--;
						}
						else
							road[curr_road].seg[direction][curr_pos + k] = 1;
						car[curr_car].velocity = car[curr_car].pos_seg - curr_pos;
						car[curr_car].distance += abs(car[curr_car].velocity);
					}
				}
			}
			else {//ʮ��·��
				for (int j = 0; j < 4; j++) {
					Road* curr_road = road + cross[i].linkroad[j];
					int direction = 0;
					if (j == 2 || j == 1) {
						direction = 2;
					}
					for (int temp_cont = 0; temp_cont < 2; temp_cont++) {//����������·
						direction += temp_cont;
						for (int k = 0; k < curr_road->vichle[direction].size(); k++) {
							Car* curr_car = car + curr_road->vichle[direction][k];
							int curr_pos = curr_car->pos_seg;
							int z = 0;//��¼����Ӧ��ǰ����λ�Ƴ���
							curr_road->seg[direction][curr_pos] = 0;//ԭ��λ�ü�¼����
							if (direction >= 2) {
								while (curr_pos + z - 1 >= 0 && curr_road->seg[direction][curr_pos + z - 1] == 0 && abs(z) < max_velocity) {
									z--;
								}
							}
							else {
								while (curr_pos + z + 1 < curr_road->len && curr_road->seg[direction][curr_pos + z + 1] == 0 && abs(z) < max_velocity) {
									z++;
								}
							}

							//				printf("the velocity:%d\n", z);

							if ((curr_pos + z >= curr_road->len - 1 || curr_pos + z <= 0) && abs(z) < max_velocity) {//·����ʻ��Ͽ�ʼת��
								//if (curr_car->turn == -1) {//�����в�Ϊ-1��bug����֪��Ϊʲô
									for (int turnId = 0; turnId < 4; turnId++) {
										if (cross[i].linkroad[turnId] == curr_car->pathroad[i]) {
											curr_car->turn = turnId;
										}
									}
								//}
								//ʮ��·��ת��
								Road* turn_road = road + cross[i].linkroad[curr_car->turn];
								int direction2;
								if (curr_car->pathroad[curr_car->pathcross[i]] == -1) {//��������һ��ת���·
									if (road[turn_road->id].inter[0] == curr_car->destination) {//���������ΪĿ�ĵ�
										direction2 = 2;
									}
									else if (road[turn_road->id].inter[1] == curr_car->destination) {//���������ΪĿ�ĵ�
										direction2 = 0;
									}
									else {
										cout << "error";
									}
								}
								else {//�������ת���·��������һ��ת���·ѡ����
									direction2 = r_map[turn_road->id][curr_car->pathroad[curr_car->pathcross[i]]];
								}
								int new_pos = turn_road->len - 1;
								if (direction2 <= 1) {
									new_pos = 0;
								}
								//string dire[4] = { "around","right","straight","left" };
								//cout << "the car" << curr_car->id << " in " << " cross" << i << " in direction" << direction2 << " turn "
								//	<< dire[(curr_car->turn - j + 4) % 4] << " from road" << curr_road->id << " to road" << turn_road->id << endl;
			//										if (turn_road->seg[direction2][new_pos] == 0 && (cross[i].status[j] == 1 || (rnd - j + 4) % 4 == 1)) {//��λ��Ϊת��·�ڵ���ʼλ��
								if (turn_road->seg[direction2][new_pos] == 0 && (cross[i].status[j][curr_car->turn] == 1)) {//��λ��Ϊת��·�ڵ���ʼλ��
									turn_road->vichle[direction2].push_back(curr_car->id);
									curr_road->vichle[direction].erase(curr_road->vichle[direction].begin());
									k--;
									curr_car->pos_road = turn_road->id;
									curr_car->pos_seg = new_pos;
									curr_car->direction = direction2;
									turn_road->seg[direction2][new_pos] = 1;
									curr_car->velocity = z;
									curr_car->turn = -1;
								}
								else {//·�ڶ���
									if (direction >= 2)
										curr_car->pos_seg = 0;
									else
										curr_car->pos_seg = curr_road->len - 1;
									curr_road->seg[direction][curr_car->pos_seg] = 1;
									curr_car->velocity = curr_car->pos_seg - curr_pos;
									//								cout << " blocked";
								}
								//							cout << endl;
							}//��ת�����λ��
							else {
								curr_road->seg[direction][curr_pos + z] = 1;
								curr_car->pos_seg = curr_pos + z;
								curr_car->velocity = z;
							}
							curr_car->distance += abs(curr_car->velocity);
						}
						
						//�����·�ϵĶ³�����
						int pos = curr_road->len - 1;
						if (direction >= 2)
							pos = 0;
						curr_road->delay[direction] = 0;
						while (pos >=0 && pos < curr_road->len && curr_road->seg[direction][pos] == 1) {
							curr_road->delay[direction]++;
							if (direction >= 2)
								pos++;
							else
								pos--;
						}
					}
				}
			}
		}

		currentTime++;
	}
	void car_coor() {
		for (int i = 0; i < C; i++) {
			if (car[i].direction >= 2) {
				if (road[car[i].pos_road].direction == 1) {//�ϱ�����
					car[i].coor[0] = (road[car[i].pos_road].start_pos[1][0] + road[car[i].pos_road].start_pos[0][0]) / 2 + grid / 4 + grid / 2 * (car[i].direction - 2);
					car[i].coor[1] = road[car[i].pos_road].start_pos[0][1] + (road[car[i].pos_road].end_pos[0][1] - road[car[i].pos_road].start_pos[0][1]) * car[i].pos_seg / road[car[i].pos_road].len;
				}
				else {//��������
					car[i].coor[0] = road[car[i].pos_road].start_pos[0][0] + (road[car[i].pos_road].end_pos[0][0] - road[car[i].pos_road].start_pos[0][0]) * car[i].pos_seg / road[car[i].pos_road].len;
					car[i].coor[1] = (road[car[i].pos_road].start_pos[1][1] + road[car[i].pos_road].start_pos[0][1]) / 2 + grid / 4 + grid / 2 * (car[i].direction - 2);
				}
			}
			else {
				if (road[car[i].pos_road].direction == 1) {//�ϱ�����
					car[i].coor[0] = (road[car[i].pos_road].start_pos[1][0] + road[car[i].pos_road].start_pos[0][0]) / 2 - grid / 4 - grid / 2 * (1 - car[i].direction);
					car[i].coor[1] = road[car[i].pos_road].start_pos[1][1] +
						(road[car[i].pos_road].end_pos[1][1] - road[car[i].pos_road].start_pos[1][1]) * car[i].pos_seg / road[car[i].pos_road].len;
				}
				else {//��������
					car[i].coor[0] = road[car[i].pos_road].start_pos[1][0] +
						(road[car[i].pos_road].end_pos[1][0] - road[car[i].pos_road].start_pos[1][0]) * car[i].pos_seg / road[car[i].pos_road].len;
					car[i].coor[1] = (road[car[i].pos_road].start_pos[1][1] + road[car[i].pos_road].start_pos[0][1]) / 2 - grid / 4 - grid / 2 * (1 - car[i].direction);
				}

			}
			//			printf("the car id %d\tthe road id %d\tthe segment id %d\tthe direction %d\tthe coordinate %f %f\n", i, car[i].pos_road, car[i].pos_seg, car[i].direction, car[i].coor[0], car[i].coor[1]);
		}
	}
	void information() {
		for (int i = 0; i < C; i++) {
			printf("the car id %d\tthe road id %d\tthe segment id %d\tthe direction %d\n", i, car[i].pos_road, car[i].pos_seg, car[i].direction);
		}
		for (int i = 0; i < maxR; i++) {
			for (int j = 0; j < 2; j++) {
				printf("road%d direction%d: ", i, j);
				for (int k = 0; k < road[i].vichle[j].size(); k++) {
					printf("car%d(%d) ", road[i].vichle[j][k], car[road[i].vichle[j][k]].pos_seg);
				}
				printf("\n");
			}
		}
	}
	void test(int state[cycle_time/phase_time][maxCross-maxE]) {
		for (int phase = 0; phase < cycle_time / phase_time; phase++) {
			traffic_signal(state[phase]);
			int start_time = currentTime;
			while (currentTime < start_time + phase_time) {
				appear_car();
				/*cout << "Time:" << T << endl;
				for (int i = 0; i < C; i++) {
					printf("the car id %d\tthe road id %d\tthe segment id %d\tthe direction %d\n", i, car[i].pos_road, car[i].pos_seg, car[i].direction);
				}
				for (int i = 0; i < maxR; i++) {
					for (int j = 0; j < 2; j++) {
						printf("road%d direction%d: ", i, j);
						for (int k = 0; k < road[i].vichle[j].size(); k++) {
							printf("car%d(%d) ", road[i].vichle[j][k], car[road[i].vichle[j][k]].pos_seg);
						}
						printf("\n");
					}
				}
				cout << endl << endl;*/
				run();		
			}
		}
	}
	float ans() {
		if (C == 0)
			return 0;
//		printf("the passed vichle number: %d\n", Pass);
		double ans = 0;
		for (int i = 0; i < C; i++) {
			double v = 0;
			if (car[i].inNet)
				v = double(car[i].distance) / (currentTime - car[i].appear_time);
			else
				v = double(car[i].distance) / (car[i].leave_time - car[i].appear_time);
			//		printf("the car%d the velocity %lf\n", i, v);
			ans += v;
		}
//		printf("the average velocity: %lf\n", Velocity / C);
//		cout << endl << endl;
		Velocity = ans / C;
		return Velocity;
	}
};