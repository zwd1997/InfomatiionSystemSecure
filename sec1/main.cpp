#include<stdio.h>
#include<string.h>

void overflow(const char* input){
    char buff[8];
    printf("Virtual address of 'buff'=0x%p\n",buff);
    strcpy(buff,input);
}

int fun(){
    printf("Function 'fun' has been called without an explictly invocation.\n");
    printf("Buffer Overflow attack succeeded!\n");
    //
    return 0;
}

int main(int argc,char* argv[]){
    printf("Virtual address of 'overflow'=0x%p\n",overflow);
    printf("Virtual address of 'fun'=0x%p\n",fun);
    char input[]="AAAAAAAAAAAAAAAAAAAAAAAA\xee\x05\x40\x00";
    overflow(input);
    return 0;
}
