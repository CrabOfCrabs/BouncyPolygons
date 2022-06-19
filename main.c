#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct vector2d{
	double x;
	double y;
}Vec2;

typedef struct rigid_object{
	Vec2 *vArr;
	unsigned int vArr_len;

	Vec2 maxV,minV;

	Vec2 cen;
	double m,I;

	Vec2 v;
	double w;
}Obj;


//
//Macros
//

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))


//
//Global Constants
//

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600

#define VERTEX_LIMIT	1000
#define OBJECT_LIMIT    100

int ms = 10;		//Step delay in ms
	//Obj defaults//
double mass = 100;
Vec2 v = {100,200};
double w = 0;
	//Colors//
SDL_Color White = {255, 255, 255};


//
//Global Variables
//

SDL_Renderer *renderer;SDL_Window *window;TTF_Font *font;SDL_Event e;

bool quit = false;// Event loop exit flag
SDL_Surface* surfaceMessage;
SDL_Texture* Message;
double tq1p = 0;

int mousex=0,mousey=0;
bool createMode = false;
int screenW= SCREEN_WIDTH,screenH = SCREEN_HEIGHT;
int objFocus_Index=0;//selected obj

Obj oT[OBJECT_LIMIT];//Obj array
int oT_S =0;
Vec2 pA[VERTEX_LIMIT];//Vertex Array for creating obj
int pAS = 0;

int dT = 0;//Delta time of current step


//
//Functions Declaration
//

void swapp(Vec2 *p1,Vec2 *p2);// swap addreses of vectors
	//making stuff//
Vec2 mkVec(double x,double y);
Obj mkObj(Vec2 *vArr,int vArrS,double mass,Vec2 v,double w);
	//Simple math//
Vec2 addVec(Vec2 p,Vec2 offs);Vec2 minusVec(Vec2 p,Vec2 offs);Vec2 mulv(Vec2 p,double u);// basic math
double dot(Vec2 p1 , Vec2 p2);double cross(Vec2 p1,Vec2 p2); //vector multiplication
double magp(Vec2 p);double dist(Vec2 p1 , Vec2 p2); //two distance funcs
Vec2 crossd(Vec2 p ,double d);Vec2 crossd2(Vec2 p ,double d); //angular v to linear v
	//Complex calculations//
double interpolate_X(Vec2 p1,Vec2 p2,double y);//interpolates p1 -> p2 with y variable
double polygon_Area(Obj o);Vec2 polygon_Center(Obj o);//to calculate center object needs proper winding
double polygon_Inertia(Obj o);
double polygon_KineticEnergy(Obj o);
    //Collision Checks//
bool line_Collision(Vec2 p1,Vec2 p2,Vec2 p3,Vec2 p4,Vec2* po);
void border_Collision(Obj* o);
bool DIAGS_Collision(Obj *obj1,Obj *obj2);
	//Movement//
void polygon_Move(Obj *o,double dT);
void polygon_Offset(Obj *o,Vec2 offs);
void vector_Offset(Vec2 *v,Vec2 offs);
	//Rendering//
void draw_Obj(Obj o);
void draw_Int(double intT);
void draw_Blueprint();
	//Main Processes//
void SDL_Setup();//setup for SDL related Stuff
void simulation_Step(double dT);
void draw_Step();
void clear_Renderer();
void event_Loop(); //events like input

//
//MAIN FUNCTIONE
//
//small main functions but noone can understand how it works
int main(){
	SDL_Setup();
	unsigned int lastTime = 0,currentTime;

	//SDL_SetWindowResizable(window,true);
	while(!quit){
		SDL_Delay(ms);
		currentTime = SDL_GetTicks64();
        while(lastTime < currentTime){
		    event_Loop();
	    	SDL_GetWindowSize(window,&screenW,&screenH);
	    	SDL_GetMouseState(&mousex,&mousey);
            dT = currentTime-lastTime;
		    lastTime = currentTime;

		    simulation_Step(dT);
            draw_Step();}}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;}

//
//FUNCTIONS (BORING!!!)
//

void swapp(Vec2 *p1,Vec2 *p2){
	Vec2 tmp = *p1;*p1 = *p2;*p2 = tmp;
	return;}

Vec2 mkVec(double x,double y){
	Vec2 v = {x,y};
	return v;}

