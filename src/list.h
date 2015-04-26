#ifndef _ROGUE14_LIST_H_
#define _ROGUE14_LIST_H_

/* takes an item out of whatever linked list it might be in */
#define detach(a,b) _detach(&a,b)
void _detach(THING **list, THING *item);

/* add an item to the head of a list */
#define attach(a,b) _attach(&a,b)
void _attach(THING **list, THING *item);

/* Throw the whole blamed thing away */
#define free_list(a) _free_list(&a)
void _free_list(THING **ptr);

/* Free up an item */
void discard(THING *item);

/* Get a new item with a specified size */
THING * new_item(void);

#endif /* _ROGUE14_LIST_H_ */