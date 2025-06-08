#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "form_selection.h"

#define MAX_SELECTION_REGIONS 100

typedef struct {
    double x, y, width, height;
    bool is_active;
} SelectionRegion_st;

typedef struct {
    SelectionRegion_st regions[MAX_SELECTION_REGIONS];
    int next_available_index;
} SelectionManager_st;

SelectionManager new_selection_manager() {
    SelectionManager_st* manager = malloc(sizeof(SelectionManager_st));
    if (manager == NULL) return NULL;
    
    for (int i = 0; i < MAX_SELECTION_REGIONS; i++) {
        manager->regions[i].is_active = false;
    }
    manager->next_available_index = 0;
    
    return (SelectionManager)manager;
}

void selection_manager_free(SelectionManager manager) {
    if (manager == NULL) return;
    free(manager);
}

int selection_manager_get_region_count() {
    return MAX_SELECTION_REGIONS;
}

void selection_manager_set_region_active(SelectionManager manager, int index, bool active) {
    if (manager == NULL || index < 0 || index >= MAX_SELECTION_REGIONS) return;
    SelectionManager_st* mgr = (SelectionManager_st*)manager;
    mgr->regions[index].is_active = active;
}

bool selection_manager_is_region_active(SelectionManager manager, int index) {
    if (manager == NULL || index < 0 || index >= MAX_SELECTION_REGIONS) return false;
    SelectionManager_st* mgr = (SelectionManager_st*)manager;
    return mgr->regions[index].is_active;
}

void selection_manager_get_region_data(SelectionManager manager, int index, double* x, double* y, double* width, double* height) {
    if (manager == NULL || index < 0 || index >= MAX_SELECTION_REGIONS || 
        x == NULL || y == NULL || width == NULL || height == NULL) return;
    
    SelectionManager_st* mgr = (SelectionManager_st*)manager;
    if (!mgr->regions[index].is_active) return; 

    *x = mgr->regions[index].x;
    *y = mgr->regions[index].y;
    *width = mgr->regions[index].width;
    *height = mgr->regions[index].height;
}

void selection_manager_set_region_data(SelectionManager manager, int index, double x, double y, double width, double height) {
    if (manager == NULL || index < 0 || index >= MAX_SELECTION_REGIONS) return;
    SelectionManager_st* mgr = (SelectionManager_st*)manager;
    mgr->regions[index].x = x;
    mgr->regions[index].y = y;
    mgr->regions[index].width = width;
    mgr->regions[index].height = height;
    mgr->regions[index].is_active = true;
}

void selection_manager_add_region(SelectionManager manager, double x, double y, double width, double height) {
    if (manager == NULL) return;
    SelectionManager_st* mgr = (SelectionManager_st*)manager;
    
    int index = mgr->next_available_index;
    while (index < MAX_SELECTION_REGIONS && mgr->regions[index].is_active) {
        index++;
    }
    
    if (index >= MAX_SELECTION_REGIONS) {
        for (index = 0; index < MAX_SELECTION_REGIONS; index++) {
            if (!mgr->regions[index].is_active) {
                break;
            }
        }
    }
    
    if (index < MAX_SELECTION_REGIONS) {
        mgr->regions[index].x = x;
        mgr->regions[index].y = y;
        mgr->regions[index].width = width;
        mgr->regions[index].height = height;
        mgr->regions[index].is_active = true;
        mgr->next_available_index = index + 1;
    }
}
