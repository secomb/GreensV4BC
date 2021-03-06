/*****************************************************
Calculate p = p(c) where p is the inverse function of f.
c = f(p) is the hill equation  of the oxygen dissociation curve,
c (input) oxygen concentration in red blood cell
p (output) partial oxygen pressure
pp (output) pp = f'(p) at the computed p
Version 3.0, May 17, 2011.
Revised TWS January 2012 to use consistent f(p) for all cases
******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "nrutil.h"

float rtflsp(float(*func)(float), float x1, float x2, float xacc);
float rtbis(float(*func)(float), float x1, float x2, float xacc);
float func(float x);
float bloodconc(float p, float h);

void blood(float c, float h, float *p, float *pp)
{
	extern float fn, alphab, p50, cs, cext, hext;
	extern float plow, phigh, clowfac, chighfac, pphighfac;
	float pl, ph;
	float clow, chigh, pphigh;

	if (h < 1.e-6) {
		*p = c / alphab;
		*pp = alphab;
		return;
	}
	if (c < 0.) {	//changed for better behavior in severe hypoxia
		*p = c / alphab;
		*pp = alphab;
		return;
	}
	clow = clowfac * h + alphab * plow;
	if (c < clow) {
		*p = c * plow / clow;
		*pp = clow / plow;
		return;
	}
	chigh = chighfac * h + alphab * phigh;
	if (c < chigh) {
		if (c / h / cs < 1.) {
			ph = pow((c / h / cs) / (1.0 - c / h / cs), 1. / fn)*p50;
			pl = 0.;	//0. here to be sure to bracket the root
		}
		else {
			ph = phigh;
			pl = plow;
		}
		hext = h;
		cext = c;
		*p = rtflsp(func, pl, ph, 0.001f);
		if (*p < 0.) *p = rtbis(func, pl, ph, 0.001f);	//if false position algorithm fails, use bisection!  June 2009.
		*pp = cs * h*fn / p50 * pow(*p / p50, (fn - 1)) / SQR(1. + pow(*p / p50, fn)) + alphab;
		return;
	}
	pphigh = pphighfac * h + alphab;
	*p = phigh + (c - chigh) / pphigh;
	*pp = pphigh;
	return;
}
//Using the false position method, find the root of a function func known to lie between x1 and x2.
//The root, returned as rtflsp, is refined until its accuracy is +- xacc.

float rtflsp(float(*func)(float), float x1, float x2, float xacc)
{
	int j;
	int maxit = 30;
	float fl, fh, xl, xh, swap, dx, del, f, rtf;
	fl = (*func)(x1);
	fh = (*func)(x2);
	if (fl*fh > 0.0) {
		printf("\n*** Warning: Root must be bracketed in rtflsp %f %f\n", x1, x2);
		return -1.0;
	}
	if (fl < 0.0) {
		xl = x1;
		xh = x2;
	}
	else {
		xl = x2;
		xh = x1;
		swap = fl;
		fl = fh;
		fh = swap;
	}
	dx = xh - xl;
	for (j = 1; j <= maxit; j++) {
		rtf = xl + dx * fl / (fl - fh);
		f = (*func)(rtf);
		if (f < 0) {
			del = xl - rtf;
			xl = rtf;
			fl = f;
		}
		else {
			del = xh - rtf;
			xh = rtf;
			fh = f;
		}
		dx = xh - xl;
		if (fabs(del) < xacc || f == 0.0) return rtf;
	}
	//	printf("\n*** Warning: Maximum number of iterations exceeded in rtflsp %f %f\n", x1,x2);
	return -1.0;
}

//bisection method, if false position method fails

float rtbis(float(*func)(float), float x1, float x2, float xacc)
{
	int j;
	int maxit = 30;
	float dx, f, fmid, xmid, rtb;
	f = (*func)(x1);
	fmid = (*func)(x2);
	if (f*fmid >= 0.0) {
		printf("*** Error: Root must be bracketed in rtbis %f %f\n", x1, x2);
		return 0.0;
	}
	rtb = f < 0.0 ? (dx = x2 - x1, x1) : (dx = x1 - x2, x2);
	for (j = 1; j <= maxit; j++) {
		fmid = (*func)(xmid = rtb + (dx *= 0.5));
		if (fmid <= 0.) rtb = xmid;
		if (fabs(dx) < xacc || fmid == 0.0) return rtb;
	}
	printf("*** Error: Maximum number of iterations exceeded in rtbis %f %f\n", x1, x2);
	return 0.0;
}

float func(float p)
{
	extern float cext, hext;
	float y;
	y = bloodconc(p, hext) - cext;
	return y;
}

float bloodconc(float p, float h)
{
	extern float fn, p50, alphab, cs;
	extern float plow, phigh, clowfac, chighfac, pphighfac;//added January 2012
	float cv;
	if (p < 0.) cv = alphab * p;  //This gives more consistent behavior when p<0
	else if (p < plow) cv = clowfac * h*p / plow + alphab * p;
	else if (p < phigh) cv = cs * h*(1.0 - 1.0 / (1.0 + pow((p / p50), fn))) + alphab * p;
	else cv = (chighfac + (p - phigh)*pphighfac)*h + alphab * p;
	return cv;
}
float bloodconcp(float p, float h)
{
	extern float fn, p50, alphab, cs;
	extern float plow, phigh, clowfac, chighfac, pphighfac;//added January 2012
	float cvp;
	if (p < 0.) cvp = alphab;  //This gives more consistent behavior when p<0
	else if (p < plow) cvp = clowfac * h / plow + alphab;
	else if (p < phigh) cvp = cs * h*fn / p50 * pow(p / p50, (fn - 1)) / SQR(1. + pow(p / p50, fn)) + alphab;
	else cvp = pphighfac * h + alphab;
	return cvp;
}