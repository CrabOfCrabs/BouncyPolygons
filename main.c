#include<stdlib.h>
#include<stdio.h>
#include<curses.h>
#include<math.h>
#include<time.h>
#include<sys/ioctl.h>
#include<unistd.h>

struct winsize win;

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
Vec2 minusVec(Vec2 p,Vec2 offs){
	Vec2 v;
	v.x =p.x- offs.x;
	v.y =p.y- offs.y;
	return v;
}
double magp(Vec2 p){double mg = sqrt((p.x*p.x) + (p.y*p.y));return mg;}
double dot(Vec2 p1 , Vec2 p2){
	double prod = p1.x * p2.x + p1.y * p2.y;
	return prod;}

Vec2 cenTri(Tri t){
	Vec2 v ={(t.p1.x+t.p2.x+t.p3.x)/3,(t.p1.y+t.p2.y+t.p3.y)/3};
	return v;}

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

void scanln(Tri tr){
        

        if(tr.p2.y < tr.p1.y){ swapp(&tr.p2, &tr.p1); }if(tr.p3.y <tr.p1.y){ swapp(&tr.p3, &tr.p1); }if(tr.p3.y < tr.p2.y){ swapp(&tr.p3, &tr.p2); }

        if(tr.p3.y == tr.p2.y){drawtup(tr);}
        else if(tr.p1.y == tr.p2.y){drawtdown(tr);}
        else{Vec2 p4 = {inpolx(tr.p1,tr.p3,tr.p2.y),tr.p2.y};
        Tri tu = {tr.p1,tr.p2,p4};Tri td = {tr.p2,p4,tr.p3};
        drawtup(tu);drawtdown(td);}}
double cross(Vec2 p1,Vec2 p2){return ((p1.x*p2.y)-(p1.y*p2.x));}

Vec2 mkvec(double x,double y){Vec2 v = {x,y};return v;}
Vec2 crossd(Vec2 p ,double d){Vec2 r = {-1*d*p.y,d*p.x};return r;}

void borderCheck(Vec2 screen ,double I,double mass,double *w,Tri *t,Vec2 *vT){
	Vec2 triT[3] = {t->p1,t->p2,t->p3}; 
	Vec2 cen = cenTri(*t);
	for(int i = 2;i>=0;i--){
		Vec2 p = triT[i];
		Vec2 r = {p.x-cen.x,p.y-cen.y};Vec2 v = addVec(*vT,crossd(r,*w));

		double j;
		double e = 1;
		double Bounds_Friction =1;
		Vec2 J;
                if(p.y > screen.y*2){
			moveTri(t,mkvec(0,(screen.y*2-p.y)*100));
			
			Vec2 po = {0,-1};     
			j = ((-1-e)*(dot(v,po)))/(1/mass+(cross(r,po)*cross(r,po))/I);
                        J.y = -1*j;}
		else if(p.y < 0){
			moveTri(t,mkvec(0,p.y*-100));	
	
			Vec2 po = {0,1};     
			j = ((-1-e)*(dot(v,po)))/(1/mass+(cross(r,po)*cross(r,po))/I);
                        J.y = 1*j;}
		else if(p.x >  screen.x){
			moveTri(t,mkvec((screen.x-p.x)*100,0));	
			
			Vec2 po = {-1,0};
			j = ((-1-e)*(dot(v,po)))/(1/mass+(cross(r,po)*cross(r,po))/I);
                        J.x = -1*j;}
			
		else if(p.x < 0){
			moveTri(t,mkvec(p.x*-100,0));		
				
			Vec2 po = {1,0};      
                	j = ((-1-e)*(dot(v,po)))/(1/mass+(cross(r,po)*cross(r,po))/I);
                        J.x = 1*j;}
		else{continue;}
		

		
		
		*vT = addVec(*vT,mkvec(J.x/mass,J.y/mass));
		*w = *w + cross(r,J)/I;

	
		}

}

