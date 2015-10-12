/* $Id: fit.c,v 1.1 1995/04/13 16:25:49 pturner Exp pturner $
 *
 * curve fitting, and other numerical routines used in compose.
 *
 * Contents:
 *
 * void gauss() - simple gauss elimination for least squares poly fit
 * void stasum() - compute mean and variance
 * double leasev() - evaluate least squares polynomial
 * void fitcurve() - compute coefficients for a polynomial fit of degree >1
 * void runavg() - compute a running average
 * void runstddev() - compute a running standard deviation
 * void runmedian() - compute a running median
 * void runminmax() - compute a running minimum or maximum
 * void filterser() - apply a digital filter
 * void linearconv() - convolve one set with another
 * int crosscorr() - cross/auto correlation
 * int linear_regression() - linear regression
 * void spline() - compute a spline fit
 * double seval() - evaluate the spline computed in spline()
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>

#include "defines.h"
#include "protos.h"

static char buf[256];

int dofitcurve(int cnt, double *xd, double *yd, int nd, double *c);

/*
	simple gauss elimination - no pivoting or scaling strategies
	all matrices are sym-pos-def
*/
void gauss(int n, double *a, int adim, double *b, double *x)
{

    int i, k, j;
    double mult;

    for (k = 0; k < n - 1; k++) {
	for (i = k + 1; i < n; i++) {
	    mult = a[adim * i + k] / a[adim * k + k];
	    for (j = k + 1; j < n; j++) {
		a[adim * i + j] = a[adim * i + j] - mult * a[adim * k + j];
	    }
	    b[i] = b[i] - mult * b[k];
	}
    }
    for (i = n - 1; i >= 0; i--) {
	x[i] = b[i];
	for (j = i + 1; j < n; j++)
	    x[i] = x[i] - a[adim * i + j] * x[j];
	x[i] = x[i] / a[adim * i + i];
    }
}

/*
	compute mean and standard dev
*/
void stasum(double *x, int n, double *xbar, double *sd, int flag)
{
    int i;

    *xbar = 0;
    *sd = 0;
    if (n < 1)
	return;
    for (i = 0; i < n; i++)
	*xbar = (*xbar) + x[i];
    *xbar = (*xbar) / n;
    for (i = 0; i < n; i++)
	*sd = (*sd) + (x[i] - *xbar) * (x[i] - *xbar);
    if (n - flag)
	*sd = sqrt(*sd / (n - flag));
    else {
	errmsg("compmean: (n-flag)==0");
	*sd = 0;
    }
}


/*
	evaluate least squares polynomial
*/
double leasev(double *c, int degree, double x)
{
    double temp;
    int i;

    temp = 0.0;
    for (i = 0; i <= degree; i++) {
	if ((i == 0) && (x == 0.0)) {
	    temp = temp + c[i];	/* avoid 0.0^0 */
	} else {
	    temp = temp + c[i] * pow(x, (double) (i));
	}
    }
    return (temp);
}

/*
	curve fitting
*/
int fitcurve(double *x, double *y, int n, int ideg, double *fitted)
{
    int i, ifail;
    double result[20], leasev(double *c, int degree, double x);

    ifail = 1;
    if (ideg > 1) {
	dofitcurve(n, x, y, ideg, result);
	for (i = 0; i < n; i++) {
	    fitted[i] = leasev(result, ideg, x[i]);
	    if (!isfinite(fitted[i])) {
	        errmsg("Linear_regression - all values of x or y are the same");
		ifail = 3;
		return ifail;
	    }
	}
	ifail = 0;
    } else {
	ifail = linear_regression(n, x, y, fitted);
	if (ifail == 1) {
	    errmsg("Linear_regression entered with N < 2");
	} else if (ifail == 2) {
	    errmsg("Linear_regression - all values of x or y are the same");
	}
    }
    return ifail;
}

