#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{   
    // Find the size of secret file data
    // return secret file size
    fseek(fptr,0,SEEK_END);
    printf("INFO: Checking for secret.txt size\n");
    int size=ftell(fptr);
    if(size>0){
        printf("INFO: Done. Not Empty\n");   
    }
    rewind(fptr);
    return size;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status read_and_validate_encode_args(int argc,char *argv[], EncodeInfo *encInfo)
{
    //step1 -> check arguments present or not using argc(count)
    if(argc<4 || argc>5){
        printf("ERROR: Insufficient arguments\n");
        return e_failure;
    }
    // step2 -> check argv[2] having .bmp or not
         // no -> return e_failure
         // yes -> store argv[2] into encInfo -> src_image_fname = agrv[2]
    int i=1,flag=1;
    while(argv[2][i]){
        if(argv[2][i]=='.'){
            if(!strcmp(&argv[2][i],".bmp")){
                encInfo->src_image_fname = argv[2];
                flag=0;
                break;
            }
        }
        i++;
    }
    if(flag){
        return e_failure;
    }
    
    // step3 -> check argv[3] having . or not
        // no -> return e_failure
        // yes -> store argv[3] into encInfo ->secret_fname = argv[3]
    i=0,flag=1; 
    while(argv[3][i]){
        if(argv[3][i]=='.'){
            if(argv[3][i+1]=='\0'){
                return e_failure;
            }
            else{
                strcpy(encInfo->extn_secret_file,&argv[3][i]);
                encInfo->secret_fname = argv[3];
                flag=0;
                break;
            }
        }
        i++;
    }
    if(flag){
        return e_failure;
    }
    // step4 -> check argv[4] is NULL or not
        // yes -> store default name encInfo -> stego_image_fname = "stego.bmp"
        
        // no ->check argv[4] is having .bmp or not
                //no -> return e_failure
                //yes -> store argv[4] into encInfo -> stego_image_fname = argv[4]
    if(argv[4]==NULL){
        printf("INFO: Output File not mentioned. Creating stego.bmp as default\n");
        encInfo -> stego_image_fname = "stego.bmp";
    }
    else{
        int len = strlen(argv[4]);
        if(!strcmp(&argv[4][len-4],".bmp")){
            encInfo -> stego_image_fname = argv[4];
        }
        else{
            return e_failure;
        } 
    }
    return e_success;
}

//Open required files
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }
    // --- NEW MEMORY-SAFE ALLOCATION LOGIC ---
    // Calculate the exact size of the secret file
    fseek(encInfo->fptr_secret, 0, SEEK_END);
    long secret_size = ftell(encInfo->fptr_secret);
    rewind(encInfo->fptr_secret);

    // Allocate exact memory + 1 byte extra for the null terminator '\0'
    encInfo->secret_data = malloc(secret_size + 1);
    if (encInfo->secret_data == NULL)
    {
        fprintf(stderr, "ERROR: Out of memory allocation\n");
        return e_failure;
    }
    // ----------------------------------------
    int i=0,ch;
    while((ch=fgetc(encInfo->fptr_secret))!=EOF){
        encInfo->secret_data[i++] = ch;
    }
    encInfo->secret_data[i]='\0';
    rewind(encInfo->fptr_secret);

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    // step1 -> encInfo -> image_capacity = get_image_size_for_bmp(encInfo -> fptr_src_image);
    encInfo -> image_capacity = get_image_size_for_bmp(encInfo -> fptr_src_image);
    // step2 -> encInfo -> size_secret_file = get_file_size(encInfo -> fptr_secret)
    encInfo -> size_secret_file = get_file_size(encInfo -> fptr_secret);
    // step3 -> check encInfo -> image_capacity > 16+32+32+32+(encInfo -> size_secret_file * 8)
                //yes -> return e_success
                //no -> return e_failure
    if(encInfo->image_capacity > ((strlen(MAGIC_STRING)*8)+32+(strlen(encInfo->extn_secret_file)*8)+32+((encInfo->size_secret_file)*8))){
        return e_success;
    }
    return e_failure;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    printf("INFO: Copying Image Header\n");
    // 1. Point to the beginning of the file
    rewind(fptr_src_image);
    // 2. Seek to byte 10 where the Pixel Data Offset is stored
    unsigned int bmp_header_size = 0;
    fseek(fptr_src_image, 10, SEEK_SET);
    // 3. Read the 4-byte offset integer
    fread(&bmp_header_size, sizeof(unsigned int), 1, fptr_src_image);
    // 4. Go back to the very beginning to prepare for copying
    rewind(fptr_src_image);
    // 5. Dynamically allocate a temporary buffer to hold this specific header size
    char *header_buffer = malloc(bmp_header_size);
    if (header_buffer == NULL)
    {
        fprintf(stderr, "ERROR: Memory allocation failed for header copy\n");
        return e_failure;
    }
    // 6. Read the entire header from source and write it cleanly to destination
    fread(header_buffer, bmp_header_size, 1, fptr_src_image);
    fwrite(header_buffer, bmp_header_size, 1, fptr_dest_image); 
    // 7. Clean up the temporary buffer memory
    free(header_buffer);
    return e_success;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    // char buffer[8];
    printf("INFO: Encoding Magic String Signature\n");
    int i=0;
    char mag_buffer[8];
    while(magic_string[i]!='\0'){
        //step1 -> read 8 bytes from src file
        fread(mag_buffer,8,1,encInfo->fptr_src_image);
        //step2 -> call encode_byte_to_lsb(magic_string[0], buffer)
        encode_byte_to_lsb(magic_string[i],mag_buffer);
        //step3 -> write the buffer into dest file
        fwrite(mag_buffer,8,1,encInfo->fptr_stego_image);
        i++;
    }
    //step4 -> repeat this for size of magic_string time
    return e_success; 
}
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    // char buffer[32];
    printf("INFO: Encoding Secret.txt File Extension size\n");
    char ex_buffer[32];
    //step1 -> read 32 bytes from src file
    fread(ex_buffer,32,1,encInfo->fptr_src_image);
    //step2 -> call encode_size_to_lsb(size, buffer)
    encode_size_to_lsb((long)size, ex_buffer);
    fwrite(ex_buffer,32,1,encInfo->fptr_stego_image);
    return e_success;
    //step3 -> write the buffer into dest file
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    // char buffer[8];
    printf("INFO: Encoding Secret.txt File Extension\n");
    char extn_buffer[8];
    //step1 -> read 8 bytes from src file
    int i=0;
    while(file_extn[i]){
        fread(extn_buffer,8,1,encInfo->fptr_src_image);
        //step2 -> call encode_byte_to_lsb
        encode_byte_to_lsb(file_extn[i], extn_buffer);
        //step3 -> write the buffer into dest file
        fwrite(extn_buffer,8,1,encInfo->fptr_stego_image);
        i++;
    }
    //step4 -> repeat this for size of extn time
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    // char buffer[32];
    printf("INFO: Encoding Secret.txt File Size\n");
    char f_buffer[32];
    //step1 -> read 32 bytes from src file
    fread(f_buffer,32,1,encInfo->fptr_src_image);
    //step2 -> call encode_size_to_lsb(size, buffer)
    encode_size_to_lsb(file_size, f_buffer);
    //step3 -> write the buffer into dest file
    fwrite(f_buffer,32,1,encInfo->fptr_stego_image); 
    return e_success; 
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    // char buffer[8];
    printf("INFO: Encoding secret.txt File Data\n");
    char fdata_buffer[8];
    int i=0;
    while(encInfo->secret_data[i]!='\0'){
        //step1 -> read 8 bytes from src file
        fread(fdata_buffer,8,1,encInfo->fptr_src_image);
        //step2 -> call encode_byte_to_lsb
        encode_byte_to_lsb(encInfo->secret_data[i], fdata_buffer);
        //step3 -> write the buffer into dest file
        fwrite(fdata_buffer,8,1,encInfo->fptr_stego_image);
        i++;
    }
    //step4 -> repeat this for size of secret_data time
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    //logic to copy reamaining data
    printf("INFO: Copying Left Over Data\n");
    
    char buffer[4096]; // Safe 4KB transfer window
    size_t bytes_read;

    // Standard stream replication loop
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fptr_src)) > 0) {
        fwrite(buffer, 1, bytes_read, fptr_dest);
    }
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    // write logic encode one char
    int mask;
    for(int i=0;i<8;i++){
        mask=(image_buffer[i]&(~1));
        image_buffer[i]=(((data>>(7-i))&1) | mask);
    }
    return e_success;
}

