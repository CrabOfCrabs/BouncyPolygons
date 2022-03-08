#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <math.h>
struct winsize w;
#define MAX_ARR 128

typedef struct Vector2{
	double x;
	double y;
}Vec2;


typedef struct Triangle{
	Vec2 *p1;
	Vec2 *p2;
	Vec2 *p3;
;

}Tri;

typedef struct Object{
	Tri arrT[MAX_ARR]; //array of triangles points in triangles point to arrP
	Vec2 *arrP; //a pointer to array with unsorted points
	Vec2 Bounds[2];
	Vec2 MassCen;
	Vec2 Velocity;
	double angularVelocity;
	unsigned int lenB[2];
}Obj;


void swapp(Vec2 *p1,Vec2 *p2){Vec2 tmp = *p1;*p1 = *p2;*p2 = tmp;}
double inpolx(Vec2 p1,Vec2 p2,double y){double x = p1.x + (p2.x - p1.x)*(y - p1.y)/(p2.y - p1.y);return x;}

Obj mkObj(){
	Obj oT;
	oT.arrP = (Vec2*) malloc(1*sizeof(Vec2));
	oT.lenB[0] = 0;
	oT.lenB[1] = 0;

	return oT;	
}

void Add_PointToObj(Vec2 p,Obj o){	
	
	o.lenB[0]+=1;
	o.arrP = realloc(o.arrP ,o.lenB[0] * sizeof(Vec2));
	o.arrP[o.lenB[0]-1] = p;

	if(o.lenB[0] < 3){return;}
	else{
		int d1,d2;
		int *distarr;
		int lenght;
		for(int i = 0;i<o.lenB[0];i++){
			Vec2 tP = o.arrP[i];
		//	(dist ==NULL || dist < sqrt(tP.x*tP.x + tP.y*tP.y))
		}
	
	}

	o.lenB[0] += 1;

}
void DSline(double x1,double x2,double y ){ //Draws Simple line
	double x,x2l;
	if(x1<=x2){x=x1;x2l = x2;}else{x=x2;x2l = x1;}
	do{mvprintw(y,x,"#");}while(x++<x2l);}

void drawtup(Tri t){
	Vec2 *p1 = t.p1,*p2 = t.p2,*p3 = t.p3;
	double invslope1 = (p2->x - p1->x) / (p2->y - p1->y),invslope2 = (p3->x - p1->x) / (p3->y - p1->y),curx1 = p1->x,curx2 = p1->x;
	for(double Y = p1->y; Y < p2->y; Y++){
		DSline(curx1,curx2,Y);
		curx1 += invslope1;curx2 += invslope2;}}
void drawtdown(Tri t){
	Vec2 *p1 = t.p1,*p2 = t.p2,*p3 = t.p3;
	double invslope1 = (p3->x - p1->x) / (p3->y - p1->y),invslope2 = (p3->x - p2->x) / (p3->y - p2->y),curx1 = p3->x,curx2 = p3->x;
	for(double Y = p3->y; Y > p1->y; Y--){
		DSline(curx1,curx2,Y);
		curx1 -= invslope1;curx2 -= invslope2;}}

void scanln(Tri tr){
	//tri tr = projt(rent(t,xr),screen);

	if(tr.p2->y < tr.p1->y){ swapp(tr.p2, tr.p1); }if(tr.p3->y <tr.p1->y){ swapp(tr.p3, tr.p1); }if(tr.p3->y < tr.p2->y){ swapp(tr.p3, tr.p2); }
	
	if(tr.p3->y == tr.p2->y){drawtup(tr);}
	else if(tr.p1->y == tr.p2->y){drawtdown(tr);}
	else{Vec2 p4 = {inpolx(*tr.p1,*tr.p3,tr.p2->y),tr.p2->y};
	Tri tu = {tr.p1,tr.p2,&p4};Tri td = {tr.p2,&p4,tr.p3};
	drawtup(tu);drawtdown(td);}}