Obj mkObj(Vec2 *vArr,int vArrS,double mass,Vec2 v,double w){
	Obj tObj;

	tObj.vArr = (Vec2*) malloc(vArrS * sizeof(Vec2));
	tObj.vArr_len = vArrS;

	for(int i = 0;i<vArrS;i++){
		tObj.vArr[i] = *(vArr+i);}

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

inline Vec2 addVec(Vec2 p,Vec2 offs){
	Vec2 v;
	v.x =p.x+ offs.x;
	v.y =p.y+ offs.y;
	return v;}

inline Vec2 minusVec(Vec2 p,Vec2 offs){
	Vec2 v;
	v.x =p.x- offs.x;
	v.y =p.y- offs.y;
	return v;}

Vec2 divVec(Vec2 p1,Vec2 p2){
	Vec2 v = {p1.x/p2.x,p1.y/p2.y};
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

double interpolate_X(Vec2 p1,Vec2 p2,double y){
	double x = p1.x + (p2.x - p1.x)*(y - p1.y)/(p2.y - p1.y);
	return x;}

double polygon_Area(Obj o){
	double area = 0;
	for(int i = 1;i < o.vArr_len-1;i++){
		Vec2 v1 = minusVec(o.vArr[i+1],o.vArr[0]);
		Vec2 v2 = minusVec(o.vArr[i],o.vArr[0]);
		area += cross(v1,v2)/2;}
	return fabs(area);}

Vec2 polygon_Center(Obj o){
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

void vector_Offset(Vec2 *v,Vec2 offs){
	v->x = offs.x;
	v->y = offs.y;
	return;}

void polygon_Offset(Obj *o,Vec2 offs){
	for(int i = 0;i<o->vArr_len;i++){
		vector_Offset(&o->vArr[i],addVec(o->vArr[i],offs));}
	o->cen = addVec(o->cen,offs);
	return;}


void polygon_Move(Obj *o,double dT){

	for(int i = 0;i<o->vArr_len;i++){
		Vec2 p = o->vArr[i];
		Vec2 v = addVec(mulv(o->v,dT/1000),o->cen);
		Vec2 r = {o->vArr[i].x-o->cen.x,o->vArr[i].y-o->cen.y};
		o->vArr[i] = addVec(v,crossd(r,o->w*(dT/1000)));}
	o->cen.x += o->v.x*(dT/1000);
	o->cen.y += o->v.y*(dT/1000);
	return;}

Vec2 crossd2(Vec2 p ,double d){
	Vec2 r = {-1*d*p.y,d*p.x};
	return r;}

void border_Collision(Obj* o){

	for(int i = 0;i<o->vArr_len;i++){
		Vec2 p = o->vArr[i];
		double j=0;
		double e = 1; //elasticity 1 = boing boing , 0 = BAM!!
		Vec2 J ={0,0};
		Vec2 po = {0,0};
                if(p.y > screenH){
			polygon_Offset(o,mkVec(0,screenH-p.y));
			po.y = -1; //wall normal
		}else if(p.y < 0){
			polygon_Offset(o,mkVec(0,p.y*-1));
			po.y = 1;
		}else if(p.x >  screenW){
			polygon_Offset(o,mkVec(screenW-p.x,0));
			po.x = -1;
		}else if(p.x < 0){
			polygon_Offset(o,mkVec(p.x*-1,0));
			po.x = 1;
		}else{continue;} //does it skip?

		Vec2 r = {p.x-o->cen.x,p.y-o->cen.y};
		Vec2 v = addVec(o->v,crossd2(r,o->w));
		j = ((-1-e)*(dot(v,po)))/(1/o->m+(cross(r,po)*cross(r,po))/o->I);

		o->v = addVec(o->v,mkVec(po.x*j/o->m,po.y*j/o->m));
		o->w = o->w + cross(r,mulv(po,j))/o->I;break;}
	return;}

double polygon_Inertia(Obj o){ //some Inertia bongle dongle doongle moong
	double density = o.m/polygon_Area(o),
	       moi = 0;
	int i;
	for(i = 1;i<o.vArr_len-1;i++){
		Vec2 	p1 = o.vArr[0],
			p2 = o.vArr[i],
		     	p3 = o.vArr[i+1];
		
		double 	w = dist(p1,p2),
		       	w1 = fabs(dot(minusVec(p1,p2),minusVec(p3,p2))/w),
		       	w2 = fabs(w-w1),
		       	signedTriArea = cross(minusVec(p3,p1),minusVec(p2,p1))/2,
		       	h = 2*fabs(signedTriArea)/w;
		
		Vec2 	p4 = addVec(p2,mulv(minusVec(p1,p2),w1/w)),
			cm1 = {(p2.x+p3.x+p4.x)/3,(p2.y+p3.y+p4.y)/3},
		     	cm2 = {(p1.x+p3.x+p4.x)/3,(p1.y+p3.y+p4.y)/3};
		
		double 	I1 = density*w1*h*((h*h/4)+(w1*w1/12)),
			I2 = density*w2*h*((h*h/4)+(w2*w2/12)),
			m1 = 0.5*w1*h*density,
			m2 = 0.5*w2*h*density,
			I1cm = I1-(m1*pow(dist(cm1,p3),2)),
			I2cm = I2-(m2*pow(dist(cm2,p3),2)),
			momentOfInertiaPart1 = I1cm+(m1*pow(magp(cm1),2)),
			momentOfInertiaPart2 = I2cm+(m2*pow(magp(cm2),2));
		
		if(cross(minusVec(p1, p3), minusVec(p4, p3)) > 0){
			moi += momentOfInertiaPart1;}
		else{moi -= momentOfInertiaPart1;}
		if(cross(minusVec(p4, p3), minusVec(p2, p3)) > 0){
			moi += momentOfInertiaPart2;}
		else{moi -= momentOfInertiaPart2;}}
	return fabs(moi);}


void draw_Int(double intT){
	char textString[1000];
	SDL_Rect Message_rect;
	Message_rect.x = 0;
	Message_rect.y = 0;
	Message_rect.w = 100;
	Message_rect.h = 100;
	sprintf(textString,"%lf",intT);
	surfaceMessage =TTF_RenderText_Solid(font, textString, White);
	Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
	SDL_RenderCopy(renderer, Message, NULL, &Message_rect);}

void draw_Obj(Obj o){
	int i;
	for(i = 0;i<o.vArr_len-1;i++){
		SDL_RenderDrawLine(renderer, o.vArr[i].x, o.vArr[i].y,o.vArr[i+1].x,o.vArr[i+1].y);}
	SDL_RenderDrawLine(renderer, o.vArr[0].x, o.vArr[0].y,o.vArr[o.vArr_len-1].x,o.vArr[o.vArr_len-1].y);
	return;}


void draw_Blueprint(){
	SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
	for(int i = 0;i<pAS-1;i++){
		SDL_RenderDrawLine(renderer, pA[i].x, pA[i].y,pA[i+1].x,pA[i+1].y);}
	SDL_RenderDrawLine(renderer, pA[0].x, pA[0].y,pA[pAS-1].x,pA[pAS-1].y);

	SDL_SetRenderDrawColor(renderer, 0, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawLine(renderer, mousex, mousey,pA[0].x,pA[0].y);
	SDL_RenderDrawLine(renderer, mousex, mousey,pA[pAS-1].x,pA[pAS-1].y);

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	return;}
double polygon_KineticEnergy(Obj o){
    double vx =o.v.x,vy =o.v.y,m =o.m,w =o.w,I =o.I;
    return 0.5*m*(vx*vx) + 0.5*m*(vy*vy) + 0.5*I*(w*w);
}
void simulation_Step(double dT){
	for(int i = 0;i<oT_S;i++){
		for(int j = 0;j<oT_S;j++){if(j==i){continue;}else{DIAGS_Collision(&oT[i],&oT[j]);}}
		border_Collision(&oT[i]);	
		polygon_Move(&oT[i],dT);}
        
	return;}

void draw_Step(){
    clear_Renderer();

    double vsum = 0;
    for(int i = 0;i<oT_S;i++){
        draw_Obj(oT[i]);
        vsum += polygon_KineticEnergy(oT[i]);
	    SDL_SetRenderDrawColor(renderer,255,255,255,SDL_ALPHA_OPAQUE);}
    
    draw_Int(vsum);

    if(createMode == true){
        draw_Blueprint();}
    
    SDL_RenderPresent(renderer);
    return;}


void clear_Renderer(){
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_DestroyTexture(Message);
	SDL_FreeSurface(surfaceMessage);
	return;}

void SDL_Setup(){
	if(SDL_Init(SDL_INIT_VIDEO) < 0){printf("SDL could not be initialized!\n""SDL_Error: %s\n", SDL_GetError());exit(EXIT_FAILURE);}
	#if defined linux && SDL_VERSION_ATLEAST(2, 0, 8)
		if(!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0")){printf("SDL can not disable compositor bypass!\n");exit(EXIT_FAILURE);}
	#endif
	window = SDL_CreateWindow("Basic C SDL project",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,SCREEN_WIDTH, SCREEN_HEIGHT,SDL_WINDOW_SHOWN);
	if(!window){printf("Window could not be created!\n""SDL_Error: %s\n", SDL_GetError());}
	else{renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        	if(!renderer){printf("Renderer could not be created!\n""SDL_Error: %s\n", SDL_GetError());}
        	else{
			TTF_Init();
			font = TTF_OpenFont("Font/Hack-Bold.ttf", 20);
			if (font == NULL){
				fprintf(stderr, "error: font not found\n");
				exit(EXIT_FAILURE);}}}
	return;}

void event_Loop(){
	while(SDL_PollEvent(&e)){
		if(e.type == SDL_QUIT || (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE)){
			quit = true;}
		if(e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON(SDL_BUTTON_LEFT)){
			if(createMode == false){
				createMode = true;
				pA[pAS] = mkVec(mousex,mousey);pAS+=1;}
			else if(createMode == true){
				pA[pAS] = mkVec(mousex,mousey);pAS+=1;}}
		if(e.type == SDL_MOUSEBUTTONDOWN && e.button.button ==SDL_BUTTON_RIGHT){
			if(createMode == true){
				createMode = false;
				oT[oT_S] = mkObj(&pA[0],pAS,mass,v,w);
				oT[oT_S].cen = polygon_Center(oT[oT_S]);oT[oT_S].I = polygon_Inertia(oT[oT_S]);
				memset(pA, 0, 1000);pAS=0;oT_S+=1;}}}
	return;}

Vec2 normalize(Vec2 v){
	Vec2 normal = {v.x/magp(v),v.y/magp(v)};
	return normal;}

bool line_Collision(Vec2 p1,Vec2 p2,Vec2 p3,Vec2 p4,Vec2* po){
	Vec2 s1 = {p2.x - p1.x, p2.y - p1.y},
	     s2 = {p4.x - p3.x, p4.y - p3.y};
	double s, t;
	s = (-s1.y * (p1.x - p3.x) + s1.x * (p1.y - p3.y)) / (-s2.x * s1.y + s1.x * s2.y);
	t = ( s2.x * (p1.y - p3.y) - s2.y * (p1.x - p3.x)) / (-s2.x * s1.y + s1.x * s2.y);
	if(s >= 0 && s <= 1 && t >= 0 && t <= 1){
		po->x = p1.x + (t * s1.x);
		po->y = p1.y + (t * s1.y);
		return true;}
	return false;}

bool DIAGS_Collision(Obj *obj1,Obj *obj2){
	Obj *o1 = obj1;
	Obj *o2 = obj2;
	Vec2 col;
	for(int polygon = 0;polygon < 2;polygon++){
		if(polygon == 1){
			o1=obj2;o2=obj1;}
		for(int Index_o1 = 0;Index_o1 < o1->vArr_len;Index_o1++){
			Vec2 ls1 = o1->cen;
			Vec2 le1 = o1->vArr[Index_o1];
			Vec2 offset = {0,0};

			for(int Index_o2 = 0;Index_o2 < o2->vArr_len;Index_o2++){
				Vec2 ls2 = o2->vArr[Index_o2];
				Vec2 le2 = o2->vArr[(Index_o2 + 1) % o2->vArr_len];

				double h = (le2.x - ls2.x)*(ls1.y - le1.y) - (ls1.x - le1.x)*(le2.y - ls2.y);
				double t1 = ((ls2.y - le2.y)*(ls1.x - ls2.x) + (le2.x- ls2.x)*(ls1.y - ls2.y)) / h;
				double t2 = ((ls1.y - le1.y)*(ls1.x - ls2.x) + (le1.x- ls1.x)*(ls1.y - ls2.y)) / h;

				if(t1 >= 0 && t1 <= 1 && t2 >= 0 && t2 <= 1 ){
					offset.x += (1 - t1)*(le1.x - ls1.x);
					offset.y += (1 - t1)*(le1.y - ls1.y);

				if(line_Collision(ls2,le2,ls1,le1,&col)==true){
					double dx = ls2.x - le2.x;
					double dy = ls2.y - le2.y;
					Vec2 po;
					if(dist(o2->cen,addVec(col,mkVec(-dy,dx))) > dist(o2->cen,addVec(col,mkVec(dy,-dx)))){
						po = mkVec(dy,-dx);}
					else{
						po = mkVec(-dy,dx);}

					po = normalize(po);
					Vec2 r1 = {col.x-o1->cen.x,col.y-o1->cen.y};
					Vec2 r2 = {col.x-o2->cen.x,col.y-o2->cen.y};
					Vec2 v = minusVec(addVec(o1->v,crossd2(r1,o1->w)),addVec(o2->v,crossd2(r2,o2->w)));
					double j = ((-2)*(dot(v,po)))/(1/o1->m+1/o2->m+(cross(r1,po)*cross(r1,po))/o1->I+(cross(r2,po)*cross(r2,po))/o2->I);
					o1->v = addVec(o1->v,mulv(mulv(po,j),1/o1->m));
					o1->w = o1->w + cross(r1,mulv(po,j))/o1->I;
					o2->v = minusVec(o2->v,mulv(mulv(po,j),1/o2->m));
					o2->w = o2->w - cross(r2,mulv(po,j))/o2->I;}}}
            polygon_Offset(o1,mulv(offset,(polygon == 0 ? -1 : +1)));
            }
		return false;
	}return false;}
//concave shapes still a "BIG NO GO" and i don't know why :(



