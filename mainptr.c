#include<stdlib.h>
#include<stdio.h>
#include<curses.h>
#include<math.h>
#include<time.h>
#include<sys/ioctl.h>
#include<sys/time.h>
#include<unistd.h>

struct winsize win;

static double G_gravity = 9.81;

typedef struct vector2d{
	double x;
	double y;}Vec2;

typedef struct triangle{
	Vec2 *p1;
	Vec2 *p2;
	Vec2 *p3;
}Tri;

typedef struct rigid_object{
	Tri *tArr;
	unsigned int tArr_len;
	Vec2 *vArr;
	unsigned int vArr_len;
	
	double m,I;
	
	
	Vec2 v;
	double w;
	

}Obj;


void swapp(Vec2 *p1,Vec2 *p2){Vec2 tmp = *p1;*p1 = *p2;*p2 = tmp;}


Vec2 mkvec(double x,double y){Vec2 v = {x,y};return v;}
double magp(Vec2 p){double mg = sqrt((p.x*p.x) + (p.y*p.y));return mg;} //magnitude of a vec from origin
double dot(Vec2 p1 , Vec2 p2){ //2d vec dot
	double prod = p1.x * p2.x + p1.y * p2.y;
	return prod;}
Vec2 addVec(Vec2 p,Vec2 offs){ //vec add
	Vec2 v;
	v.x =p.x+ offs.x;
	v.y =p.y+ offs.y;
	return v;}
Vec2 minusVec(Vec2 p,Vec2 offs){ //vec minus
	Vec2 v;
	v.x =p.x- offs.x;
	v.y =p.y- offs.y;
	return v;}
Vec2 crossd(Vec2 p ,double d){ //copied from the INTERNET
	Vec2 r = {-1*d*p.y,d*p.x};
	return r;}
double cross(Vec2 p1,Vec2 p2){ //cross product in 2d
	return ((p1.x*p2.y)-(p1.y*p2.x));}
double dist(Vec2 p1 , Vec2 p2){
	return magp(minusVec(p1,p2));}
double dens(Tri t,double mass){ //triangle math
	double a = magp(minusVec(*t.p1,*t.p2)),b = magp(minusVec(*t.p2,*t.p3)),c = magp(minusVec(*t.p1,*t.p3));
	double s = (a+b+c)/2;
	double area = fabs(sqrt(s*(s-a)*(s-b)*(s-c)));
	return mass/area;}
Vec2 mulv(Vec2 p,double u){Vec2 pr = {p.x*u,p.y*u};return pr;}

double areaObj(Obj o){
	double area = 0;	
	for(int i = 1;i < o.vArr_len-1;i++){
		Vec2 v1 = minusVec(o.vArr[i+1],o.vArr[0]);	Vec2 v2 = minusVec(o.vArr[i],o.vArr[0]);
		area += cross(v1,v2)/2;
	}
	return fabs(area);}

double inpolx(Vec2 p1,Vec2 p2,double y){ //interpolate x from p1 to p2 with y variable
	double x = p1.x + (p2.x - p1.x)*(y - p1.y)/(p2.y - p1.y);
	return x;}


Vec2 cenTri(Tri t){
	Vec2 v ={(t.p1->x+t.p2->x+t.p3->x)/3,(t.p1->y+t.p2->y+t.p3->y)/3};
	return v;}

Vec2 cenObj(Obj o){
	double Aarea = 0;
	double cenx = 0,ceny = 0;
	for(int i = 0,j=o.vArr_len-1;i < o.vArr_len;j=i++){
		double temp = o.vArr[i].x*o.vArr[j].y - o.vArr[j].x*o.vArr[i].y;
		Aarea += temp;
		cenx += (o.vArr[i].x+o.vArr[j].x)*temp;
		ceny += (o.vArr[i].y+o.vArr[j].y)*temp;


	
	}
	Aarea *= 3;
	Vec2 retCen = {fabs(cenx/Aarea),fabs(ceny/Aarea)};
	return retCen;
}
Vec2 cenObj2(Obj o){
	Vec2 centroid ={0,0};
	double area=0,x0=0,y0=0,x1=0,y1=0,a=0;

	int i = 0;
	for(i=0;i<o.vArr_len-1;++i){
		x0=o.vArr[i].x;
		y0=o.vArr[i].y;
		x1=o.vArr[i+1].x;
		y1=o.vArr[i+1].y;
		a = x0*y1 - x1*y0;
		area +=a;
		centroid.x += (x0+x1)*a;
		centroid.y += (y0+y1)*a;
	}

	x0=o.vArr[i].x;
	y0=o.vArr[i].y;
	x1=o.vArr[0].x;
	y1=o.vArr[0].y;
	a = x0*y1 - x1*y0;
	area +=a;
	centroid.x += (x0+x1)*a;
	centroid.y += (y0+y1)*a;

	area *= 0.5;
	centroid.x /= (6*area);
	centroid.y /= (6*area);
	return centroid;


}


