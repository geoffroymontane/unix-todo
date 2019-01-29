#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>

#include "memleakscheck.c"

#include "structures.c"



/*

CONFIG

 */
char *defaultPath;
char *homedir;
const char *color1="\e[92m";
const char *color2="\e[93m";
const char *color3="\e[91m";
const char *defaultColor="\e[0m\e[39m";
const char *boldColor="\e[1m";

/*

HEADER

*/
struct List_categories* readFromFile(const char *path);
void displayReminders(struct List_categories *categories);
bool writeToFile(const char *path,struct List_categories *categories);
void displayHelp();
char* readFromPrompt(int autoCompleteMode,const char *prompt);
char* strip(char *str_);
char** autoCompleteCategories(const char *text, int start, int end);
char* autoCompleteCategories_(const char *text,int state);
char** autoCompleteReminders(const char *text, int start, int end);
char* autoCompleteReminders_(const char *text,int state);



int main(int argc,char *argv[]){
	
	homedir=getenv("HOME"); 
	defaultPath=strdup(strcat(homedir,"/.uxtodo"));

	/*

	   Default command without flags

	 */
	if(argc==1){
		struct List_categories *categories=readFromFile(defaultPath);
		displayReminders(categories);
		list_categories_free_all(categories);
		free(defaultPath);
		return 0;
	}
	// With arguments
	else{

		/* 

		   Default command with flags

		 */
		if(*argv[1]=='-'){
			bool showAllCategories=true;
			char **categoriesToShow=NULL;
			int categoriesToShowCount=0;

			for(int i=1;i<argc;i++){

				// Flag -f
				if(strcmp(argv[i],"-f")==0){
					free(defaultPath);	
					defaultPath=strdup(argv[2]);
					i++;
					continue;

				}
				//Flags -c
				else if(strcmp(argv[i],"-c")==0){
					showAllCategories=false;
					int j=i+1;
					while(j<argc && strchr(argv[j],'-')==NULL){
						j++;
					}
					categoriesToShowCount=j-i-1;
					categoriesToShow=malloc(categoriesToShowCount*sizeof(char*));
					for(int n=i+1;n<j;n++){
						categoriesToShow[n]=argv[n];
					}
					i=j;
					continue;
				}


			}

			struct List_categories *categories=readFromFile(defaultPath);
			if(!showAllCategories){
				for(int i=0;i<categories->size;i++){
					bool shown=false;
					for(int j=0;j<categoriesToShowCount;j++){
						if(strcmp(categories->data[i]->name,categoriesToShow[j])==0){
							shown=true;
							break;
						}
					}
					if(!shown){
						free(categories->data[i]->name);
						categories->data[i]->name=strdup("__deleted__");
					}
				}
			}
			displayReminders(categories);

			if(categoriesToShow!=NULL){
				free(categoriesToShow);
			}
			list_categories_free_all(categories);
			free(defaultPath);
			return 0;
		}

		/* 

		Command ADD
		Flags: -n -f -c -p

		 */
		else if(strcmp(argv[1],"add")==0){

			// Flags parser
			bool interactiveMode=true;
			char *name=strdup("__none__");
			char *categoryName=strdup("default");

			int priority=3;
			for(int i=2;i<argc;i++){
				if(strcmp(argv[i],"-n")==0){ 
					if(i+1<argc){
						interactiveMode=false;
						free(name);
						name=strdup(argv[i+1]);
					}
				}
				else if(strcmp(argv[i],"-c")==0){ 
					if(i+1<argc){
						free(categoryName);
						categoryName=strdup(argv[i+1]);
					}
				}
				else if(strcmp(argv[i],"-p")==0){ 
					if(i+1<argc){
						priority=atoi(argv[i+1]);
					}
				}
				else if(strcmp(argv[i],"-f")==0){ 
					if(i+1<argc){
						free(defaultPath);
						defaultPath=strdup(argv[i+1]);
					}
				}
			}				

			// Interactive mode
			if(interactiveMode || strcmp(name,"__none__")==0){	
				free(name);
				free(categoryName);
				name=readFromPrompt(1,"Name: ");
				categoryName=readFromPrompt(0,"Category: ");

				char *line=readFromPrompt(2,"Priority in [0,1,2,3]: ");
				if(strlen(line)>0 && line[0]<='9'){
					char *priority_str=strndup(line,1);
					priority=atoi(priority_str);
					free(priority_str);
				}
				free(line);
			}

			if(strcmp(name,"__deleted__")==0 || strcmp(name,"__none__")==0 || strcmp(name,"")==0){
				printf("This name is reserved and cannot be used.");

				free(name);
				free(categoryName);
				free(defaultPath);
				return 0;
			}
			if(strcmp(categoryName,"__deleted__")==0 || strcmp(categoryName,"__none__")==0 || strcmp(categoryName,"")==0){
				printf("This category name is reserved and cannot be used.");

				free(name);
				free(categoryName);
				free(defaultPath);
				return 0;
			}

			// Change datafile
			struct List_categories *categories=readFromFile(defaultPath);
			
			for(int i=0;i<categories->size;i++){
				for(int j=0;j<categories->data[i]->reminders->size;j++){
					if(strcmp(categories->data[i]->reminders->data[j]->name,name)==0){
						printf("A reminder has already this name.\n");

						list_categories_free_all(categories);
						free(name);
						free(categoryName);
						free(defaultPath);
						return 0;
					}
				}
			}

			struct reminder *newReminder=malloc(sizeof(struct reminder));
			newReminder->name=name;
			newReminder->priority=priority;
			bool reminderAdded=false;

			for(int i=0;i<categories->size;i++){
				if(strcmp(categories->data[i]->name,categoryName)==0){
					list_reminders_add(categories->data[i]->reminders,newReminder);
					free(categoryName);
					reminderAdded=true;
					break;
				}
			}				
			if(!reminderAdded){
				struct category *newCategory=malloc(sizeof(struct category));
				newCategory->name=categoryName;
				newCategory->reminders=list_reminders_init();
				list_reminders_add(newCategory->reminders,newReminder);
				list_categories_add(categories,newCategory);
			}

			if(writeToFile(defaultPath,categories)){
				printf("Reminder added\n");
			}

			list_categories_free_all(categories);
			free(defaultPath);
			return 0;
		}
		/* 

		Command CLEAR, CLEAN
		Flags : -f -c

		 */
		else if(strcmp(argv[1],"clear")==0 || strcmp(argv[1],"clean")==0){

			// Flags parser
			char *categoryName=strdup("__deleted__");
			for(int i=2;i<argc;i++){
				if(strcmp(argv[i],"-f")==0){ 
					if(i+1<argc){
						free(defaultPath);
						defaultPath=strdup(argv[i+1]);
					}
				}
				else if(strcmp(argv[i],"-c")==0){ 
					free(categoryName);
					if(i+1<argc){
						categoryName=strdup(argv[i+1]);
					}
					else{
						categoryName=strdup("__none__");
					}
				}
			}

			if(strcmp(categoryName,"__deleted__")==0){
				char *answer=readFromPrompt(2,"Do you really want to clear all categories/reminders ? y or n: ");
				if(answer[0]=='y'){
					FILE *file;
					file=fopen(defaultPath,"w");
					fprintf(file,"");
					fclose(file);
					printf("All is cleared.\n");
				}
				free(answer);
				free(categoryName);
				free(defaultPath);
				return 0;
			}
			else if(strcmp(categoryName,"__none__")==0){
				free(categoryName);
				categoryName=readFromPrompt(0,"What category do you want to delete ?");

				struct List_categories *categories=readFromFile(defaultPath);
				for(int i=0;i<categories->size;i++){
					if(strcmp(categories->data[i]->name,categoryName)==0){
						free(categories->data[i]->name);
						categories->data[i]->name=strdup("__deleted__");	
					}
				}

				if(writeToFile(defaultPath,categories)){
					printf("Successfully deleted\n");
				}

				free(categoryName);
				list_categories_free_all(categories);
				free(defaultPath);
				return 0;
			}
			else{
				struct List_categories *categories=readFromFile(defaultPath);
				for(int i=0;i<categories->size;i++){
					if(strcmp(categories->data[i]->name,categoryName)==0){
						free(categories->data[i]->name);
						categories->data[i]->name=strdup("__deleted__");	
					}
				}

				if(writeToFile(defaultPath,categories)){
					printf("Successfully deleted\n");
				}

				list_categories_free_all(categories);
				free(categoryName);
				free(defaultPath);
				return 0;
			}
		}
		/*

		Command DELETE, REMOVE, RM, DEL
		Flags: -f -n

		 */
		else if(strcmp(argv[1],"rm")==0 || strcmp(argv[1],"remove")==0 || strcmp(argv[1],"delete")==0 || strcmp(argv[1],"del")==0){


			// Flags parser
			char *name=strdup("__none__");
			for(int i=2;i<argc;i++){
				if(strcmp(argv[i],"-n")==0){ 
					if(i+1<argc){
						free(name);
						name=strdup(argv[i+1]);
					}
				}
				else if(strcmp(argv[i],"-f")==0){ 
					if(i+1<argc){
						free(defaultPath);
						defaultPath=strdup(argv[i+1]);
					}
				}
			}				

			// Interactive mode
			if(strcmp(name,"__none__")==0){
				printf("What reminder do you want to delete ?\n");
				printf("In order to remove a category, use clean -c categoryName instead.\n");
			
				free(name);
				name=readFromPrompt(1,"Name: ");

				bool targetFound=false;
				struct List_categories *categories=readFromFile(defaultPath);
				for(int i=0;i<categories->size;i++){
					for(int j=0;j<categories->data[i]->reminders->size;j++){
						if(strcmp(categories->data[i]->reminders->data[j]->name,name)==0){
							targetFound=true;
							i=categories->size;
							break;
						}	
					}
				}

				if(!targetFound){
					printf("No such reminder.\n");
				}
				else{
					for(int i=0;i<categories->size;i++){
						for(int j=0;j<categories->data[i]->reminders->size;j++){
							if(strcmp(categories->data[i]->reminders->data[j]->name,name)==0){
								free(categories->data[i]->reminders->data[j]->name);
								categories->data[i]->reminders->data[j]->name=strdup("__deleted__");
							}
						}
					}
					if(writeToFile(defaultPath,categories)){	
						printf("Successfully deleted\n");
					}
				}

				free(name);
				list_categories_free_all(categories);
				free(defaultPath);
				return 0;

			}
			// Direct mode for reminders
			else{
				struct List_categories *categories=readFromFile(defaultPath);
				for(int i=0;i<categories->size;i++){
					for(int j=0;j<categories->data[i]->reminders->size;j++){
						if(strcmp(categories->data[i]->reminders->data[j]->name,name)==0){
							free(categories->data[i]->reminders->data[j]->name);
							categories->data[i]->reminders->data[j]->name=strdup("__deleted__");	
						}
					}
				}

				if(writeToFile(defaultPath,categories)){
					printf("Successfully deleted\n");
				}

				free(name);
				list_categories_free_all(categories);
				free(defaultPath);
				return 0;
			}
		}
		/*

		COMMAND help

		*/
		else if(strcmp(argv[1],"help")==0 || strcmp(argv[1],"-h")==0){
			displayHelp();	
			free(defaultPath);
			return 0;
		}
		/*

		COMMAND setp 
		Flags: -f -n -p

		 */
		else if(strcmp(argv[1],"setp")==0){

			// Flags parser
			char *name=strdup("__none__");
			int priority=-1;
			for(int i=2;i<argc;i++){
				if(strcmp(argv[i],"-n")==0){ 
					if(i+1<argc){
						free(name);
						name=strdup(argv[i+1]);
					}
				}
				else if(strcmp(argv[i],"-p")==0){ 
					if(i+1<argc){
						priority=atoi(argv[i+1]);
					}
				}
				else if(strcmp(argv[i],"-f")==0){ 
					if(i+1<argc){
						free(defaultPath);
						defaultPath=strdup(argv[i+1]);
					}
				}
			}				

			// Interactive mode
			if(strcmp(name,"")==0 || priority>3 || priority<0){
				free(name);
				name=readFromPrompt(1,"Name: ");
				char *line=readFromPrompt(2,"Priority in [0,1,2,3]: ");
				priority=atoi(line);
				if(priority>3 || priority<0){
				 	printf("Invalid priority.\n"); 
					printf("Must be in [0,1,2,3].\n");
				}
				free(line);
			}

			struct List_categories *categories=readFromFile(defaultPath);	
			for(int i=0;i<categories->size;i++){
				for(int j=0;j<categories->data[i]->reminders->size;j++){
					if(strcmp(categories->data[i]->reminders->data[j]->name,name)==0){
						categories->data[i]->reminders->data[j]->priority=priority;
						i=categories->size;
						break;
					}
				}
			}

			if(writeToFile(defaultPath,categories)){
				printf("Successfully changed\n");
			}
				
			free(name);	
			list_categories_free_all(categories);
			free(defaultPath);
			return 0;
		}
	}
	
	printf("\nUnknown command\n");
	printf("See 'uxtodo help'.\n\n");
	free(defaultPath);
	return 0;
}


