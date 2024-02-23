/* $Id: noxprotos.h,v 1.3 1995/05/26 05:08:01 pturner Exp pturner $
 *
 * Prototypes not involving X
 *
 */

#ifndef __NOXPROTOS_H_
#define __NOXPROTOS_H_

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include "defines.h"

void exec_cmd(char* s);
void errmsg(char* buf);
double my_hypot(double x, double y);
int checkon_ticks(int gno);
int checkon_world(int gno);
int checkon_viewport(int gno);

void set_lists_dirty(int dirtyflag);

void loadset(int gno, int selset, int toval, double startno, double stepno);
int formula(int gno, int selset, char* sscanstr);

void do_running_command(int type, int setno, int rlen);
void do_fourier_command(int ftype, int setno, int ltype);
void do_histo_command(int fromset, int toset, int tograph, double minb, double binw, int nbins);
int do_compute(int setno, int loadto, int graphto, char* fstr);
void do_load(int setno, int toval, char* startstr, char* stepstr);
void do_compute2(char* fstrx, char* fstry, char* startstr, char* stopstr, int npts, int toval);
double trapint(double* x, double* y, double* resx, double* resy, int n);
void do_digfilter(int set1, int set2);
void do_linearc(int set1, int set2);
void do_xcor(int set1, int set2, int lag);
void do_spline(int set, double start, double stop, int n, int type);
double do_int(int setno, int itype);
void do_differ(int setno, int itype);
void do_regress(int setno, int ideg, int iresid, int rno, int invr);
void do_runavg(int setno, int runlen, int runtype, int rno, int invr);
void do_fourier(int fftflag, int setno, int load, int loadx, int invflag, int type, int wind);
void do_window(int setno, int type, int wind);
void apply_window(double* xx, double* yy, int ilen, int type, int wind);
void do_histo(int fromset, int toset, int tograph, double binw, double xmin, double xmax, int hist_type);
void histogram(int fromset, int toset, int tograph, double bins, double xmin, double xmax, int hist_type);

void do_sample(int setno, int typeno, char* exprstr, int startno, int stepno);
void do_prune(int setno, int typeno, int deltatypeno, char* dxstr, char* dystr, int dxtype, int dytype);

void set_program_defaults(void);
void set_region_defaults(int i);
void set_default_message(char* buf);
void set_default_framep(framep* f);
void set_default_world(world* w);
void set_default_view(view* v);
void set_default_string(plotstr* s);
plotstr copy_plotstr(plotstr p);
void set_default_line(linetype* l);
void set_default_box(boxtype* b);
void set_default_legend(int gno, legend* l);
void set_default_plotarr(plotarr* p);
void set_default_velocityp(velocityp* vp);
void set_default_graph(int gno);
void realloc_plots(int maxplot);
void realloc_graph_plots(int gno, int maxplot);
void realloc_graphs(void);
void realloc_lines(int n);
void realloc_boxes(int n);
void realloc_strings(int n);
void alloc_blockdata(int ncols);
void set_default_annotation(void);
void set_default_ticks(tickmarks* t, int a);

double xconv(double x);
double yconv(double y);
void set_coordmap(int mapx, int mapy);
void setfixedscale(double xv1, double yv1, double xv2, double yv2, double* xg1, double* yg1, double* xg2, double* yg2);
void defineworld(double x1, double y1, double x2, double y2, int mapx, int mapy);
void viewport(double x1, double y1, double x2, double y2);
void setclipping(int fl);
int clipt(double d, double n, double* te, double* tl);
void my_draw2(double x2, double y2);
void my_move2(double x, double y);
int setpattern(int k);
void fillpattern(int n, double* px, double* py);
void fillcolor(int n, double* px, double* py);
void fillrectcolor(double x1, double y1, double x2, double y2);
void fillrectpat(double x1, double y1, double x2, double y2);
int setfont(int f);
void rect(double x1, double y1, double x2, double y2);
int symok(double x, double y);
int lengthpoly(double* x, double* y, int n);
void drawpoly(double* x, double* y, int n);
void drawpolyseg(double* x, double* y, int n);
void openclose(double x, double y1, double y2, double ebarlen, int xy);
void errorbar(double x, double y, double ebarlen, int xy);
int setcolor(int col);
int setlinestyle(int style);
int setlinewidth(int wid);
void drawtic(double x, double y, int dir, int axis);
double setcharsize(double size);
void setticksize(double sizex, double sizey);
void writestr(double x, double y, int dir, int just, char* s);
void drawtitle(char* title, int which);
void drawgrid(int dir, double start, double end, double y1, double y2, double step, int cy, int ly, int wy);
void drawpolysym(double* x, double* y, int len, int sym, int skip, int fill, double size);
void putlegend(int i,          /* which set */
               int d,          /* flag, 1 = no draw, just compute min/max
                                * for bounding box */
               int xlen,       /* length of legend */
               int ylen,       /* distance between entries */
               double size,    /* symbol size */
               double x,       /* location x */
               double y,       /* location y */
               int sy,         /* symbol */
               int ly,         /* line style */
               int cy,         /* line color */
               int wy,         /* line width */
               char* s,        /* legend string */
               int fill,       /* symbol fill */
               int sc,         /* symbol color */
               int sw,         /* symbol linewidth */
               int sl);        /* symbol linestyle */
