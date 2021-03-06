#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Use a hash table to store all inodes and files indexed by inode

struct bnode;
struct file;
struct directory;
struct inner_node;

struct file{
    int file_id;
    char file_name[100];
    unsigned char* file_contents;
    long long int file_size;
};
typedef struct file XFILE;

struct directory{
    int dir_id;
    char dir_name[100];
    struct bnode* dir_tree;
    long long int dir_file_count;
};typedef struct directory DIR;

struct inner_node{ // this is for the hash table
    int object_type; // 0 is file 1 is dir
    char name[100];
    struct file* myfile;
    struct directory* mydir;
}; typedef struct inner_node INNERNODE;

struct bnode{
        int length;
        int level;
        int controller; // 0 is leaf 1 is internal
        struct bnode * parent;
        struct bnode** children;
        char ** names; // actual index
        long int* ids;
}; typedef struct bnode BNODE;

struct sq{
        struct directory* stackqueue[100];
        int top; // top of stack and tail
        int head; //head of the queue
};typedef struct sq STACKQUEUE;

void push_stack(STACKQUEUE* s,DIR* item){
        if (s->head == -1){
                //empty
                s->head = s->top = 0;
                s->stackqueue[s->top] = item;
        }else{
                s->stackqueue[++(s->top)] = item;
        }
}
int pop_stack(STACKQUEUE* s){
        if (s->top == 0){
                //only root remains
                return 1;
        }else{
                s->top--;
                //if (s->top == -1) s->head = -1;
                return 0;
        }
}

DIR* stack_top(STACKQUEUE* s){
        if (s->top != -1){
              return s->stackqueue[s->top];  
        }
        return NULL; 
}

INNERNODE* search_for_string(BNODE* current_root,char* target);
int insert_internal(STACKQUEUE*s,BNODE* current_root,char* target,long id,BNODE* current_child,BNODE* new_child);
void insert_leaf(STACKQUEUE *s,BNODE* cr,char* target,long id);
void fx_ls(BNODE* current_dir);
int fx_touch(STACKQUEUE*s ,DIR* current_root,char* name,char* content);
int fx_mkdir(STACKQUEUE*s, DIR* current_root,char* name);
void fx_pwd(STACKQUEUE* s);
void fx_cd(STACKQUEUE* s,char * target_dir);
void fx_cat(BNODE* active_root,char* target);
void fx_rmdir(STACKQUEUE*s, DIR* current_root, char* target);
void fx_rm(STACKQUEUE*s ,DIR* current_root,char* name);
void level_order_print(BNODE* current_node);
INNERNODE** MASSIVE_HASH;

int main(){
        printf("[START] BTRFS initialising....\n");
        MASSIVE_HASH = (INNERNODE**)malloc(sizeof(INNERNODE*)*100000);

        printf("[WARNING] BTRFS is a Non-Persistent System....\n");
        printf("[INFO] BTRFS mounting....\n");
        printf("[INFO] BTRFS mounted and ready for operation....\n");
        DIR* root = (DIR*)malloc(sizeof(DIR));
        strcpy(root->dir_name,"/");
        root->dir_id = 0;
        
        root->dir_file_count = 0;
        root->dir_tree = (BNODE*)malloc(sizeof(BNODE));
        root->dir_tree->children = (BNODE**)malloc(sizeof(BNODE*)*5);
        root->dir_tree->ids = (long int*)malloc(sizeof(long int)*4);
        root->dir_tree->names = (char**)malloc(sizeof(char)*4);
        root->dir_tree->length = -1;
        root->dir_tree->controller = 0;
        root->dir_tree->parent = NULL;
       
        STACKQUEUE dirstack;dirstack.head = -1;dirstack.top = -1;
        push_stack(&dirstack,root);

        char buffer[1024];
        char* words[10];
        char* supported_comms[] = {"ls","cd","mkdir","rmdir","cat","pwd","touch","rm","exit","lop"};
        int i,p;
        int rogue = 1;
        printf("[INFO] Supported commands are: ls, cd, mkdir, rmdir, cat, pwd, touch, rm, exit\n");
        while(rogue){
                printf("\n<X>>> ");
                scanf(" %[^\n]s",buffer);
                i = 0;
                words[i] = strtok(buffer," ");
                while ((words[++i] = strtok(NULL," ")) != NULL);
                for ( p = 0;p<10;p++){
                        if (strcmp(supported_comms[p],words[0]) == 0){
                                break;
                        }
                }
                switch (p)
                {
                        case 0:{fx_ls(stack_top(&dirstack)->dir_tree);}break;
                        case 1:{fx_cd(&dirstack,words[1]);}break;
                        case 2:{fx_mkdir(&dirstack,stack_top(&dirstack),words[1]);}break;
                        case 3:{fx_rmdir(&dirstack,stack_top(&dirstack),words[1]);}break;
                        case 4:{fx_cat(stack_top(&dirstack)->dir_tree,words[1]);}break;
                        case 5:{fx_pwd(&dirstack);}break;
                        case 6:{fx_touch(&dirstack,stack_top(&dirstack),words[1],"abc");}break;
                        case 7:{fx_rm(&dirstack,stack_top(&dirstack),words[1]);}break;
                        case 8:{rogue = 0;}break;
                        case 9:{level_order_print(stack_top(&dirstack)->dir_tree);break;}
                        default:{printf("[ERROR] Is it that hard to follow instructions? That command doesn't exist\n");}
                                break;
                }
        }
        printf("TERMINATED\n");
}