struct List_categories* readFromFile(const char *path){
	FILE *file;
	file=fopen(path,"r");

	if(file == NULL){
		file=fopen(path,"w+");
		fclose(file);
		struct List_categories *categories=list_categories_init();
		return categories;
	}

	struct List_categories *categories=list_categories_init();

	char line[1024];
	while(fgets(line,1024,file)!=NULL){
		if(line[0]=='c'){
			struct category *newCategory=malloc(sizeof(struct category));
			char* name=strndup(line+2,strlen(line)-2);
			newCategory->name=strip(name);
			newCategory->reminders=list_reminders_init();
			list_categories_add(categories,newCategory);
		}
		else if(line[0]=='r'){
			struct reminder *newReminder=malloc(sizeof(struct reminder));
			char* name=strndup(line+4,strlen(line)-4);
			newReminder->name=strip(name);
			char *priority=strndup(line+2,1);
			newReminder->priority=atoi(priority);
			free(priority);
			list_reminders_add(categories->data[categories->size-1]->reminders,newReminder);
		}
	}
	fclose(file);

	// Output must be free
	return categories;
}

char* readFromPrompt(int autoCompleteMode,const char *prompt){

	if(autoCompleteMode==1){
		rl_attempted_completion_function=autoCompleteReminders;
		rl_attempted_completion_over = 1;
	}
	else if(autoCompleteMode==0){
		rl_attempted_completion_function=autoCompleteCategories;
		rl_attempted_completion_over = 1;
	}

	// Output must be free
	return strip(readline(prompt));
}

