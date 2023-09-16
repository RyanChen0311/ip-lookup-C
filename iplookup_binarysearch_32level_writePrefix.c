#include <stdio.h>
#include <stdlib.h>
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */
#include <math.h>
#include <string.h>

#define ROW 32
#define COL 1000000
#define STRING 32

int update=0;

typedef struct binIPwithLen
{
    char ip[32];
    int len;
}binIPwithLen;

struct bin_tree {
int data;
struct bin_tree * right, * left;
};
typedef struct bin_tree node;

struct prefix_info {
int index;
int len;
char Bstring[32];
};
typedef struct prefix_info prefix_info;

void insert(node ** tree, prefix_info val,int level)
{
    int n=0;
    if (level==val.len)
    {
        //printf("to bottom!\n");
        node *temp = NULL;
        if(!(*tree))
        {
        temp = (node *)malloc(sizeof(node));
        temp->left = temp->right = NULL;
        temp->data = val.index;
        *tree = temp;
        }
        return;
    }

    node *temp = NULL;
    if(!(*tree))
    {
        temp = (node *)malloc(sizeof(node));
        temp->left = temp->right = NULL;
        *tree = temp;
    }
    if(val.Bstring[level]=='1')
    {
        n=1;
    }
    else if(val.Bstring[level]=='0')
    {
        n=0;
    }
    if(n == 0)
    {
        insert(&(*tree)->left, val, level+1);
    }
    else if(n == 1)
    {
        insert(&(*tree)->right, val, level+1);
    }
}

void deltree(node * tree)
{
    if (tree)
    {
        deltree(tree->left);
        deltree(tree->right);
        free(tree);
    }
}

node* search(node ** tree, char* val, int level)
{
    int n=0;

    if(!(*tree))
    {
        return NULL;
    }

    if(((*tree)->data)<1000000 && ((*tree)->data)>0)
    {
        update=(*tree)->data;
        //printf("update=%d\n",update);
    }

    if(val[level]=='1')
    {
        n=1;
        //printf("1\n");
    }
    else 
    {
        n=0;
        //printf("0\n");
    }

    if(n == 0)
    {
        search(&((*tree)->left), val, level+1);
    }
    else if(n == 1)
    {
        search(&((*tree)->right), val,level+1);
    }
}

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
        count++;//10-5-2-1-0
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

void prefix2binary(FILE* input_file,FILE* fp)
{
    
    int i=0;
    int j=0;
    
    char leh[2];
    char StrLine[1024];
    char *bina = malloc(8 * sizeof(char));
    char *ptr1;
    
    binIPwithLen *ip_len=NULL;
    ip_len = (binIPwithLen*)malloc(sizeof(binIPwithLen));
    
    printf("Begin to transform prefix2binary...\n");
    fgets(StrLine, 1024, input_file);
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
        /*
        if (j%10000==0)
        {
            printf("prefix %d done\n",j);
        }      
        */
        j++;      
    }
    
    free(bina);
    free(ip_len);
}