double dist(Vec2 p1 , Vec2 p2){
	return magp(minusVec(p1,p2));}
double dens(Tri t,double mass){
	double a = magp(minusVec(t.p1,t.p2)),b = magp(minusVec(t.p2,t.p3)),c = magp(minusVec(t.p1,t.p3));
	double s = (a+b+c)/2;
	double area = fabs(sqrt(s*(s-a)*(s-b)*(s-c)));
	return mass/area;}
Vec2 mulv(Vec2 p,double u){Vec2 pr = {p.x*u,p.y*u};return pr;}

double calcMomentOfInertia(Tri t,double density){
    Vec2 p1 = t.p1, p2 = t.p2, p3 = t.p3;
    double moi = 0;
    double w = dist(p1, p2);
    double w1 = fabs(dot(minusVec(p1, p2), minusVec(p3, p2)) / w);
    double w2 = fabs(w - w1);
    
    double signedTriArea = cross(minusVec(p3, p1), minusVec(p2, p1)) / 2;
    double h = 2 * fabs(signedTriArea) / w;
    
    Vec2 p4 = addVec(p2, mulv(minusVec(p1, p2), w1 / w));
    
    Vec2 cm1 = {(p2.x + p3.x + p4.x) / 3,(p2.y + p3.y + p4.y) / 3};
    Vec2 cm2 = {(p1.x + p3.x + p4.x) / 3,(p1.y + p3.y + p4.y) / 3};
    
    double I1 = density * w1 * h * ((h * h / 4) + (w1 * w1 / 12));
    double I2 = density * w2 * h * ((h * h / 4) + (w2 * w2 / 12));
    double m1 = 0.5 * w1 * h * density;
    double m2 = 0.5 * w2 * h * density;

    double I1cm = I1 - (m1 * pow(dist(cm1, p3), 2));
    double I2cm = I2 - (m2 * pow(dist(cm2, p3), 2));
    
    double momentOfInertiaPart1 = I1cm + (m1 * pow(magp(cm1), 2));
    double momentOfInertiaPart2 = I2cm + (m2 * pow(magp(cm2), 2));
    if(cross(minusVec(p1, p3), minusVec(p4, p3)) > 0){
      moi += momentOfInertiaPart1;
    } else {
      moi -= momentOfInertiaPart1;
    }
    if(cross(minusVec(p4, p3), minusVec(p2, p3)) > 0) {
      moi += momentOfInertiaPart2;
    } else {
      moi -= momentOfInertiaPart2;
    }
  
  return fabs(moi);
}
void moveTri2(Tri *t,Vec2 v,double w){
		Vec2 cen = cenTri(*t);

		Vec2 r1 = {t->p1.x-cen.x,t->p1.y-cen.y};
		moveVec(&t->p1,addVec(v,crossd(r1,w)));

		Vec2 r2 = {t->p2.x-cen.x,t->p2.y-cen.y};
		moveVec(&t->p2,addVec(v,crossd(r2,w)));

		Vec2 r3 = {t->p3.x-cen.x,t->p3.y-cen.y};
		moveVec(&t->p3,addVec(v,crossd(r3,w)));}

int main(){
	int ms = 10;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
	time_t start, now;struct timespec delay;delay.tv_sec = 0;delay.tv_nsec = ms * 999999L;time(&start);
	
	initscr();
	clear();
	Vec2 p[3] = {{95,80},{40,90},{100,100}};Tri T_test = {p[0],p[1],p[2]};


	double mass = 100;
	double density = dens(T_test,mass);
	double I = calcMomentOfInertia(T_test,density);

	Vec2 v = {50,100};
	double w = 0;


	while(1){
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
                Vec2 screen = {win.ws_col,win.ws_row};

		borderCheck(screen,I,mass,&w,&T_test,&v);

		//v.y += 0.5;
		moveTri2(&T_test,v,w);
	
		nanosleep(&delay,NULL);
                clear();
                scanln(T_test);
                refresh();}
	getch();
	return 0;}