void fx_ls(BNODE* current_dir){
        //traversal of the file_tree starting at current_dir
        if (current_dir == NULL)return;
        int flag = 0;
        
        for (int j = 0; j <= current_dir->length+1; j++){
                if (current_dir->children[j]!=NULL){
                        //printf("call pass to child %d\n",j);
                        fx_ls(current_dir->children[j]);
                        flag = 1;
                }
        }
        
        if (flag == 0){
                printf("This is a new node\n");fflush(stdout);
                for (int i = 0; i < current_dir->length+1;i++){ 
                                //printf("i %d %d\n",i,current_dir->ids[i]);fflush(stdout);
                                INNERNODE*n = MASSIVE_HASH[current_dir->ids[i]];
                                int discriminator = n->object_type;
                                
                                if (discriminator == 0){
                                        XFILE* f = n->myfile;
                                        printf("%s %d %s %lld\n","FILE",f->file_id,f->file_name,f->file_size);
                                }else{
                                        DIR* d = n->mydir;
                                        printf("%s %d %s\n","DIR",d->dir_id,d->dir_name);
                                }
                        }
        }
}
int insert_internal(STACKQUEUE*s,BNODE* current_root,char* target,long id,BNODE* current_child,BNODE* new_child){
        if (current_root != NULL){ //non dead
                if (current_root->length < 2){
                        //there is some space
                        int i = 0;
                        for (i = 0; i < current_root->length+1; i++){
                                if (strcmp(target,current_root->names[i])<0){
                                        //create space and insert
                                        for (int x = current_root->length+1;x>i;x--){
                                                if (current_root->names[x]== NULL){
                                                        current_root->names[x] = (char*)malloc(sizeof(char)*100);
                                                }
                                                strcpy(current_root->names[x],current_root->names[x-1]);
                                                current_root->ids[x] = current_root->ids[x-1];
                                                current_root->children[x+1] = current_root->children[x];
                                        }
                                        current_root->ids[i] = id;
                                        strcpy(current_root->names[i],target);
                                        current_root->length++;
                                        current_root->children[i+1] = new_child; //possible
                                        new_child->parent = current_root;
                                        break;
                                }
                         }
                         if (i == current_root->length+1){
                                //printf("i is %d ",i);fflush(stdout);
                                current_root->names[i] = (char*)malloc(sizeof(char)*100);
                                strcpy(current_root->names[i],target);
                                current_root->length++;
                                current_root->children[i+1] = new_child;
                                new_child->parent = current_root;
                        }
                }else{
                        //danger: no space in the damned internal
                        //split node into 2
                        //create new bnode
                        int i = 0;
                        for (i = 0; i < current_root->length+1; i++){
                                if (strcmp(target,current_root->names[i]) <0){
                                        //create space and insert
                                        for (int x = current_root->length+1;x>i;x--){
                                                if (current_root->names[x]== NULL){
                                                        current_root->names[x] = (char*)malloc(sizeof(char)*100);
                                                }
                                                strcpy(current_root->names[x], current_root->names[x-1]);
                                                current_root->ids[x] = current_root->ids[x-1];
                                                current_root->children[x+1] = current_root->children[x];
                                        }
                                        
                                        current_root->ids[i] = id;
                                        strcpy(current_root->names[i],target);
                                        current_root->children[i+1] = new_child;
                                        new_child->parent = current_root;
                                       
                                        break;
                                }
                         }
                         //printf("checkpoint 1");fflush(stdout);
                         if (i == current_root->length+1){
                                current_root->names[i] = (char*)malloc(sizeof(char)*100);
                                strcpy(current_root->names[i],target);
                                current_root->children[i+1] = new_child;
                                new_child->parent = current_root;
                                //current_root->length++;
                        }
                        //printf("checkpoint 2");fflush(stdout);
                        BNODE* bnode = (BNODE*)malloc(sizeof(BNODE));
                        bnode->children = (BNODE**)malloc(sizeof(BNODE*)*5);
                        bnode->ids = (long int*)malloc(sizeof(long int)*4);
                        bnode->names = (char**)malloc(sizeof(char)*4);
                        bnode->length = -1;
                        bnode->controller = 1; bnode->parent = NULL;

                       //redistribute
                        bnode->ids[0] = current_root->ids[3];
                        
                        bnode->length +=1;
                        bnode->names[0] = (char*)malloc(sizeof(char)*100);
                        
                        strcpy(bnode->names[0],current_root->names[3]);
                       
                        //don't forget to redistribute children
                        bnode->children[0] = current_root->children[3];
                        bnode->children[0]->parent = bnode;
                        bnode->children[1] = current_root->children[4];
                        bnode->children[1]->parent = bnode;
                       
                        current_root->length -= 1; //possible
                        //printf("about");fflush(stdout);
                        insert_internal(s,current_root->parent,current_root->names[2],current_root->ids[2],current_root,bnode);
                }
        }else{
                //root insert,new root to be created
                //printf("what is going on");fflush(stdout);
                BNODE* bnode = (BNODE*)malloc(sizeof(BNODE));
                bnode->children = (BNODE**)malloc(sizeof(BNODE*)*5);
                bnode->ids = (long int*)malloc(sizeof(long int)*4);
                bnode->names = (char**)malloc(sizeof(char)*4);
                bnode->length = -1;
                bnode->controller = 1; bnode->parent = NULL;
                
                bnode->children[0] = current_child;
                bnode->children[1] = new_child;
                current_child->parent = bnode;
                new_child->parent = bnode;
                bnode->length += 1;
                bnode->ids[0] = id;
                bnode->names[0] = (char*)malloc(sizeof(char)*100);
                strcpy(bnode->names[0],target);
                printf(" [NOTE] root change complete, target : %s\n",target);fflush(stdout);
                stack_top(s)->dir_tree = bnode;
        }     
}
void insert_leaf(STACKQUEUE *s,BNODE* cr,char* target,long id){
        BNODE* current_root = cr;
        int i = 0;
        while (current_root->controller != 0){
                i = 0;
                for (i = 0;i<current_root->length+1;i++){
                        if (strcmp(target,current_root->names[i]) < 0){
                                break;
                        }
                }
               
                current_root = current_root->children[i];
        }
        if (current_root->controller == 0){ //leaf
                if (current_root->length < 2){
                        //there is some space
                        int i = 0;
                        //printf("len %d",current_root->length);fflush(stdout);
                        for ( i = 0; i < current_root->length+1; i++){
                                //printf("i %d\n",i);fflush(stdout);
                               // printf("compare: %s %s %d \n",target, current_root->names[i],strcmp(target,current_root->names[i]));fflush(stdout);
                                if (strcmp(target,current_root->names[i]) < 0){
                                        //create space and insert

                                        for (int x = current_root->length+1;x>i;x--){
                                                if (current_root->names[x]== NULL){
                                                        current_root->names[x] = (char*)malloc(sizeof(char)*100);

                                                }
                                                strcpy(current_root->names[x],current_root->names[x-1]);
                                                current_root->ids[x] = current_root->ids[x-1];
                                        }
                                        
                                        current_root->ids[i] = id;
                                        strcpy(current_root->names[i],target);
                                        current_root->length++;
                                        break;
                                }
                                if (strcmp(target,current_root->names[i]) == 0){printf("[WARNING] File exists\n");return;}
                        }
                        if (i == current_root->length+1){
                                current_root->names[i] = (char*)malloc(sizeof(char)*100);
                                strcpy(current_root->names[i],target);
                                current_root->ids[i] = id;
                                current_root->length++;
                        }
                        //printf("i %d",i);fflush(stdout);
                        
                }else{
                        //danger: no space in the damned leaf
                        //split node into 2
                        //create new bnode
                        int i = 0;
                        for ( i = 0; i < current_root->length+1; i++){
                                if (strcmp(target,current_root->names[i]) <0){
                                        //create space and insert
                                        for (int x = current_root->length+1;x>i;x--){
                                                if (current_root->names[x]== NULL){
                                                        current_root->names[x] = (char*)malloc(sizeof(char)*100);
                                                }
                                                strcpy(current_root->names[x], current_root->names[x-1]);
                                                current_root->ids[x] = current_root->ids[x-1];
                                        }
                                        
                                        current_root->ids[i] = id;
                                        strcpy(current_root->names[i],target);
                                        break;
                                }
                                if (strcmp(target,current_root->names[i]) == 0){printf("[WARNING] File exists\n");return;}
                        }
                        
                        if (i == current_root->length+1){
                                current_root->names[i] = (char*)malloc(sizeof(char)*100);
                                strcpy(current_root->names[i],target);
                                current_root->ids[i] = id;
                        }

                        BNODE* bnode = (BNODE*)malloc(sizeof(BNODE));
                        bnode->children = (BNODE**)malloc(sizeof(BNODE*)*5);
                        bnode->ids = (long int*)malloc(sizeof(long int)*4);
                        bnode->names = (char**)malloc(sizeof(char)*4);
                        bnode->length = -1;
                        bnode->controller = 0; 
                        
                       //redistribute
                        bnode->ids[0] = current_root->ids[2];
                        bnode->ids[1] = current_root->ids[3];
                        
                        bnode->length +=2;
                        bnode->names[0] = (char*)malloc(sizeof(char)*100);
                        bnode->names[1] = (char*)malloc(sizeof(char)*100);
                        strcpy(bnode->names[0],current_root->names[2]);
                        strcpy(bnode->names[1],current_root->names[3]);
                        current_root->length -= 1;
                        insert_internal(s,current_root->parent,bnode->names[0],bnode->ids[0],current_root,bnode);
                }
        }        
}