Vec2 add_Points(Vec2 p1,Vec2 p2){Vec2 pR = {p1.x+p2.x,p1.y+p2.y};return pR; }

void move_Tri(Tri* tm,Vec2 aF){
	tm->p1->x = tm->p1->x + aF.x;
	tm->p2->x = tm->p2->x + aF.x;
	tm->p3->x = tm->p3->x + aF.x;
	tm->p1->y = tm->p1->y + aF.y;
	tm->p2->y = tm->p2->y + aF.y;
	tm->p3->y = tm->p3->y + aF.y;}



double border_Check(Vec2 screen ,Vec2 *p,Vec2 *f,Tri *tp,Vec2 cen){
		if(p->y > screen.y){Vec2 po = {0,screen.y - p->y};	//workspace roof

			move_Tri(tp,po);f->y*=-1;
		Vec2 U = {f->x,f->y};Vec2 V = {p->x - cen.x , p->y - cen.y};return ((U.x*V.y)-(U.y*V.x));
		}			
		else if(p->y < 0){Vec2 po = {0,-p->y};			//workspace bottom

			move_Tri(tp,po);f->y*=-1;
		Vec2 U = {f->x,f->y};Vec2 V = {p->x - cen.x , p->y - cen.y};return ((U.x*V.y)-(U.y*V.x));		
		}
		if(p->x >  screen.x){Vec2 po = {screen.x - p->x,0};	//workspace left
									
			move_Tri(tp,po);f->x*=-1;
		Vec2 U = {f->x,f->y};Vec2 V = {p->x - cen.x , p->y - cen.y};return ((U.x*V.y)-(U.y*V.x));
		}
		else if(p->x < 0){Vec2 po = {-p->x,0};			//workspace right

			move_Tri(tp,po);f->x*=-1;
		Vec2 U = {f->x,f->y};Vec2 V = {p->x - cen.x , p->y - cen.y};return ((U.x*V.y)-(U.y*V.x));
		}
		else{return 0;}
		


}


void rotateVec(Vec2 *p,Vec2 cen,double torque){
	Vec2 pR = {((p->x-cen.x)*cos(torque)-(p->y-cen.y)*sin(torque))+cen.x , ((p->y-cen.y)*cos(torque)+(p->x-cen.x)*sin(torque))+cen.y};
	
	p->x = pR.x;p->y = pR.y;
}

void rotateTri(Tri *tp,Vec2 cen,double torque){
	
	rotateVec(tp->p1,cen,torque);
	rotateVec(tp->p2,cen,torque);
	rotateVec(tp->p3,cen,torque);
}


Vec2 tricenp(Tri t){
	Vec2 p = {(t.p1->x+t.p2->x+t.p3->x)/3,(t.p1->y+t.p2->y+t.p3->y)/3};
	return p;}
int main(){int ms = 20;
ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	time_t start, now;struct timespec delay;delay.tv_sec = 0;delay.tv_nsec = ms * 999999L;time(&start);

	initscr();
        clear();
	Vec2 pt1 = {25,20},pt2 = {20,10},pt3 = {30,20};
	Tri tp = {&pt1,&pt2,&pt3};
	Vec2 force = {0.1,0};
	double torque = 0;
	int gravacc = 0;
	while(1){
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
		Vec2 screen = {w.ws_col,w.ws_row};
		//gravacc =+ 0.1;
		Vec2 cen = tricenp(tp);rotateTri(&tp,cen,torque/180);

		force.y =force.y+ 0.01;move_Tri(&tp,force);

		torque = torque + border_Check(screen,tp.p1,&force,&tp,cen);					
		torque = torque + border_Check(screen,tp.p2,&force,&tp,cen);	
		torque = torque + border_Check(screen,tp.p3,&force,&tp,cen);
		nanosleep(&delay,NULL);
		clear();
		scanln(tp);
		refresh();}

        getch();
	return 0;}
