#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "processor_form.h"
#include "form_style.h"
#include "form.h"
#include "form_circle.h"
#include "form_rect.h"
#include "form_line.h"
#include "form_text.h"
#include "smu_treap.h"

#define ARG_SIZE 20

typedef struct {
    DescritorTipoInfo tp;
    Info form;
} FormInfo_st;

FormInfo new_form_info(DescritorTipoInfo tp, Info form) {
    FormInfo_st *formInfo = (FormInfo_st *) malloc(sizeof(FormInfo_st));
    if (formInfo == NULL) {
        fprintf(stderr, "(ERROR) form: couldn't allocate memory for FormInfo_st");
        return NULL; 
    }

    formInfo->tp = tp;
    formInfo->form = form;

    return formInfo;
}

Info get_form_info(FormInfo formInfo) {
    if (formInfo == NULL) {
        fprintf(stderr, "(ERROR) form: the form info is NULL");
        return NULL; 
    }

    FormInfo_st *info = (FormInfo_st *) formInfo;
    return (Info) info->form;
}

DescritorTipoInfo get_form_type(FormInfo formInfo) {
    if (formInfo == NULL) {
        fprintf(stderr, "(ERROR) form: the form info is NULL");
        return -1; 
    }

    FormInfo_st *info = (FormInfo_st *) formInfo;
    return info->tp;
}

void free_form_info(FormInfo formInfo) {
    if (formInfo == NULL) {
        fprintf(stderr, "(ERROR) form: the form info is NULL");
        return; 
    }

    FormInfo_st *info = (FormInfo_st *) formInfo;
    free_form(info->tp, info->form);
    free(formInfo);
}

void free_form_info_wrapper_only(FormInfo formInfo) {
    if (formInfo == NULL) {
        fprintf(stderr, "(ERROR) form: the form info is NULL");
        return; 
    }

    free(formInfo);
}

static FormInfo process_circle(char *line_buffer) {
    char cmd[10];
    int id = 0; 
    double x = 0.0, y = 0.0, r = 0.0;
    char fillColor[ARG_SIZE] = {0}, borderColor[ARG_SIZE] = {0}; 
    int parsed = sscanf(line_buffer, "%s %d %lf %lf %lf %s %s", cmd, &id, &x, &y, &r, borderColor, fillColor);
    if (parsed != 7) {
        fprintf(stderr, "(ERROR) form: couldn't parse circle parameters from line: %s", line_buffer);
        return NULL;
    }
    FormStyle style = new_form_style(borderColor, fillColor, NULL, NULL, NULL, NULL, "2"); 
    Circle circle = new_circle(id, x, y, r, style);
    if (circle == NULL) {
        fprintf(stderr, "(ERROR) form: couldn't create circle with id %d", id);
        return NULL; 
    }

    return new_form_info(CIRCLE, circle); 
}

static FormInfo process_rect(char * line_buffer) {
    char cmd[10];
    int id = 0; 
    double x = 0.0, y = 0.0, w = 0.0, h = 0.0;
    char fillColor[ARG_SIZE] = {0}, borderColor[ARG_SIZE] = {0}; 
    int parsed = sscanf(line_buffer, "%s %d %lf %lf %lf %lf %s %s", cmd, &id, &x, &y, &w, &h, borderColor, fillColor);
    if (parsed != 8) {
        fprintf(stderr, "(ERROR) form: couldn't parse rect parameters from line: %s", line_buffer);
        return NULL;
    }
    FormStyle style = new_form_style(borderColor, fillColor, NULL, NULL, NULL, NULL, "2"); 
    Rect rect = new_rect(id, x, y, w, h, style);
    if (rect == NULL) {
        fprintf(stderr, "(ERROR) form: couldn't create rect with id %d", id);
        return NULL; 
    }

    return new_form_info(RECT, rect); 
}

static FormInfo process_text(char *line_buffer, FormStyle *actual_font_style) {
    char cmd[10];
    int id = 0; 
    double x = 0.0, y = 0.0; 
    char text[ARG_SIZE] = {0}, fillColor[ARG_SIZE] = {0}, anchor[ARG_SIZE] = {0}, borderColor[ARG_SIZE] = {0}; 
    int parsed = sscanf(line_buffer, "%s %d %lf %lf %s %s %s \"%[^\"]\"", cmd, &id, &x, &y, borderColor, fillColor, anchor, text);
    if (parsed != 8) {
        fprintf(stderr, "(ERROR) form: couldn't parse text parameters from line: %s", line_buffer);
        return NULL;
    }

    FormStyle style = new_form_style(borderColor, fillColor, 
        get_form_style_font_family(*actual_font_style), get_form_style_font_weight(*actual_font_style), 
        get_form_style_text_anchor(*actual_font_style), get_form_style_font_size(*actual_font_style),
        "0.5"
    ); 

    Text textForm = new_text(id, x, y, text, style);
    if (textForm == NULL) {
        fprintf(stderr, "(ERROR) form: couldn't create text with id %d", id);
        return NULL; 
    }

    return new_form_info(TEXT, textForm); 
}