void prefix_to_binaryForm(char* region)
{
    FILE* input_file;
    FILE* input_file2;
    FILE* fp;
    FILE* fp2;
    if(!(strncmp(region,"rrc00",5)))
    {
        const char* filename = "./rrc00/prefix_20211122.txt";
        const char* filename2 = "./rrc00/prefix_20211123.txt";
        const char* filename1 = "./iplookup/finalTest/prefix_binary_length_22.txt";
        const char* filename3 = "./iplookup/finalTest/prefix_binary_length_23.txt";
        input_file = fopen(filename, "r");
        input_file2 = fopen(filename2, "r");
        fp = fopen(filename1, "w");
        fp2 = fopen(filename3, "w");
    }
    else if(!(strncmp(region,"rrc01",5)))
    {
        const char* filename = "./rrc01/prefix_20211122.txt";
        const char* filename2 = "./rrc01/prefix_20211123.txt";
        const char* filename1 = "./iplookup/finalTest/prefix_binary_length_22.txt";
        const char* filename3 = "./iplookup/finalTest/prefix_binary_length_23.txt";
        input_file = fopen(filename, "r");
        input_file2 = fopen(filename2, "r");
        fp = fopen(filename1, "w");
        fp2 = fopen(filename3, "w");
    }
    else if(!(strncmp(region,"rrc03",5)))
    {
        const char* filename = "./rrc03/prefix_20211122.txt";
        const char* filename2 = "./rrc03/prefix_20211123.txt";
        const char* filename1 = "./iplookup/finalTest/prefix_binary_length_22.txt";
        const char* filename3 = "./iplookup/finalTest/prefix_binary_length_23.txt";
        input_file = fopen(filename, "r");
        input_file2 = fopen(filename2, "r");
        fp = fopen(filename1, "w");
        fp2 = fopen(filename3, "w");
    }
    else if(!(strncmp(region,"rrc04",5)))
    {
        const char* filename = "./rrc04/prefix_20211122.txt";
        const char* filename2 = "./rrc04/prefix_20211123.txt";
        const char* filename1 = "./iplookup/finalTest/prefix_binary_length_22.txt";
        const char* filename3 = "./iplookup/finalTest/prefix_binary_length_23.txt";
        input_file = fopen(filename, "r");
        input_file2 = fopen(filename2, "r");
        fp = fopen(filename1, "w");
        fp2 = fopen(filename3, "w");
    }
    else if(!(strncmp(region,"rrc05",5)))
    {
        const char* filename = "./rrc05/prefix_20211122.txt";
        const char* filename2 = "./rrc05/prefix_20211123.txt";
        const char* filename1 = "./iplookup/finalTest/prefix_binary_length_22.txt";
        const char* filename3 = "./iplookup/finalTest/prefix_binary_length_23.txt";
        input_file = fopen(filename, "r");
        input_file2 = fopen(filename2, "r");
        fp = fopen(filename1, "w");
        fp2 = fopen(filename3, "w");
    }
    else if(!(strncmp(region,"Final",5)))
    {
        const char* filename = "./iplookup/finalTest/prefix_20211122.txt";
        const char* filename2 = "./iplookup/finalTest/prefix_20211123.txt";
        const char* filename1 = "./iplookup/finalTest/prefix_binary_length_22.txt";
        const char* filename3 = "./iplookup/finalTest/prefix_binary_length_23.txt";
        input_file = fopen(filename, "r");
        input_file2 = fopen(filename2, "r");
        fp = fopen(filename1, "w");
        fp2 = fopen(filename3, "w");
    }
    else
    {
        printf("Input is not one of the five regions.");
    }

    prefix2binary(input_file,fp);
    prefix2binary(input_file2,fp2);

    fclose(input_file);
    fclose(input_file2);
    fclose(fp);
    fclose(fp2);
}


