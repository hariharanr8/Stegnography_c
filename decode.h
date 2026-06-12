#ifndef DECODE_H
#define DECODE_H
#include<stdio.h>

#include "types.h" // Contains user defined types

/*
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

typedef struct _DecodeInfo
{
    /* Source Image info */
    // char *src_image_fname;
    // FILE *fptr_src_image;  
    // uint image_capacity;   

    /* Secret File Info */
    char secret_fname[30];       
    FILE *fptr_secret;     
    char extn_secret_file[5];
    char secret_data[1000000];
    int size_secret_extn_file;  
    long size_secret_file;   

    /* Stego Image Info */
    char *stego_image_fname; 
    FILE *fptr_stego_image;  

} DecodeInfo;

/* Decoding function prototype */

/* Read and validate decode args from argv */
Status read_and_validate_decode_args(int argc,char *argv[], DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_decode_files(DecodeInfo *decInfo);

/* Get Magic String */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/*Decode extension size*/
Status decode_secret_file_extn_size(DecodeInfo *decInfo);

/* Decode secret file extenstion */
Status decode_secret_file_extn(char *file_extn, DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode a byte into LSB of image data array */
Status decode_byte_to_lsb(char *data, char *image_buffer);

// Decode a size to lsb
Status decode_size_to_lsb(int *size, char *imageBuffer);

#endif

