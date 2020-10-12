#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<dirent.h>
#include <limits.h>
#include<readline/readline.h>
#include <sys/stat.h>
#include <ftw.h>

char input[20]; // Array to hold the user input
char sub[10];   // Array to hold the substrings of the user input
int len=0;  //  Length of user input
int flag=0; // Variable to store the position at which the Linux command ends and arguments begin in the user input
char sub1[20], sub2[20];    //Stores the two arguments in a multi argument command


int count_arg(char *sub){   // Function to count the number of arguments excluding the linux command in the user input
    int i;
    int count=0;
    for (i = 0; sub[i] != '\0'; i++){
        if (sub[i] == ' ' && sub[i+1] != ' ')
            count++;    
    }
    return count+1;
}

void mv_sub(char *str,char *sub1 ,char *sub2){ // Function to extract both the arguments in separate arrays incase 2 arguments are passed with certain linux commands 
    int i=0, k=0;
    while(str[i]!=' ' && str[i]!='\0'){
        sub1[i]=str[i];
        i++;
    }
    sub1[i]='\0';   //sub1[] contains the first argument
    i++;
    while(i<strlen(sub)){
        sub2[k]=str[i];
        i++;
        k++;
    }
    sub2[k]='\0';   //sub2[] contains the second argument
    return;
}

void arg_sub(char *str,char *sub ,int beg, int n) // Function will extract the argument other than the linux command and store it in the sub[] array
{
   int i=0;
   if(n==0)    // Take string till the end if 0 is passed as n
       n=strlen(str)-beg;
   while(i<n){
       sub[i]=str[beg + i];
       i++;
   }
   sub[i]='\0';
   return;
}

void command_sub(char *str, char *sub){ // Function will extract the linux command and store it in the sub[] array
    int i=0;
    while (str[i]!=' ' && str[i]!='\0'){
        sub[i]=str[i];
        i++;
    }
    sub[i]='\0';
    flag=i+1;
    return;
}
 
void ls2(char *directory){ // Implementation of ls linux command to print the contents of a directory
    DIR *d;
    struct dirent *dir;
    d = opendir(directory);
    if(d){
        while ((dir = readdir(d)) != NULL){ // Read if directory exists
            printf("%s\n", dir->d_name);
        }       
    }
    else if(d==NULL){
        printf("Cannot open directory!\n");
        return;
    }   
    closedir(d);    // Close the directory
    return;
}

void ls1(){ // Function will check if any arguments are passed along with ls and will call ls2 accordingly
    char ch=input[2];
    if(len==2)  // If no arguments passed with ls command, call ls2 on current directory
        ls2(".");
    else if(len==3 && ch==32)   // If no arguments passed with ls command followed by a space, call ls2 on current directory
        ls2(".");
    else if(len>3 && ch==32){   // If an argument is passed, call ls2 on that directory
        strcpy(sub," ");
        arg_sub(input, sub, flag,0);
        ls2(sub);
    }
    return;       
}

void cat(){ // Implementation of cat linux command to print the contents of the file specified

    FILE *fp;
    int maxline = 1024;
    char line[maxline];
    strcpy(sub," ");
    arg_sub(input, sub, flag,0); // Get arguments passed
    if(input[flag]==' ' || input[flag]=='\0'){  // Check if argument specified
        printf("\nFile not specified!\n");
        return;
    }
    if((fp = fopen(sub, "rb")) == NULL){    //If file is not readable
        printf("\nCannot read file!\n");
        return;
    }
    while(fgets(line, maxline, fp)){ // Print the file passed as argument 
        printf("%s", line);
    }
    fclose(fp); // Close the file
    return;
}

void pwd(){ // Implementation of pwd linux command to print current path
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) { // getcwd() will store the current path in cwd[] character array passed
        printf("\nCurrent working dir: %s\n", cwd);
    } 
    else {
        perror("getcwd() error");   // If getcwd() fails
    }

    return;
}

void cd(){  // Implementation of cd linux command to navigate to a different path
    char cwd[PATH_MAX];
    if(input[flag]==' ' || input[flag]=='\0'){  // Check if argument is passed
        printf("\nDirectory not specified!\n");
        return;
    }
    getcwd(cwd, sizeof(cwd));   // Get cuurent working directory
    arg_sub(input, sub, flag, 0);   // Get arguments passed
    strcat(cwd, "/"); 
    strcat(cwd, sub);   // Concatenate cwd with the argument passed by the user

    if(chdir(cwd)==0){  // Change path
        printf("\nCurrent Working Directory changed to %s\n", getcwd(cwd, sizeof(cwd)));   
    }
    else
    {
        printf("\nCannot change directory! Check directory name!\n");   // If chdir() fails.
    }
    return;
}