void putbarlegend(int i,       /* which set */
                  int d,       /* flag, 1 = no draw, just compute min/max
                                * for bounding box */
                  int xlen,    /* length of legend */
                  int ylen,    /* distance between entries */
                  double size, /* symbol size */
                  double x,    /* location x */
                  double y,    /* location y */
                  int sy,      /* symbol */
                  int ly,      /* line style */
                  int cy,      /* line color */
                  int wy,      /* line width */
                  char* s,     /* legend string */
                  int fill,    /* symbol fill */
                  int fu,      /* fill using pattern or color */
                  int fc,      /* fill color */
                  int fp);     /* fill pattern */
void my_doublebuffer(int mode);
void my_frontbuffer(int mode);
void my_backbuffer(int mode);
void my_swapbuffer(void);
int initgraphics(int device);
void leavegraphics(void);

void drawaxes(int gno);
void drawxaxisbar(int gno, int caxis);
void drawyaxisbar(int gno, int caxis);
void create_ticklabel(int form, int prec, double loc, char* s);
void drawxticklabels(int gno, int caxis);
void drawyticklabels(int gno, int caxis);
void drawxtickmarks(int gno, int caxis);
void drawytickmarks(int gno, int caxis);
int check_nticks(int gno, int axis, double gmin, double gmax, double tm, int maxnt);

int getbinary(int gno, char* fname, int embed);
int putbinary(int gno, char* fname, int embed);

int getdata(int gno, char* fn, int src, int type);
int readxy(int gno, char* fn, FILE* fp, int readone);
int readihl(int gno, char* fn, FILE* fp);
int readnxy(int gno, char* fn, FILE* fp);
int readbinary(int gno, char* fn, FILE* fp);
int readxystring(int gno, char* fn, FILE* fp, int readone);
int readxxyy(int gno, char* fn, FILE* fp, int type);
int read_set_fromfile(int gno, int setno, char* fn, int src);
int readnetcdf(int gno, int setno, char* netcdfname, char* xvar, char* yvar, int start, int stop, int stride);
int write_netcdf(int gno, int setno, char* fname);
int readblockdata(int gno, char* fn, FILE* fp);
void create_set_fromblock(int gno, int type, char* cols);

void gauss(int n, double* a, int adim, double* b, double* x);
void stasum(double* x, int n, double* xbar, double* sd, int flag);
double leasev(double* c, int degree, double x);
int fitcurve(double* x, double* y, int n, int ideg, double* fitted);
void runavg(double* x, double* y, double* ax, double* ay, int n, int ilen);
void runstddev(double* x, double* y, double* ax, double* ay, int n, int ilen);
void runmedian(double* x, double* y, double* ax, double* ay, int n, int ilen);
void runminmax(double* x, double* y, double* ax, double* ay, int n, int ilen, int type);
void filterser(int n, double* x, double* y, double* resx, double* resy, double* h, int len);
void linearconv(double* x, double* h, double* y, int n, int m);
int crosscorr(double* x, double* y, int n, int lag, double* xcov, double* xcor);
int transfit(int type, int n, double* x, double* y, double* fitted);
int linear_regression(int n, double* x, double* y, double* fitted);
void spline(int n, double* x, double* y, double* b, double* c, double* d);
void aspline(int n, double* x, double* y, double* b, double* c, double* d);
double seval(int n, double u, double* x, double* y, double* b, double* c, double* d);
double aseval(int n, double u, double* x, double* y, double* b, double* c, double* d);

