/* 
   Static Encoding of Unconstrainted Delaunay Triangulation
                           Algorithm by Dr. B. Kaan Karamete

  Mesh Generator:mesh2d.c
   Must be compiled like: cc mesh2d.c -lm -O -o mesh2d

  Input : mesh2d.dat
  Input Generator: model2d.c with modler2d.h
  Input Format:
                  alpha beta <- local and global adaptation parameters.
                  NB		 <- number of boundaries
                  bindx[1].n1 bindx[1].n2 <- Each boundary start and end nodes
                        ..		  ..
                  bindx[NB].n1 bindx[NB].n2
                  NBP		 <-  Total number of nodes
                  pc[1].x[1]  pc[1].x[2]	pc[1].x[0] <- Each node's coords and
                                                                                                  point spacing value.
                        ..			..			   ..
                        ..			..			   ..
                  pc[NBP].x[1] pc[NBP].x[2]  pc[NBP].x[0]

  Output: mesh2d.out

*/

#include <stdio.h>
#include <math.h>
#include <string.h>

#include <stdlib.h>
#include "mesh2d.h"

#define SUFFERED 50
#define eq		 ==
#define or		 ||
#define neq 	 !=
#define and 	 &&
#define true	 1
#define false	 0
#define sqr(x) (x)*(x)

/* Data Structure Definitions */

typedef struct point_list {double x[3];int bnd,next;int m;} point;

typedef struct triangle_list 
{
    int n[4];
    int t[4],nt;
    point xc;
    double R;
}
triangle;

typedef struct boundary_index {int n1,n2;} boundary;

typedef struct integer_point {long x[3];} ipoint;

/* Function Prototyping */

void Load_Triangle(int,int,int,int,int);
point Vector(point,point);
double Cross(point,point);
double Dot(point,point);
int Node_In(int,int);
void Add_Boundary_Node(int);
short InCircleTest(point,triangle);
triangle Compute_Circumcircle(triangle);
void Initialize(void);
void Suffered(int,int);
void Construct_New_Triangles(int);
int Newplaces(int);
int Readfile(void);
void Write2tecfile(void);
int Engine(int);
void Construct_New_Neighbors1(void);
void Construct_New_Neighbors2(void);
int Find_Missing_Edges(void);
void Set_Next_Fields(void);
int Node_Renumber(void);
/* Global Variables */

int NP,NE,NBP,NT,NSE,NewE,NE1,NB,MNP,nblocks,nbnd;
point	 pc[MAXPSIZE];
triangle tc[MAXESIZE];
int 	 suf[SUFFERED+1];
triangle ntc[3*SUFFERED+1];
boundary bindx[10];
int near2[10],Nnear2S;
double alpha,beta;
int nadapt=8; /*default number of adaptation cycles*/
int bloking=3;/*default node numbering style */
int smoothing=1; /*default flag for coordinate smoothing*/
point max,min,a,b;
int sufdel[SUFFERED];
int extra=0; /* extra nodes to be put in case of missing edges*/ 
char *LastUpdate="  Last Update: 4 Dec 1997";
/* Implementation of Functions */
int restart;
void Load_Triangle(id,n1,n2,n3,nt)
int id,n1,n2,n3,nt;
{ 
tc[id].n[1]=n1;
tc[id].n[2]=n2;
tc[id].n[3]=n3;
tc[id].nt=nt;
}


point Vector(p,q)
point p,q;
{ int i;
point pq;
for(i=1;i<=2;++i) pq.x[i]=q.x[i]-p.x[i];
return(pq);
}

ipoint Vectori(p,q)
point p,q;
{ int i;
ipoint pp,qq;
ipoint pq;
int j;
for(j=1;j<=2;++j)
{
    pp.x[j]=(long)(a.x[j]*p.x[j]+b.x[j]);
    qq.x[j]=(long)(a.x[j]*q.x[j]+b.x[j]);
}

for(i=1;i<=2;++i) pq.x[i]=qq.x[i]-pp.x[i];
return(pq);
}

double Cross(p1,p2)
point p1,p2;
{
return((p1.x[1]*p2.x[2]-p1.x[2]*p2.x[1]));
} 

long Crossi(p1,p2)
ipoint p1,p2;
{ 
return((p1.x[1]*p2.x[2]-p1.x[2]*p2.x[1]));
} 

