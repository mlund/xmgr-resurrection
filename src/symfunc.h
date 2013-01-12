#ifndef SYMFUNC_H
#define SYMFUNC_H
typedef void symfunc(int,int,double,int);

/*
 * devsym* are all optional functions that a driver is free to provide
 * or not.  They are initialized to the be the corresponding sym* functions.
 * The corresponding *initgraphics routine should initialize them as
 * appropriates.  They will automatically be reset.
 */
extern symfunc *devsymcircle;
extern symfunc *devsymsquare;
extern symfunc *devsymdiamond;
extern symfunc *devsymtriangle1;
extern symfunc *devsymtriangle2;
extern symfunc *devsymtriangle3;
extern symfunc *devsymtriangle4;
extern symfunc *devsymplus;
extern symfunc *devsymx;
extern symfunc *devsymsplat;

/*
 * Function for resetting devsym* functions to their defaults.
 */
void devsymreset(void);

#endif
