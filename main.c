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
	v->x += offs.x/100;
	v->y += offs.y/100;}
void moveTri(Tri *t,Vec2 offs){
	moveVec(&t->p1,offs);
	moveVec(&t->p2,offs);
	moveVec(&t->p3,offs);}
Vec2 addVec(Vec2 p,Vec2 offs){
	Vec2 v;
	v.x =p.x+ offs.x;
	v.y =p.y+ offs.y;
	return v;
}

double dot(Vec2 p1 , Vec2 p2){
	double prod = p1.x * p2.x + p1.y * p2.y;
	return prod;}

Vec2 cenTri(Tri t){
	Vec2 v ={(t.p1.x+t.p2.x+t.p3.x)/3,(t.p1.y+t.p2.y+t.p3.y)/3};
	return v;}

double mmoiTri(Tri t,double mass){
	double I = 1/6*mass*(dot(t.p1,t.p1)+dot(t.p2,t.p2)+dot(t.p3,t.p3)+dot(t.p1,t.p2)+dot(t.p2,t.p3)+dot(t.p1,t.p3));
	return I;}
void rotateVec(Vec2 *p,Vec2 cen,double torque){
        Vec2 pR = {((p->x-cen.x)*cos(torque)-(p->y-cen.y)*sin(torque))+cen.x , ((p->y-cen.y)*cos(torque)+(p->x-cen.x)*sin(torque))+cen.y};

        p->x = pR.x;p->y = pR.y;
}
void rotateTri(Tri *tp,Vec2 cen,double torque){

        rotateVec(&tp->p1,cen,torque);
        rotateVec(&tp->p2,cen,torque);
        rotateVec(&tp->p3,cen,torque);
}
void swapp(Vec2 *p1,Vec2 *p2){Vec2 tmp = *p1;*p1 = *p2;*p2 = tmp;}
double inpolx(Vec2 p1,Vec2 p2,double y){double x = p1.x + (p2.x - p1.x)*(y - p1.y)/(p2.y - p1.y);return x;}


void DSline(double x1,double x2,double y ){ //Draws Simple line
        double x,x2l;
        if(x1<=x2){x=x1;x2l = x2;}else{x=x2;x2l = x1;}
        do{mvprintw(y/2,x,"#");}while(x++<x2l);}

void drawtup(Tri t){
        Vec2 p1 = t.p1,p2 = t.p2,p3 = t.p3;
        double invslope1 = (p2.x - p1.x) / (p2.y - p1.y),invslope2 = (p3.x - p1.x) / (p3.y - p1.y),curx1 = p1.x,curx2 = p1.x;
        for(double Y = p1.y; Y < p2.y; Y++){
                DSline(curx1,curx2,Y);
                curx1 += invslope1;curx2 += invslope2;}}
void drawtdown(Tri t){
        Vec2 p1 = t.p1,p2 = t.p2,p3 = t.p3;
        double invslope1 = (p3.x - p1.x) / (p3.y - p1.y),invslope2 = (p3.x - p2.x) / (p3.y - p2.y),curx1 = p3.x,curx2 = p3.x;
        for(double Y = p3.y; Y > p1.y; Y--){
                DSline(curx1,curx2,Y);
                curx1 -= invslope1;curx2 -= invslope2;}}

void scanln(Tri t){
        Tri tr = t;

        if(tr.p2.y < tr.p1.y){ swapp(&tr.p2, &tr.p1); }if(tr.p3.y <tr.p1.y){ swapp(&tr.p3, &tr.p1); }if(tr.p3.y < tr.p2.y){ swapp(&tr.p3, &tr.p2); }

        if(tr.p3.y == tr.p2.y){drawtup(tr);}
        else if(tr.p1.y == tr.p2.y){drawtdown(tr);}
        else{Vec2 p4 = {inpolx(tr.p1,tr.p3,tr.p2.y),tr.p2.y};
        Tri tu = {tr.p1,tr.p2,p4};Tri td = {tr.p2,p4,tr.p3};
        drawtup(tu);drawtdown(td);}}
double cross(Vec2 p1,Vec2 p2){return ((p1.x*p2.y)-(p1.y*p2.x));}

Vec2 mkvec(double x,double y){Vec2 v = {x,y};return v;}

Vec2 border_Check(Vec2 screen ,Vec2 p,Vec2 v,Vec2 cen,double w,double I,double mass,Tri *t){
		Vec2 r = {p.x-cen.x,p.y-cen.y};
		double j;
		Vec2 J={0,0};
                if(p.y > screen.y){
			moveTri(t,mkvec(0,screen.y-p.y));
			Vec2 po = {0,-1};      //workspace roof
                	j = (-2*(dot(v,po)))/(1/mass+(cross(r,po)*cross(r,po)));
                        Vec2 J1 = {0,-1*j};return J1;}
		if(p.y < 0){  //workspace bottom
			moveTri(t,mkvec(0,-p.y));
			Vec2 po = {0,1};      //workspace roof
                	j = (-2*(dot(v,po)))/(1/mass+(cross(r,po)*cross(r,po)));
                        Vec2 J1 = {0,1*j};return J1;}
	/*	else if(p.x >  screen.x){

			Vec2 po = {1,0};      //workspace roof
                	j = (-2*(dot(v,po)+w*cross(r,po)))/(1/mass+(cross(r,po)*cross(r,po)));
                        J.x = 1*j;return J;}
			
		else if(p.x < 0){
			Vec2 po = {1,0};      //workspace roof
                	j = (-2*(dot(v,po)))/(1/mass+(cross(r,po)*cross(r,po)));
                        J.x = 1*j;return J;}*/
		return J;



}

int main(){
	int ms = 10;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	time_t start, now;struct timespec delay;delay.tv_sec = 0;delay.tv_nsec = ms * 999999L;time(&start);
	
	initscr();
	clear();
	Vec2 p[3] = {{12,33},{6,39},{12,42}};Tri T_test = {p[0],p[1],p[2]};


	double mass = 0.1;
	
	double MMOI = mmoiTri(T_test,mass);

	Vec2 v = {0,-10};
	double angular_V = 0;



	while(1){
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
                Vec2 screen = {w.ws_col,w.ws_row};
	
	Vec2 cen = cenTri(T_test);

		Vec2 j3 = border_Check(screen ,T_test.p1,v,cen,angular_V,MMOI,mass,&T_test);
		Vec2 aj1 ={1/mass*j3.x,1/mass*j3.y};
		v = addVec(v,aj1);

		Vec2 j2 =border_Check(screen ,T_test.p2,v,cen,angular_V,MMOI,mass,&T_test);
		Vec2 aj2 ={1/mass*j2.x,1/mass*j2.y};
		v = addVec(v,aj2);

		Vec2 j1 =border_Check(screen ,T_test.p3,v,cen,angular_V,MMOI,mass,&T_test);
		Vec2 aj3 ={1/mass*j1.x,1/mass*j1.y};
		v = addVec(v,aj3);
moveTri(&T_test,v);
		nanosleep(&delay,NULL);
                clear();
                scanln(T_test);
                refresh();}
	getch();
	return 0;}