double Dot(p1,p2)
point p1,p2;
{
return((p1.x[1]*p2.x[1]+p1.x[2]*p2.x[2]));
}


int Node_In(node,prev)
int node,prev;
{
double epsnodein;
/*	long epsnodein;*/
int i,j,tt,k,search;
int n1,n2,n3,n4;
short notin,ok;


epsnodein=-1e-8;
/*	epsnodein=0; */
search=prev;
do
{
notin=false;
for(i=1;i<=3;++i)
if(Cross(Vector(pc[tc[search].n[i]],pc[tc[search].n[(i%3)+1]]),
Vector(pc[tc[search].n[i]],pc[node]))
< epsnodein)
{ notin=true;
    n1=tc[search].n[i];n2=tc[search].n[(i%3)+1];
    ok=false;
    for(j=1;j<=tc[search].nt;++j)
    {
        tt=tc[search].t[j];
        for(k=1;k<=3;++k)
        {
            n3=tc[tt].n[k]; n4=tc[tt].n[(k%3)+1];
            if(((n1 eq n3) and (n2 eq n4)) or
                    ((n1 eq n4) and (n2 eq n3)))
            {
                search=tt;
                ok=true;
                break;
            }
        }
        if(ok) break;
    }
    if(ok) break;
}
} while(notin);

return(search);
} 

short InCircleTest(node,tri)
point node;
triangle tri;
{ double epsradius=-1e-5;
point point2R ;
point2R=Vector(node,tri.xc);
if(Dot(point2R,point2R)<(1.0+epsradius)*tri.R) return(true);
else return(false);
}

triangle Compute_Circumcircle(tri)
triangle tri;
{
int i;
point pq,pr,pxc,xc;
double Area, f[4];
triangle triupdate;

pq=Vector(pc[tri.n[1]],pc[tri.n[2]]);
pr=Vector(pc[tri.n[1]],pc[tri.n[3]]);
Area=Cross(pq,pr);
for(i=1;i<=3;++i) f[i]=Dot(pc[tri.n[i]],pc[tri.n[i]]);
xc.x[1]=((f[2]-f[1])*pr.x[2]-(f[3]-f[1])*pq.x[2])/(2.0*Area);
xc.x[2]=((f[3]-f[1])*pq.x[1]-(f[2]-f[1])*pr.x[1])/(2.0*Area);
triupdate=tri;
triupdate.xc=xc;
pxc=Vector(pc[tri.n[1]],xc);
triupdate.R=Dot(pxc,pxc);
return(triupdate);
} 

Test_Cavity(node)
int node;
{
int i,k,n1,n2,j,l;
short olmadi;
long Area;
for(i=1;i<=NSE;++i) sufdel[i]=1;
for(i=1;i<=NSE;++i)
for(k=1;k<=3;++k)
{
    n1=tc[suf[i]].n[k];
    n2=tc[suf[i]].n[(k%3)+1];
    olmadi=false;
    for(j=1;j<=NSE;++j)
        if(i neq j)
        {
            for(l=1;l<=3;++l)
                if(( (tc[suf[j]].n[l] eq n1) and
                     (tc[suf[j]].n[(l%3)+1] eq n2)
                     ) or
                        ( (tc[suf[j]].n[l] eq n2) and
                          (tc[suf[j]].n[(l%3)+1] eq n1) ))
                {olmadi=true;break;}

        }
    if(!olmadi)
    { ntc[1].n[1]=n1;
        ntc[1].n[2]=n2;
        ntc[1].n[3]=node;

        Area=Crossi(Vectori(pc[ntc[1].n[1]],pc[ntc[1].n[2]]),
                Vectori(pc[ntc[1].n[1]],pc[ntc[1].n[3]]));
        if(Area<0) sufdel[i]=-1;
    }
}
}

