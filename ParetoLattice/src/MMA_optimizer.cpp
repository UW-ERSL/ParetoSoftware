#include "MMA_optimizer.hpp"
//#include "GaussElimination.cpp"
//#include<stdlib.h>

#include<iostream>
#include <cmath>
#include <vector>
#include <iomanip>


using namespace std;

vector<double> gauss(vector< vector<double> > A) {
    int n = A.size();

    for (int i=0; i<n; i++) {
        // Search for maximum in this column
        double maxEl = abs(A[i][i]);
        int maxRow = i;
        for (int k=i+1; k<n; k++) {
            if (abs(A[k][i]) > maxEl) {
                maxEl = abs(A[k][i]);
                maxRow = k;
            }
        }

        // Swap maximum row with current row (column by column)
        for (int k=i; k<n+1;k++) {
            double tmp = A[maxRow][k];
            A[maxRow][k] = A[i][k];
            A[i][k] = tmp;
        }

        // Make all rows below this one 0 in current column
        for (int k=i+1; k<n; k++) {
            double c = -A[k][i]/A[i][i];
            for (int j=i; j<n+1; j++) {
                if (i==j) {
                    A[k][j] = 0;
                } else {
                    A[k][j] += c * A[i][j];
                }
            }
        }
    }

    // Solve equation Ax=b for an upper triangular matrix A
    vector<double> x(n);
    for (int i=n-1; i>=0; i--) {
        x[i] = A[i][n]/A[i][i];
        for (int k=i-1;k>=0; k--) {
            A[k][n] -= A[k][i] * x[i];
        }
    }
    return x;
}
/************************************/
void MMA_optimizer::initialize(int numOfDesignVariables, int numOfConstraints, int maxOuterIterations, double relFuncTol, double kktTolerance, double epsiminTilde, double cTilde, double dTilde, double a0Tilde, double aTilde, double raa0Tilde, double raaepsTilde) {
    //    setInitialValues();
    m = numOfConstraints; //No. of Constraints
    n = numOfDesignVariables;  //No. of design variables

    epsimin = epsiminTilde; //some number

    sens.resize(n);
    constraint.resize(m);
    constrSens.resize(n*m);

    xval.resize(n);

    xold1.resize(n);
    xold2.resize(n);

    xmin.resize(n);
    xmax.resize(n);

    low.resize(n);
    upp.resize(n);
    xmami.resize(n);

    c.resize(m);
    for(int i = 0;i<m;i++)
        c[i] = cTilde;

    d.resize(m);
    for(int i = 0;i<m;i++)
        d[i] = dTilde;

    a0      = a0Tilde;
    a.resize(m);
    for(int i = 0 ; i < m ; i++)
        a[i] = aTilde;

    raa0    = raa0Tilde;
    raa.resize(m);
    for(int i = 0;i<m;i++)
        raa[i] = raa0;
    raa0eps    = raaepsTilde;
    raaeps.resize(m);
    for(int i = 0;i<m;i++)
        raaeps[i] =  raa0eps;

    outerIter = 0;
    maxOutIt  = maxOuterIterations; // Convergence criteria
    kkttol  = kktTolerance; // Convergence criteria
    relativeFuncTol = relFuncTol; // Convergence criteria

    x.resize(n);
    y.resize(m);
    xsi.resize(n);
    eta.resize(n);
    mu.resize(m);
    s.resize(m);
    lam.resize(m);
    fapp.resize(m);

}
/************************************/
void MMA_optimizer::setDesignVariableInitialValuesAndBounds(vector<double> xInit,vector<double> xLow,vector<double> xUpp)
{
    xval = xInit;
    xold1 = xval;
    xold2 = xval;
    xmin = xLow;
    low = xLow;
    xmax = xUpp;
    upp = xUpp;
    for(int i = 0; i < n ; i++)
    {
        xmami[i] = max(xmax[i]-xmin[i],0.00001);
    }
}
/************************************/
void MMA_optimizer::asymp(vector<double> df0dx,vector<double> dfdx)
{
    raa0 = 0;
    for(int j = 0;j<m;j++)
        raa[j] = 0;
    for(int i = 0;i<n;i++){

        raa0 = raa0 + fabs(df0dx[i]) * xmami[i];
        for(int j = 0;j<m;j++)
            raa[j] = raa[j] + fabs(dfdx[j*n+i]) * xmami[i];
    }
    raa0 = max(raa0eps, (0.1 / n)*raa0);
    for(int j = 0;j<m;j++)
        raa[j] = max(raaeps[j], (0.1 / n)*raa[j]);

    if (outerIter < 2.5) {
        for(int i = 0;i<n;i++){
            low[i] = xval[i] - 0.5*xmami[i];
            upp[i] = xval[i] + 0.5*xmami[i];
        }
    }
    else {
        double xxx,factor;
        for(int i = 0;i<n;i++){
            xxx = (xval[i] - xold1[i])*(xold1[i] - xold2[i]);
            if(xxx>0)
                factor = 1.2;
            else if (xxx<0)
                factor = 0.7;
            else
                factor = 1;
            low[i] = xval[i] - factor*(xold1[i] - low[i]);
            upp[i] = xval[i] + factor*(upp[i] - xold1[i]);
            low[i] = min(max(low[i],xval[i]-10*xmami[i]),xval[i]-0.01*xmami[i]);
            upp[i] = max(min(upp[i],xval[i]+10*xmami[i]),xval[i]+0.01*xmami[i]);
        }
    }
}
/************************************/
void MMA_optimizer::gcmmasub(double f0val,vector<double> df0dx, vector<double> fval,vector<double> dfdx) {

    // Calculations of the bounds alfa and beta.

    vector<double> alfa,beta,ux1,xl1,p0,q0;
    alfa.resize(n);
    beta.resize(n);
    ux1.resize(n);
    xl1.resize(n);
    p0.resize(n);
    q0.resize(n);

    double albefa = 0.1, r0 = f0val;
    double pq0;
    // Calculations of p0, q0, r0, P, Q, r and b.
    for(int i = 0;i<n;i++){
        alfa[i] = max(low[i] + albefa*(xval[i]-low[i]),xmin[i]);
        beta[i] = min(upp[i] - albefa*(upp[i]-xval[i]),xmax[i]);

        ux1[i] = upp[i]-xval[i];
        xl1[i] = xval[i]-low[i];
        p0[i] = max(df0dx[i],0.0);
        q0[i] = max(-df0dx[i],0.0);
        pq0 = p0[i]+q0[i];
        p0[i] = (p0[i] + 0.001*(pq0) + raa0/xmami[i])*ux1[i]*ux1[i];
        q0[i] = (q0[i] + 0.001*(pq0) + raa0/xmami[i])*xl1[i]*xl1[i];
        r0 = r0 - p0[i]/ux1[i] - q0[i]/xl1[i];
    }
    std::vector<double> P,Q,r,b;
    P.resize(m*n);
    Q.resize(m*n);
    r.resize(m);
    b.resize(m);

    for(int j = 0;j<m;j++){
        r[j] = fval[j];
    }

    for(int j = 0;j<m;j++){
        for(int i = 0;i<n;i++){
            P[j*n + i] = max(dfdx[j*n + i],0.0);
            Q[j*n + i] = max(-dfdx[j*n + i],0.0);
            pq0 = (P[j*n + i]+Q[j*n + i])*0.001;
            P[j*n + i] = (P[j*n + i] +pq0 + raa[j]/xmami[i])*ux1[i]*ux1[i];
            Q[j*n + i] = (Q[j*n + i] +pq0 + raa[j]/xmami[i])*xl1[i]*xl1[i];
            r[j] = r[j] - P[j*n + i]/ux1[i] - Q[j*n + i]/xl1[i];
        }
        b[j] = -r[j];
    }

    // Solving the subproblem by a primal-dual Newton method

    subsolv(alfa,beta,p0,q0,P,Q,b);

    //Calculations of f0app and fapp.
    f0app = r0;
    for(int j = 0;j<m;j++)
        fapp[j] = r[j];

    for(int i = 0;i<n;i++){
        ux1[i] = upp[i]-x[i];
        xl1[i] = x[i]-low[i];
        f0app = f0app + p0[i]/ux1[i] + q0[i]/xl1[i];
    }
    for(int j = 0;j<m;j++){
        for(int i = 0;i<n;i++){
            fapp[j] = fapp[j] + P[j*n + i]/ux1[i] + Q[j*n + i]/xl1[i];
        }
    }

}
/************************************/
void MMA_optimizer::subsolv(vector<double> alfa,vector<double> beta,vector<double> p0,vector<double> q0,vector<double> P,vector<double> Q,vector<double> b){

    double epsi = 1,rez,rezet,residu,residunorm,residumax;
    int itto,ittt;

    for(int i = 0;i<n;i++) {
        x[i] = 0.5*(alfa[i]+beta[i]);
    }
    for(int j = 0 ; j < m ; j++)
    {
        y[j] = 1;s[j] = 1; lam[j] = 1;
        mu[j] = max(1.0,0.5*c[j]);
    }
    z = 1;

    for(int i = 0;i<n;i++) {
        xsi[i] = max(1.0,1./(x[i]-alfa[i]));
        eta[i] = max(1.0,1./(beta[i]-x[i]));
    }
    zet = 1;
    std::vector<double> ux1,xl1,GG,delx,diagx;
    ux1.resize(n);
    xl1.resize(n);

    GG.resize(m*n);

    double delz;

    delx.resize(n);

    diagx.resize(n);

    std::vector<double> diagxinv,dely,diagy,diagyinv,dellam,blam,diaglam,diaglamyi;
    diagxinv.resize(n);
    dely.resize(m);
    diagy.resize(m);
    diagyinv.resize(m);
    dellam.resize(m);
    blam.resize(m);
    diaglam.resize(m);
    diaglamyi.resize(m);;

    std::vector<double> plam,qlam,gvec,dpsidx;
    plam.resize(n);
    qlam.resize(n);
    gvec.resize(m); // GVEC IS OF SIZE M
    dpsidx.resize(n);

    std::vector<double>rex, rey, relam, rexsi, reeta, remu,res;
    rex.resize(n);
    rey.resize(m);
    relam.resize(m);
    rexsi.resize(n);
    reeta.resize(n);
    remu.resize(m);
    res.resize(m);

    double dz,dzet,steg,stmxx;
    std::vector<double> dlam,dy,dx,dxsi,deta,dmu,ds;
    dlam.resize(m);
    dy.resize(m);
    dx.resize(n);
    dxsi.resize(n);
    deta.resize(n);
    dmu.resize(m);
    ds.resize(m);

    std::vector<double> xold,yold,lamold,xsiold,etaold,muold,sold;
    xold.resize(n);
    yold.resize(m);
    lamold.resize(m);
    xsiold.resize(n);
    etaold.resize(n);
    muold.resize(m);
    sold.resize(m);
    double zold;
    double zetold = zet;

    std::vector<double> AA,bb;
    if (m<n){

        AA.resize((m+1)*(m+1));
        bb.resize(m+1);
    }
    else{

        AA.resize((n+1)*(n+1));
        bb.resize(n+1);
    }

    int itera = 0;
    while (epsi > epsimin) {
        for(int i = 0;i<n;i++){
            plam[i] = p0[i];
            qlam[i] = q0[i];
        }

        for(int i = 0 ; i < gvec.size() ; i++)
            gvec[i] = 0;

        for(int i = 0;i<n;i++){
            ux1[i] = upp[i]-x[i];
            xl1[i] = x[i]-low[i];

            for( int j = 0;j<m;j++){
                plam[i] = plam[i] + P[j*n + i]*lam[j];
                qlam[i] = qlam[i] + Q[j*n + i]*lam[j];
                gvec[j] = gvec[j]+P[j*n + i]/ux1[i]+Q[j*n + i]/xl1[i];
            }
        }

        for( int i = 0;i<n;i++){
            dpsidx[i] = plam[i]/(ux1[i]*ux1[i]) - qlam[i]/(xl1[i]*xl1[i]);
            rex[i] = dpsidx[i] - xsi[i] + eta[i];
        }
        double temp = 0;
        for( int j = 0;j<m;j++){
            rey[j] = c[j] + d[j]*y[j] - mu[j] - lam[j];
            temp = temp + a[j]*lam[j];
            relam[j] = gvec[j] - a[j]*z - y[j] + s[j] - b[j];
            remu[j] = mu[j]*y[j] - epsi;
            res[j] = lam[j] *s[j] -epsi;
        }
        rez = a0 -zet - temp;
        rezet = zet*z-epsi;
        for( int i = 0;i<n;i++){
            rexsi[i] = xsi[i]*(x[i]-alfa[i]) - epsi;
            reeta[i] = eta[i]*(beta[i]-x[i]) - epsi;
        }
        residu = 0;
        residumax = -HUGE_VAL;
        for( int i = 0;i<n;i++){
            residu = residu + rex[i]*rex[i];
            residu = residu + rexsi[i]*rexsi[i];
            residu = residu + reeta[i]*reeta[i];
            residumax = max(residumax,fabs(rex[i]));
            residumax = max(residumax,fabs(rexsi[i]));
            residumax = max(residumax,fabs(reeta[i]));
        }
        for( int j = 0;j<m;j++){
            residu = residu + rey[j]*rey[j];
            residu = residu + relam[j]*relam[j];
            residu = residu + remu[j]*remu[j];
            residu = residu + res[j]*res[j];
            residumax = max(residumax,fabs(rey[j]));
            residumax = max(residumax,fabs(relam[j]));
            residumax = max(residumax,fabs(remu[j]));
            residumax = max(residumax,fabs(res[j]));
        }
        residu = residu + rez*rez;
        residu = residu + rezet*rezet;
        residumax = max(residumax,fabs(rez));
        residumax = max(residumax,fabs(rezet));
        residunorm = sqrt(residu);

        ittt = 0;
        while (residumax > 0.9*epsi && ittt < 200) {
            ittt=ittt + 1;
            itera=itera + 1;
            for(int i = 0;i<n;i++){
                plam[i] = p0[i];
                qlam[i] = q0[i];
            }
            for(int i = 0 ; i < gvec.size() ; i++)
                gvec[i] = 0;
            for(int i = 0;i<n;i++){
                ux1[i] = upp[i]-x[i];
                xl1[i] = x[i]-low[i];

                for( int j = 0;j<m;j++){
                    plam[i] = plam[i] + P[j*n + i]*lam[j];
                    qlam[i] = qlam[i] + Q[j*n + i]*lam[j];
                    gvec[j] = gvec[j]+P[j*n + i]/ux1[i]+Q[j*n + i]/xl1[i];
                    GG[j*n + i] =  P[j*n + i]/(ux1[i]*ux1[i]) - Q[j*n + i]/(xl1[i]*xl1[i]);
                }
            }
            for( int i = 0;i<n;i++){
                dpsidx[i] = plam[i]/(ux1[i]*ux1[i]) - qlam[i]/(xl1[i]*xl1[i]);
                delx[i] = dpsidx[i] - epsi/(x[i] - alfa[i]) + epsi/(beta[i] - x[i]);
                diagx[i] = 2*(plam[i]/(ux1[i]*ux1[i]*ux1[i]) + qlam[i]/(xl1[i]*xl1[i]*xl1[i])) + xsi[i]/(x[i] - alfa[i]) + eta[i]/(beta[i] - x[i]);
                diagxinv[i] = 1/diagx[i];
            }
            double temp = 0;
            for( int j = 0;j<m;j++){
                dely[j] = c[j] + d[j]*y[j] - epsi/y[j] - lam[j];
                temp = temp + a[j]*lam[j];
                dellam[j] = gvec[j] - a[j]*z - y[j] + epsi/lam[j] - b[j];
                diagy[j] = d[j] + mu[j]/y[j];
                diagyinv[j] = 1/diagy[j];
                diaglam[j] = s[j]/lam[j];
                diaglamyi[j] = diaglam[j] + diagyinv[j];
            }
            delz = a0 - temp - epsi/z;
            if (m < n){
                for( int j = 0;j<m;j++){
                    temp = 0;
                    for( int i = 0;i<n;i++)
                        temp = temp + GG[j*n +i]*(delx[i]/diagx[i]);
                    blam[j] = dellam[j] + dely[j]/diagy[j] - temp;
                    bb[j] = blam[j];
                }
                bb[m] = delz;

                //This is for creating matrix AA
                std::vector<double> temp3;
                temp3.resize(m*m);
                for(int i = 0 ; i < m*m ; i++)
                    temp3[i] = 0;
                for( int j = 0;j<m;j++){
                    for (int k = 0;k<m;k++){
                        for (int i = 0;i<n;i++){
                            temp3[m*j+k] = temp3[m*j+k] + GG[j*n+i]*diagxinv[i]*GG[k*n+i];
                        }
                    }
                }
                for( int j = 0;j<m;j++){
                    AA[j*(m+1) + j] = diaglamyi[j];
                }
                for( int j = 0;j<m;j++){
                    for (int k = 0;k<m;k++)
                        AA[j*(m+1) + k] = AA[j*(m+1) + k] + temp3[m*j+k];
                }
                for( int j = 0;j<m;j++){
                    AA[j*(m+1) + m] = a[j];
                    AA[m*(m+1) + j] = a[j];
                }
                AA[(m+1)*(m+1)-1] = -zet/z;
                vector<double> line((m+1)+1,0);
                vector< vector<double> > A((m+1),line);
                for (int j=0; j<m+1; j++) {
                    for (int k=0; k<m+1; k++) {
                        A[j][k] = AA[j*(m+1) + k];
                    }
                    A[j][m+1] = bb[j];
                }

                vector<double> xvec(m+1);
                xvec = gauss(A);
                for (int j=0; j<m; j++)
                    dlam[j] = xvec[j];

                dz = xvec[m];

                for (int i = 0;i<n;i++){
                    temp = 0;
                    for (int j=0; j<m; j++)
                        temp = temp + GG[j*n + i]*xvec[j];
                    dx[i] = -delx[i]/diagx[i] - temp/diagx[i];
                }
            }
            else {
                std::vector<double> dellamyi;
                dellamyi.resize(m);

                for (int j = 0;j<m;j++)
                    dellamyi[j] = dellam[j] + dely[j]/diagy[j];

                std::vector<double> temp3;
                temp3.resize(n*n);
                for( int i = 0;i<n;i++){
                    for (int k = 0;k<n;k++){
                        for (int j = 0;j<m;j++){
                            temp3[n*i+k] = temp3[n*i+k] + GG[j*n+i]/diaglamyi[j]*GG[j*n+k];
                        }
                    }
                }
                for( int i = 0;i<n;i++){
                    AA[i*(n+1) + i] = diagx[i];
                }

                for( int i = 0;i<n;i++){
                    for (int k = 0;k<n;k++)
                        AA[i*(n+1) + k] = AA[i*(n+1) + k] + temp3[n*i+k];
                }
                for( int i = 0;i<n;i++){
                    temp = 0;
                    for(int j= 0;j<m;j++)
                        temp = temp + -GG[j*m+i]*(a[j]/diaglamyi[j]);
                    AA[i*(n+1) + n] = temp;
                    AA[n*(n+1) + i] = temp;
                }
                temp = 0;
                for(int j= 0;j<m;j++)
                    temp = temp + a[j]*(a[j]/diaglamyi[j]);
                AA[(n+1)*(n+1)-1] = zet/z + temp;

                for( int i = 0;i<n;i++){
                    temp = 0;
                    for(int j= 0;j<m;j++)
                        temp = temp + GG[j*m+i]*(dellamyi[j]/diaglamyi[j]);
                    bb[i] = -(delx[i] + temp);
                }
                temp = delz;
                for(int j= 0;j<m;j++)
                    temp = temp - a[j]*(dellamyi[j]/diaglamyi[j]);
                bb[n] = -temp;

                vector<double> line((n+1)+1,0);
                vector< vector<double> > A((n+1),line);
                for (int i=0; i<n+1; i++) {
                    for (int k=0; k<n+1; k++) {
                        A[i][k] = AA[i*(n+1) + k];
                    }
                    A[i][n+1] = bb[i];
                }
                vector<double> xvec(n+1);
                xvec = gauss(A);
                for (int i=0; i<n; i++)
                    dx[i] = xvec[i];
                dz = xvec[n];
                for(int i = 0 ; i < m ; i++)
                    dlam[i] = 0;

                for (int j = 0;j<m;j++){
                    temp = 0;
                    for (int i=0; i<n; i++)
                        temp = temp + GG[j*n+i]*dx[i];
                    dlam[j] = temp/diaglamyi[j] - dz*(a[j]/diaglamyi[j]) + dellamyi[j]/diaglamyi[j];
                }
            }
            for (int j = 0;j<m;j++){
                dy[j] = -dely[j]/diagy[j] + dlam[j]/diagy[j];
                dmu[j]  = -mu[j] + epsi/y[j] - (mu[j]*dy[j])/y[j];
                ds[j]   = -s[j] + epsi/lam[j] - (s[j]*dlam[j])/lam[j];
            }
            for (int i=0; i<n; i++){
                dxsi[i] = -xsi[i] + epsi/(x[i]-alfa[i]) - (xsi[i]*dx[i])/(x[i]-alfa[i]);
                deta[i] = -eta[i] + epsi/(beta[i]-x[i]) + (eta[i]*dx[i])/(beta[i]-x[i]);
            }

            dzet = -zet + epsi/z - zet*dz/z;
            stmxx = -HUGE_VAL;

            for (int j = 0;j<m;j++){
                stmxx = max(stmxx,-1.01*dy[j]/y[j]);
                stmxx = max(stmxx, -1.01*dlam[j]/lam[j]);
                stmxx = max(stmxx,-1.01*dmu[j]/mu[j]);
                stmxx = max(stmxx,-1.01*ds[j]/s[j]);
            }
            for (int i=0; i<n; i++){
                stmxx = max(stmxx,-1.01*dxsi[i]/xsi[i]);
                stmxx = max(stmxx,-1.01*deta[i]/eta[i]);
            }
            stmxx = max(stmxx,-1.01*dz/z);
            stmxx = max(stmxx,-1.01*dzet/zet);

            double stmalfa = -HUGE_VAL,stmbeta = -HUGE_VAL;
            for (int i = 0;i<n;i++){
                stmalfa  = max(-1.01*dx[i]/(x[i]-alfa[i]),stmalfa);
                stmbeta = max(1.01*dx[i]/(beta[i]-x[i]),stmbeta);
            }
            steg= 1/max( max(max(stmalfa,stmbeta),stmxx),1.0);

            for (int i = 0;i<n;i++){
                xold[i] = x[i];
                xsiold[i] = xsi[i];
                etaold[i] = eta[i];
            }

            for (int j = 0;j<m;j++){
                yold[j] = y[j];
                lamold[j] = lam[j];
                muold[j] = mu[j];
                sold[j] = s[j];
            }
            zold = z;
            zetold = zet;
            itto = 0;
            double resinew = 2*residunorm;
            while (resinew > residunorm && itto < 50){
                itto = itto+1;
                for (int i = 0;i<n;i++){
                    x[i]   =   xold[i] + steg*dx[i];
                    xsi[i] = xsiold[i] + steg*dxsi[i];
                    eta[i] = etaold[i] + steg*deta[i];
                    ux1[i] = upp[i]-x[i];
                    xl1[i] = x[i]-low[i];
                }
                for (int j = 0;j<m;j++){
                    y[j]   =   yold[j] + steg*dy[j];
                    lam[j] = lamold[j] + steg*dlam[j];
                    mu[j]  = muold[j]  + steg*dmu[j];
                    s[j]  =   sold[j] + steg*ds[j];
                }
                z   =   zold + steg*dz;
                zet = zetold + steg*dzet;

                for(int i = 0 ; i < gvec.size() ; i++)
                    gvec[i] = 0;
                for( int i = 0;i<n;i++){
                    for( int j = 0;j<m;j++){
                        plam[i] = p0[i] + P[j*n + i]*lam[j];
                        qlam[i] = q0[i] + Q[j*n + i]*lam[j];
                        gvec[j] = gvec[j]+P[j*n + i]/ux1[i]+Q[j*n + i]/xl1[i];
                    }
                }

                for( int i = 0;i<n;i++){
                    dpsidx[i] = plam[i]/(ux1[i]*ux1[i]) - qlam[i]/(xl1[i]*xl1[i]);
                    rex[i] = dpsidx[i] - xsi[i] + eta[i];
                    rexsi[i] = xsi[i]*(x[i]-alfa[i]) - epsi;
                    reeta[i] = eta[i]*(beta[i]-x[i]) - epsi;
                }
                temp = 0;
                for( int j = 0;j<m;j++){
                    temp = temp + a[j]*lam[j];
                    rey[j] = c[j] + d[j]*y[j] - mu[j] - lam[j];
                    relam[j] = gvec[j] - a[j]*z - y[j] + s[j] - b[j];
                    remu[j] = mu[j]*y[j] - epsi;
                    res[j] = lam[j]*s[j] - epsi;
                }

                rez = a0 - zet - temp;
                rezet = zet*z - epsi;

                residu = 0;
                residumax = -HUGE_VAL;
                for( int i = 0;i<n;i++){
                    residu = residu + rex[i]*rex[i];
                    residu = residu + rexsi[i]*rexsi[i];
                    residu = residu + reeta[i]*reeta[i];
                    residumax = max(residumax,fabs(rex[i]));
                    residumax = max(residumax,fabs(rexsi[i]));
                    residumax = max(residumax,fabs(reeta[i]));
                }
                for( int j = 0;j<m;j++){
                    residu = residu + rey[j]*rey[j];
                    residu = residu + relam[j]*relam[j];
                    residu = residu + remu[j]*remu[j];
                    residu = residu + res[j]*res[j];
                    residumax = max(residumax,fabs(rey[j]));
                    residumax = max(residumax,fabs(relam[j]));
                    residumax = max(residumax,fabs(remu[j]));
                    residumax = max(residumax,fabs(res[j]));
                }
                residu = residu + rez*rez;
                residu = residu + rezet*rezet;
                residumax = max(residumax,fabs(rez));
                residumax = max(residumax,fabs(rezet));

                resinew = sqrt(residu);
                steg = steg/2;
            }
            residunorm=resinew;
            steg = 2*steg;
        }
        if (ittt > 198) {
            cout<<"epsi = "<<epsi <<" ittt = "<<ittt << endl;
        }
        epsi = 0.1*epsi;
    }

}
/************************************/
int MMA_optimizer::concheck(double f0valnew,vector<double> fvalnew)
{
    int conserv = 1;

    double f0appe = f0app+epsimin;
    vector<double> fappe(m);
    for(int j= 0 ;j<m;j++)
        fappe[j] = fapp[j]+epsimin;

    if (f0appe < f0valnew){
        conserv = 0;
    }
    else{
        for(int j= 0 ;j<m;j++){
            if (fappe[j] < fvalnew[j]){
                conserv = 0;
                break;
            }
        }
    }
    return conserv;
}
/************************************/
void MMA_optimizer::raaupdate(double f0valnew, vector<double> fvalnew) {
    double raacofmin = pow(10.0,(-12.0));
    double raacof = 0,xxul,ulxx,xxux,xxxl;
    for( int i = 0;i<n;i++){
        xxux = (x[i]-xval[i])/(upp[i]-x[i]);
        xxxl = (x[i]-xval[i])/(x[i]-low[i]);
        xxul = xxux*xxxl;
        ulxx = (upp[i]-low[i])/xmami[i];
        raacof = raacof + xxul*ulxx;
    }
    raacof = max(raacof,raacofmin);
    double f0appe = f0app + 0.5*epsimin;
    if (f0valnew > f0appe){
        double deltaraa0,zz0;
        deltaraa0 = (1/raacof)*(f0valnew-f0app);
        zz0 = 1.1*(raa0 + deltaraa0);
        zz0 = min(zz0,10*raa0);
        //  zz0 = min(zz0,1000*raa0);
        raa0 = zz0;
    }
    std::vector<double> fappe, fdelta, deltaraa, zzz;
    fappe.resize(m);
    fdelta.resize(m);
    deltaraa.resize(m);
    zzz.resize(m);

    for(int j = 0;j<m;j++){
        fappe[j] = fapp[j] + 0.5*epsimin;
        fdelta[j] = fvalnew[j]-fappe[j];
        deltaraa[j] = (1/raacof)*(fvalnew[j]-fapp[j]);
        zzz[j] = 1.1*(raa[j] + deltaraa[j]);
        zzz[j] = min(zzz[j],10*raa[j]);
        //zzz[j] = min(zzz[j],1000*raa[j]);
        if (fdelta[j]>0){
            raa[j] = zzz[j];
        }
    }
}
/************************************/
double MMA_optimizer::kktcheck( vector<double> fval,vector<double>  df0dx,vector<double>  dfdx){


    std::vector<double> rex,rey,relam,rexsi,reeta;
    rex.resize(n);
    rey.resize(m);
    relam.resize(m);
    rexsi.resize(n);
    reeta.resize(n);

    std::vector<double> remu,res,residue;
    remu.resize(m);
    res.resize(m);
    residue.resize(3*m+4*n+2);

    for(int i = 0 ; i < n ; i++)
        rex[i] = 0;

    double temp;
    for( int i = 0;i<n;i++){
        rex[i] =  df0dx[i] - xsi[i] + eta[i];
        rexsi[i] = xsi[i]*(x[i]-xmin[i]);
        reeta[i] = eta[i]*(xmax[i]-x[i]);
        temp = 0;
        for( int j = 0;j<m;j++){
            temp = temp + dfdx[j*n + i]*lam[j];
        }
        rex[i] = rex[i] + temp;
    }
    temp = 0;
    for( int j = 0;j<m;j++){
        temp = temp + a[j]*lam[j];
        rey[j] = c[j] + d[j]*y[j] - mu[j] - lam[j];
        relam[j] = fval[j] - a[j]*z - y[j] + s[j];
        remu[j] = mu[j]*y[j];
        res[j] = lam[j]*s[j];
    }

    double rez = a0 - zet - temp;
    double rezet = zet*z;
    double residu = 0;
    double residumax = -HUGE_VAL;
    for( int i = 0;i<n;i++){
        residu = residu + rex[i]*rex[i];
        residu = residu + rexsi[i]*rexsi[i];
        residu = residu + reeta[i]*reeta[i];
        residumax = max(residumax,fabs(rex[i]));
        residumax = max(residumax,fabs(rexsi[i]));
        residumax = max(residumax,fabs(reeta[i]));
    }
    for( int j = 0;j<m;j++){
        residu = residu + rey[j]*rey[j];
        residu = residu + relam[j]*relam[j];
        residu = residu + remu[j]*remu[j];
        residu = residu + res[j]*res[j];
        residumax = max(residumax,fabs(rey[j]));
        residumax = max(residumax,fabs(relam[j]));
        residumax = max(residumax,fabs(remu[j]));
        residumax = max(residumax,fabs(res[j]));
    }
    residu = residu + rez*rez;
    residu = residu + rezet*rezet;
    residumax = max(residumax,fabs(rez));
    residumax = max(residumax,fabs(rezet));

    double residunorm = sqrt(residu);
    return residunorm;
}
/************************************/
void MMA_optimizer::optimize(std::function<double(vector<double>)>compObj,std::function<bool(vector<double>,vector<double>&)>compConstr,std::function<bool(vector<double>,vector<double>&)>compObjSensi,std::function<bool(vector<double>,vector<double>&)>compConstrSensi,std::function< bool( vector<double>& ) >modifyDesignVar)
{

    double objectiveNew;
    vector<double> constraintNew(m);
    bool TERMINATE_OPT = false; // set this to be true of you want opt to quit midway.
    int innerIt;
    double maxChange,changeTol = 1e-3;
    bool isConverged = false;

    obj = compObj(xval);     //f0val
    compObjSensi(xval,sens);//df0dx
    compConstr(xval,constraint);//fval
    compConstrSensi(xval,constrSens);//dfdx

    double objectiveNormalizer = obj/100;
    obj = obj/objectiveNormalizer;
    for(int i = 0;i<n;i++)
        sens[i] = sens[i]/objectiveNormalizer;

    innerIt = 0;
    fstream logFile;
    double objTemp ; // temporary variables to check objective convg. criteria

    if(MMA_VERBOSE)
    {
        logFile.open(" Optimization.log",ios::out);
        logFile<<" Iteration \t Obj \t Constraint"<<endl;
    }

    double kktnorm = kkttol + 10;
    int outIt = 0;
    while (kktnorm>kkttol && outIt<maxOutIt && (!isConverged)){
        outIt   = outIt+1;
        outerIter = outerIter+1;

        asymp(sens,constrSens);
        // The GCMMA subproblem is solved at the point xval:
        //    gcmmasub(f0val,df0dx,fval,dfdx);

        gcmmasub(obj,sens,constraint,constrSens);
        objectiveNew = compObj(x)/objectiveNormalizer;     //f0val
        compConstr(x,constraintNew);//fval
        int conserv = concheck(objectiveNew,constraintNew);
        innerIt=0;
        if (conserv == 0){
            while (conserv == 0 && innerIt <= 5){
                innerIt = innerIt+1;
                // New values on the parameters raa0 and raa are calculated:
                raaupdate(objectiveNew,constraintNew);
                // The GCMMA subproblem is solved with these new raa0 and raa:
                gcmmasub(obj,sens,constraint,constrSens);
                objectiveNew = compObj(x)/objectiveNormalizer;     //f0val
                compConstr(x,constraintNew);//fval
                conserv = concheck(objectiveNew,constraintNew);
            }
        }
        TERMINATE_OPT = modifyDesignVar(x); // Aaditya : With great power comes great responsibility. Use this func with care
        if(TERMINATE_OPT)
        {
            cout<<" Interrupt signal in optimizer ..."<<endl;
            return ;
        }

        // checking for convergence
        maxChange = -HUGE_VAL;
        for(int i =0 ;i < n ; i++)
        {
            if (xval[i] < 1e-5) continue;
            maxChange = max(fabs((x[i]-xval[i])/xval[i]),maxChange);
        }

        for(int i =0 ;i < n ; i++)
        {
            xold2[i] = xold1[i];
            xold1[i] = xval[i];
            xval[i] = x[i];
        }

        if ((maxChange < changeTol) && (outIt > 2)){
            cout<<" maxChange <  changeTol"<<endl;
            return;
        }

        objTemp = compObj(xval)/objectiveNormalizer;     //f0val
        if(fabs( (obj - objTemp)/obj ) <= relativeFuncTol)  {isConverged = true;}
        obj = objTemp;
        compObjSensi(xval,sens);//df0dx
        compConstr(xval,constraint);//fval
        compConstrSensi(xval,constrSens);//dfdx
        for(int i = 0;i<n;i++)
            sens[i] = sens[i]/objectiveNormalizer;

        kktnorm = kktcheck(constraint,sens,constrSens);
        if (MMA_VERBOSE){
            logFile<<outerIter<<"\t"<<obj*objectiveNormalizer<<"\t";
            for(int ms = 0 ; ms < m ; ms++)
                logFile<<constraint[ms]<<"\t";
            logFile<<endl;
            cout<<"\nOuterIter = "<<outerIter<<" innerIter = "<<innerIt<<" Objective = "<<obj*objectiveNormalizer<<" ("<<obj<<" %) constraint : ";
            for(int ms = 0;ms<m;ms++)
                cout<<constraint[ms]<<", ";
            cout << endl;
        }
    }
    logFile.close();
}
/************************************/

