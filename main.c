#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>
#include <libgen.h>

#define LOG_FILE "/home/hossameleraqi/labs OS/lab1/log.txt"


void reap_child_zombile(){
   
    while (1)
    {
       int status;
       pid_t son= waitpid((pid_t) (-1),&status,WNOHANG);
       if(son==0){
        break;
       }
       else if(son==-1){
        break;
       }
    }
    
   
}
void write_to_log_file(char *massage){
    FILE *lf;
    lf=fopen(LOG_FILE,"a");
    fprintf(lf,"%s",massage);
     fclose(lf);

}

void read_input(char input []){                 //reads the input and put \0 in the last instead of \n
fgets(input,1000,stdin);
input[strcspn(input,"\n")]='\0';

}
char *remove_spaces(char *input){                   // remove spaces in first and last of input 
while (input[0]==' '){
  input++;
}
while (input[strlen(input)-1]==' '){
  input[strlen(input)-1]='\0';
}
return input;
}

char *evaluate_expression (char * original , int foundat ){             // exchange variables after $ with their values 
  char variables[1000]; int cvar=0;
  char res[1000]; int cres=0;

  for (int i = 0; i < foundat; i++)                  // put parts before $ in res
  {
    res[cres++]=original[i];
  }
  
  for (int i = foundat+1; i < strlen(original); i++)          // put variables after $ in variables 
  {
    if(original[i]!='$'&&original[i]!= ' '&&original[i]!= '"'){
      variables[cvar++]=original[i];
    }
    else {
      variables[cvar++]='\0';
      break;
    }
  }
  char *value =getenv(variables);                          // put values of variables after $
  if(value!= NULL){
    for (int i = 0; i < strlen(value); i++)
    {
      res[cres++]=value[i];
    }
  }

  for(int i=foundat+ strlen(variables)+1;i<strlen(original);i++){              //put all the last parts in res 
    res[cres++]=original[i];
  }
  res[cres++]='\0';
  strcpy(original,res);
  return original;                                                            //after added \0 in the end and copy all res to orginal back it 
}

void parse_input(char input[], char *param[]){
  char *token,*copy ,*delim=" ";
  int count=0;

  replaceAll :                                                   //this part for change all variables after $ with their values 
  {
    int flag =0;
    int i=0;
    while (i<strlen(input)){
      if(input[i]=='$'){
        flag=1;
        break;
      }
      i++;
    }
    if(flag){
      input=evaluate_expression(input,i);
      goto replaceAll ;
    }
  }

 copy =strdup(input);
 token=strtok(copy,delim);
 char *tmp = strdup(token);
 if (strcmp(tmp,"cd")==0 || strcmp(tmp,"export")==0 || strcmp(tmp,"echo")==0 )
 {
  while (token !=NULL && count<256)
  {
    param[count++]=strdup(token);
    token=strtok(NULL,"");
  }
 }
 else{
  while (token !=NULL && count<256)
  {
     param[count++]=token;
    token=strtok(NULL," ");
  }
 }
}

void cd (char *parameters[]) {
    char path[1000]; char *ptr; int c = 0; char prev[1000];
    int n=0;
    if(parameters[1] == NULL || strcmp(parameters[1], "~") == 0) {
       printf("%s\n",getenv("HOME"));
        chdir(getenv("HOME"));
        return;
    }
    else if(*parameters[1] == '~') {
        ptr = getenv("HOME");
        for (int i = 0; i < strlen(ptr); i++) {
            path[c++] = ptr[i];
        }
        path[c++] = '\0';
        parameters[1]++;
        strcat(path, parameters[1]);
          printf("%s\n",path);
        if (chdir(path) != 0) {
            perror("cd");
            usleep(200000);
        }
    }
     else if(strcmp(parameters[1] , "..")==0){
     //  printf("h\n");
       getcwd(path,1000);
     //  printf("%s\n",path);
       printf("%s\n",dirname(path));
        if (chdir(dirname(path)) != 0) {
            perror("cd");
            usleep(200000);
        }
    }
    else {
        if (chdir(parameters[1]) != 0) {
            perror("cd");
            usleep(200000);
        }
    }
}

// function to execute export command
void export (char command[]) {
    char *name, *value;
    name = strsep(&command, "=");
    value = strdup(command);
    if (value[0] == '"' && value[strlen(value) - 1] == '"') {
        value[strlen(value) - 1] = '\0';
        value++;
    }
   // printf("%s\n",name);
  //  printf("%s",value);
    setenv(name, value, 1);
}
 void echo(char rule[]){
    if(rule[0]=='"'&&rule[strlen(rule)-1]=='"'){
      rule[strlen(rule)-1]='\0';
      rule++;
    }
    printf("%s\n",rule);
 }

void excute_shell_builtin(char *param[]){
  char *command = param[0];

  if (strcmp(command,"cd")==0){
    cd(param);
  }

 else if (strcmp(command,"echo")==0){
    echo(param[1]);
  }

  else if (strcmp(command,"export")==0){
    export(param[1]);
  }

}
void execute_command(char *parameters[], int background) {
    pid_t child_id = fork();
    int status;
    if (child_id < 0) {
        perror("Forking failed");
        exit(1);
    }
    else if (child_id == 0) {
        if (background) {
            printf("process: %d\n", getpid());
            usleep(20000);
        }
        execvp(parameters[0], parameters);
        printf("%s: command not found\n", parameters[0]);
        exit(1);
    }
    else {
        if (background) {
            waitpid(child_id, &status, WNOHANG);  // dont wait for any child
        }
        else {
            printf("wait the child to be killed xd\n");
            waitpid(child_id, &status, 0);              // wait for the child
            printf("lets back again !\n");
        }
    }
}
void shell(){

int commant_is_not_exist=0;
do
{
  int child=0;
  char input[1000]={};
  char *param[1000]={};
  int state =0;


  read_input(input);
 
 
  strcpy(input,remove_spaces(input));
   int i=0;
  // while(input[i]!='\0'){
  //   printf("%c\n",input[i]);
  //   i++;
  // }
  // printf("%s\n",input);
  if(input[strlen(input)-1]=='&'){
    child=1;
    input[strlen(input)-1]='\0';
  }



  parse_input(remove_spaces(input),param);

  // while(param[i]!='\0'){
  //   printf("%s\n",param[i]);
  //   i++;
  // }
  if(strcmp(param[0],"exit")==0){
    commant_is_not_exist=1;
    state=1;
  }
  else  if(strcmp(param[0],"cd")==0 || strcmp(param[0],"export")==0  || strcmp(param[0],"echo")==0){
    state=2;
  }
  else {
    state=3;
  }


  switch (state)
  {
  case 1:
    break;
  
  case 2:
    excute_shell_builtin(param);
    break;

  case 3 :
     execute_command(param,child);
     break;
  }
  
} while (!commant_is_not_exist);

exit(0);

}


void setup_environment(){
chdir(getenv("HOME"));
}
void on_child_exit(){
 reap_child_zombile();
write_to_log_file("Child process was terminated\n");
}



int main (){
 fclose(fopen(LOG_FILE , "w"));
 signal(SIGCHLD, on_child_exit);
 setup_environment();
 shell();
  return(0);

}