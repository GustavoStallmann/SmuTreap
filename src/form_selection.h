#ifndef FORM_SELECTION_H
#define FORM_SELECTION_H

#include <stdbool.h>

#define MAX_SELECTION_REGIONS 100

typedef void* SelectionManager;

/*
 * Creates a new selection manager
 * @return a new SelectionManager object
 */
SelectionManager new_selection_manager();

/*
 * Destroys the selection manager and frees its memory
 * @param manager the selection manager to destroy
 */
void selection_manager_free(SelectionManager manager);

/*
 * Gets the total number of selection regions
 * @return the number of selection regions
 */
int selection_manager_get_region_count();

/*
 * Sets the active state of a selection region
 * @param manager the selection manager to modify
 * @param index the index of the region to modify
 * @param active true to set the region as active, false otherwise
 */
void selection_manager_set_region_active(SelectionManager manager, int index, bool active);

/*
 * Checks if a selection region is active
 * @param manager the selection manager to check
 * @param index the index of the region to check
 * @return true if the region is active, false otherwise
 */
bool selection_manager_is_region_active(SelectionManager manager, int index);

/*
 * Gets the data of a selection region
 * @param manager the selection manager to query
 * @param index the index of the region to get data from
 * @param x, y the coordinates of the region
 * @param width, height the dimensions of the region
 */
void selection_manager_get_region_data(SelectionManager manager, int index, double* x, double* y, double* width, double* height);

/*
 * Sets the data of a selection region
 * @param manager the selection manager to modify
 * @param index the index of the region to modify
 * @param x, y the coordinates of the region
 * @param width, height the dimensions of the region
 */
void selection_manager_set_region_data(SelectionManager manager, int index, double x, double y, double width, double height);

/*
 * Adds a new selection region to the manager
 * @param manager the selection manager to add the region to
 * @param x, y the coordinates of the new region
 * @param width, height the dimensions of the new region
 */
void selection_manager_add_region(SelectionManager manager, double x, double y, double width, double height);

#endif
