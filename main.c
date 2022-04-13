#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include<sys/ioctl.h>
#include<sys/time.h>
#include<unistd.h>




#include <SDL.h>

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
	
	Vec2 cen;	
	
	double m,I;
	
	Vec2 v;
	double w;
}Obj;

// Define MAX and MIN macros
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

// Define screen dimensions
#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600


void swapp(Vec2 *p1,Vec2 *p2){
	Vec2 tmp = *p1;*p1 = *p2;*p2 = tmp;
	return;}

Vec2 mkvec(double x,double y){
	Vec2 v = {x,y};
	return v;}

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

double magp(Vec2 p){
	double mg = sqrt((p.x*p.x) + (p.y*p.y));
	return mg;} 

double dot(Vec2 p1 , Vec2 p2){
	double prod = p1.x * p2.x + p1.y * p2.y;
	return prod;}

Vec2 addVec(Vec2 p,Vec2 offs){ 
	Vec2 v;
	v.x =p.x+ offs.x;
	v.y =p.y+ offs.y;
	return v;}

Vec2 minusVec(Vec2 p,Vec2 offs){ 
	Vec2 v;
	v.x =p.x- offs.x;
	v.y =p.y- offs.y;
	return v;}

Vec2 crossd(Vec2 p ,double d){
	double s = sin(d); double c = cos(d); 
	Vec2 r = { c*p.x - s*p.y, s*p.x + c*p.y }; 
	return r;}

double cross(Vec2 p1,Vec2 p2){
	return ((p1.x*p2.y)-(p1.y*p2.x));}

double dist(Vec2 p1 , Vec2 p2){
	return magp(minusVec(p1,p2));}

Vec2 mulv(Vec2 p,double u){
	Vec2 pr = {p.x*u,p.y*u};
	return pr;}

double inpolx(Vec2 p1,Vec2 p2,double y){
	double x = p1.x + (p2.x - p1.x)*(y - p1.y)/(p2.y - p1.y);
	return x;}

double areaObj(Obj o){
	double area = 0;	
	for(int i = 1;i < o.vArr_len-1;i++){
		Vec2 v1 = minusVec(o.vArr[i+1],o.vArr[0]);	
		Vec2 v2 = minusVec(o.vArr[i],o.vArr[0]);
		area += cross(v1,v2)/2;}
	return fabs(area);}

Vec2 cenObj(Obj o){
	double Aarea = 0;
	double cenx = 0,ceny = 0;
	for(int i = 0,j=o.vArr_len-1;i < o.vArr_len;j=i++){
		double temp = o.vArr[i].x*o.vArr[j].y - o.vArr[j].x*o.vArr[i].y;
		Aarea += temp;
		cenx += (o.vArr[i].x+o.vArr[j].x)*temp;
		ceny += (o.vArr[i].y+o.vArr[j].y)*temp;	}
	Aarea *= 3;
	Vec2 retCen = {fabs(cenx/Aarea),fabs(ceny/Aarea)};
	return retCen;}

void moveVec(Vec2 *v,Vec2 offs){
	v->x = offs.x;
	v->y = offs.y;
	return;}

void moveObj(Obj *o,double dT,Vec2 cen){
	for(int i = 0;i<o->vArr_len;i++){
		Vec2 p = o->vArr[i];
		Vec2 v = addVec(o->v,cen);
		Vec2 r = {o->vArr[i].x-cen.x,o->vArr[i].y-cen.y};
		o->vArr[i] = addVec(v,crossd(r,o->w));}
	return;}
Vec2 crossd2(Vec2 p ,double d){ 
	Vec2 r = {-1*d*p.y,d*p.x};
	return r;}

void moveObjV(Obj *o,Vec2 offs){
	for(int i = 0;i<o->vArr_len;i++){
		o->vArr[i] = addVec(o->vArr[i],offs);}
	return;}