static int rmFiles(const char *pathname, const struct stat *sbuf, int type, struct FTW *ftwb)
{
    if(remove(pathname) < 0)
    {
        perror("ERROR: remove");
        return -1;
    }
    return 0;
}

void rm(){  // Implementation of rm linux command to remove a file
    if(input[flag]==' ' || input[flag]=='\0'){  // Check if argument is passed
        printf("\nFile not specified!\n");
        return;
    }
    arg_sub(input, sub, flag, 0);   // Get arguments passed and store it in sub[] array
    if(sub[0]=='-' && sub[1]=='r'){ // If rm -r is the command passed
        mv_sub(sub, sub1, sub2);
        if (nftw(sub2, rmFiles,10, FTW_DEPTH|FTW_MOUNT|FTW_PHYS) < 0) // Use ntfw() to remove a non-empty directory
        {
            printf("ERROR: ntfw");
        }
        return;
    }   
    if(remove(sub)){    // If function returns error
        printf("\nError deleting file!\n");
        return;
    }
    else{   // Delete the file specified as the argument
       printf("\nDeleted File %s\n", sub);
    }
    
    return;
}

void makedir(){ // Implementation of mkdir linux command to create a directory
    if(input[flag]==' ' || input[flag]=='\0'){  // Check if argument is passed
        printf("\nName not specified!\n");
        return;
    }
    arg_sub(input, sub, flag, 0);   // Get arguments passed and store it in sub[] array
    if(mkdir(sub, (S_IRWXG | S_IRWXU))==0){ // Create directory named as per the argument passed by the user
        printf("\nDirectory '%s' Created!\n", sub);
    }
    else{
        printf("\nError!\n");   // If function returns error
    }   
}

void mv(){  // Implementation of mv linux command to rename a file
    int count=0, i;
    if(input[flag]==' ' || input[flag]=='\0'){
        printf("\nCheck Syntax!\n");
        return;
    }
    arg_sub(input, sub, flag, 0);      // Extract argument substring of the input as a single string
    char *s = strchr (sub, ' ');    // 's' will point to the first occurence of space in the argument substring
    if (s==NULL){       // Checks if there are exactly 2 arguments or filenames passed
        printf("\nToo few arguments!\n");
        return;
    }
    else{
        for (i = 0; sub[i] != '\0'; ++i) {  // Count number of arguments
            if (sub[i]==' ')
                ++count;
        }
        if(count>1){
            printf("\nToo many arguments!\n");
            return;
        }
    }
    mv_sub(sub, sub1, sub2);    // Extract the 2 arguments passed, into separate arrays
    if(rename(sub1, sub2) == 0){ // Rename second file as the first file
        printf("\nDone!\n");
    }
    else
    {
        printf("\nError! Try Again!\n");
    }
    
   return;
    
}

void cp(){  // Implementation of cp linux command to copy a file to another file
    if(input[flag]==' ' || input[flag]=='\0'){
        printf("\nCheck Syntax!\n");
        return;
    }
    arg_sub(input, sub, flag, 0);   // Extract argument substring of the input as a single string
    mv_sub(sub, sub1, sub2);    // Extract the 2 arguments passed in separate arrays
    int src;    // Source file
    int dst;    // Destination file
    int n;      // Tracking variable
    char buf[1];
    src = open(sub1,O_RDONLY);  // Open source file
    if(src == -1){
        perror("\nCannot find the source file!\n");
        return;
    }

    dst = creat(sub2,777);  // Create destination file if it does not exist
    dst = open(sub2,O_WRONLY);  // Open destination file if it exists
    if(dst == -1){
        perror("\nCannot create or open destination file!\n");
        return;
    }
    else{
        while((n=read(src,buf,1))>0){   // Read source file
            write(dst,buf,1);   // Write the contents read from the source file to the destination file
        }
        printf("\nDone!\n");
    }
    close(src); // Close source file
    close(dst); // Close destination file
    return;
}