void Suffered(node,pivot)
int node,pivot;
{ int i;
short olmadi;
int ind,j;
/*	int update[SUFFERED],nnse;	*/

NSE=1;
suf[NSE]=pivot;
ind=0;
while(ind<NSE and NSE<SUFFERED)
{++ind;
    pivot=suf[ind];
    for(i=1;i<=tc[pivot].nt;++i)
    { olmadi=false;
        for(j=1;j<=NSE;++j)
            if(tc[pivot].t[i] eq suf[j]) {olmadi=true;break;}
        if(!olmadi)
            if(InCircleTest(pc[node],tc[tc[pivot].t[i]]))
                if (tc[tc[pivot].t[i]].t[0] neq 1)
                { ++NSE;
                    suf[NSE]=tc[pivot].t[i];
                }
    }
}
/*
 Test_Cavity(node);
 nnse=0;
 for(i=1;i<=NSE;++i)
  if(sufdel[i] neq -1) { ++nnse; update[nnse]=suf[i];}
 NSE=nnse;
 for(i=1;i<=NSE;++i) suf[i]=update[i];
*/
}

void Construct_New_Triangles(Newnode)
int Newnode;
{
int i,j,k,l;
short olmadi;
int n1,n2;

for(i=1;i<=3*SUFFERED-1;++i) ntc[i].nt=0;
NewE=0;

for(i=1;i<=NSE;++i)
for(k=1;k<=3;++k)
{
    n1=tc[suf[i]].n[k];
    n2=tc[suf[i]].n[(k%3)+1];
    olmadi=false;
    for(j=1;j<=NSE;++j)
        if(i neq j)
        {
            for(l=1;l<=3;++l)
                if(( (tc[suf[j]].n[l] eq n1) and
                     (tc[suf[j]].n[(l%3)+1] eq n2)
                     ) or
                        ( (tc[suf[j]].n[l] eq n2) and
                          (tc[suf[j]].n[(l%3)+1] eq n1) ))
                {olmadi=true;break;}

        }
    if(!olmadi) { ++NewE;
        ntc[NewE].n[1]=n1;
        ntc[NewE].n[2]=n2;
        ntc[NewE].n[3]=Newnode;
        ntc[NewE].n[0]=suf[i];
        ntc[NewE]=Compute_Circumcircle(ntc[NewE]);

    }
}
}  

int Newplaces(newel)
int newel;
{
if(newel<=NSE) return(suf[newel]);
else return(NE+(newel-NSE));
}
/* construct neighborhood info for new triangles
  for new triangles
  if any two nodes of the new triangle=the other new triangles
  or
  the neighborhood triangles of its suffered triangle
  stored at ntc[i].n[0].
*/

void Construct_New_Neighbors1(void)
{
    int i,j,k,l,n1,n2,kk;
    short oldu;
    int neybor;

    /* check for neighborhood triangles of its suffered triangle*/

    for(i=1;i<=NewE;++i)
        for(k=1;k<=3;++k)
        {
            n1=ntc[i].n[k];
            n2=ntc[i].n[(k%3)+1];
            for (j=1;j<=tc[ntc[i].n[0]].nt;++j)
            { oldu=false;
                neybor=tc[ntc[i].n[0]].t[j];
                for(l=1;l<=3;++l)
                    if( ((tc[neybor].n[l] eq n1) and (tc[neybor].n[(l%3)+1] eq n2))
                            or
                            ((tc[neybor].n[l] eq n2) and (tc[neybor].n[(l%3)+1] eq n1)))
                    { oldu=true; break;}
                if(oldu) break;
            }
            if(oldu)
            {
                ++ntc[i].nt;
                ntc[i].t[ntc[i].nt]=neybor;
                for(kk=1;kk<=tc[neybor].nt;++kk)
                    if(tc[neybor].t[kk] eq ntc[i].n[0]) break;
                tc[neybor].t[kk]=Newplaces(i);

            }
        }

}

void Construct_New_Neighbors2(void)
{
    int i,j,k,l,n1,n2;
    short oldu;
    oldu=false;

    for(i=1;i<=NewE;++i)
        for(k=1;k<=3;++k)
        {
            n1=ntc[i].n[k];
            n2=ntc[i].n[(k%3)+1];
            for(j=1;j<=NewE;++j)
                if(i neq j)
                { oldu=false;
                    for(l=1;l<=3;++l)
                        if( ((ntc[j].n[l] eq n1) and (ntc[j].n[(l%3)+1] eq n2))
                                or
                                ((ntc[j].n[l] eq n2) and (ntc[j].n[(l%3)+1] eq n1)))
                        { oldu=true; break;}
                    if (oldu) break;
                }
            if(oldu)
            {
                ++ntc[i].nt;
                ntc[i].t[ntc[i].nt]=Newplaces(j);
            }
        }

    for(i=1;i<=NewE;++i)
        tc[Newplaces(i)]=ntc[i];
    NE=NE+NewE-NSE;
}

