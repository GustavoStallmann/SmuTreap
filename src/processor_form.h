#ifndef PROCESSOR_FORM_H
#define PROCESSOR_FORM_H
#include "smu_treap.h"
#include "form_style.h"

typedef void* FormInfo;

/*
 * Process the given form obtained form geo file
 * @param formType the form type readed from geo
 * @param line_buffer the line with the form specs from geo
 * @param actual_font_style the actual setted font_style
 * @return the created form information
*/
FormInfo process_form(char *formType, char *line_buffer, FormStyle *actual_font_style); 

/* 
 * Get the form from the given FormInfo
 * @param formInfo the form information to get the form
 * @return Info the form of the given FormInfo
*/
Info get_form_info(FormInfo formInfo);

/* 
 * Get the form type 
 * @param formInfo the form information to get the type of form
 * @return DescritorTipoInfo the type of form of the given FormInfo
*/
DescritorTipoInfo get_form_type(FormInfo formInfo);

/* 
 * Clones a form with the given specs
 * @param formType the type of the form 
 * @param form the form to be cloned
 * @param id the id for the cloned form 
 * @param target_x the x coord for the cloned form
 * @param target_y the y coord for the cloned form
 * @return FormInfo of the cloned form
*/
FormInfo clone_form(DescritorTipoInfo formType, Info form, int id, double target_x, double target_y);

/* 
 * Free up the FormInfo from memory and the form
 * @param formInfo the formInfo to be freed
*/
void free_form_info(FormInfo formInfo);

/* 
 * Free only the form info wrapper not the form
 * @param formInfo the formInfo to be freed
*/
void free_form_info_wrapper_only(FormInfo formInfo);

#endif