void borderCheck2(Vec2 screen ,Obj* o,Vec2 *cen){
	for(int i = 0;i<o->vArr_len;i++){
		Vec2 p = o->vArr[i];	
				double j;
		double e = 0.2; //elasticity 1 = boing boing , 0 = BAM!! 

		Vec2 J;
                if(p.y > screen.y){
			moveObjV(o,mkvec(0,screen.y-p.y));
*cen = addVec(*cen,mkvec(0,screen.y-p.y));
Vec2 r = {p.x-cen->x,p.y-cen->y};

			Vec2 v = addVec(o->v,crossd2(r,o->w));
			Vec2 po = {0,-1}; //wall normal
			j = ((-1-e)*(dot(v,po)))/(1/o->m+(cross(r,po)*cross(r,po))/o->I); // <- impulse on wall calculation
                        J.y = -1*j;} //mult by surface normal
		else if(p.y < 0){ 
			moveObjV(o,mkvec(0,p.y*-1));
*cen = addVec(*cen,mkvec(0,p.y*-1));
Vec2 r = {p.x-cen->x,p.y-cen->y};

			Vec2 v = addVec(o->v,crossd2(r,o->w));
			Vec2 po = {0,1};     
			j = ((-1-e)*(dot(v,po)))/(1/o->m+(cross(r,po)*cross(r,po))/o->I);
                        J.y = 1*j;}
		else if(p.x >  screen.x){
			moveObjV(o,mkvec(screen.x-p.x,0));
			*cen = addVec(*cen,mkvec(screen.x-p.x,0));
			Vec2 r = {p.x-cen->x,p.y-cen->y};

			Vec2 v = addVec(o->v,crossd2(r,o->w));
			Vec2 po = {-1,0};
			j = ((-1-e)*(dot(v,po)))/(1/o->m+(cross(r,po)*cross(r,po))/o->I);
                        J.x = -1*j;}	
		else if(p.x < 0){
			moveObjV(o,mkvec(p.x*-1,0));
			*cen = addVec(*cen,mkvec(p.x*-1,0));
Vec2 r = {p.x-cen->x,p.y-cen->y};

			Vec2 v = addVec(o->v,crossd2(r,o->w));
			Vec2 po = {1,0};      
                	j = ((-1-e)*(dot(v,po)))/(1/o->m+(cross(r,po)*cross(r,po))/o->I);
                        J.x = 1*j;}
		else{continue;} //does it skip?
Vec2 r = {p.x-cen->x,p.y-cen->y};

		o->v = addVec(o->v,mkvec(J.x/o->m,J.y/o->m));
		o->w = o->w + cross(r,J)/o->I;}}

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
			moi += momentOfInertiaPart1;}
		else{moi -= momentOfInertiaPart1;}
		if(cross(minusVec(p4, p3), minusVec(p2, p3)) > 0){
			moi += momentOfInertiaPart2;} 
		else{moi -= momentOfInertiaPart2;}}
	return fabs(moi);}





void drawTri(SDL_Renderer* Renderer,Tri t){
	Vec2 p1 = *t.p1,p2=*t.p2,p3=*t.p3;
	SDL_RenderDrawLine(Renderer, p1.x, p1.y, p2.x,p2.y);
	SDL_RenderDrawLine(Renderer, p1.x, p1.y, p3.x,p3.y);
	SDL_RenderDrawLine(Renderer, p2.x, p2.y, p3.x,p3.y);}

void draw(SDL_Renderer* Renderer,Obj o){
	for(int i = 0;i<o.tArr_len;i++){
		drawTri(Renderer,o.tArr[i]);}}


int main(){
	if(SDL_Init(SDL_INIT_VIDEO) < 0){printf("SDL could not be initialized!\n""SDL_Error: %s\n", SDL_GetError());return 0;}
	#if defined linux && SDL_VERSION_ATLEAST(2, 0, 8)
		if(!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0")){printf("SDL can not disable compositor bypass!\n");return 0;}
	#endif

	SDL_Window *window = SDL_CreateWindow("Basic C SDL project",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,SCREEN_WIDTH, SCREEN_HEIGHT,SDL_WINDOW_SHOWN);
	if(!window){printf("Window could not be created!\n""SDL_Error: %s\n", SDL_GetError());}
	else{SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        	if(!renderer){printf("Renderer could not be created!\n""SDL_Error: %s\n", SDL_GetError());}
        	else{bool quit = false;// Event loop exit flag

			int ms = 10;
			time_t start, now, s_T, e_T;struct timespec delay;delay.tv_sec = 0;delay.tv_nsec = ms * 999999L;time(&start);

			Vec2 p[4] = {{20,21},{22,45},{36,47},{70,33}};Tri t[2] = {{&p[0],&p[1],&p[2]},{&p[0],&p[2],&p[3]}};
			double mass = 100;	
			Vec2 v = {1,2};
			double w = 0;

			Obj oT = mkObj(&p[0],4,&t[0],2,mass,v,w);Vec2 cen = cenObj(oT);

			oT.I = objInertia(oT);
			struct timeval t1,t2;
			double elapsedTime;

			Vec2 screen = {800,600};
			while(!quit){
				gettimeofday(&t1, NULL);nanosleep(&delay,NULL);

				SDL_Event e;

				while(SDL_PollEvent(&e)) {
					if(e.type == SDL_QUIT || (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE)){
            					quit = true;}}

				SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
				SDL_RenderClear(renderer);
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

				borderCheck2(screen ,&oT,&cen);
				gettimeofday(&t2, NULL);double seconds =(t2.tv_sec - t1.tv_sec);double dT =((seconds * 1000000) + t2.tv_usec) - (t1.tv_usec);
				moveObj(&oT,dT,cen);
				cen.x +=oT.v.x;cen.y+=oT.v.y;

				oT.v.y+=0.01;
				draw(renderer,oT);SDL_RenderPresent(renderer);}

			SDL_DestroyRenderer(renderer);}
		SDL_DestroyWindow(window);}        
	SDL_Quit();
	return 0;}