int Engine(Newnode)
int Newnode;
{
static int pivot=1;

if (restart==1) {// added by Suresh
    pivot = 1;
    restart = 0;
}

pivot=Node_In(Newnode,pivot);
Suffered(Newnode,pivot);
Construct_New_Triangles(Newnode);
Construct_New_Neighbors1();
Construct_New_Neighbors2();
return (1);

}
void Set_Next_Fields(void)
{ int i,j;
    for(i=1;i<=NBP;++i) {pc[i].next=0;pc[i].bnd=0;}
    for(i=1;i<=NB;++i)
    {
        for(j=bindx[i].n1;j<=bindx[i].n2;++j)
        {
            pc[j].next=j+1;
            pc[j].bnd=i;
        }
        pc[bindx[i].n2].next=bindx[i].n1;
    }
    for(i=1;i<=NBP;++i) pc[i].m=pc[i].next;
}

int Readfile(void)
{ 
    FILE *in;
    int i,j;

    in=fopen("simple.dat","r");
    if(in eq NULL) return(0);

    fscanf(in,"%lf %lf\n",&alpha,&beta);
    fscanf(in,"%d\n",&NB);
    for(i=1;i<=NB;++i)
    {
        fscanf(in,"%d %d\n",&bindx[i].n1,&bindx[i].n2);
        bindx[i].n1=bindx[i].n1+4;
        bindx[i].n2=bindx[i].n2+4;
    }
    fscanf(in,"%d\n",&NT); /* NP olacak */
    NBP=NT+4;
    fscanf(in,"%lf %lf %lf\n",&pc[5].x[1],&pc[5].x[2],&pc[5].x[0]);
    min=pc[5];
    max=pc[5];
    for(i=6;i<=NBP;++i)
    {
        fscanf(in,"%lf %lf %lf\n",&pc[i].x[1],&pc[i].x[2],&pc[i].x[0]);
        for(j=1;j<=2;++j)
        {
            if(pc[i].x[j]<min.x[j]) min.x[j]=pc[i].x[j];
            if(pc[i].x[j]>max.x[j]) max.x[j]=pc[i].x[j];
        }
    }
    fclose(in);
    nbnd=bindx[NB].n2;
    Set_Next_Fields();
    return(1);
}

void Initialize()
{
    int i;
    point boxmax,boxmin;

    /* create initial convex hull */

    for(i=1;i<=2;++i)
    {
        pc[1].x[i]=min.x[i]-fabs((max.x[i]-min.x[i])*0.2);
        pc[3].x[i]=max.x[i]+fabs((max.x[i]-min.x[i])*0.2);
    }
    pc[2].x[1]=pc[3].x[1];
    pc[2].x[2]=pc[1].x[2];
    pc[4].x[1]=pc[1].x[1];
    pc[4].x[2]=pc[3].x[2];

    Load_Triangle(1,1,2,3,1);
    tc[1].t[1]=2;
    Load_Triangle(2,1,3,4,1);
    tc[2].t[1]=1;
    NP=4;
    tc[1]=Compute_Circumcircle(tc[1]);
    tc[2]=Compute_Circumcircle(tc[2]);
    NE=2;
    /*
 Points 1-4 and Triangles 1-2 are used for creating  the initial template
*/
    for(i=1;i<=2;++i)
    {
        boxmin.x[i]=1.0;
        boxmax.x[i]=10000.0;
        a.x[i]=(boxmax.x[i]-boxmin.x[i])/(pc[3].x[i]-pc[1].x[i]);
        b.x[i]=boxmax.x[i]-a.x[i]*pc[3].x[i];
    }
    //puts("**********************************************************");
    //printf("\nGenerator in progress starting with %d nodes and\n",NBP-4);
    //printf(" with the parameters n1=%d n2=%d n3=%d\n\n",nadapt,bloking,smoothing);
    //puts("**********************************************************");

}

