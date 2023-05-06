/*
 * Copyright (c) 2022 Institute of Parallel And Distributed Systems (IPADS)
 * ChCore-Lab is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *     http://license.coscl.org.cn/MulanPSL
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v1 for more details.
 */

#include <stdio.h>
#include <string.h>
#include <chcore/types.h>
#include <chcore/fsm.h>
#include <chcore/tmpfs.h>
#include <chcore/ipc.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/internal/server_caps.h>
#include <chcore/procm.h>
#include <chcore/fs/defs.h>

typedef __builtin_va_list va_list;
#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v)      __builtin_va_end(v)
#define va_arg(v, l)   __builtin_va_arg(v, l)
#define va_copy(d, s)  __builtin_va_copy(d, s)


extern struct ipc_struct *fs_ipc_struct;

/* You could add new functions or include headers here.*/
/* LAB 5 TODO BEGIN */
//#include "FILE.h"

extern int simple_vsprintf(char **out, const char *format, va_list ap);
static u64 fd =0 ;
/* LAB 5 TODO END */


FILE *fopen(const char * filename, const char * mode) {

	/* LAB 5 TODO BEGIN */
    
    
    FILE *file = (FILE*)malloc(sizeof(struct FILE));
    int ret; 
    
    struct ipc_msg *ipc_msg = ipc_create_msg(fs_ipc_struct, sizeof(struct fs_request), 0);
    chcore_assert(ipc_msg);
    struct fs_request * fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
    fr->req = FS_REQ_OPEN;
    fr->open.new_fd = fd;
    memcpy(fr->open.pathname, filename,strlen(filename));
    ret = ipc_call(fs_ipc_struct, ipc_msg);
    ipc_destroy_msg(fs_ipc_struct, ipc_msg);

    if(ret>=0){
        file->fd = ret;
        fd++;
        printf("file->fd  %d \n", file->fd);
        return file;
    }else if(ret== -ENOENT && (strcmp(mode,"w")==0)){
        
        ipc_msg = ipc_create_msg(fs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
        fr->req = FS_REQ_CREAT;
        memcpy(fr->creat.pathname, filename,strlen(filename));
        ret = ipc_call(fs_ipc_struct, ipc_msg);
        if(ret) {   
            ipc_destroy_msg(fs_ipc_struct, ipc_msg);
            return NULL;
        }

        //open again

        fr->req = FS_REQ_OPEN;
        fr->open.new_fd = fd;
        memcpy(fr->open.pathname, filename,strlen(filename));
        ret = ipc_call(fs_ipc_struct, ipc_msg);
        ipc_destroy_msg(fs_ipc_struct, ipc_msg);

        if(ret >= 0){
            file->fd = ret;
            fd++;
            printf("file->fd  %d \n", file->fd);
            return file;
        }
    }

    
	/* LAB 5 TODO END */
    return NULL;
}

size_t fwrite(const void * src, size_t size, size_t nmemb, FILE * f) {

	/* LAB 5 TODO BEGIN */
    size_t to_write;
    struct ipc_msg *ipc_msg = ipc_create_msg(
        fs_ipc_struct, sizeof(struct fs_request)+nmemb*size, 0);
	chcore_assert(ipc_msg);
	struct fs_request * fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
    fr->req = FS_REQ_WRITE;
    fr->write.fd = f->fd;
    fr->write.count = nmemb;
    memcpy((void *)fr + sizeof(struct fs_request),src,nmemb);
    //memcpy(&fr->write.write_buff_begin+1,src,nmemb);
	to_write = ipc_call(fs_ipc_struct, ipc_msg);
    ipc_destroy_msg(fs_ipc_struct, ipc_msg);
    // return ret;
	/* LAB 5 TODO END */
    return to_write;

}

size_t fread(void * destv, size_t size, size_t nmemb, FILE * f) {

	/* LAB 5 TODO BEGIN */
    size_t to_read;
    struct ipc_msg *ipc_msg = ipc_create_msg(
        fs_ipc_struct, sizeof(struct fs_request)+nmemb*size, 0);
	chcore_assert(ipc_msg);
	struct fs_request * fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
    fr->req = FS_REQ_READ;
    fr->read.fd = f->fd;
    fr->read.count = nmemb;
    to_read = ipc_call(fs_ipc_struct, ipc_msg);
    memcpy(destv,(char*)fr,to_read);

    ipc_destroy_msg(fs_ipc_struct, ipc_msg);
    return to_read;
	/* LAB 5 TODO END */
    return 0;

}

int fclose(FILE *f) {
    int ret;
	/* LAB 5 TODO BEGIN */
    struct ipc_msg *ipc_msg = ipc_create_msg(
        fs_ipc_struct, sizeof(struct fs_request), 0);
	chcore_assert(ipc_msg);
	struct fs_request * fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
    fr->req = FS_REQ_CLOSE;
    fr->close.fd = f->fd;
    ret = ipc_call(fs_ipc_struct, ipc_msg);
    ipc_destroy_msg(fs_ipc_struct, ipc_msg);
    return ret;
	/* LAB 5 TODO END */
    return 0;

}

/* Need to support %s and %d. */

 u64 token_addr(const char *fmt, va_list ap,char* dest){
    u64 addr;
    char buffer[256];
    int i=0;
    while(*fmt!='\0' && *dest!='\0'){

          if(*fmt == '%'){
                fmt++;

                while(*dest != '\0'){
                    if(*dest != ' ' && *dest != '\n' && *dest != '\t'){
                        buffer[i] = *dest; 
                        i++;
                    
                    }else {
                        buffer[i] = '\0';
                        i=0;
                        
                        if(strlen(buffer)>0){
                            printf("get token %s \n", buffer);
                            break;
                        }
                    }
                   dest++;   
                }

                if(*dest == '\0'){
                    buffer[i] = '\0';
                    i=0;
                    if(strlen(buffer)>0){
                        printf("get token %s \n", buffer);
                    }
                }

                switch (*fmt)
                {
                case 's':
                    //get %s
                    addr = (u64)va_arg(ap, char *);
                    printf("---addr--- : %lx \n",addr);
                    memcpy((char*)addr,buffer,strlen(buffer));
                    printf("---addr--- str  : %s \n",addr);
                    
                    break;
                case 'd':
                    //get %d
                    addr = (u64)va_arg(ap, int*);
                    printf("---addr--- : %lx \n",addr);
                    //str to int .
                    int base = 1 ;
                    int val ;
                    int sum = 0;
                    printf("---strlen buffer---  : %d \n",strlen(buffer));

                    for(int i=strlen(buffer)-2;i >= 0;i--){
                        val =  buffer[i] - '0';
                        sum = val * base  + sum;
                        base *= 10;
                        //printf("---addr--- val  : %d \n",val);

                    }
                    printf("---addr--- sum  : %d \n",sum);

                    memcpy((char*)addr,(char*)&sum,sizeof(int));
                    printf("---addr--- int  : %d \n",*(int *)addr);

                    break;
                default:
                    break;
                }

            }
            
            fmt++;
            
        }
         return 0;
 } 


int fscanf(FILE * f, const char * fmt, ...) {

	/* LAB 5 TODO BEGIN */
    char* destv = (char*)malloc(256);
    size_t to_read;
    to_read = fread(destv,sizeof(char),256,f);

    va_list va;
    va_start(va, fmt);
    token_addr(fmt,va,destv);
    va_end(va);
    
    printf("to read  : len %d , str: %s \n",to_read,destv);

	/* LAB 5 TODO END */
    return 0;
}

/* Need to support %s and %d. */
int fprintf(FILE * f, const char * fmt, ...) {

	/* LAB 5 TODO BEGIN */
    int res;
    char* src_tmp = (char*)malloc(256);
    u64 buffer = (u64)src_tmp;
    printf("addr of src_tmp %lx \n", src_tmp);
    va_list va;
    va_start(va, fmt);
    res = simple_vsprintf(&src_tmp, fmt, va);
    va_end(va);
    src_tmp = (char*)buffer;

    printf("src_tmp :%s \n", src_tmp);
    printf("len of src_tmp :%d \n", strlen(src_tmp));
    

    res = fwrite(src_tmp,sizeof(char),strlen(src_tmp),f);
    printf("writer to %d \n",res);
    return res;
	/* LAB 5 TODO END */
    return 0;
}