void dft(double* jr, double* ji, int n, int iflag);
void fft(double* real_data, double* imag_data, int n_pts, int nu, int inv);

int getparms(char* plfile);
int read_param(char* pbuf);

void putparms(int gno, FILE* pp, int embed);
void put_fitparms(FILE* pp, int embed);

int iscontained(int gno, double wx, double wy);
int nextcontained(int gno, double wx, double wy);
int islogx(int gno);
int islogy(int gno);
char* graph_types(int it, int which);
int get_format_index(int f);
int get_pagelayout(void);
char* get_format_types(int f);
void kill_graph(int gno);
void copy_graph(int from, int to);
void copy_graph_sets_only(int from, int to);
void swap_graph(int from, int to);
void do_flipxy(void);
void flipxy(int gno);
void do_invertx(void);
void do_inverty(void);
void invertx(int gno);
void inverty(int gno);
void get_graph_box(int i, boxtype* b);
void get_graph_line(int i, linetype* l);
void get_graph_string(int i, plotstr* s);
void get_graph_framep(int gno, framep* f);
void get_graph_world(int gno, world* w);
void get_graph_view(int gno, view* v);
void get_graph_labels(int gno, labels* labs);
void get_graph_plotarr(int gno, int i, plotarr* p);
void get_graph_tickmarks(int gno, tickmarks* t, int a);
void get_graph_legend(int gno, legend* leg);
void set_graph_box(int i, boxtype* b);
void set_graph_line(int i, linetype* l);
void set_graph_string(int i, plotstr* s);
void set_graph_active(int gno);
void set_graph_framep(int gno, framep* f);
void set_graph_view(int gno, view* v);
void set_graph_labels(int gno, labels* labs);
void set_graph_plotarr(int gno, int i, plotarr* p);
void set_graph_tickmarks(int gno, tickmarks* t, int a);
void set_graph_legend(int gno, legend* leg);
void defaultgraph(int gno);
void defaultx(int gno, int setno);
void defaulty(int gno, int setno);
void defaultsetgraph(int gno, int setno);
void default_axis(int gno, int method, int axis);
void newworld(int gno, int lz, int axes, double wx1, double wy1, double wx2, double wy2);
void autoscale_graph(int gno, int axis);
void do_autoscale_set(int gno, int setno);
void autoscale_set(int gno, int setno, int axis);
int wipeout(int ask);
void update_all(int gno);

void arrange_graphs(int grows, int gcols);
void gwindleft_proc(void);
void gwindright_proc(void);
void gwinddown_proc(void);
void gwindup_proc(void);
void gwindshrink_proc(void);
void gwindexpand_proc(void);
void scroll_proc(int value);
void scrollinout_proc(int value);
void push_and_zoom(void);
void cycle_world_stack(void);
void clear_world_stack(void);
void show_world_stack(int n);
void add_world(int gno, double x1, double x2, double y1, double y2, double t1, double t2, double u1, double u2);
void push_world(void);
void pop_world(void);
void make_format(int gno);
void arrange_graphs2(int grows, int gcols, double vgap, double hgap, double sx, double sy, double wx, double wy,
                     int applyto);
void define_autos(int aon, int au, int ap, int ameth, int antx, int anty);
void define_arrange(int nrows, int ncols, int pack, double vgap, double hgap, double sx, double sy, double wx,
                    double wy);

void putstrhp(char* s);
int hpsetmode(int mode);
void drawhp(int x2, int y2, int mode);
int xconvhp(double x);
int yconvhp(double y);
void hpsetfont(int n);
int hpsetcolor(int c);
int hpsetlinewidth(int c);
void hpdrawtic(int x, int y, int dir, int updown);
int hpsetlinestyle(int style);
void dispstrhp(int x, int y, int rot, char* s, int just, int fudge);
int hpsetpat(int pat);
int setpathp(int pat);
void hpfill(int n, int* px, int* py);
void hpfillcolor(int n, int* px, int* py);
void hpleavegraphics(void);
void hpdrawarc(int x, int y, int r);
void hpfillarc(int x, int y, int r);
void hpdrawellipse(int x, int y, int xm, int ym);
void hpfillellipse(int x, int y, int xm, int ym);
int hpinitgraphics(int dmode);

