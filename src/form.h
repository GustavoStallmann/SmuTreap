#ifndef FORM_H
#define FORM_H

#include "smu_treap.h"
#include "form_style.h"
#include "form_state.h"

enum Form {
    CIRCLE, 
    RECT, 
    TEXT, 
    LINE
};

/* 
 * Sets the minimum bounding box of the form in the given variables
 * @param tp the type of the form
 * @param form the form to get the bounding box
 * @param x, y, w, h the results of the bounding box coordinates
 */
void get_form_bounding_box(DescritorTipoInfo tp, Info form, double *x, double *y, double *w, double *h);

/*
 * Gets the style of the form
 * @param tp the type of the form
 * @param form the form to get the style
 * @return the style of the form
 */
FormStyle get_form_style(DescritorTipoInfo tp, Info form); 

/*
 * Gets the coordinates of the form
 * @param tp the type of the form
 * @param form the form to get the coordinates
 * @param x, y the results of the coordinates
 */
void get_form_coordinates(DescritorTipoInfo tp, Info form, double *x, double *y);

/*
 * Gets the dimensions of the form
 * @param tp the type of the form
 * @param form the form to get the dimensions
 * @param w, h the results of the width and height
 */
void get_form_dimensions(DescritorTipoInfo tp, Info form, double *w, double *h);

/*
 * Gets the state of the form
 * @param tp the type of the form
 * @param form the form to get the state
 * @return the state of the form
 */
FormState get_form_state(DescritorTipoInfo tp, Info form);

/*
 * Checks if a form is inside the specified region
 * @param t the treap structure
 * @param n the node in the treap
 * @param i the form info
 * @param x1, y1 the top-left coordinates of the region
 * @param x2, y2 the bottom-right coordinates of the region
 * @return true if the form is inside the region, false otherwise
 */
bool is_form_inside_region(SmuTreap t, Node n, Info i, double x1, double y1, double x2, double y2);

/*
 * Gets the displacement distance of the form
 * @param tp the type of the form
 * @param form the form to get the displacement distance
 * @return the displacement distance of the form
 */
double get_form_distance_disp(DescritorTipoInfo tp, Info form);

/*
 * Gets the name of the form type
 * @param tp the type of the form
 * @return the name of the form type as a string
 */
char* get_form_name(DescritorTipoInfo tp);

/* 
 * Translates the form to the given coordinates
 * @param tp the type of the form
 * @param form the form to translate
 * @param x, y the new coordinates of the form
*/
void transp_form(DescritorTipoInfo tp, Info form, double x, double y); 

/*
 * Gets the id of the form
 * @param tp the type of the form
 * @param form the form to get the id
 * @return the id of the form
*/
int get_form_id(DescritorTipoInfo tp, Info form);

/*
 * Frees the memory of the form
 * @param tp the type of the form
 * @param form the form to free
 */
void free_form(DescritorTipoInfo tp, Info form);

#endif