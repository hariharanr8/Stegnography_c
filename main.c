/*
NAME:         HARIHARAN R
DATE:         25/05/2026
Description: 
    This project is a command-line C program that hides a secret file inside a BMP image 
    using Least Significant Bit (LSB) Image Steganography, allowing users to both encode (hide) 
    and decode (extract) data. When encoding, the program uses bitwise operators to carefully 
    embed a unique 2-character magic string, the secret file's extension type, and the actual
    secret message into the image by swapping out the very last bit of the image’s color bytes—a 
    microscopic change that keeps the visual appearance of the photo completely unchanged 
    to the human eye. To ensure seamless recovery during decoding, the program extracts 
    this hidden blueprint bit-by-bit, automatically reconstructs the original 
    file extension (such as .txt), and recreates the secret file with its exact text perfectly intact.

*/

#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{
    //step1 -> call check_operation_type(argv[1]);
            // print msg
    int ret=check_operation_type(argv[1]);
    if(ret==e_encode){
        EncodeInfo encInfo;
        //check read_and_validate_encode_args(argc,argv, &encInfo) is e_success or not
                // no -> print error msg and return e_failure;
                // yes -> check do_encoding(&encInfo) returning e_success or not
                            // no -> print error msg and stop 
                            // yes -> print success msg and stop
        if((read_and_validate_encode_args(argc,argv,&encInfo))==e_success){
            do_encoding(&encInfo);
        }
        else{
            printf("read and validate error!\n");
        }
    }
    else if(ret==e_decode){
        DecodeInfo decInfo;
        //check read_and_validate_decode_args(argc,argv, &decInfo) is e_success or not
                // no -> print error msg and return e_failure;
                // yes -> check do_decoding(&decInfo) returning e_success or not
                            // no -> print error msg and stop 
                            // yes -> print success msg and stop
        if((read_and_validate_decode_args(argc,argv,&decInfo))==e_success){
            do_decoding(&decInfo);
        }
        else{
            printf("read and validate error!\n");
        }
    }
    else{
        printf("Invalid operation!\n");
    }

}

OperationType check_operation_type(char *symbol)
{
    //step -> check it is -e or -d
    if(!strcmp(symbol,"-e")){
        return e_encode;
    }
    if(!strcmp(symbol,"-d")){
        return e_decode;
    }
    return e_unsupported;
}