int fx_touch(STACKQUEUE*s,DIR* current_root,char* name,char* content){
        BNODE* current_dir = current_root->dir_tree;
        XFILE* newfile = (XFILE*)malloc(sizeof(XFILE));
        strcpy(newfile->file_name,name);
        long int id = rand() % 10000;
        //printf("%ld",id);fflush(stdout);
        newfile->file_id = id;
        newfile->file_contents = (unsigned char*)malloc(sizeof(unsigned char)*strlen(content));
        strcpy(newfile->file_contents,content);
        newfile->file_size = strlen(content);
        INNERNODE* newi = (INNERNODE*)malloc(sizeof(INNERNODE));
        newi->myfile = newfile;
        newi->object_type = 0;
        MASSIVE_HASH[id] = newi;
        current_root->dir_file_count++;
        insert_leaf(s,current_dir,name,id);
        return 0;
}

int fx_mkdir(STACKQUEUE*s, DIR* current_root,char* name){
        BNODE* current_dir = current_root->dir_tree;
        DIR* newdir = (DIR*)malloc(sizeof(DIR));
        strcpy(newdir->dir_name,name);
        long int id = rand() % 10000;
        //printf("%ld",id);
        newdir->dir_id = id;
        newdir->dir_file_count= 0;
        INNERNODE* newi = (INNERNODE*)malloc(sizeof(INNERNODE));
        newi->mydir = newdir;
        newi->object_type = 1;
        newdir->dir_tree = (BNODE*)malloc(sizeof(BNODE));
        newdir->dir_tree->children = (BNODE**)malloc(sizeof(BNODE*)*5);
        newdir->dir_tree->ids = (long int*)malloc(sizeof(long int)*4);
        newdir->dir_tree->names = (char**)malloc(sizeof(char)*4);
        newdir->dir_tree->length = -1;
        newdir->dir_tree->controller = 0;
        newdir->dir_tree->parent = NULL;
        MASSIVE_HASH[id] = newi;
        current_root->dir_file_count++;
        insert_leaf(s,current_dir,name,id);
        return 0;
}

