#include <config.h>

#include "draw.h"
#include "symfunc.h"

symfunc *devsymcircle=&symcircle;
symfunc *devsymsquare=&symsquare;
symfunc *devsymdiamond=&symdiamond;
symfunc *devsymtriangle1=&symtriangle1;
symfunc *devsymtriangle2=&symtriangle2;
symfunc *devsymtriangle3=&symtriangle3;
symfunc *devsymtriangle4=&symtriangle4;
symfunc *devsymplus=&symplus;
symfunc *devsymx=&symx;
symfunc *devsymsplat=&symsplat;

void devsymreset(void) {
  devsymcircle=&symcircle;
  devsymsquare=&symsquare;
  devsymdiamond=&symdiamond;
  devsymtriangle1=&symtriangle1;
  devsymtriangle2=&symtriangle2;
  devsymtriangle3=&symtriangle3;
  devsymtriangle4=&symtriangle4;
  devsymplus=&symplus;
  devsymx=&symx;
  devsymsplat=&symsplat;
}

