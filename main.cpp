#include <stdio.h>      //printf/getch itp.
#include <conio.h>      //kbhit()
#include <graphics.h>   //funkcje graficzne BGI
#include <windows.h>    //Sleep(xx)
#include <time.h>
#include <math.h>
#define MAX_ARR 128

typedef struct Vector2{
	double x;
	double y;
}Vec2;


typedef struct Triangle{
	Vec2 *p1;
	Vec2 *p2;
	Vec2 *p3;
}Tri;
void scanln(Tri tr){
    line(tr.p1->x,tr.p1->y,tr.p2->x,tr.p2->y);
    line(tr.p2->x,tr.p2->y,tr.p3->x,tr.p3->y);
    line(tr.p3->x,tr.p3->y,tr.p1->x,tr.p1->y);
}


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

			move_Tri(tp,po);
		Vec2 U = {f->x,f->y};Vec2 V = {p->x - cen.x , p->y - cen.y};f->y*=-1;return ((U.x*V.y)-(U.y*V.x));
		}
		else if(p->y < 0){Vec2 po = {0,-p->y};			//workspace bottom

			move_Tri(tp,po);
		Vec2 U = {f->x,f->y};Vec2 V = {p->x - cen.x , p->y - cen.y};f->y*=-1;return ((U.x*V.y)-(U.y*V.x));
		}
		if(p->x >  screen.x){Vec2 po = {screen.x - p->x,0};	//workspace left

			move_Tri(tp,po);
		Vec2 U = {f->x,f->y};Vec2 V = {p->x - cen.x , p->y - cen.y};f->x*=-1;return ((U.x*V.y)-(U.y*V.x));
		}
		else if(p->x < 0){Vec2 po = {-p->x,0};			//workspace right

			move_Tri(tp,po);
		Vec2 U = {f->x,f->y};Vec2 V = {p->x - cen.x , p->y - cen.y};f->x*=-1;return ((U.x*V.y)-(U.y*V.x));
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
int main()
{
    int GraphDriver = DETECT;
    int GraphMode;
    int ErrorCode;
    initgraph(&GraphDriver, &GraphMode,"");
    ErrorCode = graphresult();
    if(ErrorCode != grOk)
    {
        printf("Blad trybu graficznego: %s\n", grapherrormsg(ErrorCode) );
        exit(1);
    }
    //Tutaj kod związany z rysowaniem
    //******************************************************************************************************************

/*
	int ms = 9;
	float //declare all the values
	      cx = getmaxx()/2,cy = getmaxy()/2,  	//starting x,y
	      gF = 0.1, 				//gravity
	      Vx = 4,Vy = 1, 				//initial velocity
	      Fy = 0.98,Fx = 0.98; 			//floor & wall friction

	time_t start, now;struct timespec delay;delay.tv_sec = 0;delay.tv_nsec = ms * 999999L;time(&start);

	while(1){

		int sx = getmaxx()-1,sy = getmaxy()-1;
		int r = 10;

		Vy += gF;cx += Vx;cy += Vy;
		//y boundries collision
		if(cy+r/2 > sy){cy = sy-(r/2);Vy*=-Fy;Vx*=Fx;}
		else if(cy-r/2 < 1){cy = 1+(r/2); Vy*=-Fy;Vx*=Fx;}
		//x boundries collision
		if(cx+r/2 > sx){cx = sx-(r/2);Vx*=-Fx;Vy*=Fy;}
		else if(cx-r/2 < 1){cx = 1+(r/2);Vx*=-Fx;Vy*=Fy;}

		nanosleep(&delay,NULL);

        clearviewport();
		circle(cx,cy,10);
	}*/
ellipse(100,100,0,360,50,80);



int ms = 9;
	time_t start, now;struct timespec delay;delay.tv_sec = 0;delay.tv_nsec = ms * 999999L;time(&start);



	Vec2 pt1 = {25,20},pt2 = {20,10},pt3 = {30,20};
	Tri tp = {&pt1,&pt2,&pt3};
	Vec2 force = {0.1,0};
	double torque = 0;
	int gravacc = 0;

	time_t lastTime = 0;
	lastTime = time(&start);
	while(1){
        Vec2 screen = {getmaxx(),getmaxy()};
        //double delta = difftime(time(&start),lastTime);
        //while(lastTime < time(&start)){
            force.y =force.y+ 0.01;

        //}

		//gravacc =+ 0.1;
		Vec2 cen = tricenp(tp);rotateTri(&tp,cen,torque/180);

		move_Tri(&tp,force);

		torque = torque + border_Check(screen,tp.p1,&force,&tp,cen);
		torque = torque + border_Check(screen,tp.p2,&force,&tp,cen);
		torque = torque + border_Check(screen,tp.p3,&force,&tp,cen);
		nanosleep(&delay,NULL);
		clearviewport();
		scanln(tp);
		}




    //******************************************************************************************************************
    //koniec rysowania
    //program czeka na naciœniêcie klawisza i ZAMYKA tryb graficzny!
    fflush(stdin);
    getch(); //tylko zatrzymanie programu, żeby nie zamykał natychmiast okna graficznego
    closegraph(); //koniec trybu graficznego

    return(0);
}
