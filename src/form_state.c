#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "form_state.h"

typedef struct {
    bool is_camouflaged;
    bool is_selected;
    bool is_blown;
    bool is_hit; 
} FormState_st;

FormState new_form_state() {
    FormState_st *state = (FormState_st *) malloc(sizeof(FormState_st));
    if (state == NULL) {
        fprintf(stderr, "ERROR: Could not allocate memory for FormState\n");
        exit(EXIT_FAILURE);
    }
    
    state->is_camouflaged = false;
    state->is_selected = false;
    state->is_blown = false; 
    state->is_hit = false;
    
    return (FormState) state;
}

void set_form_state_hit(FormState state, bool hit) {
    if (state == NULL) return; 
    FormState_st *form_state = (FormState_st *) state; 
    form_state->is_hit = hit; 
}

void set_form_state_camouflaged(FormState state, bool camouflaged) {
    if (state == NULL) return; 
    FormState_st *form_state = (FormState_st *) state;
    form_state->is_camouflaged = camouflaged;
}

void set_form_state_blown(FormState state, bool blown) {
    if (state == NULL) return; 
    FormState_st *form_state = (FormState_st *) state;
    form_state->is_blown = blown;
}

void set_form_state_selected(FormState state, bool selected) {
    if (state == NULL) return; 
    FormState_st *form_state = (FormState_st *) state;
    form_state->is_selected = selected;
}

bool is_form_state_hit(FormState state) {
    if (state == NULL) return false; 
    FormState_st *form_state = (FormState_st *) state;

    return form_state->is_hit; 
}

bool is_form_state_blown(FormState state) {
    if (state == NULL) return false; 
    FormState_st *form_state = (FormState_st *) state;
    return form_state->is_blown;
}

bool is_form_state_camouflaged(FormState state) {
    if (state == NULL) return false; 
    FormState_st *form_state = (FormState_st *) state;
    return form_state->is_camouflaged;
}


bool is_form_state_selected(FormState state) {
    if (state == NULL) return false; 
    FormState_st *form_state = (FormState_st *) state;
    return form_state->is_selected;
}

void free_form_state(FormState state) {
    if (state == NULL) return; 
    free(state);
}