void trace2binary(FILE* input_file,FILE* fp)
{
    char *bina = malloc(8 * sizeof(char));
    char *ptr1;
    int i=0;
    int j=0;
    char StrLine[1024];
    char leh[2];
    binIPwithLen *ip_len=NULL;
    ip_len = (binIPwithLen*)malloc(sizeof(binIPwithLen));    

    printf("Begin to transform trace2binary...\n");
    fgets(StrLine, 1024, input_file);
    while (fgets(StrLine, 1024, input_file))
    {
        int n=0;
        char length[1024];
        char line[1]="\n";
        strncpy(length,StrLine,sizeof(length));

        char *test = strtok(StrLine, " ");        
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
            fwrite("\n", 1, 1, fp);
            test = strtok(NULL, " ");
        }
        /*
        if (j%1000==0)
        {
            printf("trace %d done\n",j);            
        }   
        */
        j++;
    }
    
    free(bina);
    free(ip_len);
}
void trace_to_binaryForm(char* region)
{
    FILE* input_file;
    FILE* input_file2;
    FILE* fp;
    FILE* fp2;
    if(!(strncmp(region,"rrc00",5)))
    {
        const char* filename = "./rrc00/trace_20211122.txt";
        const char* filename2 = "./rrc00/trace_20211123.txt";
        const char* filename1 = "./iplookup/finalTest/trace_binary_length_22.txt";
        const char* filename3 = "./iplookup/finalTest/trace_binary_length_23.txt";
        input_file = fopen(filename, "r");
        input_file2 = fopen(filename2, "r");
        fp = fopen(filename1, "w");
        fp2 = fopen(filename3, "w");
    }
    else if(!(strncmp(region,"rrc01",5)))
    {
        const char* filename = "./rrc01/trace_20211122.txt";
        const char* filename2 = "./rrc01/trace_20211123.txt";
        const char* filename1 = "./iplookup/finalTest/trace_binary_length_22.txt";
        const char* filename3 = "./iplookup/finalTest/trace_binary_length_23.txt";
        input_file = fopen(filename, "r");
        input_file2 = fopen(filename2, "r");
        fp = fopen(filename1, "w");
        fp2 = fopen(filename3, "w");
    }
    else if(!(strncmp(region,"rrc03",5)))
    {
        const char* filename = "./rrc03/trace_20211122.txt";
        const char* filename2 = "./rrc03/trace_20211123.txt";
        const char* filename1 = "./iplookup/finalTest/trace_binary_length_22.txt";
        const char* filename3 = "./iplookup/finalTest/trace_binary_length_23.txt";
        input_file = fopen(filename, "r");
        input_file2 = fopen(filename2, "r");
        fp = fopen(filename1, "w");
        fp2 = fopen(filename3, "w");
    }
    else if(!(strncmp(region,"rrc04",5)))
    {
        const char* filename = "./rrc04/trace_20211122.txt";
        const char* filename2 = "./rrc04/trace_20211123.txt";
        const char* filename1 = "./iplookup/finalTest/trace_binary_length_22.txt";
        const char* filename3 = "./iplookup/finalTest/trace_binary_length_23.txt";
        input_file = fopen(filename, "r");
        input_file2 = fopen(filename2, "r");
        fp = fopen(filename1, "w");
        fp2 = fopen(filename3, "w");
    }
    else if(!(strncmp(region,"rrc05",5)))
    {
        const char* filename = "./rrc05/trace_20211122.txt";
        const char* filename2 = "./rrc05/trace_20211123.txt";
        const char* filename1 = "./iplookup/finalTest/trace_binary_length_22.txt";
        const char* filename3 = "./iplookup/finalTest/trace_binary_length_23.txt";
        input_file = fopen(filename, "r");
        input_file2 = fopen(filename2, "r");
        fp = fopen(filename1, "w");
        fp2 = fopen(filename3, "w");
    }
    else if(!(strncmp(region,"Final",5)))
    {
        const char* filename = "./iplookup/finalTest/trace_20211122.txt";
        const char* filename2 = "./iplookup/finalTest/trace_20211123.txt";
        const char* filename1 = "./iplookup/finalTest/trace_binary_length_22.txt";
        const char* filename3 = "./iplookup/finalTest/trace_binary_length_23.txt";
        input_file = fopen(filename, "r");
        input_file2 = fopen(filename2, "r");
        fp = fopen(filename1, "w");
        fp2 = fopen(filename3, "w");
    }
    else
    {
        printf("Input is not one of the five regions.");
    }
    
    trace2binary(input_file,fp);
    trace2binary(input_file2,fp2);

    fclose(input_file);
    fclose(input_file2);
    fclose(fp);
    fclose(fp2);
}