void moveVec(Vec2 *v,Vec2 offs){ //brooom
	v->x += offs.x;
	v->y += offs.y;}
void moveTri(Tri *t,Vec2 offs){
	moveVec(t->p1,offs);
	moveVec(t->p2,offs);
	moveVec(t->p3,offs);}
void moveObj(Obj o,double dT,Vec2 cen){
		//Vec2 cen = {50,50};
	for(int i = 0;i<o.vArr_len;i++){
		Vec2 p = o.vArr[i];
		Vec2 r = {p.x-cen.x,p.y-cen.y};
		moveVec(&o.vArr[i],mulv(addVec(o.v,crossd(r,o.w)),dT/1000000));}}










void DSline(double x1,double x2,double y ){ //Draws Simple line
	
	double x,x2l;
	if(x1<x2){x=x1;x2l = x2;}else{x=x2;x2l = x1;}
	do{mvprintw(y/2,x,"#");}while(x++<x2l);}
	

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

void scanln(Tri t){
	//tri tr = projt(rent(t,xr),screen);
	Tri tr = t;
	if(tr.p2->y < tr.p1->y){ swapp(tr.p2, tr.p1); }if(tr.p3->y <tr.p1->y){ swapp(tr.p3, tr.p1); }if(tr.p3->y < tr.p2->y){ swapp(tr.p3, tr.p2); }

	if(tr.p3->y == tr.p2->y){drawtup(tr);}
	else if(tr.p1->y == tr.p2->y){drawtdown(tr);}
	else{Vec2 p4 = {inpolx(*tr.p1,*tr.p3,tr.p2->y),tr.p2->y};
	Tri tu = {tr.p1,tr.p2,&p4};Tri td = {tr.p2,&p4,tr.p3};
	drawtup(tu);drawtdown(td);}}


void drawObj(Obj o){
	for(int i = 0;i<o.tArr_len;i++){
		scanln(o.tArr[i]);}}



void borderCheck(Vec2 screen ,double I,double mass,double *w,Tri *t,Vec2 *vT){
	Vec2 *triT[3] = {t->p1,t->p2,t->p3}; 
		for(int i = 2;i>=0;i--){
		Vec2 p = *triT[i];
		
		double j;
		double e = 0.5; //elasticity 1 = boing boing , 0 = BAM!! 
		Vec2 J;
                if(p.y > screen.y*2){
			moveTri(t,mkvec(0,(screen.y*2-p.y)*1)); //shitty offset makes triangle grow
			Vec2 cen = cenTri(*t);
			Vec2 r = {p.x-cen.x,p.y-cen.y};Vec2 v = addVec(*vT,crossd(r,*w));

			Vec2 po = {0,-1}; //wall normal
			j = ((-1-e)*(dot(v,po)))/(1/mass+(cross(r,po)*cross(r,po))/I); // <- impulse on wall calculation
                        J.y = -1*j;} //mult by surface normal
		else if(p.y < 0){
			moveTri(t,mkvec(0,p.y*-1));	
			Vec2 cen = cenTri(*t);
			Vec2 r = {p.x-cen.x,p.y-cen.y};Vec2 v = addVec(*vT,crossd(r,*w));

			Vec2 po = {0,1};     
			j = ((-1-e)*(dot(v,po)))/(1/mass+(cross(r,po)*cross(r,po))/I);
                        J.y = 1*j;}
		else if(p.x >  screen.x){
			moveTri(t,mkvec((screen.x-p.x)*1,0));	
			Vec2 cen = cenTri(*t);
			Vec2 r = {p.x-cen.x,p.y-cen.y};Vec2 v = addVec(*vT,crossd(r,*w));

			Vec2 po = {-1,0};
			j = ((-1-e)*(dot(v,po)))/(1/mass+(cross(r,po)*cross(r,po))/I);
                        J.x = -1*j;}
			
		else if(p.x < 0){
			moveTri(t,mkvec(p.x*-1,0));		
			Vec2 cen = cenTri(*t);
			Vec2 r = {p.x-cen.x,p.y-cen.y};Vec2 v = addVec(*vT,crossd(r,*w));

			Vec2 po = {1,0};      
                	j = ((-1-e)*(dot(v,po)))/(1/mass+(cross(r,po)*cross(r,po))/I);
                        J.x = 1*j;}
		else{continue;} //does it skip?
		
		Vec2 cen = cenTri(*t);
		Vec2 r = {p.x-cen.x,p.y-cen.y};
		
		
		*vT = addVec(*vT,mkvec(J.x/mass,J.y/mass));
		*w = *w + cross(r,J)/I;}}


