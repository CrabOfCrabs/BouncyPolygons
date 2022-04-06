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

Vec2 border_Check(Vec2 screen ,Vec2 p,Vec2 v,Vec2 cen,double I,double mass,Tri *t){
		Vec2 r = {p.x-cen.x,p.y-cen.y};
		double j;
		Vec2 J={0,0};
                if(p.y > screen.y){
			moveTri(t,mkvec(0,screen.y-p.y));
			Vec2 po = {0,-1};      //workspace roof
                	j = (-2*(dot(v,po)))/(1/mass+(cross(r,po)*cross(r,po))/I);
                        Vec2 J1 = {0,-1*j};return J1;}
		else if(p.y < 0){  //workspace bottomi
			moveTri(t,mkvec(0,p.y*-1));
			Vec2 po = {0,1};      //workspace roof
                	j = (-2*(dot(v,po)))/(1/mass+(cross(r,po)*cross(r,po))/I);
                        Vec2 J1 = {0,1*j};return J1;}
		else if(p.x >  screen.x){
			moveTri(t,mkvec(screen.x-p.x,0));
			Vec2 po = {-1,0};      //workspace roof
                	j = (-2*(dot(v,po)))/(1/mass+(cross(r,po)*cross(r,po))/I);
                        Vec2 J1 = {-1*j,0};return J1;}
			
		else if(p.x < 0){
			moveTri(t,mkvec(p.x*-1,0));			
			Vec2 po = {1,0};      //workspace roof
                	j = (-2*(dot(v,po)))/(1/mass+(cross(r,po)*cross(r,po))/I);
                        Vec2 J1 = {1*j,0};return J1;}
		return J;



}

double dist(Vec2 p1 , Vec2 p2){
	return magp(minusVec(p1,p2));}
double dens(Tri t,double mass){
	double a = magp(minusVec(t.p1,t.p2)),b = magp(minusVec(t.p2,t.p3)),c = magp(minusVec(t.p1,t.p3));
	double s = (a+b+c)/2;
	double area = sqrt(s*(s-a)*(s-b)*(s-c));
	return mass/area;}
Vec2 mulv(Vec2 p,double u){Vec2 pr = {p.x*u,p.y*u};return pr;}
double calcMomentOfInertia(Tri t,double density){
    Vec2 p1 = t.p1, p2 = t.p2, p3 = t.p3;
    double moi = 0;
    double w = dist(p1, p2);
    double w1 = abs(dot(minusVec(p1, p2), minusVec(p3, p2)) / w);
    double w2 = abs(w - w1);
    
    double signedTriArea = cross(minusVec(p3, p1), minusVec(p2, p1)) / 2;
    double h = 2 * abs(signedTriArea) / w;
    
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
  
  return moi;
}



int main(){
	int ms = 10;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	time_t start, now;struct timespec delay;delay.tv_sec = 0;delay.tv_nsec = ms * 999999L;time(&start);
	
	initscr();
	clear();
	Vec2 p[3] = {{12,33},{6,39},{22,42}};Tri T_test = {p[0],p[1],p[2]};


	double mass = 100;
	double density = dens(T_test,mass);
	double MMOI = calcMomentOfInertia(T_test,density);

	Vec2 v = {30,20};
	double av = 0;



	while(1){
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
                Vec2 screen = {w.ws_col,w.ws_row};
	
	Vec2 cen = cenTri(T_test);

		Vec2 j3 = border_Check(screen ,T_test.p1,v,cen,MMOI,mass,&T_test);
		Vec2 aj1 ={j3.x/mass,j3.y/mass};
		v = addVec(v,aj1);
		Vec2 r1 = {T_test.p1.x-cen.x,T_test.p1.y-cen.y};
		av = av + cross(r1,j3)/MMOI;

		Vec2 j2 =border_Check(screen ,T_test.p2,v,cen,MMOI,mass,&T_test);
		Vec2 aj2 ={j2.x/mass,j2.y/mass};
		v = addVec(v,aj2);
		Vec2 r2 = {T_test.p2.x-cen.x,T_test.p2.y-cen.y};
		av = av + cross(r2,j2)/MMOI;

		Vec2 j1 =border_Check(screen ,T_test.p3,v,cen,MMOI,mass,&T_test);
		Vec2 aj3 ={j1.x/mass,j1.y/mass};
		v = addVec(v,aj3);
		Vec2 r3 = {T_test.p3.x-cen.x,T_test.p3.y-cen.y};
		av = av + cross(r3,j1)/MMOI;
rotateTri(&T_test,cen,av/10);
moveTri(&T_test,v);
		nanosleep(&delay,NULL);
                clear();
                scanln(T_test);
                refresh();}
	getch();
	return 0;}

