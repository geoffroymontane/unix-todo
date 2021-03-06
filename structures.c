/* Unix-Todo, a simple and scriptable to-do list in your terminal
 * Copyright (C) 2019 Geoffroy Montané (geoffroymontane on github)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


/*

Bool type

*/
typedef char bool;
#define true 1
#define false 0

/*

Reminder

*/
struct reminder{
	char *name;
	int priority;
};

/*

Reminders list

*/
struct List_reminders{
	int capacity;
	int size;
	struct reminder **data;
};

struct List_reminders* list_reminders_init(){
	struct List_reminders* list;
	list=malloc(sizeof(struct List_reminders));

	list->size=0;
	list->capacity=4;
	list->data=malloc(4*sizeof(struct reminder*));
	
	return list;
}

void list_reminders_free_all(struct List_reminders *list){
	for(int i=0;i<list->size;i++){
		free(list->data[i]->name);
		free(list->data[i]);
	}
	free(list->data);
	free(list);
}

void list_reminders_add(struct List_reminders *list, struct reminder *element){
	if(list->capacity-list->size>0){
		list->data[list->size++]=element;
	}
	else{
		struct reminder **newData=malloc(2*list->capacity*sizeof(struct reminder*));
		list->capacity*=2;

		if(newData==NULL){
			perror("Fatal error");
			exit(1);
		}

		for(int i=0;i<list->size;i++){
			newData[i]=list->data[i];
		}
		free(list->data);
		list->data=newData;
		list->data[list->size++]=element;
	}
}

/*

Category

*/
struct category{
	char *name;
	struct List_reminders *reminders;
};

/*

Categories list

*/
struct List_categories{
	int capacity;
	int size;
	struct category **data;
};

struct List_categories* list_categories_init(){
	struct List_categories* list;
	list=malloc(sizeof(struct List_categories));

	list->size=0;
	list->capacity=4;
	list->data=malloc(4*sizeof(struct category*));
	
	return list;
}

void list_categories_free_all(struct List_categories *list){
	for(int i=0;i<list->size;i++){
		list_reminders_free_all(list->data[i]->reminders);
		free(list->data[i]->name);
		free(list->data[i]);
	}
	free(list->data);
	free(list);
}

void list_categories_add(struct List_categories *list, struct category *element){
	if(list->capacity-list->size>0){
		list->data[list->size++]=element;
	}
	else{
		struct category **newData=malloc(2*list->capacity*sizeof(struct category*));
		list->capacity*=2;

		for(int i=0;i<list->size;i++){
			newData[i]=list->data[i];
		}
		free(list->data);
		list->data=newData;
		list->data[list->size++]=element;
	}
}


