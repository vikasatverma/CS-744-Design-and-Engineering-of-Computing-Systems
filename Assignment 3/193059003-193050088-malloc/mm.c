/*
 *
 * struct block
 * size: represents the available size in the block and last bit if 1 means it is free and 0 means allocated.
 * next: points to the next adjescent block of memory
 *
 * mm_init()
 * Only job is to set the head to null.
 *
 * mm_malloc(size_t request_size)
 * if it is the first request, it makes the request for memory and initializes the head.
 * For any other forthcoming requests, we look for an empty block linearly (first fit) and allocate it but if
 * empty block not found, we request for memory an allocate it.
 *
 * mm_free(ptr)
 * changes the last bit of size variable indicating that the block is free
 * calls to coalesce adjescent free blocks after a threshold
 *
 * mm_realloc(ptr, size)
 * if size has to be reduced, then we do not make any changes and return the same pointer
 * if it is 0, then we call mm_free
 * if it requires to be increased, we give it the required amount of memory + 10000 bytes in hopes that it would
 * ask for reallocation again.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
        /* Team name */
        "The_plagiarists",
        /* First member's full name */
        "Vikas Verma",
        /* First member's email address */
        "vikasv@cse.iitb.ac.in",
        /* Second member's full name (leave blank if none) */
        "Aditya Kumar",
        /* Second member's email address (leave blank if none) */
        "adityakumar@cse.iitb.ac.in"
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*
 * mm_init - initialize the malloc package.
 */
struct block {
    unsigned  size;
    struct block *next;
};
struct block *head;

int mm_init(void) // realsize has 1 bit added to it
{
//    mem_reset_brk();
    head = NULL;

    return 0;


}

/* free=c->size | 1;
 * allocate=c->size & ~1
 * checksize= c->Size & ~1
 * 0= allocated.
 * 1=free.
 *
 *
 *
 */
int no_of_free=0;
int req_count=1;
long long total_req_size;
long long total_free_size;
void *mm_malloc(size_t request_size) {
    if(request_size<1)
        return NULL;
    request_size = ALIGN(request_size);
    req_count++;
    total_req_size+=request_size;
    //printf("\n***********%d request size = %x **************\n",req_count++,request_size);
    struct block *c;
    void *result;
    if (head == NULL) {
        //printf("initializing head with req size = %x\n",request_size);
        struct block *x = mem_sbrk(ALIGN(request_size + sizeof(struct block)));
        head=x;
        head->size = request_size;
        head->size = head->size & ~1;
        head->next=NULL;
        //printf(" address of head free space %p \n",head+1);
        return (void *)(head+1);
    }

    //printf("request size=%x and c->size=%x \n",request_size,c->size & ~1);
    for(c=head;(((c->size & ~1) < request_size) || (((c->size) & 1) == 0)) && (c->next != NULL) && req_count-no_of_free<1500;c=c->next);

    if ((c->size&1)==1 &&  (c->size & ~1) == request_size)
      return  (c->size = c->size & ~1, result=(void *)(c+1));

        //printf(" return address = %p \n",result);


     else if ((c->size) > (request_size + sizeof(struct block)) && (c->size&1)==1) {

        struct block *new = ((void *) c + request_size + sizeof(struct block));
        //printf("address of new  =%p serving request= %x\n",new,request_size);
        new->size = (c->size) - request_size - sizeof(struct block);
        //printf("new ka size = %x \n",new->size & ~1);
        new->size = new->size | 1;
        //printf("c ka next = %x \n",c->next);
        new->next = c->next;
        c->size = request_size;
        //printf("c ka size = %x \n",c->size & ~1);
        c->size = c->size & ~1;
        c->next = new;

        return (void *) (++c);

    }
        /*    void *p = mem_sbrk(newsize);
        if (p == (void *)-1)
        return NULL;
        else {
            *(size_t *)p = size;
            return (void *)((char *)p + SIZE_T_SIZE);
        }
    */
    else { // increase memory
        int flag=0;
        if((c->size & 1) == 1){
            int required_size = request_size-(c->size & ~1);
            request_size=required_size;
            flag=1;
        }
        struct block *new=mem_sbrk(ALIGN(request_size+sizeof(struct block)));
        if(flag==0)
        {
            c->next=new;
        }
        if(new==NULL)
        { //printf("memory not available");
            result=NULL;
            return result;
        }

        //printf("address of new  =%p serving request= %x\n",new,request_size);
//        c->next=NULL;
        if(flag)
        {
            new=c;
        }


        new->size=request_size;
        new->size=new->size & ~1;
        new->next=NULL;
        //printf("returning address = %p \n",(void *)new+1);
        return (void *)(new+1);
    }
}

void coalescing(){

    if(head->next==NULL)
        return;
    struct block *current=head;

    while (current->next != NULL){
        //printf("\ncurrent block = %p\n", current);

        if(((current->size & 1) == 1) && ((current->next->size & 1)==1)){
            current->size=(current->size & ~1 )+(current->next->size & ~1 )+ sizeof(struct block);
            current->size=current->size | 1;
            current->next=current->next->next;
        }
        if(current->next!=NULL)
        current=current->next;
    }
}

void defragmentation()
{
    struct block *tmp = head;
    while (tmp!=NULL && (tmp->size & 1)==1 && tmp->next!=NULL && (tmp->next->size & 1)==0 ){
        printf("************here**************");
        size_t total_size=2* sizeof(struct block)+(tmp->size&~1)+(tmp->next->size & ~1);
        memmove((void *)tmp + sizeof(struct block),(void *)tmp->next + sizeof(struct block), tmp->next->size & ~1   );
        tmp->size=tmp->next->size;
        struct block *nextblock=tmp+sizeof(struct block)+(tmp->next->size & ~1);
        nextblock->next=tmp->next->next;
        tmp->next=nextblock;
        nextblock->size=total_size-2* sizeof(struct block)-tmp->size;
        tmp=tmp->next;
    }
}


/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
    no_of_free++;
    //printf("****************Free %p**************",ptr);
    struct block *freeptr=ptr-sizeof(struct block);
    total_free_size+=freeptr->size & -1;
    freeptr->size=freeptr->size | 1;
    //printf("\n%p freed, size= %x\n head addr is %p\n",ptr,freeptr->size & ~1,head);
//
    if((req_count-no_of_free)<2 || (total_req_size-total_free_size)<1000)
        coalescing();
    //if((req_count-no_of_free)>50)
    //{
    //defragmentation();
    //}
}








/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
      void *oldptr = ptr;
      void *newptr;
      size_t copySize;
      if(ptr==NULL)
          return ptr;
    struct block *freeptr=ptr-sizeof(struct block);
    if(size<=0){
        mm_free(ptr);
        return NULL;
    }
    if((freeptr->size & -1)>size)
        return ptr;

      newptr = mm_malloc(size+70000);
      if (newptr == NULL)
        return NULL;
      copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
      if (size < copySize)
        copySize = size;
      memcpy(newptr, oldptr, copySize);
      mm_free(oldptr);
      return newptr;

}