/*
	compute a running average
*/
void runavg(double *x, double *y, double *ax, double *ay, int n, int ilen)
{
    int i;
    double sumy = 0.0;
    double sumx = 0.0;

    for (i = 0; i < ilen; i++) {
	sumx = sumx + x[i];
	sumy = sumy + y[i];
    }
    ax[0] = sumx / ilen;
    ay[0] = sumy / ilen;
    for (i = 1; i < (n - ilen + 1); i++) {
	sumx = x[i + ilen - 1] - x[i - 1] + sumx;
	ax[i] = sumx / ilen;
	sumy = y[i + ilen - 1] - y[i - 1] + sumy;
	ay[i] = sumy / ilen;
    }
}

/*
	compute a running standard deviation
*/
void runstddev(double *x, double *y, double *ax, double *ay, int n, int ilen)
{
    int i;
    double ybar, ysd;
    double sumx = 0.0;

    for (i = 0; i < ilen; i++) {
	sumx = sumx + x[i];
    }
    ax[0] = sumx / ilen;
    stasum(y, ilen, &ybar, &ysd, 0);
    ay[0] = ysd;
    for (i = 1; i < (n - ilen + 1); i++) {
	stasum(y + i, ilen, &ybar, &ysd, 0);
	sumx = x[i + ilen - 1] - x[i - 1] + sumx;
	ax[i] = sumx / ilen;
	ay[i] = ysd;
    }
}

/*
	compute a running median
*/
void runmedian(double *x, double *y, double *ax, double *ay, int n, int ilen)
{
    int i, j, nlen = n - ilen + 1;
    double *tmpx, *tmpy;

    tmpx = (double *) calloc(ilen, sizeof(double));
    if (tmpx == NULL) {
	errmsg("Can't calloc tmpx in runmedian");
	return;
    }
    tmpy = (double *) calloc(ilen, sizeof(double));
    if (tmpy == NULL) {
	errmsg("Can't calloc tmpy in runmedian");
	cxfree(tmpx);
	return;
    }
    for (i = 0; i < nlen; i++) {
	for (j = 0; j < ilen; j++) {
	    tmpx[j] = x[j + i];
	    tmpy[j] = y[j + i];
	}
	sort_xy(tmpx, tmpy, ilen, 1, 0);

	if (ilen % 2) {
	    ax[i] = x[i + (ilen / 2)];
	    ay[i] = tmpy[ilen / 2];
	} else {
	    ax[i] = (x[i + ilen / 2] + x[i + (ilen - 1) / 2]) * 0.5;
	    ay[i] = (tmpy[ilen / 2] + tmpy[(ilen - 1) / 2]) * 0.5;
	}
    }
    cxfree(tmpx);
    cxfree(tmpy);
}

/*
	compute a running minimum or maximum
*/
void runminmax(double *x, double *y, double *ax, double *ay, int n, int ilen, int type)
{
    int i, j;
    double min, max;
    double sumx = 0.0;

    min = max = y[0];
    for (i = 0; i < ilen; i++) {
	sumx = sumx + x[i];
	if (min > y[i])
	    min = y[i];
	if (max < y[i])
	    max = y[i];
    }
    ax[0] = sumx / ilen;
    if (type == 0) {
	ay[0] = min;
    } else if (type == 1) {
	ay[0] = max;
    } else {
	errmsg("Unknown type in runminmax, setting type = min");
	type = 0;
    }
    for (i = 1; i < (n - ilen + 1); i++) {
	sumx = x[i + ilen - 1] - x[i - 1] + sumx;
	ax[i] = sumx / ilen;
	min = y[i];
	max = y[i];
	for (j = 0; j < ilen; j++) {
	    if (min > y[i + j])
		min = y[i + j];
	    if (max < y[i + j])
		max = y[i + j];
	}
	if (type == 0) {
	    ay[i] = min;
	} else if (type == 1) {
	    ay[i] = max;
	}
    }
}

