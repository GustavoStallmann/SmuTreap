#include <stdio.h>
#include <string.h>

#include "form.h"
#include "form_circle.h"
#include "form_line.h"
#include "form_rect.h"
#include "form_state.h"
#include "form_style.h"
#include "form_text.h"
#include "smu_treap.h"

void get_form_bounding_box(DescritorTipoInfo tp, Info form, double *x, double *y, double *w, double *h) {
    switch (tp) {
        case CIRCLE:
            get_circle_bounding_box((Circle) form, x, y, w, h);
            break; 
        case RECT: 
            get_rect_bounding_box((Rect) form, x, y, w, h); 
            break; 
        case TEXT:
            get_text_bounding_box((Text) form, x, y, w, h); 
            break;
        case LINE: 
            get_line_bounding_box((Line) form, x, y, w, h);
            break;
        
        default:
            fprintf(stderr, "(ERROR) form: invalid form provided to compute the bounding box");
            break;
    }
}

FormStyle get_form_style(DescritorTipoInfo tp, Info form) {
    switch (tp) {
        case CIRCLE:
            return get_circle_style((Circle) form);
        case RECT: 
            return get_rect_style((Rect) form);
        case TEXT:
            return get_text_style((Text) form);
        case LINE: 
            return get_line_style((Line) form);
        
        default:
            fprintf(stderr, "(ERROR) form: invalid form provided to get style");
            return NULL;
    }
}

void get_form_coordinates(DescritorTipoInfo tp, Info form, double *x, double *y) {
    switch (tp) {
        case CIRCLE:
            get_circle_positions((Circle) form, x, y, NULL);
            break; 
        case RECT: 
            get_rect_positions((Rect) form, x, y, NULL, NULL); 
            break; 
        case TEXT:
            get_text_positions((Text) form, x, y); 
            break;
        case LINE: 
            get_line_positions((Line) form, x, y, NULL, NULL);
            break;
        
        default:
            fprintf(stderr, "(ERROR) form: invalid form provided to get coordinates");
            break;
    }
}

void get_form_dimensions(DescritorTipoInfo tp, Info form, double *w, double *h) {
    switch (tp) {
        case CIRCLE:
            get_circle_positions((Circle) form, NULL, NULL, w);
            if (h != NULL) *h = *w;
            break; 
        case RECT: 
            get_rect_positions((Rect) form, NULL, NULL, w, h); 
            break; 
        case TEXT:
            get_text_bounding_box((Text) form, NULL, NULL, w, h); 
            break;
        case LINE: 
            get_line_positions((Line) form, NULL, NULL, w, h);
            break;
        
        default:
            fprintf(stderr, "(ERROR) form: invalid form provided to get dimensions");
            break;
    }
}

bool is_form_inside_region(SmuTreap t, Node n, Info i, double x1, double y1, double x2, double y2) {
    DescritorTipoInfo form_type = getTypeInfoSmuT(t, n);
    double x, y, w, h; 
    get_form_bounding_box(form_type, i, &x, &y, &w, &h);
    
    double min_x = (x1 < x2) ? x1 : x2;
    double max_x = (x1 < x2) ? x2 : x1;
    double min_y = (y1 < y2) ? y1 : y2;
    double max_y = (y1 < y2) ? y2 : y1;

    return (x >= min_x && (x + w) <= max_x && y >= min_y && (y + h) <= max_y);
}

FormState get_form_state(DescritorTipoInfo tp, Info form) {
    switch (tp) {
        case CIRCLE:
            return get_circle_state((Circle) form);
        case RECT: 
            return get_rect_state((Rect) form);
        case TEXT:
            return get_text_state((Text) form);
        case LINE: 
            return get_line_state((Line) form);
        
        default:
            fprintf(stderr, "(ERROR) form: invalid form provided to get state");
            return NULL;
    }
}

int get_form_id(DescritorTipoInfo tp, Info form) {
    switch (tp) {
        case CIRCLE:
            return get_circle_id((Circle) form);
        case RECT: 
            return get_rect_id((Rect) form);
        case TEXT:
            return get_text_id((Text) form);
        case LINE: 
            return get_line_id((Line) form);
        
        default:
            fprintf(stderr, "(ERROR) form: invalid form provided to get id");
            return -1;
    }
}

void transp_form(DescritorTipoInfo tp, Info form, double x, double y) {
    switch (tp) {
        case CIRCLE:
            circle_transp((Circle) form, x, y);
            break;
        case RECT:
            rect_transp((Rect) form, x, y);
            break;
        case TEXT:
            text_transp((Text) form, x, y);
            break;
        case LINE:  
            line_transp((Line) form, x, y);
            break;
        default:
            fprintf(stderr, "(ERROR) form: invalid form provided to translate");
            break;
    }
}

double get_form_distance_disp(DescritorTipoInfo tp, Info form) {
    switch (tp) {
        case CIRCLE:
            return get_circle_distance_disp((Circle) form);
        case RECT:
            return get_rect_distance_disp((Rect) form);
        case LINE:
            return get_line_distance_disp((Line) form);
        case TEXT:
            return get_text_distance_disp((Text) form);
        default:
            return 0.0;
    }
}

char* get_form_name(DescritorTipoInfo tp) {
    switch (tp) {
        case CIRCLE: 
            return "Circulo";
        case RECT: 
            return "Retangulo";
        case LINE: 
            return "Linha"; 
        case TEXT:
            return "Texto"; 
        default:
            return "Desconhecido";
    }
}

void free_form(DescritorTipoInfo tp, Info form) {
    switch (tp) {
        case CIRCLE:
            free_circle((Circle) form);
            break; 
        case RECT: 
            free_rect((Rect) form);
            break; 
        case TEXT:
            free_text((Text) form);
            break;
        case LINE: 
            free_line((Line) form);
            break;
        
        default:
            fprintf(stderr, "(ERROR) form: invalid form provided to free memory");
            break;
    }
}