char* strip(char *str_){
	char *str;

	if(str_[strlen(str_)-1]==10){
		char *str__=strndup(str_,strlen(str_)-1);
		free(str_);
		str_=str__;
	}	
	int i;
	for(i=strlen(str_)-1;i>=0;i--){
		if(str_[i]!=' '){
			break;
		}
	}
	str=strndup(str_,i+1);	
	free(str_);
	
	// Output must be free
	return str;
}

/*

AUTOCOMPLETE

*/
char** autoCompleteCategories(const char *text, int start, int end){
	return rl_completion_matches(text, autoCompleteCategories_);
}

char** autoCompleteReminders(const char *text, int start, int end){
	return rl_completion_matches(text, autoCompleteReminders_);
}

char* autoCompleteCategories_(const char *text,int state){
	struct List_categories *categories=readFromFile(defaultPath);
	if(!state){
		for(int i=0;i<categories->size;i++){
			bool match=true;
			for(int c=0;c<strlen(text);c++){
				if(c>=strlen(categories->data[i]->name) || text[c]!=categories->data[i]->name[c]){
					match=false;	
				}
			}
			if(match){
				char* name=strdup(categories->data[i]->name);
				list_categories_free_all(categories);
				return name;
			}
		}
	}
	list_categories_free_all(categories);
	return NULL;
}