/*
	Apply a digital filter of length len to a set in x, y,
	of length n with the results going to resx, resy.
	the length of the result is set by the caller
*/
void filterser(int n, double *x, double *y, double *resx, double *resy, double *h, int len)
{
    int i, j, outlen, eo, ld2;
    double sum;

    outlen = n - len + 1;
    eo = len % 2;
    ld2 = len / 2;
    for (i = 0; i < outlen; i++) {
	sum = 0.0;
	for (j = 0; j < len; j++) {
	    sum = sum + h[j] * y[j + i];
	}
	resy[i] = sum;
	if (eo)
	    resx[i] = x[i + ld2];
	else
	    resx[i] = (x[i + ld2] + x[i + ld2 - 1]) / 2.0;
    }
}

/*
	linear convolution of set x (length n) with h (length m) and
	result to y. the length of y is set by the caller
*/
void linearconv(double *x, double *h, double *y, int n, int m)
{
    int i, j, itmp;

    for (i = 0; i < n + m - 1; i++) {
	for (j = 0; j < m; j++) {
	    itmp = i - j;
	    if ((itmp >= 0) && (itmp < n)) {
		y[i] = y[i] + h[j] * x[itmp];
	    }
	}
    }
}

/*
 * cross correlation
 */
int crosscorr(double *x, double *y, int n, int maxlag, double *xcov, double *xcor)
{
    double xbar, xsd;
    double ybar, ysd;
    int i, j;

    if (maxlag + 2 > n)
	return 1;
    for (i = 0; i <= maxlag; i++) {
        stasum(&x[i], n - i, &xbar, &xsd, 0);
        if (xsd == 0.0)
	    return 2;
        stasum(y, n - i, &ybar, &ysd, 0);
        if (ysd == 0.0)
	    return 3;
	xcov[i] = 0.0;
	for (j = 0; j < n - i; j++) {
	    xcov[i] += (y[j] - ybar) * (x[j + i] - xbar);
	}
	xcov[i] /= n - i;
	xcor[i] = xcov[i] / (xsd * ysd);
    }
    return 0;
}

/*
	References,

	_Aplied Linear Regression_, Weisberg
	_Elements of Statistical Computing_, Thisted

	Fits y = coef*x + intercept + e

	uses a 2 pass method for means and variances

*/