void borderCol_Obj(Obj o,Vec2 screen){
//for(int i = 0;o.tArr_len > i;i++){
//borderCheck(screen ,o.I,o.m,&o.w,&o.tArr[i],&o.v);}
}



double calcMomentOfInertia(Tri t,double density){ //some Inertia bongle dongle doongle moongle
    Vec2 p1 = *t.p1, p2 = *t.p2, p3 = *t.p3;
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
double objInertia(Obj o){ //some Inertia bongle dongle doongle moong
	double density = o.m/areaObj(o);
	double moi = 0;int i;
	for(i = 1;i< o.vArr_len-1;i++){
	Vec2 p1 = o.vArr[0], p2 = o.vArr[i], p3 = o.vArr[i+1];

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
	}
  return fabs(moi);
}
void updateTri(Tri *t,Vec2 v,double w,double dT){
		Vec2 cen = cenTri(*t);
		Vec2 r1 = {t->p1->x-cen.x,t->p1->y-cen.y};
		moveVec(t->p1,mulv(addVec(v,crossd(r1,w)),dT/1000000));

		Vec2 r2 = {t->p2->x-cen.x,t->p2->y-cen.y};
		moveVec(t->p2,mulv(addVec(v,crossd(r2,w)),dT/1000000));

		Vec2 r3 = {t->p3->x-cen.x,t->p3->y-cen.y};
		moveVec(t->p3,mulv(addVec(v,crossd(r3,w)),dT/1000000));}
Obj mkObj(Vec2 *vArr,int vArrS,Tri *tArr,int tArrS,double mass,Vec2 v,double w){
	Obj tObj;
	tObj.tArr = tArr;
	tObj.tArr_len = tArrS;
	
	tObj.vArr = vArr;
	tObj.vArr_len = vArrS;

	tObj.m = mass;
	tObj.I = 0;

	tObj.v = v;
	tObj.w = w;

	return tObj;}


int main(){
	int ms = 10;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
	time_t start, now, s_T, e_T;struct timespec delay;delay.tv_sec = 0;delay.tv_nsec = ms * 999999L;time(&start);
	
	initscr();
	clear();
	Vec2 p[4] = {{20,26},{22,45},{46,25},{45,43}};Tri t[2] = {{&p[0],&p[1],&p[2]},{&p[1],&p[2],&p[3]}};


	double mass = 100;
	//double density = dens(T_test,mass);
	//double I = calcMomentOfInertia(T_test,density);

	Vec2 v = {0,0};
	double w = 2;

	Obj oT = mkObj(&p[0],4,&t[0],2,mass,v,w);
	//oT.I = objInertia(oT);
	//oT.I = 2;
	struct timeval t1,t2;
	double elapsedTime;
	while(1){gettimeofday(&t1, NULL);nanosleep(&delay,NULL);
Vec2 cen = cenObj2(oT);

		ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
                Vec2 screen = {win.ws_col,win.ws_row};

		//borderCheck(screen,I,mass,&w,&T_test,&v); //check for screen boundry collsion
		//borderCol_Obj(oT,screen);
	
		
		gettimeofday(&t2, NULL);double seconds =(t2.tv_sec - t1.tv_sec); 	double dT =((seconds * 1000000) + t2.tv_usec) - (t1.tv_usec);
		moveObj(oT,dT,cen);
		//updateTri(&T_test,v,w,dT); //updates tri rotation and position with velocities in delta time
		clear();
		//printf(" %lf\n %lf\n\n",cen.x,cen.y);
		drawObj(oT);


                refresh();
		}
	getch();
	return 0;}