char* autoCompleteReminders_(const char *text,int state){
	struct List_categories *categories=readFromFile(defaultPath);
	if(!state){
		for(int j=0;j<categories->size;j++){
			for(int i=0;i<categories->data[j]->reminders->size;i++){
				bool match=true;
				for(int c=0;c<strlen(text);c++){
					if(c>=strlen(categories->data[j]->reminders->data[i]->name) || text[c]!=categories->data[j]->reminders->data[i]->name[c]){
						match=false;	
					}
				}
				if(match){
					char* name=strdup(categories->data[j]->reminders->data[i]->name);
					list_categories_free_all(categories);
					return name;
				}
			}
		}
	}
	list_categories_free_all(categories);
	return NULL;
}

bool writeToFile(const char *path,struct List_categories *categories){
	FILE *file;
	file=fopen(path,"w+");

	if(file==NULL){
		perror("Error while writing the file\n");
		exit(1);
	}

	for(int i=0;i<categories->size;i++){
		if(strcmp(categories->data[i]->name,"__deleted__")!=0 && categories->data[i]->reminders->size!=0){
			fprintf(file,"c ");
			fprintf(file,categories->data[i]->name);
			fprintf(file,"\n");
			for(int j=0;j<categories->data[i]->reminders->size;j++){
				if(strcmp(categories->data[i]->reminders->data[j]->name,"__deleted__")!=0){ 
					fprintf(file,"r ");
					fprintf(file,"%d",categories->data[i]->reminders->data[j]->priority);
					fprintf(file," ");
					fprintf(file,categories->data[i]->reminders->data[j]->name);
 					fprintf(file,"\n");
				}
			}
		}

	}

	fclose(file);
	return true;
}

