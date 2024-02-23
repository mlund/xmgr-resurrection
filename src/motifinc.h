/*
 * for Motif specific items
 */

#ifndef __MOTIFINC_H_
#define __MOTIFINC_H_

#if (XmVersion < 1002)
#define XmStringCreateLocalized XmStringCreateSimple
#endif

extern Widget app_shell;        /* defined in xmgr.c */
extern XmStringCharSet charset; /* defined in xmgr.c */

/* set selection gadget */
typedef struct _SetChoiceItem {
    int type;
    int display;
    int gno;
    int spolicy;
    int fflag; /* if 0, no filter gadgets */
    int indx;
    Widget list;
    Widget rb;
    Widget but[8];
} SetChoiceItem;

void update_set_list(int gno, SetChoiceItem l);
int save_set_list(SetChoiceItem l);
void update_save_set_list(SetChoiceItem l, int newgr);
SetChoiceItem CreateSetSelector(Widget parent, char* label, int type, int ff, int gtype, int stype);
void SetSelectorFilterCB(Widget parent, XtPointer cld, XtPointer calld);
int GetSelectedSet(SetChoiceItem l);

int GetSelectedSets(SetChoiceItem l, int** sets);
void DefineSetSelectorFilter(SetChoiceItem* s);
int SetSelectedSet(int gno, int setno, SetChoiceItem l);

#endif /* __MOTIFINC_H_ */