int isdir(char* f);

void otherdefs(void);
int leafsetmode(int mode);
void drawleaf(int x2, int y2, int mode);
int xconvleaf(double x);
int yconvleaf(double y);
int leafsetcolor(int c);
int leafsetlinewidth(int c);
void leafdrawtic(int x, int y, int dir, int updown);
int leafsetlinestyle(int style);
void leafsetfont(int n);
void leafsetfontsize(double size);
int leafsetpat(int k);
void leaffill(int n, int* px, int* py);
void leaffillcolor(int n, int* px, int* py);
void leafdrawarc(int x, int y, int r);
void leaffillarc(int x, int y, int r);
void leafdrawellipse(int x, int y, int xm, int ym);
void leaffillellipse(int x, int y, int xm, int ym);
void leafleavegraphics(void);
int leafinitgraphics(int dmode);

int is_state_save(char* fname);

int mifsetmode(int mode);
void drawmif(int x2, int y2, int mode);
int xconvmif(double x);
int yconvmif(double y);
int mifsetcolor(int c);
int mifsetlinewidth(int c);
void mifdrawtic(int x, int y, int dir, int updown);
int mifsetlinestyle(int style);
void mifsetfont(int n);
void mifsetfontsize(double size);
void dispstrmif(int x, int y, int rot, char* s, int just, int fudge);
void putmif(char* s);
int mifsetpat(int k);
void miffill(int n, int* px, int* py);
void miffillcolor(int n, int* px, int* py);
void mifdrawarc(int x, int y, int r);
void miffillarc(int x, int y, int r);
void mifleavegraphics(void);
int mifinitgraphics(int dmode);

void find_item(int gno, double x, double y, int* type, int* numb);
int isactive_line(int lineno);
int isactive_box(int boxno);
int isactive_string(int strno);
int next_line(void);
int next_box(void);
int next_ellipse(void);
int next_string(void);
void copy_object(int type, int from, int to);
void kill_box(int boxno);
void kill_line(int lineno);
void kill_string(int stringno);
void do_boxes_proc(void);
void do_lines_proc(void);
void do_move_proc(void);
void do_delete_object_proc(void);
void do_copy_object_proc(void);
void do_cut_object_proc(void);
void edit_objects_proc(void);
int define_string(char* s, double wx, double wy);
void strings_loc_proc(void);
void strings_ang_proc(void);
void strings_edit_proc(void);
void do_clear_lines(void);
void do_clear_boxes(void);
void do_clear_text(void);
void do_clear_ellipses(void);

void lowtoupper(char* s);

void scanner(char* s, double* x, double* y, int len, double* a, double* b, double* c, double* d, int lenscr, int i,
             int setno, int* errpos);
double rnorm(double mean, double sdev);
double fx(double x);
void init_symtab(void);
int addto_symtab(symtab_entry newkey);

void draw_polar_graph(int gno);
void plotone(int gno);
void draw_ref_point(int gno);
void draw_annotation(int gno);
void dolegend(int gno);
void boxplot(int gno);
void draw_string(int gno, int i);
void draw_box(int gno, int i);
void draw_ellipse(int gno, int i);
void draw_line(int gno, int i);
void drawsetfill(int gno, plotarr* p);
void drawsetxy(int gno, plotarr* p, int i);
void drawsethilo(plotarr* p);
void drawval(plotarr* p);
void drawdensity(plotarr* p);
void drawboxcolor(plotarr* p);
void drawcirclexy(plotarr* p);
void drawsetbar(int gno, int setno, double cset, double bsize);
void drawsethbar(int gno, int setno, double cset, double bsize);
void drawsetstackedbar(int gno, int maxn, double bsize);
void drawsetstackedhbar(int gno, int maxn, double bsize);
void drawseterrbar(int gno, int setno, double offsx, double offsy);
void drawflow(int gno, int setno, velocityp vp);

int pssetmode(int mode);
void drawps(int x2, int y2, int mode);
int xconvps(double x);
int yconvps(double y);
int pssetcolor(int c);
int pssetlinewidth(int c);
void psdrawtic(int x, int y, int dir, int updown);
int pssetlinestyle(int style);
void pssetfont(int n);
void pssetfontsize(double size);
void dispstrps(int x, int y, int rot, char* s, int just, int fudge);
int pssetpat(int k);
void psfill(int n, int* px, int* py);
void psfillcolor(int n, int* px, int* py);
void psdrawarc(int x, int y, int r);
void psfillarc(int x, int y, int r);
void psdrawellipse(int x, int y, int xm, int ym);
void psfillellipse(int x, int y, int xm, int ym);
void psfillellipsecolour(int x, int y, int xm, int ym);
void psleavegraphics(void);
int psinitgraphics(int dmode);