void Write2tecfile(void)
{
    FILE *t,*f;
    int i,j,trino;

    t=fopen("mesh2d.out","w");
    fprintf(t,"VARIABLES=x y dpi bnd\n");
    fprintf(t,"ZONE N=%d,E=%d,F=FEPOINT,ET=TRIANGLE\n",NP,NE-NE1);
    for(i=1;i<=NP;++i)
        fprintf(t,"%f %f %f %d\n",pc[i].x[1],pc[i].x[2],pc[i].x[0],pc[i].bnd);
    for(i=1;i<=NE;++i)
        if(tc[i].t[0] neq 1)
            fprintf(t,"%d %d %d\n",tc[i].n[1],tc[i].n[2],tc[i].n[3]);
    fclose(t);
    printf("Output file mesh2d.out is generated!\n");

    f=fopen("ptc.dat","w");
    fprintf(f,"%d\n",NE);
    for(i=1;i<=NE;++i)
    { fprintf(f,"%d ",i);
        for(j=1;j<=tc[i].nt;++j)
        { trino=tc[i].t[j];
            if(tc[trino].t[0] neq 1)
                fprintf(f,"%d ",trino);
            else fprintf(f,"-1 ");
        }
        fprintf(f,"\n");
    }
    fclose(f);
    printf("Triangles info file ptc.dat is generated!\n");


}  

void Remove_Unwanted_Triangles(void)
{ int i,j;
    int pivot;

    //puts("Unwanted triangles are being swept off...");
    for(i=1;i<=NE;++i)
        for(j=1;j<=3;++j)
            if(tc[i].n[j] <= 4) {tc[i].t[0]=1;++NE1;break;}

    for(i=1;i<=NE;++i)
        if (
                pc[tc[i].n[2]].bnd eq pc[tc[i].n[1]].bnd and
                pc[tc[i].n[3]].bnd eq pc[tc[i].n[1]].bnd and
                pc[tc[i].n[1]].bnd neq 0)
        {
            j=1;
            pivot=tc[i].n[1];
            do
            {
                if(pc[pivot].next eq tc[i].n[(j%3)+1]) break;
                else if(pc[pivot].next eq tc[i].n[(((j%3)+1)%3)+1])
                {tc[i].t[0]=1;++NE1;break;}
                else if(j<3) pivot=tc[i].n[++j];
                else pivot=pc[pivot].next;
            }while(true);
        }
} 

int Find_Missing_Edges(void)
{
    int i,j,k;
    int missing[100];
    int MNP=0;

    for(i=1;i<=NE;++i)
        for(j=1;j<=3;++j)
            for(k=1;k<=3;++k)
                if(pc[tc[i].n[j]].m eq tc[i].n[k])
                    pc[tc[i].n[j]].m=0;

    for(i=1;i<=NP;++i)
        if(pc[i].m neq 0) missing[++MNP]=i;

    for(i=1;i<=MNP;++i)
    {
        ++NBP;
        ++NP;
        pc[NP].next=pc[missing[i]].next;
        pc[NP].m=pc[missing[i]].next;
        pc[NP].bnd=pc[missing[i]].bnd;
        pc[NP].x[1]=(pc[missing[i]].x[1]+pc[pc[missing[i]].next].x[1])/2.0;
        pc[NP].x[2]=(pc[missing[i]].x[2]+pc[pc[missing[i]].next].x[2])/2.0;
        pc[NP].x[0]=pc[missing[i]].x[0];
        /* printf("%lf %lf\n",pc[NP].x[1],pc[NP].x[2]);*/
        pc[missing[i]].next=NP;
        pc[missing[i]].m=NP;
        pc[missing[i]].bnd=pc[missing[i]].bnd;
        nbnd=bindx[NB].n2+1;
    }
    //printf("Detecting and Curing %d Missing Edges...\n",MNP);
    extra=extra+MNP;
    return(MNP);
}

void near2_Nodes(tno,nn)
int tno;
int *nn;
{ int i,k,l;
int pivot[5];
pivot[1]=tno;
for(i=1;i<=tc[tno].nt;++i) pivot[i+1]=tc[tno].t[i];
k=0;
for(l=1;l<=tc[tno].nt+1;++l)
for(i=1;i<=tc[pivot[l]].nt;++i)
if(tc[tc[pivot[l]].t[i]].n[0] neq 0)
{++k;near2[k]=tc[tc[pivot[l]].t[i]].n[0];}
*nn=k;
} 

