#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct vector2d{
	double x;
	double y;}Vec2;

typedef struct triangle{
	Vec2 *p1;
	Vec2 *p2;
	Vec2 *p3;
}Tri;

typedef struct rigid_object{
	Vec2 *vArr;
	unsigned int vArr_len;

	Vec2 maxV,minV;

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

#define VERTEX_LIMIT	1000
#define OBJECT_LIMIT    100


SDL_Renderer *renderer;SDL_Window *window;TTF_Font *font;SDL_Event e;
bool quit = false;// Event loop exit flag


Obj oT[OBJECT_LIMIT];int oT_S =0;
Vec2 pA[VERTEX_LIMIT];int pAS = 0;

int mousex=0,mousey=0;
bool createMode = false;
int screenW= SCREEN_WIDTH,screenH = SCREEN_HEIGHT;
int polyFocus_Index=0;

double mass = 100;
Vec2 v = {1,2};
double w = 0;

SDL_Color White = {255, 255, 255};

// swap addreses of vectors
void swapp(Vec2 *p1,Vec2 *p2);

Vec2 mkvec(double x,double y);
Obj mkObj(Vec2 *vArr,int vArrS,double mass,Vec2 v,double w);

Vec2 addVec(Vec2 p,Vec2 offs);Vec2 minusVec(Vec2 p,Vec2 offs);Vec2 mulv(Vec2 p,double u);// basic math
double dot(Vec2 p1 , Vec2 p2);double cross(Vec2 p1,Vec2 p2); //vector multiplication
double magp(Vec2 p);double dist(Vec2 p1 , Vec2 p2); //two distance funcs
Vec2 crossd(Vec2 p ,double d);Vec2 crossd2(Vec2 p ,double d); //angular v to linear v
double inpolx(Vec2 p1,Vec2 p2,double y);//interpolates p1 -> p2 with y variable
double areaObj(Obj o);Vec2 cenObj(Obj o);//to calculate center object needs proper winding


void moveVec(Vec2 *v,Vec2 offs);
void moveObj(Obj *o,double dT);
void moveObjV(Obj *o,Vec2 offs);

void borderCheck2(Obj* o);
double objInertia(Obj o);

void draw2(Obj o);


void get_text_and_rect(int x, int y, char *text,TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect);

void drawMode_Render();
void simulation_Step(double dT);
void clear_Renderer();
void eventLoop(); //events like input

void SDL_setup();

//small main functions but noone can understand how it works
int main(){
	SDL_setup();


	int ms = 10;
	double dT =0;
	//SDL_SetWindowResizable(window,true);
	while(!quit){char ymouse[1000];
					char xmouse[1000];
					char velocy[1000];SDL_Rect Message_rect; //create a rect
					Message_rect.x = 0;  //controls the rect's x coordinate
					Message_rect.y = 0; // controls the rect's y coordinte
					Message_rect.w = 100; // controls the width of the rect
					Message_rect.h = 100; // controls the height of the rect
		SDL_Delay(ms);
		SDL_GetWindowSize(window,&screenW,&screenH);
		SDL_GetMouseState(&mousex,&mousey);
		sprintf(ymouse,"%d",mousey);
					sprintf(xmouse,"%d",mousex);
					sprintf(velocy,"%lf",oT[oT_S-1].v.x);
		eventLoop();
		SDL_Surface* surfaceMessage =
		    TTF_RenderText_Solid(font, velocy, White);

		// now you can convert it into a texture
		SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);


		clear_Renderer();
SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
		//showMousePos(font); //eats ram
		if(createMode == true){
			drawMode_Render();}



		simulation_Step(dT);
		SDL_RenderPresent(renderer);}
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

Vec2 mkvec(double x,double y){
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

void moveObj(Obj *o,double dT){
	for(int i = 0;i<o->vArr_len;i++){
		Vec2 p = o->vArr[i];
		Vec2 v = addVec(o->v,o->cen);
		Vec2 r = {o->vArr[i].x-o->cen.x,o->vArr[i].y-o->cen.y};
		o->vArr[i] = addVec(v,crossd(r,o->w));}
	return;}
Vec2 crossd2(Vec2 p ,double d){
	Vec2 r = {-1*d*p.y,d*p.x};
	return r;}

void moveObjV(Obj *o,Vec2 offs){
	for(int i = 0;i<o->vArr_len;i++){
		o->vArr[i] = addVec(o->vArr[i],offs);}
	return;}

void borderCheck2(Obj* o){

	for(int i = 0;i<o->vArr_len;i++){
		Vec2 p = o->vArr[i];
		double j;
		double e = 1; //elasticity 1 = boing boing , 0 = BAM!!
		Vec2 J;
                if(p.y > screenH){
			moveObjV(o,mkvec(0,screenH-p.y));
			o->cen = addVec(o->cen,mkvec(0,screenH-p.y));
			Vec2 r = {p.x-o->cen.x,p.y-o->cen.y};

			Vec2 v = addVec(o->v,crossd2(r,o->w));
			Vec2 po = {0,-1}; //wall normal
			j = ((-1-e)*(dot(v,po)))/(1/o->m+(cross(r,po)*cross(r,po))/o->I); // <- impulse on wall calculation
                        J.y = -1*j;} //mult by surface normal
		else if(p.y < 0){
			moveObjV(o,mkvec(0,p.y*-1));
			o->cen = addVec(o->cen,mkvec(0,p.y*-1));
			Vec2 r = {p.x-o->cen.x,p.y-o->cen.y};

			Vec2 v = addVec(o->v,crossd2(r,o->w));
			Vec2 po = {0,1};
			j = ((-1-e)*(dot(v,po)))/(1/o->m+(cross(r,po)*cross(r,po))/o->I);
                        J.y = 1*j;}
		else if(p.x >  screenW){
			moveObjV(o,mkvec(screenW-p.x,0));
			o->cen = addVec(o->cen,mkvec(screenW-p.x,0));
			Vec2 r = {p.x-o->cen.x,p.y-o->cen.y};

			Vec2 v = addVec(o->v,crossd2(r,o->w));
			Vec2 po = {-1,0};
			j = ((-1-e)*(dot(v,po)))/(1/o->m+(cross(r,po)*cross(r,po))/o->I);
                        J.x = -1*j;}
		else if(p.x < 0){
			moveObjV(o,mkvec(p.x*-1,0));
			o->cen = addVec(o->cen,mkvec(p.x*-1,0));
			Vec2 r = {p.x-o->cen.x,p.y-o->cen.y};

			Vec2 v = addVec(o->v,crossd2(r,o->w));
			Vec2 po = {1,0};
                	j = ((-1-e)*(dot(v,po)))/(1/o->m+(cross(r,po)*cross(r,po))/o->I);
                        J.x = 1*j;}
		else{continue;} //does it skip?

		Vec2 r = {p.x-o->cen.x,p.y-o->cen.y};

		o->v = addVec(o->v,mkvec(J.x/o->m,J.y/o->m));
		o->w = o->w + cross(r,J)/o->I;break;}}

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





void draw2(Obj o){
	int i;
	for(i = 0;i<o.vArr_len-1;i++){
		SDL_RenderDrawLine(renderer, o.vArr[i].x, o.vArr[i].y,o.vArr[i+1].x,o.vArr[i+1].y);}
	SDL_RenderDrawLine(renderer, o.vArr[0].x, o.vArr[0].y,o.vArr[o.vArr_len-1].x,o.vArr[o.vArr_len-1].y);}


void drawMode_Render(){
	SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
	for(int i = 0;i<pAS-1;i++){
		SDL_RenderDrawLine(renderer, pA[i].x, pA[i].y,pA[i+1].x,pA[i+1].y);}
		SDL_RenderDrawLine(renderer, pA[0].x, pA[0].y,pA[pAS-1].x,pA[pAS-1].y);

		SDL_SetRenderDrawColor(renderer, 0, 255, 255, SDL_ALPHA_OPAQUE);
		SDL_RenderDrawLine(renderer, mousex, mousey,pA[0].x,pA[0].y);
		SDL_RenderDrawLine(renderer, mousex, mousey,pA[pAS-1].x,pA[pAS-1].y);

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);};

void simulation_Step(double dT){
	for(int i = 0;i<oT_S;i++){
		moveObj(&oT[i],dT);
		oT[i].cen.x +=oT[i].v.x;
		oT[i].cen.y+=oT[i].v.y;
		borderCheck2(&oT[i]);
		draw2(oT[i]);}}


void clear_Renderer(){
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);};