int linear_regression(int n, double *x, double *y, double *fitted)
{
    double xbar, ybar;		/* sample means */
    double sdx, sdy;		/* sample standard deviations */
    double sxy, rxy;		/* sample covariance and sample correlation */
    double SXX, SYY, SXY;	/* sums of squares */
    double RSS;			/* residual sum of squares */
    double rms;			/* residual mean square */
    double sereg;		/* standard error of regression */
    double seslope, seintercept;
    double slope, intercept;	/* */
    double SSreg, F, R2;
    int i;

    if (n < 2) {
	return 1;
    } 
    xbar = ybar = 0.0;
    SXX = SYY = SXY = 0.0;
    for (i = 0; i < n; i++) {
	xbar = xbar + x[i];
	ybar = ybar + y[i];
    }
    xbar = xbar / n;
    ybar = ybar / n;
    for (i = 0; i < n; i++) {
	SXX = SXX + (x[i] - xbar) * (x[i] - xbar);
	SYY = SYY + (y[i] - ybar) * (y[i] - ybar);
	SXY = SXY + (x[i] - xbar) * (y[i] - ybar);
    }
    sdx = sqrt(SXX / (n - 1));
    sdy = sqrt(SYY / (n - 1));
    if (sdx == 0.0) {
	return 2;
    }
    if (sdy == 0.0) {
	return 2;
    }
    sxy = SXY / (n - 1);
    rxy = sxy / (sdx * sdy);
    slope = SXY / SXX;
    intercept = ybar - slope * xbar;
       
    RSS = SYY - slope * SXY;

    sprintf(buf, "Number of observations\t\t\t = %d\n", n);
    stufftext(buf, 0);
    sprintf(buf, "Mean of independent variable\t\t = %.7g\n", xbar);
    stufftext(buf, 0);
    sprintf(buf, "Mean of dependent variable\t\t = %.7g\n", ybar);
    stufftext(buf, 0);
    sprintf(buf, "Standard dev. of ind. variable\t\t = %.7g\n", sdx);
    stufftext(buf, 0);
    sprintf(buf, "Standard dev. of dep. variable\t\t = %.7g\n", sdy);
    stufftext(buf, 0);
    sprintf(buf, "Correlation coefficient\t\t\t = %.7g\n", rxy);
    stufftext(buf, 0);
    sprintf(buf, "Regression coefficient (SLOPE)\t\t = %.7g\n", slope);
    stufftext(buf, 0);

    if (n == 2) {
        fitted[0] = y[0];
        fitted[1] = y[1];
        return 0;
    } 
    
    rms = RSS / (n - 2);
    sereg = sqrt(rms);
    seintercept = sqrt(rms * (1.0 / n + xbar * xbar / SXX));
    seslope = sqrt(rms / SXX);
    SSreg = SYY - RSS;
    F = SSreg / rms;
    R2 = SSreg / SYY;

    sprintf(buf, "Standard error of coefficient\t\t = %.7g\n", seslope);
    stufftext(buf, 0);
    sprintf(buf, "t - value for coefficient\t\t = %.7g\n", slope / seslope);
    stufftext(buf, 0);
    sprintf(buf, "Regression constant (INTERCEPT)\t\t = %.7g\n", intercept);
    stufftext(buf, 0);
    sprintf(buf, "Standard error of constant\t\t = %.7g\n", seintercept);
    stufftext(buf, 0);
    sprintf(buf, "t - value for constant\t\t\t = %.7g\n", intercept / seintercept);
    stufftext(buf, 0);
    sprintf(buf, "\nAnalysis of variance\n");
    stufftext(buf, 0);
    sprintf(buf, "Source\t\t d.f\t Sum of squares\t Mean Square\t F\n");
    stufftext(buf, 0);
    sprintf(buf, "Regression\t   1\t%.7g\t%.7g\t%.7g\n", SSreg, SSreg, F);
    stufftext(buf, 0);
    sprintf(buf, "Residual\t%5d\t%.7g\t%.7g\n", n - 2, RSS, rms);
    stufftext(buf, 0);
    sprintf(buf, "Total\t\t%5d\t%.7g\n\n", n - 1, SYY);
/*
    stufftext(buf, 2);
*/
    for (i = 0; i < n; i++) {
	fitted[i] = slope * x[i] + intercept;
    }
    return 0;
}

