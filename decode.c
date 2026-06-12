#include <stdio.h>
#include<string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

Status read_and_validate_decode_args(int argc,char *argv[], DecodeInfo *decInfo)
{
    //check command line arguments 
    if(argc<3 || argc>4){
        printf("ERROR: Insufficient arguments\n");
        return e_failure;
    }
    //check .bmp present in argv[2]
    int i=1,flag=1;
    while(argv[2][i]){
        if(argv[2][i]=='.'){
            if(!strcmp(&argv[2][i],".bmp")){
                flag=0;
                decInfo->stego_image_fname=argv[2];
            }
        }
        i++;
    }
    if(flag){
        return e_failure;
    }
    //check argv[3] is present or not
    if(argv[3]==NULL){
         printf("INFO: Output File not mentioned. Creating decoded name as default\n");
        strcpy(decInfo->secret_fname,"decoded");
    }
    else{
        //if present.store the name without extn.
        int len=strlen(argv[3]);
        i=len,flag=1;
        while(i>=0){
            if(argv[3][i]=='.'){
                flag=0;
                argv[3][i]='\0';
                strcpy(decInfo->secret_fname,argv[3]);
            }
            i--;
        }
        if(flag){
            return e_failure;
        }
    }
    return e_success;
}

Status open_decode_files(DecodeInfo *decInfo)
{
    //Open required file to decode
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname,"rb");
    if(decInfo->fptr_stego_image == NULL){
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }
    return e_success;
}

//check magic string is present or not
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    // char buffer[8];
    printf("INFO: Decoding Magic String Signature\n");
    int i=0;
    char mag_buffer[8],ch;
    unsigned int bmp_header_size = 0;
    fseek(decInfo->fptr_stego_image, 10, SEEK_SET);
    // 2. Read the 4-byte header size value
    fread(&bmp_header_size, sizeof(unsigned int), 1, decInfo->fptr_stego_image);
    //skip header size in stego_bmp_file
    fseek(decInfo->fptr_stego_image,bmp_header_size,SEEK_SET);
    while(magic_string[i]!='\0'){
        //step1 -> read 8 bytes from stego img file
        fread(mag_buffer,8,1,decInfo->fptr_stego_image);
        decode_byte_to_lsb(&ch,mag_buffer);
        if(ch!=magic_string[i]){
            printf("ERROR: Magic string mismatch\n");
            return e_failure;
        }
        i++;
    }
    return e_success;
}

Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    // char buffer[32];
    printf("INFO: Decoding Output File Extension Size\n");
    char ex_buffer[32];
    //step1 -> read 32 bytes from stego img file
    fread(ex_buffer,32,1,decInfo->fptr_stego_image);
    //step2 -> call decode_size_to_lsb
    decode_size_to_lsb(&decInfo->size_secret_extn_file,ex_buffer);
    return e_success;
}

Status decode_secret_file_extn(char *file_extn, DecodeInfo *decInfo)
{
    // char buffer[8];
    printf("INFO: Decoding Output File Extension\n");
    char extn_buffer[8];
    //step1 -> read 8 bytes from stego img file
    int i=0;
    char ch;
    while(i<decInfo->size_secret_extn_file){
        fread(extn_buffer,8,1,decInfo->fptr_stego_image);
        //step2 -> call decode_byte_to_lsb
        decode_byte_to_lsb(&ch, extn_buffer);
        file_extn[i]=ch;
        i++;
    }
    //step3 -> repeat this for size of extn time.
    file_extn[i]='\0';
    strcat(decInfo->secret_fname,file_extn);
    //step4 -> Open decoded file name with these extn.
    decInfo->fptr_secret=fopen(decInfo->secret_fname,"w");
    if(decInfo->fptr_secret==NULL){
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->secret_fname);
        return e_failure;
    }
    printf("INFO: Opened decoded.extn file\n");
    return e_success;
    
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    // char buffer[32];
    printf("INFO: Decoding decoded.extn File Size\n");
    char f_buffer[32];
    int size;
    //step1 -> read 32 bytes from src file
    fread(f_buffer,32,1,decInfo->fptr_stego_image);
    //step2 -> call decode_size_to_lsb(size, buffer)
    decode_size_to_lsb(&size, f_buffer);
    decInfo->size_secret_file=size;
    return e_success; 
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    // char buffer[8];
    printf("INFO: Decoding decoded.extn File Data\n");
    char fdata_buffer[8];
    int i=0;
    char ch;
    //copy all secret data to decoded file
    while(i<decInfo->size_secret_file){
        fread(fdata_buffer,8,1,decInfo->fptr_stego_image);
        decode_byte_to_lsb(&ch, fdata_buffer);
        decInfo->secret_data[i]=ch;
        i++;
    }
    decInfo->secret_data[i]='\0';
    fprintf(decInfo->fptr_secret,"%s",decInfo->secret_data);
    return e_success;
}


Status decode_byte_to_lsb(char *data, char *image_buffer)
{
    // write logic decode one char
    for(int i=0;i<8;i++){
        *data= *data&(~(1<<(7-i)));
        *data= ((*data) | ((image_buffer[i] & 1)<<(7-i)));
    }
    return e_success;
}

Status decode_size_to_lsb(int *size, char *imageBuffer)
{
    // write logic decode size
    *size=0;
    for(int i=0;i<32;i++){
        *size = (*size | ((imageBuffer[i] & 1)<<(31-i)));
    }
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{   
    // step1 -> check open_files(encInfo) returning e_success or not
        // yes -> print success msg goto next step
        // no -> return e_failure
    if((open_decode_files(decInfo))==e_success){
        printf("INFO: ## Decoding Procedure Started ##\n");
        printf("INFO: Opening required files\n");
        printf("INFO: Opened steged_beautiful.bmp\n");
    }
    else{
        return e_failure;
    }

    // step2 -> call decode_magic_string(MAGIC_STRING, decInfo)
    if((decode_magic_string(MAGIC_STRING, decInfo))==e_success){
        printf("INFO: Done\n");
    }
    else{
        return e_failure;
    }
    // step5 -> call decode_secret_file_extn_size(decInfo)
    if((decode_secret_file_extn_size(decInfo))==e_success){
        printf("INFO: Done\n");
    }

    // //here step6 -> call decode_secret_file_extn(decInfo -> extn_secret_file, decInfo)
    if((decode_secret_file_extn(decInfo -> extn_secret_file, decInfo))==e_success){
        printf("INFO: Done\n");
    }
    // //step7 -> Call decode_secret_file_size(decInfo)
    if((decode_secret_file_size(decInfo))==e_success){
        printf("INFO: Done\n");
    }
    // //step8 -> call decode_secret_file_data(decInfo)
    if((decode_secret_file_data(decInfo))==e_success){
        printf("INFO: Done\n");
    }
    printf("INFO: ## Decoding Done Successfully ##\n");
    return e_success;
}