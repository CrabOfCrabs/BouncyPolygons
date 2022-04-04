#include<stdlib.h>
#include<stdio.h>
#include<curses.h>
#include<math.h>
#include<time.h>
#include<sys/ioctl.h>
#include<unistd.h>

struct winsize w;

static double G_gravity = 9.81;

typedef struct vector2d{
	double x;
	double y;}Vec2;

typedef struct triangle{
	Vec2 p1;
	Vec2 p2;
	Vec2 p3;
}Tri;

void moveVec(Vec2 *v,Vec2 offs){
	v->x += offs.x;
	v->y += offs.y;}
void moveTri(Tri *t,Vec2 offs){
	moveVec(&t->p1,offs);
	moveVec(&t->p2,offs);
	moveVec(&t->p3,offs);}

double dot(Vec2 p1 , Vec2 p2){
	double prod = p1.x * p2.x + p1.y + p2.y;
	return prod;}

Vec2 cenTri(Tri t){
	Vec2 v ={(t.p1.x+t.p2.x+t.p3.x)/3,(t.p1.y+t.p2.y+t.p3.y)/3};
	return v;}

double mmoiTri(Tri t,double mass){
	double I = 1/6*mass*(dot(t.p1,t.p1)+dot(t.p2,t.p2)+dot(t.p3,t.p3)+dot(t.p1,t.p2)+dot(t.p2,t.p3)+dot(t.p1,t.p3));
	return I;}

int main(){
	int ms = 9;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	time_t start, now;struct timespec delay;delay.tv_sec = 0;delay.tv_nsec = ms * 999999L;time(&start);
	
	initscr();
	clear();
	Vec2 p[3] = {{1,2},{2,3},{4,2}};Tri T_test = {p[0],p[1],p[2]};
	Vec2 cen = cenTri(T_test);

	double mass = 1;
	
	double MMOI = mmoiTri(T_test,mass);

	Vec2 linear_V = {0,0};
	double angular_V = 0;



	while(1){



	}

	return 0;}