Insert_Nodes(alpha,beta)
double alpha,beta;
{
int  i,j;
point p[5];
double dm[4],sj;
short	reject;
short EXCEEDED=false;
int nold=0;
int cnt=0;

while(cnt<nadapt and (NP-nold)>(0.1*nold))
{ ++cnt;
    nold=NP;
    for(i=1;i<=NE;++i) tc[i].n[0]=0;
    for(i=1;i<=NE;++i)
        if(tc[i].t[0] eq 0)
        {
            reject=0;
            for (j=1;j<=3;++j) p[j]=pc[tc[i].n[j]];
            p[4].x[0]=(p[1].x[0]+p[2].x[0]+p[3].x[0])/3.0;
            p[4].x[1]=(p[1].x[1]+p[2].x[1]+p[3].x[1])/3.0;
            p[4].x[2]=(p[1].x[2]+p[2].x[2]+p[3].x[2])/3.0;
            for (j=1;j<=3;++j)
                dm[j]=sqrt((p[j].x[1]-p[4].x[1])*(p[j].x[1]-p[4].x[1])
                        +(p[j].x[2]-p[4].x[2])*(p[j].x[2]-p[4].x[2]));
            for (j=1;j<=3;++j)  if (dm[j]<(alpha*p[4].x[0])) { reject=1; break;}
            if(!reject)
            {
                near2_Nodes(i,&Nnear2S);
                for(j=1;j<=Nnear2S;++j)
                {
                    sj=sqrt(sqr(pc[near2[j]].x[1]-p[4].x[1])+
                            sqr(pc[near2[j]].x[2]-p[4].x[2]));
                    if (sj<(beta*p[4].x[0])) { reject=1;break;}
                }
            }
            if(!reject)
            {
                ++NP;
                pc[NP]=p[4];
                pc[NP].next=0;
                pc[NP].bnd=0;
                tc[i].n[0]=NP;
                if(NP eq MAXPSIZE-1) { EXCEEDED=true;break;}
            }
        }
    for(i=nold+1;i<=NP;++i) Engine(i);
}
}

Compute_Centroids(tno,xc,yc)
int tno;
double *xc;
double *yc;
{
int i;
double sumx,sumy;
sumx=0.0;sumy=0.0;
for(i=1;i<=3;++i)
{
    sumx=sumx+pc[tc[tno].n[i]].x[1];
    sumy=sumy+pc[tc[tno].n[i]].x[2];
}
*xc=sumx/3.0;
*yc=sumy/3.0;
}