Status encode_size_to_lsb(long size, char *imageBuffer)
{
    // write logic encode size
    int mask;
    for(int i=31;i>=0;i--){
        mask=(imageBuffer[31-i]&(~1));//clear lsb bit
        imageBuffer[31-i]=(((size>>i)&1) | mask);
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{   
    //step1 -> check open_files(encInfo) returning e_success or not
        // yes -> print success msg goto next step
        // no -> return e_failure
    if((open_files(encInfo))==e_success){
        printf("INFO: Opening required files\n");
        printf("INFO: Opened sample.bmp\n");
        printf("INFO: Opened secret.txt\n");
        printf("INFO: Opened stego.bmp\n");
        printf("INFO: Done\n");
        printf("INFO: ## Encoding Procedure Started ##\n");
    }
    else{
        return e_failure;
    }
    
    //step2 -> check check_capacity(encInfo) is returning success or not
            // yes -> print success msg and goto next step
            // no -> print error msg and return e_failure
    if(check_capacity(encInfo)==e_success){
        printf("INFO: Checking for beautiful.bmp capacity to handle secret.txt\n");
        printf("INFO: Done. Data Sufficient\n");
    }
    else{
        printf("ERROR: Capacity not Found\n");
        return e_failure;
    }
    //step3 -> call copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image)
    if(copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image)==e_success){
        printf("INFO: Done\n");
    }
    //step4 -> call encode_magic_string(MAGIC_STRING, encInfo)
    if((encode_magic_string(MAGIC_STRING, encInfo))==e_success){
        printf("INFO: Done\n");
    }
    //step5 -> call encode_secret_file_extn_size(strlen(encInfo -> extn_secret_file), encInfo)
    if((encode_secret_file_extn_size(strlen(encInfo -> extn_secret_file), encInfo))==e_success){
        printf("INFO: Done\n");
    }
    //step6 -> call encode_secret_file_extn(encInfo -> extn_secret_file, encInfo)
    if((encode_secret_file_extn(encInfo -> extn_secret_file, encInfo))==e_success){
        printf("INFO: Done\n");
    }
    //step7 -> Call encode_secret_file_size(encInfo -> size_secret_file, encInfo)
    if((encode_secret_file_size(encInfo -> size_secret_file, encInfo))==e_success){
        printf("INFO: Done\n");
    }
    //step8 -> call encode_secret_file_data(encInfo)
    if((encode_secret_file_data(encInfo))==e_success){
        printf("INFO: Done\n");
    }
    //step9 -> call copy_remaining_img_data(encInfo -> fptr_src_image, encInfo -> stego_image_fname);
    if((copy_remaining_img_data(encInfo -> fptr_src_image, encInfo -> fptr_stego_image))==e_success){
        printf("INFO: Done\n");
    }
    free(encInfo->secret_data);
    printf("INFO: ## Encoding Done Successfully ##\n");
    return e_success;
}