void grep(){     // Implementation of grep linux command to search for pattern occurences in a file
    if(input[flag]==' ' || input[flag]=='\0'){
        printf("\nCheck Syntax!\n");
        return;
    }
    char lineBuffer[512];
    FILE *fp = NULL;
    int count = 0;  // count stores number of occurences of pattern
    arg_sub(input, sub, flag, 0);   // Extract argument substring of the input
    mv_sub(sub, sub1, sub2);    // Extract 2 arguments in separate arrays (1 pattern and 1 file name)
    fp = fopen( sub2, "r" );    // Open file specified
    if( ! fp )
    {
        fprintf( stderr, "\nError - unable to open %s\n", sub2 );
        return;
    }

    while( fgets( lineBuffer, 511, fp ) )   // Check if pattern specified is a substring of the contents in the file
    {
        if( strstr( lineBuffer, sub1 ) )
        {
            printf( "\n%s", lineBuffer );
            ++count;    // Count number of occurences of pattern
        }
    }
    fclose( fp );
    printf( "\nFound %d occurrences!\n", count );
    return;
}

void cmod(char path[]){ // Implementation of chmod() command in Linux
    arg_sub(input, sub, flag, 0); 
    mv_sub(sub, sub1, sub2);
    strcat(path,"/");
    strcat(path,sub2);  // Array path now contains the current working directory path
    if(chmod(sub2,strtol(sub1, 0, 8))==0){   // strtol() is used to convert string to long integer
        printf("\nPermissions changed successfully.\n"); 
    }  
    else{
        printf("\nIncorrect arguments of chmod. \nCorrect usage: chmod permission filename\nEg: chmod 777 filename\n");
    }
    return;
}

void backg(){   // Function to enable execution of a program in the background
    char temp[20];
    int i=0;
    while(sub[i]!='&'){ // Extract program name
        temp[i]=sub[i];
        i++;
    }
    temp[i]='\0';
    int f=fork();   // Fork a child process
    if(f < 0){
        printf("\nFork Error!");
        return;
    }
    else if(f==0){
        if(execvp(temp, NULL)< 0){  // Use execvp() to replace the child process with the program passed
            printf("%s: Program not found! Try again.\n", temp);
            exit(0);
        }       
    }
    else{
        return; // Parent process can continue executing while the child runs in the background
    }   
    exit(0); // Child process will exit after completion
}

// Main Function
int main(){ 
    char cwd[PATH_MAX];
    printf("\nWelcome to Ash!\nType 'exit' to exit the shell.\n");
    do{
        printf("\n%s > ", getcwd(cwd, sizeof(cwd)));    // Print current working directory 
        gets(input);    // Get user input
        strcpy(sub," ");
        len = strlen(input);    // len stores the length of the user input

        command_sub(input,sub); // Extracts the linux command part of the user input and stores it in sub[] array

        int NoOfInbuilt = 4, i, over=0; 
        char* Inbuilt[NoOfInbuilt]; // Array to store inbuilt binaries (Can be expanded as required)
  
        Inbuilt[0] = "ps"; 
        Inbuilt[1] = "pmap"; 
        Inbuilt[2] = "wget"; 

        for (i = 0; i < NoOfInbuilt; i++) {     // Check if input contains commands in the Inbuilt[] array
            if (strcmp(sub, Inbuilt[i]) == 0) { 
                system(input);
                over=1;
                break; 
            } 
        } 
        if(over==1){
            continue;   // If command matches with any of the inbuilt binaries, skip the rest
        }
        if(strcmp(sub,"ls")==0){    // Call implementation of ls if the command part of the user input matches with "ls"
            ls1();
        }
        else if (strcmp(sub,"cat")==0)
        {
            cat();
        }
        else if (strcmp(sub,"pwd")==0){
            pwd();
        }
        else if (strcmp(sub,"cd")==0)
        {
            cd();
        }
        else if (strcmp(sub,"rm")==0)
        {
            rm();
        }
        else if (strcmp(sub,"mkdir")==0)
        {
            makedir();
        }
        else if (strcmp(sub,"mv")==0)
        {
            mv();
        }
        else if (strcmp(sub,"cp")==0)
        {
            cp();
        }
        else if (strcmp(sub,"grep")==0)
        {
            grep();
        }
        else if (strcmp(sub,"chmod")==0)
        {
            cmod(cwd);
        }
        else if (sub[flag-2]=='&')
        {
            backg();
    
        }

        else if (strcmp(sub,"exit")==0) // If user types in exit, exit the program
        {
            return 0;
        }
        else    // If user types in any other command, ask the user to check the command used
        {
            printf("\nCheck command!\n");
        }
        
        
   }while(1);   // Loop continues execution untill user enters exit command

   return 0;
   
}