Node_Renumber()
{

    int tri[MAXESIZE],node[MAXPSIZE],nnode[MAXPSIZE],ntri[MAXESIZE];
    int i,j,pivot,NNE,NNP,cpivot,trino,cnode,ind,ok;
    double xc,yc,xsc,ysc,minimum,diff;
    int k,m,z,maxz,band[4],maxband,neks;
    int piv,sakla,stack[500],nstack,maxinter;
    point orta;
    //puts("Node and Triangle Renumbering...");
    NNP=0;
    NNE=1;
    for(i=1;i<=NE;++i) tri[i]=0;
    for(i=1;i<=NP;++i) node[i]=0;
    if(bloking eq 3){
        orta.x[1]=min.x[1]+(max.x[1]-min.x[1])/2.0;
        orta.x[2]=min.x[2]+(max.x[2]-min.x[2])/2.0;}

    minimum=sqr(max.x[1]);
    for(i=1;i<=NE;++i)
        if(tc[i].t[0] neq 1)
        {
            Compute_Centroids(i,&xc,&yc);
            switch (bloking)
            { case 1: diff=sqr(xc-max.x[1]);
                if(diff<minimum) {minimum=diff;pivot=i;}
                break;
            case 2: diff=sqr(yc-max.x[2]);
                if(diff<minimum) {minimum=diff;pivot=i;}
                break;
            case 3:
                diff=sqr(xc-orta.x[1])+sqr(yc-orta.x[2]);
                if(diff<minimum) {minimum=diff;pivot=i;}
                break;
            default:
                diff=sqr(xc-orta.x[1])+sqr(yc-orta.x[2]);
                if(diff<minimum) {minimum=diff;pivot=i;}
                break;
            }
        }
    tri[pivot]=NNE;
    for(i=1;i<=3;++i) {++NNP;node[tc[pivot].n[i]]=NNP;}
    Compute_Centroids(pivot,&xc,&yc);
    xsc=xc;ysc=yc;
    nstack=0;
    stack[++nstack]=pivot;
    maxinter=0;
    maxband=0;
    do
    { ind=0;
        if(nstack>maxinter) maxinter=nstack;
        for(j=1;j<=nstack;++j)
        { pivot=stack[j];
            sakla=ind;
            for(i=1;i<=tc[pivot].nt;++i)
            { trino=tc[pivot].t[i];
                if((tri[trino] eq 0) and (tc[trino].t[0] neq 1))
                {
                    Compute_Centroids(trino,&xc,&yc);
                    ++ind;
                    switch (bloking)
                    { case 1: diff=sqr(xc-xsc);break;
                    case 2: diff=sqr(yc-ysc);break;
                    case 3: diff=sqr(xc-xsc)+sqr(yc-ysc);break;
                    case 4: z=0;
                        for(k=1;k<=3;++k)
                            for(m=1;m<=3;++m)
                                if(tc[pivot].n[k]==tc[trino].n[m])
                                    band[++z]=tc[pivot].n[k];
                        for(m=1;m<=3;++m)
                        { ok=0;
                            for(k=1;k<=z;++k)
                                if(band[k] eq tc[trino].n[m]) {ok=1; break;}
                            if(!ok) {neks=tc[trino].n[m];break;}
                        }
                        if(node[neks] eq 0) neks=NNP+1; else neks=node[neks];
                        maxz=0;
                        for(k=1;k<=z;++k)
                            if(abs(node[band[k]]-neks)>maxz)
                                maxz=abs(node[band[k]]-neks);
                        diff=(sqr(xc-xsc)+sqr(yc-ysc))*(maxz);
                        break;
                    default: diff=sqr(xc-xsc)+sqr(yc-ysc);break;
                    }
                    if(ind eq 1) { minimum=diff; cpivot=trino;piv=pivot;}
                    else if (minimum>diff) {minimum=diff; cpivot=trino;piv=pivot;}
                }
            }
            if(sakla eq ind) stack[j]=0;
        }
        for(i=1;i<=nstack;++i)
            if(stack[i] eq 0)
            {
                for(j=i+1;j<=nstack;++j)
                    stack[j-1]=stack[j];
                --nstack;
            }

        if(ind>0)
        {
            stack[++nstack]=cpivot;
            Compute_Centroids(cpivot,&xc,&yc);
            xsc=(xsc*NNE+xc)/(NNE+1);
            ysc=(ysc*NNE+yc)/(NNE+1);
            ++NNE;
            tri[cpivot]=NNE;
            for(i=1;i<=3;++i)
            {
                ok=0;
                for(j=1;j<=3;++j)
                    if(tc[cpivot].n[i] eq tc[piv].n[j])
                    { ok=1;break;}
                if(!ok)
                {
                    cnode=tc[cpivot].n[i];
                    break;
                }
            }
            if(node[cnode] eq 0) {++NNP;node[cnode]=NNP;}

            for(k=1;k<=3;++k)
                if(abs(node[tc[cpivot].n[k]]-node[tc[cpivot].n[(k%3)+1]])>maxband)
                    maxband=abs(node[tc[cpivot].n[k]]-node[tc[cpivot].n[(k%3)+1]]);
        }
        else
            break;
    }while(NNE<(NE-NE1));

    for(i=1;i<=NP;++i)
        nnode[node[i]]=i;
    for(i=1;i<=NE;++i)
        ntri[tri[i]]=i;

    return(1);
}