void fx_pwd(STACKQUEUE* s){
        int t = s->head;
        printf("[PATH] ");
        DIR* a;
        while (t <= s->top) {
                a = s->stackqueue[t++];
                printf("%s/",a->dir_name);
        }
        printf("\n");
}
INNERNODE* search_for_string(BNODE* current_root,char* target){
        int i = 0;
        while (current_root->controller != 0){
               i = 0;
                for (i = 0;i<current_root->length+1;i++){
                        if (strcmp(target,current_root->names[i]) < 0){
                                break;
                        }
                }
                current_root = current_root->children[i];
        }
        if (current_root->controller ==0){
                //leaf
                for (int i = 0; i < current_root->length+1;i++){
                        if (strcmp(target,current_root->names[i]) == 0){
                               //found
                               long int found = current_root->ids[i];
                               return MASSIVE_HASH[found];
                        }
                }
                return NULL;
        }
}

void fx_cd(STACKQUEUE* s,char * target_dir){
        if (strcmp(target_dir,"..")==0){
                //move to parent
                int k = pop_stack(s);
                if (k == 1){
                        printf("[WARN] Can't go higher than root\n");
                }else{
                        printf("[INFO] Directory has been changed\n");
                        fx_pwd(s);
                }
        }else{
                INNERNODE* mynode = search_for_string(stack_top(s)->dir_tree,target_dir);
                if (mynode==NULL){
                        printf("[ERROR] Directory not found\n");
                        return;
                }
                if (mynode->object_type == 1){ //dir
                        //successfully located dir
                        push_stack(s,mynode->mydir);
                        printf("[INFO] Directory has been changed\n");
                        fx_pwd(s);
                }else{
                        printf("[ERROR] You can't change directory to a FILE\n");
                }
        }
}
void fx_cat(BNODE* active_root,char* target){
        //find target and then print contents
        INNERNODE* mynode = search_for_string(active_root,target);
        if (mynode == NULL){printf("[ERROR] File not found\n"); return;}
        if (mynode->object_type == 0){
                printf("[OUTPUT] File Contents: %s | %lld bytes\n",mynode->myfile->file_contents,mynode->myfile->file_size);
        }else{
                printf("[OUTPUT] This is a directory\n");
        }
}

