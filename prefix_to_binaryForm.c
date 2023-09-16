#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct binIPwithLen
{
    char ip[32];
    int len;
}binIPwithLen;

char* dec_to_bin (int dec,char *binar)
{
    char *ptr;
    ptr=binar;
    int count=0;
    char *temp;
    int i;

    while(dec>=1)
    {
        if(dec%2==1)
        {
            ptr[count]='1';
        }
        else
        {
            ptr[count]='0';
        }            
        dec/=2;
        count++;
    }
    for(i=0;i<(count+1)/2;i++)
    {
        *temp=ptr[i];
        ptr[i]=ptr[count-1-i];
        ptr[count-1-i]=*temp;
    }
    for(i=0;i<count;i++)
    {
        ptr[7-i]=ptr[count-1-i];
    }
    for(i=0;i<8-count;i++)
    {
        ptr[i]='0';
    }

    return ptr;
}
int main()
{
    char *bina = malloc(8 * sizeof(char));
    char *ptr1;
    int i;
    int j=0;
    char StrLine[1024];
    char leh[2];
    binIPwithLen *ip_len=NULL;
    ip_len = (binIPwithLen*)malloc(sizeof(binIPwithLen));    

    const char* filename = "prefix_20211122.txt";
    const char* filename1 = "prefix_binary_length.txt";
    FILE* input_file = fopen(filename, "r");
    FILE* fp = fopen(filename1, "w");

    if (!input_file)
    {
        exit(EXIT_FAILURE);
    }
        
    while (fgets(StrLine, 1024, input_file))
    {
        int n=0;
        char length[1024];
        char line[1]="\n";
        strncpy(length,StrLine,sizeof(length));

        char *test = strtok(StrLine, "/");        
        while (test != NULL) 
        {   
            char *seg = strtok(test, ".");
            while (seg != NULL)
            {
                ptr1=dec_to_bin(atoi(seg),bina);
                for(i=0;i<8;i++)
                {
                    ip_len->ip[n]=ptr1[i];
                    n++;
                }
                seg = strtok(NULL, ".");
            }
            fwrite(ip_len->ip, 1, sizeof(ip_len->ip), fp);
            fwrite(" ", 1, 1, fp);
            test = strtok(NULL, "/");
        }

        char *len = strtok(length, " ");
        while (len != NULL) 
        {   
            char *l = strtok(len, "/");
            l = strtok(NULL, "/");
            ip_len->len=atoi(l);
            sprintf(leh,"%d",ip_len->len);
            if (ip_len->len==0)
            {
                fwrite(leh, 1, 1, fp);
            }
            else
            {
                fwrite(leh, 1, sizeof(leh), fp);
            }                
            fwrite(line, 1, sizeof(line), fp);
            len = strtok(NULL, "/");       
        }
        
        if (j%1000==0)
        {
            printf("prefix %d done\n",j);
        }      
        j++;      
    }

    fclose(input_file);
    fclose(fp);
    exit(EXIT_SUCCESS);

    free(bina);
    free(ip_len);
    return 0;
}