int inbounds(int gno, double x, double y);
int isactive_region(int regno);
char* region_types(int it, int which);
void kill_region(int r);
void activate_region(int r, int type);
void define_region(int nr, int regionlinkto, int rtype);
void extract_region(int gno, int fromset, int toset, int regno);
void delete_region(int gno, int setno, int regno);
void evaluate_region(int regno, int gno, int setno, char* buf);
void load_poly_region(int r, int n, double* x, double* y);
void draw_region(int r);
int intersect_to_left(double x, double y, double x1, double y1, double x2, double y2);
int inbound(double x, double y, double* xlist, double* ylist, int n);
int isleft(double x, double y, double x1, double y1, double x2, double y2);
int isright(double x, double y, double x1, double y1, double x2, double y2);
int isabove(double x, double y, double x1, double y1, double x2, double y2);
int isbelow(double x, double y, double x1, double y1, double x2, double y2);
int inregion(int regno, double x, double y);

char* set_types(int it);
void setdefaultcolors(int gno);
void allocxy(plotarr* p, int len);
int init_array(double** a, int n);
int init_scratch_arrays(int n);
void getsetminmax(int gno, int setno, double* x1, double* x2, double* y1, double* y2);
void getminmaxall(int gno, int setno);
void minmax(double* x, int n, double* xmin, double* xmax, int* imin, int* imax);
void getsetdxdyminmax(int gno, int setno, double* dx1, double* dx2, double* dy1, double* dy2);
void updatesetminmax(int gno, int setno);
void set_point(int gno, int setn, int seti, double wx, double wy);
void get_point(int gno, int setn, int seti, double* wx, double* wy);
void setcol(int gno, double* x, int setno, int len, int col);
void* geteditpoints(int gno, int setno);
int getncols(int gno, int setno);
void setxy(int gno, double** ex, int setno, int len, int ncols);
void setlength(int gno, int i, int length);
void copycol(int gno, int setfrom, int setto, int col);
void copycol2(int gfrom, int setfrom, int gto, int setto, int col);
void moveset(int gnofrom, int setfrom, int gnoto, int setto);
void copyset(int gnofrom, int setfrom, int gnoto, int setto);
void copysetprops(int gnofrom, int setfrom, int gnoto, int setto);
void copysetdata(int gnofrom, int setfrom, int gnoto, int setto);
void packsets(int gno);
void do_packsets(void);
int nextset(int gno);
void killset(int gno, int setno);
void softkillset(int gno, int setno);
void activateset(int gno, int setno);
int activeset(int gno);
void droppoints(int gno, int setno, int startno, int endno, int dist);
void joinsets(int g1, int j1, int g2, int j2);
void sort_xy(double* tmp1, double* tmp2, int up, int sorton, int stype);
void findpoint(int gno, double x, double y, double* xs, double* ys, int* setno, int* loc);
void del_point(int gno, int setno, int pt);
void add_point(int gno, int setno, double px, double py, double tx, double ty, int type);
void add_point_at(int gno, int setno, int ind, int where, double px, double py, double tx, double ty, int type);
double* getvptr(int gno, int setno, int v);

void do_copyset(int gfrom, int j1, int gto, int j2);
void do_moveset(int gfrom, int j1, int gto, int j2);
void do_swapset(int gfrom, int j1, int gto, int j2);
void do_activateset(int gno, int setno, int len);
void do_splitsets(int gno, int setno, int lpart);
int do_writesets(int gno, int setno, int embed, char* fn, char* format);
void do_activate(int setno, int type, int len);
void do_deactivate(int gno, int setno);
void do_reactivate(int gno, int setno);
void do_changetype(int setno, int type);
void do_setlength(int setno, int len);
void do_copy(int j1, int gfrom, int j2, int gto);
void do_move(int j1, int gfrom, int j2, int gto);
void do_swap(int j1, int gfrom, int j2, int gto);
void do_drop_points(int setno, int startno, int endno);
void do_join_sets(int gfrom, int j1, int gto, int j2);
void do_reverse_sets(int setno);
void do_coalesce_sets(int setno);
void do_kill(int gno, int setno, int soft);
void do_flush(void);
void do_sort(int setno, int sorton, int stype);
void sort_set(int setno, int sorton, int stype);
void do_kill_nearest(void);
void do_copy_nearest(void);
void do_move_nearest(void);
void do_reverse_nearest(void);
void do_deactivate_nearest(void);
void do_join_nearest(void);
void do_delete_nearest(void);
void do_cancel_pickop(void);
void do_writesets_binary(int gno, int setno, char* fn);
void outputset(int gno, int setno, char* fname, char* dformat);