void SDL_setup(){
	if(SDL_Init(SDL_INIT_VIDEO) < 0){printf("SDL could not be initialized!\n""SDL_Error: %s\n", SDL_GetError());exit(EXIT_FAILURE);}
	#if defined linux && SDL_VERSION_ATLEAST(2, 0, 8)
		if(!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0")){printf("SDL can not disable compositor bypass!\n");exit(EXIT_FAILURE);}
	#endif

	window = SDL_CreateWindow("Basic C SDL project",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,SCREEN_WIDTH, SCREEN_HEIGHT,SDL_WINDOW_SHOWN);
	if(!window){printf("Window could not be created!\n""SDL_Error: %s\n", SDL_GetError());}
	else{renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        	if(!renderer){printf("Renderer could not be created!\n""SDL_Error: %s\n", SDL_GetError());}
        	else{TTF_Init();
						font = TTF_OpenFont("Font/Hack-Bold.ttf", 20);
    if (font == NULL) {
        fprintf(stderr, "error: font not found\n");
        exit(EXIT_FAILURE);}
		}}}

void eventLoop(){
	while(SDL_PollEvent(&e)){
		if(e.type == SDL_QUIT || (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE)){
			quit = true;}
		if(e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON(SDL_BUTTON_LEFT)){
			if(createMode == false){
				createMode = true;
				pA[pAS] = mkvec(mousex,mousey);pAS+=1;}
			else if(createMode == true){
				pA[pAS] = mkvec(mousex,mousey);pAS+=1;}}
		if(e.type == SDL_MOUSEBUTTONDOWN && e.button.button ==SDL_BUTTON_RIGHT){
			if(createMode == true){
				createMode = false;
				oT[oT_S] = mkObj(&pA[0],pAS,mass,v,w);
				oT[oT_S].cen = cenObj(oT[oT_S]);oT[oT_S].I = objInertia(oT[oT_S]);
				memset(pA, 0, 1000);pAS=0;oT_S+=1;}}}}