void fix_keys(BNODE *current_root, int old_id, char *old_name, int new_id, char *newname)
{
        if (current_root->parent == NULL)
        {
                return;
        }
        else
        {
                BNODE *parent = current_root->parent;
                int i = 0;
                while ((parent->ids[i] != old_id && strcmp(parent->names[i],old_name)!=0) && i <= parent->length)
                {                
                        printf("%s\n", parent->names[i]);
                        i++;
                }
                if (parent->ids[i] = old_id || strcmp(parent->names[i],old_name)==0)
                {
                        parent->ids[i] = new_id;
                        strcpy(parent->names[i], newname);
                }
                printf("%s\n", parent->names[i]);
                fix_keys(current_root->parent, old_id,old_name, new_id, newname);
        }
}
void delete_leaf(STACKQUEUE *s, char *target)
{
        BNODE *current_root = stack_top(s)->dir_tree;
        int j = 0;
        while (current_root->controller != 0)
        {
                j = 0;
                for (j = 0; j < current_root->length + 1; j++)
                {
                        if (strcmp(target, current_root->names[j]) < 0)
                        {
                                break;
                        }
                }
                current_root = current_root->children[j];
        }
        if (current_root->controller == 0)
        {
                //leaf
                for (int i = 0; i < current_root->length + 1; i++)
                {
                        if (strcmp(target, current_root->names[i]) == 0)
                        {
                                //found
                                //printf("found at %d\n", i);
                                fflush(stdout);
                                long int found = current_root->ids[i];
                                free(MASSIVE_HASH[found]);
                                MASSIVE_HASH[found] = NULL;
                                for (int x = i; x < current_root->length; x++)
                                {
                                        if (current_root->names[x] == NULL)
                                        {
                                                current_root->names[x] = (char *)malloc(sizeof(char) * 100);
                                        }
                                        strcpy(current_root->names[x], current_root->names[x + 1]);
                                        current_root->ids[x] = current_root->ids[x + 1];
                                        current_root->children[x] = current_root->children[x + 1];
                                }
                                if (current_root->length - 1 == -1)
                                {
                                        //leaf becomes low
                                        //printf("Leaf is low\n");
                                        if (j <= current_root->parent->length && (current_root->parent->children[j + 1] != NULL && current_root->parent->children[j + 1]->length>0))
                                         {
                                                //printf("Right sibling\n");
                                                BNODE *sibling = current_root->parent->children[j + 1];
                                                if (sibling->length - 1 != -1)
                                                {
                                                        //printf("If excess\n");
                                                        int old_id = sibling->ids[0];
                                                        char *old_name = malloc(sizeof(char) * 100);
                                                        strcpy(old_name, sibling->names[0]);
                                                        current_root->ids[0] = sibling->ids[0];
                                                        strcpy(current_root->names[0], sibling->names[0]);
                                                        //printf("adjusting keys\n");
                                                        for (int x = 0; x < sibling->length; x++)
                                                        {
                                                                strcpy(sibling->names[x], sibling->names[x + 1]);
                                                                sibling->ids[x] = sibling->ids[x + 1];
                                                                sibling->children[x] = sibling->children[x + 1];
                                                        }
                                                        sibling->length--;
                                                        //printf("before fixing keys\n");
                                                        fix_keys(sibling, old_id,old_name, sibling->ids[0], sibling->names[0]);
                                                }
                                        }
                                        else if (j >= 1 && current_root->parent->children[j - 1]->length > 0)
                                        {
                                               // printf("Left sibling\n");
                                                //printf("adjusting keys\n");
                                                int old_id = current_root->ids[0];
                                                char *old_name = malloc(sizeof(char) * 100);
                                                strcpy(old_name, current_root->names[0]);
                                                BNODE *sibling = current_root->parent->children[j - 1];
                                                current_root->ids[i] = sibling->ids[sibling->length];
                                                strcpy(current_root->names[i], sibling->names[sibling->length]);
                                                //printf("before fixing keys\n");
                                                fix_keys(current_root, old_id, old_name, current_root->ids[i], current_root->names[i]);
                                                sibling->length--;
                                        }
                                        else
                                        {
                                                //merging nodes
                                                if (j ==2 && current_root->parent->children[j - 1]->length == 0)
                                                {
                                                        //printf("Rightmost child\n");
                                                        current_root->parent->length--;
                                                }
                                                else
                                                {
                                                       // printf("Middle child\n");
                                                        int x = j;
                                                        BNODE *parent = current_root->parent;
                                                        while (x < parent->length + 1)
                                                        {
                                                                parent->children[x] = parent->children[x + 1];
                                                                parent->ids[x - 1] = parent->ids[x];
                                                                strcpy(parent->names[x - 1], parent->names[x]);
                                                                x++;
                                                        }
                                                        parent->length--;
                                                }
                                        }
                                }
                                else
                                {
                                        if(j>0 && i==0){
                                                fix_keys(current_root,found,target,current_root->ids[i],current_root->names[i]);
                                        }
                                        current_root->length--;
                                }
                        }
                }
        }
}


