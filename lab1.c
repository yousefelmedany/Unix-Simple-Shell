#include <stdio.h>
#include <string.h>
#include<unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#define SIZE 50
char dir[256];
char path[]="/home/yousef";

void execute_command(char **command); 
void execute_shell_builtin(char **command);
char* evaluate_exp(char *arr);


void setup_environment(){ //this function is used to setup the home working directory
    chdir(path);

}
void write_log_file(){  //this function is used to write within the log file when a signal is sent by a child process to the OS upon termination to clear up the resources
    int status;
    waitpid(-1,&status,WNOHANG);
    FILE* myfile=fopen("/home/yousef/logfile.txt","a");
    fprintf(myfile,"CHILD TERMINATED SUCCESSFULLY!\n");
    fclose(myfile);
}


int gettype(char *type){ //this function is just used to get the type of command. command 1 is either cd, export or echo.command 2 is anything else.
    if(!strcmp(type,"cd") || !strcmp(type,"export") || !strcmp(type,"echo")){
        return 1;
    }else{
        return 2;
    }

}
void shell(){  //this is the main shell which interacts with the user
    char cmd[SIZE];
    char *exp[10];
    int i,x=0;
    do{
        i=0;
        getcwd(dir,256);
        printf("\033[31mYousef@Yousef_Terminal:\033[0m\033[34m%s\033[0m\033[38m$ \033[0m",dir);
        fgets(cmd,SIZE,stdin);
        char* res=evaluate_exp(cmd);
        strcpy(cmd,res);
        char *token=strtok(cmd,"\n");
        token=strtok(cmd," ");
        while(token!=NULL){
            exp[i++]=token;
            token=strtok(NULL," ");
        }
        exp[i]=NULL;
        int check= gettype(exp[0]);
        switch (check)
        {
            case 1:
                execute_shell_builtin(exp);
                break;
            case 2:
                execute_command(exp);
                break;
        }

    }
    while(strcmp(exp[0],"exit"));

}

void cd_handle(char **cdcommand){ //this function is used to handle and execute the cd command with all of its cases.
    if(cdcommand[2]!=NULL){
        printf("cd: too many arguments\n");
    }else{
        if(!strcmp(cdcommand[1],"..")){
            chdir("..");
            getcwd(dir,256);
        }else if(!strcmp(cdcommand[1],"~")){
            chdir(path);
            getcwd(dir,256);
        }else{
            int c=chdir(cdcommand[1]);
            if(c!=0){
                printf("cd: %s: No such file or directory\n",cdcommand[1]);
            }else{
                getcwd(dir,256);
            }
        }
    }
}

void export_handle(char **expcommand){ //this function is used to handle the export command.
    char *exp[]={NULL,NULL};
    char *res=strtok(expcommand[1],"=");
    exp[0]=res;
    res=strtok(NULL,"=");
    exp[1]=res;
    if(exp[1]==NULL){
        printf("export: '%s': not a valid argument\n",exp[0]);
        return;
    }
    char var_name[30],var_value[50];
    strcpy(var_name,exp[0]);
    strcpy(var_value,exp[1]);
    if(var_value[0]=='\"'){
        char *p1=var_value;
        p1=strtok(p1,"\"");
        strcpy(var_value,p1);
        int h=2;
        while(expcommand[h]!=NULL){
            strcat(var_value," ");
            strcat(var_value,expcommand[h++]);
        }
        char *p2=var_value;
        int len=strlen(p2);
        p2[len-1]='\0';
    }
    setenv(var_name,var_value,1);
}


void echo_handle(char **command){ //this function is used to handle the echo command.
    char value[100];
    strcpy(value,command[1]);
    int j=2;
    while(command[j]!=NULL){
        strcat(value," ");
        strcat(value,command[j++]);
    }
    char *ptr=value;
    char res[100]={};
    char *tok=strtok(ptr,"\"");
    while(tok!=NULL){
        strcat(res,tok);
        tok=strtok(NULL,"\"");
    }
    printf("%s\n",res);
}


char* evaluate_exp(char *arr){ //this function is usd to substitute the $ signs within a command with ts corresponding environment variable.
    char str[100]={};
    char substr[10]={};
    int i=0,j=0,h=0;
    int length=strlen(arr);
    while(i<length-1){
        if(arr[i]=='$'&&arr[i+1]!=' '&&arr[i+1]!='\n'){
            i++;
            h=0;
            while(arr[i]!='$'&&arr[i]!=' '&&arr[i]!='\n'&&arr[i]!='\"'){
                substr[h++]=arr[i++];

            }
            char *res=getenv(substr);
            if(res!=NULL){
                int len=strlen(res);
                for(int x=0;x<len;x++){
                    str[j++]=*res;
                    res++;
                }
            }
        }else{
            str[j++]=arr[i++];
        }
    }
    char* ptr=str;
    return ptr;
}


void execute_shell_builtin(char **command){ //this function is use to execute one of the 3 builtin commands either cd, export or echo.
    if(!strcmp(command[0],"cd")){
        cd_handle(command);
    }else if(!strcmp(command[0],"export")){
        export_handle(command);
    }else{
        echo_handle(command);
    }
}

void execute_command(char **command){ //this function is used to handle and execute any other command rather than the builtin ones.
    int cid=fork();
    if(cid==0){
        if(!strcmp(command[0],"exit")){
            kill(0,SIGKILL);
            exit(0);
        }
        int c=execvp(command[0],command);
        if(c<0){
            printf("%s: command not found\n",command[0]);
        }
    }else{
        int status,pid;
        if((command[1]!=NULL)&&(*command[1]=='&')){
            if(strlen(command[1])!=1){
                command[1]++;
                printf("%s: command not found\n",command[1]);
            }
        }
        else{
            pid=waitpid(cid,&status,0);
            if(pid<0){
                printf("wait failed");
            }
        }
    }
}

int main(){
    signal(SIGCHLD,write_log_file); //this is a bultin function that performs the write_log_file function upon recieving a signal from a child process upon termination to prevent the existence of any zombie process. 
    setup_environment();
    shell();
}