/*
	a literal translation of the spline routine in
	Forsyth, Malcolm, and Moler
*/
void spline(int n, double *x, double *y, double *b, double *c, double *d)
{
/*
c
c  the coefficients b(i), c(i), and d(i), i=1,2,...,n are computed
c  for a cubic interpolating spline
c
c    s(x) = y(i) + b(i)*(x-x(i)) + c(i)*(x-x(i))**2 + d(i)*(x-x(i))**3
c
c    for  x(i) .le. x .le. x(i+1)
c
c  input..
c
c    n = the number of data points or knots (n.ge.2)
c    x = the abscissas of the knots in strictly increasing order
c    y = the ordinates of the knots
c
c  output..
c
c    b, c, d  = arrays of spline coefficients as defined above.
c
c  using  p  to denote differentiation,
c
c    y(i) = s(x(i))
c    b(i) = sp(x(i))
c    c(i) = spp(x(i))/2
c    d(i) = sppp(x(i))/6  (derivative from the right)
c
c  the accompanying function subprogram  seval	can be used
c  to evaluate the spline.
c
c
*/

    int nm1, ib, i;
    double t;

/*
Gack!
*/
    x--;
    y--;
    b--;
    c--;
    d--;

/*
Fortran 66
*/
    nm1 = n - 1;
    if (n < 2)
	return;
    if (n < 3)
	goto l50;
/*
c
c  set up tridiagonal system
c
c  b = diagonal, d = offdiagonal, c = right hand side.
c
*/
    d[1] = x[2] - x[1];
    c[2] = (y[2] - y[1]) / d[1];
    for (i = 2; i <= nm1; i++) {
	d[i] = x[i + 1] - x[i];
	b[i] = 2.0 * (d[i - 1] + d[i]);
	c[i + 1] = (y[i + 1] - y[i]) / d[i];
	c[i] = c[i + 1] - c[i];
    }
/*
c
c  end conditions.  third derivatives at  x(1)	and  x(n)
c  obtained from divided differences
c
*/
    b[1] = -d[1];
    b[n] = -d[n - 1];
    c[1] = 0.0;
    c[n] = 0.0;
    if (n == 3)
	goto l15;
    c[1] = c[3] / (x[4] - x[2]) - c[2] / (x[3] - x[1]);
    c[n] = c[n - 1] / (x[n] - x[n - 2]) - c[n - 2] / (x[n - 1] - x[n - 3]);
    c[1] = c[1] * d[1] * d[1] / (x[4] - x[1]);
    c[n] = -c[n] * d[n - 1] * d[n - 1] / (x[n] - x[n - 3]);
/*
c
c  forward elimination
c
*/
l15:;
    for (i = 2; i <= n; i++) {
	t = d[i - 1] / b[i - 1];
	b[i] = b[i] - t * d[i - 1];
	c[i] = c[i] - t * c[i - 1];
    }
/*
c
c  back substitution
c
*/
    c[n] = c[n] / b[n];
    for (ib = 1; ib <= nm1; ib++) {
	i = n - ib;
	c[i] = (c[i] - d[i] * c[i + 1]) / b[i];
    }
/*
c
c  c(i) is now the sigma(i) of the text
c
c  compute polynomial coefficients
c
*/
    b[n] = (y[n] - y[nm1]) / d[nm1] + d[nm1] * (c[nm1] + 2.0 * c[n]);
    for (i = 1; i <= nm1; i++) {
	b[i] = (y[i + 1] - y[i]) / d[i] - d[i] * (c[i + 1] + 2.0 * c[i]);
	d[i] = (c[i + 1] - c[i]) / d[i];
	c[i] = 3.0 * c[i];
    }
    c[n] = 3.0 * c[n];
    d[n] = d[n - 1];
    return;

l50:;
    b[1] = (y[2] - y[1]) / (x[2] - x[1]);
    c[1] = 0.0;
    d[1] = 0.0;
    b[2] = b[1];
    c[2] = 0.0;
    d[2] = 0.0;
    return;
}

/***************************************************************************
 * aspline - modified version of David Frey's spline.c                     *
 *                                                                         *    
 * aspline does an Akima spline interpolation.                             *
 ***************************************************************************/