Smooth()
{
    int ptc[MAXPSIZE][20];
    int i,j,tno,k;
    double xc,yc,xsc,ysc;

    //puts("Smoothing three times...");
    for(i=1;i<=NP;++i) ptc[i][0]=0;

    for(i=1;i<=NE;++i)
        if(tc[i].t[0] neq 1)
            for(j=1;j<=3;++j)
                if(tc[i].n[j]>bindx[NB].n2+4+extra)
                {++ptc[tc[i].n[j]][0];
                    tno=ptc[tc[i].n[j]][0];
                    ptc[tc[i].n[j]][tno]=i;
                }

    for(k=1;k<=3;++k)
        for(i=1;i<=NP;++i)
            if(i>bindx[NB].n2+4+extra)
            { xsc=0.0;
                ysc=0.0;
                for(j=1;j<=ptc[i][0];++j)
                {
                    Compute_Centroids(ptc[i][j],&xc,&yc);
                    xsc=xsc+xc;
                    ysc=ysc+yc;
                }
                if(ptc[i][0]>0)
                {
                    pc[i].x[1]=xsc/ptc[i][0];
                    pc[i].x[2]=ysc/ptc[i][0];
                }
            }

}
int mesh2D(MESH2D_DATA input,MESH2D_MESH *output)
{
    int i, j;
    int loop,tr=0;
    restart = 1;
    nadapt = 8; /* number of adaption fixed */
    bloking = 0;
    smoothing = 1;
    extra = 0;
    tr = 0;
    alpha = 0.4;
    beta = 0.4;

    for ( i = 0; i <SUFFERED+1; i++)
        suf[i] = 0;
    for ( i = 0; i <SUFFERED; i++)
        sufdel[i] = 0;
    for ( i = 0; i <10; i++)
        near2[i] = 0;

    for (i = 0; i <MAXPSIZE;i++) {
        pc[i].bnd = 0;
        pc[i].next = 0;
        pc[i].m = 0;
    }
    for (i = 0; i <10;i++) {
        bindx[i].n1 = 0;
        bindx[i].n2 = 0;
    }
    for (i = 0; i <MAXESIZE;i++) {
        for ( j = 0; j <4; j++) {
            tc[i].n[j] = 0;
            tc[i].t[j] = 0;
        }
        tc[i].nt = 0;
        tc[i].xc.bnd = 0;
        tc[i].xc.next = 0;
        tc[i].xc.m = 0;
    }


    NB = input.nLoops; // number of loops
    i = 1;
    for (loop =0; loop < input.nLoops; loop++) {
        bindx[loop+1].n1 = input.loopStart[loop];
        bindx[loop+1].n2 = input.loopEnd[loop];

        bindx[loop+1].n1 += 4;
        bindx[loop+1].n2 += 4;
    }

    NT = input.nPoints;
    NBP=NT+4;

    // the first point
    pc[5].x[1] = input.xPt[0];
    pc[5].x[2] = input.yPt[0];
    pc[5].x[0] = input.elementSize;
    min=pc[5];
    max=pc[5];


    for(i=6;i<=NBP;++i){
        pc[i].x[1] = input.xPt[i-5];
        pc[i].x[2] = input.yPt[i-5];
        pc[i].x[0] = input.elementSize;

        for(j=1;j<=2;++j) {
            if(pc[i].x[j]<min.x[j]) min.x[j]=pc[i].x[j];
            if(pc[i].x[j]>max.x[j]) max.x[j]=pc[i].x[j];
        }
    }

    nbnd=bindx[NB].n2;
    Set_Next_Fields();

    Initialize();

    do { ++NP;Engine(NP); } while(NP<NBP);

    while((MNP=Find_Missing_Edges()) neq 0)
        for(i=NP-MNP+1;i<=NP;++i) Engine(i);

    for(i=1;i<=NE;++i)
        tc[i].t[0]=0;
    NE1=0;

    Remove_Unwanted_Triangles();//commented by suresh since it causes problems during multiple calls to mesh2d
    //printf("Bnd. Triangulation: %d nodes %d triangles\n",NP-4,NE-NE1);
    Insert_Nodes(alpha,beta);

    if(smoothing) Smooth();


    if(bloking!=0) {
        if(!Node_Renumber())
            ;
    }
    else
        ;

    /* set output */
    (*output).nNodes = NP;
    (*output).nTriangles = NE-NE1;
    for(i=1;i<=NP;++i) {
        (*output).nodeXY[i-1][0] = pc[i].x[1];
        (*output).nodeXY[i-1][1] = pc[i].x[2];
        (*output).nodeMarker[i-1] = pc[i].bnd;	}
    for(i=1;i<=NE;++i) {
        if(tc[i].t[0] neq 1) {
            (*output).triangles[tr][0] = tc[i].n[1];
            (*output).triangles[tr][1] = tc[i].n[2];
            (*output).triangles[tr][2] = tc[i].n[3];
            tr++;
        }
    }

    return (1);

}