void fx_rmdir(STACKQUEUE*s, DIR* current_root, char* target){
        delete_leaf(s,target);
}
void fx_rm(STACKQUEUE*s ,DIR* current_root,char* name){
        delete_leaf(s,name);
}
struct q{
        int head;
        int tail;
        BNODE* queue[100];
};

void enqueue(struct q* queue,BNODE* item){
        if (queue->head == -1){
                queue->head = queue->tail = 0;
                queue->queue[queue->tail] = item;
        }
        else queue->queue[++(queue->tail)] = item;
}

BNODE* dequeue(struct q* queue){
        BNODE * item;
        if (queue->head == -1) return NULL;
        if (queue->tail == queue->head && queue->tail != -1){
                item = queue->queue[queue->head];
                queue->tail = queue->head = -1;
                return item;
        }else{
                return queue->queue[(queue->head)++];
        }
        return NULL;
}

void printer(struct q* queue){
        BNODE* current;
        while ((current= dequeue(queue)) != NULL){
                printf("LEVEL %d\n",current->level);                
                for (int d = 0; d <= current->length;d++){
                        fflush(stdout);
                        printf("id  %ld  name   %s ||\t",current->ids[d],current->names[d]);
                }
                for (int x = 0; x <= current->length+1;x++){
                        if (current->children[x] != NULL){
                                current->children[x]->level = current->level +1;
                                enqueue(queue,current->children[x]);
                        }
                }
                printf("\n");
        }
}
void level_order_print(BNODE* current_node){
       struct q mq;
       mq.head = -1;
       mq.tail = -1;
       current_node->level = 0;
       enqueue(&mq,current_node);
       printer(&mq);
}