void displayReminders(struct List_categories *categories){

	if(categories->size==0 || (categories->size==1 && categories->data[0]->reminders->size==0)){
		printf("\nYou have nothing to do.\n");
		printf("See 'uxtodo help'.\n\n");
		return;
	}

	printf("\nToDo list:\n");
	for(int i=0;i<categories->size;i++){

		if(strcmp(categories->data[i]->name,"__deleted__")!=0){
			if(categories->data[i]->reminders->size>0){
				printf(categories->data[i]->name);
				printf("\n");
			}
			for(int j=0;j<categories->data[i]->reminders->size;j++){

				const char *color=categories->data[i]->reminders->data[j]->priority==1 ? color1 :
						categories->data[i]->reminders->data[j]->priority==2 ? color2 :
						categories->data[i]->reminders->data[j]->priority==3 ? color3 : "";

				if(j==categories->data[i]->reminders->size-1){
					printf("└── ");
					printf(color);
					printf(categories->data[i]->reminders->data[j]->name);
					printf(defaultColor);
					printf("\n\n");
				}
				else{
					printf("├── ");
					printf(color);
					printf(categories->data[i]->reminders->data[j]->name);
					printf(defaultColor);
					printf("\n");
				}
			}
		}
	}
}

void displayHelp(){

	printf("\nCOMMANDS\n\n");

	printf(color3);printf("todo\n\n");printf(defaultColor);
	printf("Will show reminder list. If -c is provided, it will show only\nthose of the specified categories.\n");
	printf(boldColor);printf("Flags : [-f otherFilename] [-c categoryName1 categoryName2 ...]\n\n");printf(defaultColor);

	printf(color3);printf("todo help\n\n");printf(defaultColor);
	printf("Will show how to use this software.\n\n");

	printf(color3);printf("todo add\n\n");printf(defaultColor);
	printf("Add a reminder. If -n is not provided, it will ask for name,\ncategory and priority.\n");
	printf(boldColor);printf("Flags : [-f otherFilename] [-n reminderName] [-c categoryName]\n[-p priorityInteger in [0,3]]\n\n");printf(defaultColor);

	printf(color3);printf("todo del\n\n");printf(defaultColor);
	printf("Delete a reminder. If -n is not provided, it will ask for name.\n");
	printf(boldColor);printf("Flags : [-f otherFilename] [-n reminderName]\n\n");printf(defaultColor);

	printf(color3);printf("todo clear\n\n");printf(defaultColor);
	printf("Clear a category. If -c is not provided, it will ask if you\nwant to clear all categories.\n");
	printf("If -c is provided without argument, it will ask for a category name.\n");
	printf(boldColor);printf("Flags : [-f otherFilename] [-c categoryName]\n\n");printf(defaultColor);

	printf(color3);printf("todo setp\n\n");printf(defaultColor);
	printf("Set reminder priority. If -n is not provided, it will ask\nfor name and priority in interactive mode.\n");
	printf(boldColor);printf("Flags :  [-f otherFilename] [-n reminderName] [-p priorityInteger\nin [0,3]]\n\n");printf(defaultColor);

}