void iplookup(FILE* routingTable,FILE* trace,FILE* NextHop,FILE* writefile)
{
    int i=0;
    int j=0;
    int k=0;
    int n=0;
    int line=0;
    int count=0;
    char StrLine[1024];

    clock_t t;

    t = clock();
    char **NextHoptable=NULL;
    NextHoptable = (char**)malloc(sizeof(char*)*1000000);
    for(i=0;i<1000000;i++)
    {
        NextHoptable[i]=(char*)malloc(sizeof(char*)*32);
    }
    t = clock() - t;
    printf ("It took me %d clicks (%f seconds) for Allocating Memory.\n",t,((float)t)/CLOCKS_PER_SEC);

    t = clock();
    fgets(StrLine, 1024, NextHop);
    while (fgets(StrLine, 1024, NextHop))
    {        
        char *test = strtok(StrLine, " ");              
        while (test != NULL) 
        {   
            strcpy(NextHoptable[line],test);
            test = strtok(NULL, " ");
        }        
        //printf("%d and %s\n",line,NextHoptable[line]);        
        line++;
        /*
        if (line%100==0)
        {
            //printf("%d and %s\n",line,NextHoptable[line]);
            break;
        }  
        */
    }
    t = clock() - t;
    printf ("It took me %d clicks (%f seconds) for build NextHoptable.\n",t,((float)t)/CLOCKS_PER_SEC);

    printf("0's NextHop=%s",NextHoptable[0]);

    int p=0;
    node *root;
    node *tmp;
    prefix_info* prefixes = malloc(1000000* sizeof *prefixes);

    t = clock();
    while (fgets(StrLine, 1024, routingTable))
    {
        prefixes[p].index=p;
        
        char *test = strtok(StrLine, " ");        
        strncpy(prefixes[p].Bstring,test,32);      
        while (test != NULL) 
        {   
            prefixes[p].len=atoi(test);
            test = strtok(NULL, " ");
        }
        /*
        if (p%1000==0)
        {
            printf("p %d done\n",p);
            printf("%s and %d\n",prefixes[p].Bstring,prefixes[p].len);
        }   
        */
        p++;
    }
    t = clock() - t;
    printf ("It took me %d clicks (%f seconds) for prefixes_struct.\n",t,((float)t)/CLOCKS_PER_SEC);
    
    t = clock();
    /* Inserting nodes into tree */
    root = NULL;    
    for(i=0;i<p;i++)
    {
        insert(&root,prefixes[i],0);
        /*
        if(i%1000==0)
        {
            printf("%d insert ok\n",i);
        }
        */
    }        
    t = clock() - t;
    printf ("It took me %d clicks (%f seconds) for build binarytree.\n",t,((float)t)/CLOCKS_PER_SEC);

    line=0;
    t = clock();
    /* Search node into tree */
    while (fgets(StrLine, 1024, trace))
    {
        tmp = search(&root, StrLine, 0);
        if (!strcmp(StrLine,"00000000000000000000000000000000\n"))
        {
            printf("after cmp:Data Not found in tree.\n%s not found\n",StrLine);
            fwrite(NextHoptable[0], 1, strlen(NextHoptable[0]), writefile);
        }
        else if (update>0)
        {
            //printf("line = %d, Searched index=%d\n",line, update);
            //printf("%s\n",NextHoptable[update]);
            fwrite(NextHoptable[update], 1, strlen(NextHoptable[update]), writefile);            
        }
        else
        {
            printf("Data Not found in tree. %s not found\n",StrLine);
            fwrite(NextHoptable[0], 1, strlen(NextHoptable[0]), writefile);
        }
        update=0;
        /*
        if(line==3)
        {
            break;
        }
        line++;
        */
    }
    t = clock() - t;
    printf ("It took me %d clicks (%f seconds) for binarysearch.\n",t,((float)t)/CLOCKS_PER_SEC);

    /* Deleting all nodes of tree */
    deltree(root);
    for(i=0; i<1000000; i++)   
        free(NextHoptable[i]);   
    free(NextHoptable); 
}

int main()
{   
    char command[80];

    strcpy(command,"rrc04");
    
    prefix_to_binaryForm(command);
    trace_to_binaryForm(command);
    printf("prefix and trace have been transformed to binary form!\n");

    const char* filename = "./iplookup/finalTest/prefix_binary_length_22.txt";
    FILE* input_file = fopen(filename, "r");
    const char* filename1 = "./iplookup/finalTest/trace_binary_length_22.txt";
    FILE* input_file1 = fopen(filename1, "r");
    const char* filename4 = "./rrc04/prefix_20211122.txt";
    FILE* input_file4 = fopen(filename4, "r");
    const char* filename2 = "./iplookup/finalTest/prefix_binary_length_23.txt";
    FILE* input_fil2 = fopen(filename2, "r");
    const char* filename3 = "./iplookup/finalTest/trace_binary_length_23.txt";
    FILE* input_file3 = fopen(filename3, "r");
    const char* filename5 = "./rrc04/prefix_20211123.txt";
    FILE* input_file5 = fopen(filename5, "r");

    const char* filename6 = "./iplookup/finalTest/Nexthop_22.txt";
    FILE* input_file6 = fopen(filename6, "w");
    const char* filename7 = "./iplookup/finalTest/Nexthop_23.txt";
    FILE* input_file7 = fopen(filename7, "w");

    printf("Begin to perform IP lookup...\n");
    iplookup(input_file,input_file1,input_file4,input_file6);
    iplookup(input_fil2,input_file3,input_file5,input_file7);

    return 0;
}