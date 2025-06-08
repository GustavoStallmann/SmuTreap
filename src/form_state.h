#ifndef FORM_STATE_H
#define FORM_STATE_H

#include <stdbool.h>
typedef void *FormState; 

/*
 * Creates a new form state
 * @return a new FormState object
 */
FormState new_form_state();

/*
 * Sets the camouflaged state of the form
 * @param state the form state to modify
 * @param camouflaged true to set as camouflaged, false otherwise
 */
void set_form_state_camouflaged(FormState state, bool camouflaged);

/*
 * Checks if the form is camouflaged
 * @param state the form state to check
 * @return true if the form is camouflaged, false otherwise
 */
bool is_form_state_camouflaged(FormState state);

/*
 * Sets the selected state of the form
 * @param state the form state to modify
 * @param selected true to set as selected, false otherwise
 */
void set_form_state_selected(FormState state, bool selected);

/*
 * Checks if the form is selected
 * @param state the form state to check
 * @return true if the form is selected, false otherwise
 */
bool is_form_state_selected(FormState state);

/*
 * Sets the blown state of the form
 * @param state the form state to modify
 * @param blown true to set as blown, false otherwise
 */
void set_form_state_blown(FormState state, bool blown);

/*
 * Checks if the form is blown
 * @param state the form state to check
 * @return true if the form is blown, false otherwise
 */
bool is_form_state_blown(FormState state);

/*
 * Sets the hit state of the form
 * @param state the form state to modify
 * @param hit true to set as hit, false otherwise
 */
void set_form_state_hit(FormState state, bool hit);

/*
 * Checks if the form is hit
 * @param state the form state to check
 * @return true if the form is hit, false otherwise
 */
bool is_form_state_hit(FormState state);

/*
 * Frees the memory of the form state
 * @param state the form state to free
 */
void free_form_state(FormState state);

#endif