void aspline(int n, double *x, double *y, double *b, double *c, double *d)
{
  int i;
 	
  double num, den;
  double m_m1, m_m2, m_p1, m_p2;
  double x_m1, x_m2, x_p1, x_p2;
  double y_m1, y_m2, y_p1, y_p2;

#define dx(i) (x[i+1]-x[i])
#define dy(i) (y[i+1]-y[i])
#define  m(i) (dy(i)/dx(i))

  if (n > 0)		     /* we have data to process */
  {

      /*
       * calculate the coefficients of the spline 
       * (the Akima interpolation itself)                      
       */

      /* b) interpolate the missing points: */

      x_m1 = x[0] + x[1] - x[2]; 
      y_m1 = (x[0]-x_m1) * (m(1) - 2 * m(0)) + y[0];

      m_m1 = (y[0]-y_m1)/(x[0]-x_m1);
       
      x_m2 = 2 * x[0] - x[2];
      y_m2 = (x_m1-x_m2) * (m(0) - 2 * m_m1) + y_m1;
       
      m_m2 = (y_m1-y_m2)/(x_m1-x_m2);

      x_p1 = x[n-1] + x[n-2] - x[n-3];
      y_p1 = (2 * m(n-2) - m(n-3)) * (x_p1 - x[n-1]) + y[n-1];

      m_p1 = (y_p1-y[n-1])/(x_p1-x[n-1]);
      
      x_p2 = 2 * x[n-1] - x[n-3];
      y_p2 = (2 * m_p1 - m(n-2)) * (x_p2 - x_p1) + y_p1;
      
      m_p2 = (y_p2-y_p1)/(x_p2-x_p1);
           
      /* i = 0 */
      num=fabs(m(1) - m(0)) * m_m1 + fabs(m_m1 - m_m2) * m(0);
      den=fabs(m(1) - m(0)) + fabs(m_m1 - m_m2);
    	
      if (den != 0.0) b[0]=num / den;
      else            b[0]=0.0;
		
      /* i = 1 */
      num=fabs(m(2) - m(1)) * m(0) + fabs(m(0) - m_m1) * m(1);
      den=fabs(m(2) - m(1)) + fabs(m(0) - m_m1);

      if (den != 0.0) b[1]=num / den;
      else            b[1]=0.0;
			
      for (i=2; i < n-2; i++)
      {

	num=fabs(m(i+1) - m(i)) * m(i-1) + fabs(m(i-1) - m(i-2)) * m(i);
	den=fabs(m(i+1) - m(i)) + fabs(m(i-1) - m(i-2));

	if (den != 0.0) b[i]=num / den;
	else            b[i]=0.0;
      }

      /* i = n - 2 */
      num=fabs(m_p1 - m(n-2)) * m(n-3) + fabs(m(n-3) - m(n-4)) * m(n-2);
      den=fabs(m_p1 - m(n-2)) + fabs(m(n-3) - m(n-4));

      if (den != 0.0) b[n-2]=num / den;
      else	      b[n-2]=0.0;
 
      /* i = n - 1 */
      num=fabs(m_p2 - m_p1) * m(n-2) + fabs(m(n-2) - m(n-3)) * m_p1;
      den=fabs(m_p2 - m_p1) + fabs(m(n-2) - m(n-3));

      if (den != 0.0) b[n-1]=num / den;
      else	      b[n-1]=0.0;
 
      for (i=0; i < n-1; i++)
      {
  	   double dxv = dx(i);
  	   c[i]=(3 * m(i) - 2 * b[i] - b[i+1]) / dxv;
	   d[i]=(b[i] + b[i+1] - 2 * m(i)) / (dxv * dxv);
      }
  }
#undef dx
#undef dy
#undef  m
}

double seval(int n, double u, double *x, double *y, double *b, double *c, double *d)
{
/*
c
c  this subroutine evaluates the cubic spline function
c
c    seval = y(i) + b(i)*(u-x(i)) + c(i)*(u-x(i))**2 + d(i)*(u-x(i))**3
c
c    where  x(i) .lt. u .lt. x(i+1), using horner's rule
c
c  if  u .lt. x(1) then  i = 1	is used.
c  if  u .ge. x(n) then  i = n	is used.
c
c  input..
c
c    n = the number of data points
c    u = the abscissa at which the spline is to be evaluated
c    x,y = the arrays of data abscissas and ordinates
c    b,c,d = arrays of spline coefficients computed by spline
c
c  if  u  is not in the same interval as the previous call, then a
c  binary search is performed to determine the proper interval.
c
*/
    int j, k;
    double dx;
    int i;

/*
c
c  binary search
c
*/
    if (u < x[0]) {
	i = 0;
    } else if (u >= x[n - 1]) {
	i = n - 1;
    } else {
	i = 0;
	j = n;
l20:	;
	k = (i + j) / 2;
	if (u < x[k])
	    j = k;
	if (u >= x[k])
	    i = k;
	if (j > i + 1)
	    goto l20;
    }
/*
c
c  evaluate spline
c
*/
    dx = u - x[i];
    return (y[i] + dx * (b[i] + dx * (c[i] + dx * d[i])));
}