void set_hotlink(int gno, int setno, int onoroff, char* fname, int src);
int is_hotlinked(int gno, int setno);
void do_update_hotlink(int gno, int setno);
char* get_hotlink_file(int gno, int setno);
int get_hotlink_src(int gno, int setno);

void create_default_frame(void);
void define_colors_popup(void);
void update_editp_proc(void);

void cxfree(void* ptr);
void fswap(double* x, double* y);
void iswap(int* x, int* y);
int isoneof(int c, char* s);
int argmatch(char* s1, char* s2, int atleast);
void convertchar(char* s);
int ilog2(int n);
double comp_area(int n, double* x, double* y);
double comp_perimeter(int n, double* x, double* y);
double fmin(double x, double y);
double fmax(double x, double y);
double julday(int mon, int day, int year, int h, int mi, double se);
void calcdate(double jd, int* m, int* d, int* y, int* h, int* mi, double* sec);
int dayofweek(double j);
int leapyear(int year);
void getmoday(int days, int yr, int* mo, int* da);
int getndays(double j);
void stripspecial(char* s, char* cs);

void hselectfont(int f);
void puthersh(int xpos, int ypos, double scale, int dir, int just, int color, void (*vector)(), char* s);
int stringextentx(double scale, char* s);
int stringextenty(double scale, char* s);

void xy2r(double x, double ytmp, double* r, double* theta);
void uv2rx(double u, double v, double* r, double* x);
void kill_ellipse(int ellipseno);
void do_breakset(int gno, int setno, int ind);
void findpoint_inset(int gno, int setno, double x, double y, int* loc);
void set_graph_viewport(int gno, double vx1, double vy1, double vx2, double vy2);
void log_results(char* buf);
int set_workingdir(char* wd);
void expand_tilde(char* buf);
void echomsg(char* msg);
void set_work_pending(int d);
void update_set_lists(int gno);
void do_seasonal_diff(int setno, int period);
void extractsets_region(int gfrom, int gto, int rno);
void deletesets_region(int gno, int rno);
void reporton_region(int gno, int rno, int type, int strict);
void set_plotstr_string(plotstr* pstr, char* buf);
int isactive_ellipse(int ellipno);
int isxreversed(int gno);
int isyreversed(int gno);
void my_ellipse(double xc, double yc, double w, double h, int fill);
void getlineprops(int* col, int* style, int* width);
void stufftext(char* s, int sp);
void get_graph_ellipse(int i, ellipsetype* b);
int lists_dirty(void);
int nactive(int gno);
int work_pending(void);
void set_printer(int device, char* prstr);
void do_hardcopy(void);
void initialize_cms_data(void);
void do_main_loop(void);
void update_printer_setup(void);
int do_nonlfit(int gno, int setno, int nsteps);
void sortset(int gno, int setno, int sorton, int stype);

int yesno(char* msg1, char* msg2, char* s1, char* help_anchor);
int fexists(char* to);
void setbgcolor(int c);

void set_frame_bound(int gno);
void set_drawing_finished(void);
void change_suffix(char* s, char* newsuf);
char* mybasename(char* s);

void do_interp(int yset, int xset, int method);

void initialize_nonl(void);
void reset_nonl(void);

#ifndef HAVE_STRDUP
char* strdup(char* str);
#endif

char* escapequotes(char* s);

void set_leg_bb(int gno, int hardcopy);

int pstexinitgraphics(int dmode);

void set_dirtystate(void);
void lock_dirtystate(void);
void clear_dirtystate(void);
int is_dirtystate(void);

#endif /* __NOXPROTOS_H_ */