static FormInfo process_line(char *line_buffer) {
    char cmd[10];
    int id = 0; 
    double x = 0.0, y = 0.0, x2 = 0.0, y2 = 0.0;
    char borderColor[ARG_SIZE] = {0}; 
    int parsed = sscanf(line_buffer, "%s %d %lf %lf %lf %lf %s", cmd, &id, &x, &y, &x2, &y2, borderColor);
    if (parsed != 7) {
        fprintf(stderr, "(ERROR) form: couldn't parse line parameters from line: %s", line_buffer);
        return NULL;
    }

    FormStyle style = new_form_style(borderColor, NULL, NULL, NULL, NULL, NULL, "2"); 
    Line line = new_line(id, x, y, x2, y2, style);
    if (line == NULL) {
        fprintf(stderr, "(ERROR) form: couldn't create line with id %d", id);
        return NULL; 
    }

    return new_form_info(LINE, line); 
}

static FormStyle process_ts(char *line_buffer) {
    char cmd[10];
    char font_family[ARG_SIZE] = {0}, font_weight[ARG_SIZE] = {0}, font_size[ARG_SIZE] = {0};
    int parsed = sscanf(line_buffer, "%s %s %s %s", cmd, font_family, font_weight, font_size);
    if (parsed != 4) {
        fprintf(stderr, "(ERROR) form: couldn't parse ts parameters from line: %s", line_buffer);
        return NULL;
    }

    FormStyle ts = new_form_style("#ffffff", "#ffffff", font_family, font_weight, "start", font_size, "1");
    if (ts == NULL) {
        fprintf(stderr, "(ERROR) form: couldn't create ts style");
        return NULL; 
    }
    return ts; 
}

FormInfo process_form(char *formType, char *line_buffer, FormStyle *actual_font_style) {
    if (formType == NULL || line_buffer == NULL) {
        fprintf(stderr, "(ERROR) form: the form type or line buffer is NULL");
        return NULL; 
    }

    if (strcmp(formType, "ts") == 0) {
        FormStyle new_style = process_ts(line_buffer);
        if (new_style != NULL) {
            if (*actual_font_style != NULL) {
                free_form_style(*actual_font_style);
            }
            *actual_font_style = new_style;
        } else {
            fprintf(stderr, "(ERROR) form: failed to process text style from line: %s", line_buffer);
        }
        return NULL; 
    }
    else if (strcmp(formType, "r") == 0) 
        return process_rect(line_buffer);
    else if (strcmp(formType, "c") == 0) 
        return process_circle(line_buffer); 
    else if (strcmp(formType, "l") == 0) 
        return process_line(line_buffer);
    else if (strcmp(formType, "t") == 0) 
        return process_text(line_buffer, actual_font_style);
    else {
        fprintf(stderr, "(ERROR) form: the .geo form type informed is invalid: %s", formType); 
    }

    return NULL; 
}

FormInfo clone_form(DescritorTipoInfo formType, Info form, int id, double target_x, double target_y) {
    if (formType == -1 || form == NULL) {
        fprintf(stderr, "(ERROR) form: clone_form requires a valid form type and info");
        return NULL; 
    }

    FormStyle style = get_form_style(formType, form);
    if (style == NULL) {
        fprintf(stderr, "(ERROR) form: couldn't get style for form with id %d", id);
        return NULL; 
    }

    FormStyle new_style = new_form_style(
        "purple", "red", 
        get_form_style_font_family(style), get_form_style_font_weight(style), 
        get_form_style_text_anchor(style), get_form_style_font_size(style),
        get_form_style_stroke_width(style)
    );

    Info new_form = NULL; 

    switch (formType) {
        case CIRCLE: {
            double r; 
            get_form_dimensions(formType, form, &r, NULL);
            
            new_form = (Info) new_circle(id, target_x, target_y, r, new_style);
            break;
        }
        case RECT: {
            double w, h; 
            get_form_dimensions(formType, form, &w, &h);
            
            new_form = (Info) new_rect(id, target_x, target_y, w, h, new_style);
            break;
        }
        case TEXT: {
            char *text = get_text_string((Text) form);
            if (text == NULL) {
                fprintf(stderr, "(ERROR) form: couldn't get text content for form with id %d", id);
                return NULL; 
            }
            
            new_form = new_text(id, target_x, target_y, text, new_style);
            break;
        }  
        case LINE: {
            double x1, y1, x2, y2; 
            get_line_positions((Line) form, &x1, &y1, &x2, &y2);
            
            double offset_x = target_x - x1;
            double offset_y = target_y - y1;
            
            new_form = (Info) new_line(id, target_x, target_y, x2 + offset_x, y2 + offset_y, new_style);
            break;
        }            
        default:
            fprintf(stderr, "(ERROR) form: invalid form type for cloning");
            return NULL; 
    }

    return new_form_info(formType, new_form); 
}