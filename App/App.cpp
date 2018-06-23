/*
 * Copyright (C) 2011-2018 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


// App.cpp : Defines the entry point for the console application.
#include <stdio.h>
#include <map>
#include "../Enclave1/Enclave1_u.h"
#include "../Enclave2/Enclave2_u.h"
#include "../Enclave3/Enclave3_u.h"
#include "sgx_eid.h"
#include "sgx_urts.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>


#define UNUSED(val) (void)(val)
#define TCHAR   char
#define _TCHAR  char
#define _T(str) str
#define scanf_s scanf
#define _tmain  main

extern std::map<sgx_enclave_id_t, uint32_t>g_enclave_id_map;


sgx_enclave_id_t e1_enclave_id = 0;
sgx_enclave_id_t e2_enclave_id = 0;
sgx_enclave_id_t e3_enclave_id = 0;

#define ENCLAVE1_PATH "libenclave1.so"
#define ENCLAVE2_PATH "libenclave2.so"
#define ENCLAVE3_PATH "libenclave3.so"

void waitForKeyPress()
{
    char ch;
    int temp;
    printf("\n\nHit a key....\n");
    temp = scanf_s("%c", &ch);
}

uint32_t load_enclaves()
{
    uint32_t enclave_temp_no;
    int ret, launch_token_updated;
    sgx_launch_token_t launch_token;

    enclave_temp_no = 0;

    ret = sgx_create_enclave(ENCLAVE1_PATH, SGX_DEBUG_FLAG, &launch_token, &launch_token_updated, &e1_enclave_id, NULL);
    if (ret != SGX_SUCCESS) {
                return ret;
    }

    enclave_temp_no++;
    g_enclave_id_map.insert(std::pair<sgx_enclave_id_t, uint32_t>(e1_enclave_id, enclave_temp_no));

    ret = sgx_create_enclave(ENCLAVE2_PATH, SGX_DEBUG_FLAG, &launch_token, &launch_token_updated, &e2_enclave_id, NULL);
    if (ret != SGX_SUCCESS) {
                return ret;
    }

    enclave_temp_no++;
    g_enclave_id_map.insert(std::pair<sgx_enclave_id_t, uint32_t>(e2_enclave_id, enclave_temp_no));

    ret = sgx_create_enclave(ENCLAVE3_PATH, SGX_DEBUG_FLAG, &launch_token, &launch_token_updated, &e3_enclave_id, NULL);
    if (ret != SGX_SUCCESS) {
                return ret;
    }

    enclave_temp_no++;
    g_enclave_id_map.insert(std::pair<sgx_enclave_id_t, uint32_t>(e3_enclave_id, enclave_temp_no));



    return SGX_SUCCESS;
}

void print(const char* str){
    printf("%s\n", str);
}

void print_num(uint32_t id){
    printf("%u\n", id);
}

void print_to_file(const char* file_path, const char* str){
    FILE* file = fopen(file_path, "w");
    fwrite(str,strlen(str),1,file);
    fclose(file);
    // printf("%u\n", id);
}

int _tmain(int argc, _TCHAR* argv[]){
    uint32_t ret_status;
    sgx_status_t status;

    UNUSED(argc);
    UNUSED(argv);

    if(load_enclaves() != SGX_SUCCESS){
        printf("\nLoad Enclave Failure");
    }

    printf("\nAvailable Enclaves");
    printf("\nEnclave1 - EnclaveID %" PRIx64, e1_enclave_id);
    printf("\nEnclave2 - EnclaveID %" PRIx64, e2_enclave_id);
    printf("\nEnclave3 - EnclaveID %" PRIx64, e3_enclave_id);

    const int KEY_DATA_LEN = 16;
    const int MAC_DATA_LEN = 16;
    char key[KEY_DATA_LEN], mac_data[MAC_DATA_LEN];
    char *plaintext, *ciphertext;
    uint32_t plaintext_len, ciphertext_len;

    // Read key from key.txt.
    FILE* key_file = fopen("./key.txt","r");
    if (key_file == NULL){
        printf("\nRead key file failed!");
        return 1;
    }
    fread(key, KEY_DATA_LEN, 1, key_file); key[KEY_DATA_LEN] = 0;
    fclose(key_file);
    printf("\nKey: %s", key);

    // Read plaintext from plaintext.txt.
    FILE* plain_file = fopen("./ciphertext.txt","r");
    if (plain_file == NULL){
        printf("\nRead plaintext file failed!");
        return 1;
    }
    fseek(plain_file, 0, SEEK_END);
    plaintext_len = ftell(plain_file);
    rewind(plain_file);
    plaintext = (char *)malloc(sizeof(char) * (plaintext_len + 1));
    fread(plaintext, plaintext_len, 1, plain_file);
    printf("\nPlaintext: %s", plaintext);
    fclose(plain_file);
    
    //initial ciphertext
    ciphertext = (char *)malloc(sizeof(char) * (1024 + 1));
    memset(ciphertext, 0, sizeof(ciphertext));
    ciphertext_len = 0;

    // Read mac_data from mac.txt.
    FILE* mac_file = fopen("./mac.txt","r");
    if (mac_file == NULL){
        printf("\nRead mac file failed!");
        memset(mac_data, 0, sizeof(mac_data));
    }else{
        fread(mac_data, MAC_DATA_LEN, 1, mac_file); mac_data[MAC_DATA_LEN] = 0;
        fclose(mac_file);
        printf("\nMac: %s", mac_data);
    }

    // Create session between Enclave1(Source) and Enclave2(Destination)
    status = Enclave1_test_create_session(e1_enclave_id, &ret_status, e1_enclave_id, e2_enclave_id);
    if (status!=SGX_SUCCESS){
        printf("\nEnclave1_test_create_session Ecall failed: Error code is %x", status);
        return 1;
    }
    else {
        if(ret_status==0){
            printf("\n\nSecure Channel Establishment between Source (E1) and Destination (E2) Enclaves successful !!!");
        }
        else{
            printf("\nSession establishment and key exchange failure between Source (E1) and Destination (E2): Error code is %x", ret_status);
            return 1;
        }
    }
    
    // printf("\nfinishA\n");
    // Call Encrypt function of Enclave2
    status = Enclave1_test_call_encrypt(e1_enclave_id, &ret_status, 
                                        e1_enclave_id, e2_enclave_id, key, mac_data,
                                        plaintext, &plaintext_len, ciphertext, &ciphertext_len, 0);
    if (status!=SGX_SUCCESS){
        printf("\nEnclave1_test_enclave_to_enclave_call Ecall failed: Error code is %x", status);
        return 1;
    }
    else{
        if(ret_status==0){
            printf("\n\nEnclave to Enclave Call between Source (E1) and Destination (E2) Enclaves successful !!!");
        }
        else{
            printf("\n\nEnclave to Enclave Call failure between Source (E1) and Destination (E2): Error code is %x", ret_status);
            return 1;
        }
    }
    // print_to_file("mid3.txt", ciphertext);
    // print("test");
    // print_num(ciphertext_len);

    // write ciphertext to file
    print(ciphertext);
    FILE* cipher_file = fopen("./plain.txt", "w");
    if (cipher_file == NULL){
        printf("\nOpen plaintext file failed!");
        return 1;
    }
    fwrite(ciphertext, ciphertext_len, 1, cipher_file);
    fclose(cipher_file);
    

    // write mac_data to file
    FILE* mac_out_file = fopen("./mac.txt", "w");
    if (mac_out_file == NULL){
        printf("\nOpen plaintext file failed!");
        return 1;
    }
    fwrite(mac_data, MAC_DATA_LEN, 1, mac_out_file);
    fclose(mac_out_file);

    printf("\nEncrypt Successful!!!");

    // Call Encrypt 
    //
    /*do
    {
        //Test Create session between Enclave1(Source) and Enclave2(Destination)
        status = Enclave1_test_create_session(e1_enclave_id, &ret_status, e1_enclave_id, e2_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave1_test_create_session Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nSecure Channel Establishment between Source (E1) and Destination (E2) Enclaves successful !!!");
            }
            else
            {
                printf("\nSession establishment and key exchange failure between Source (E1) and Destination (E2): Error code is %x", ret_status);
                break;
            }
        }

        //Test Enclave to Enclave call between Enclave1(Source) and Enclave2(Destination)
        status = Enclave1_test_enclave_to_enclave_call(e1_enclave_id, &ret_status, e1_enclave_id, e2_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave1_test_enclave_to_enclave_call Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nEnclave to Enclave Call between Source (E1) and Destination (E2) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nEnclave to Enclave Call failure between Source (E1) and Destination (E2): Error code is %x", ret_status);
                break;
            }
        }
        //Test message exchange between Enclave1(Source) and Enclave2(Destination)
        status = Enclave1_test_message_exchange(e1_enclave_id, &ret_status, e1_enclave_id, e2_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave1_test_message_exchange Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nMessage Exchange between Source (E1) and Destination (E2) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nMessage Exchange failure between Source (E1) and Destination (E2): Error code is %x", ret_status);
                break;
            }
        }
        //Test Create session between Enclave1(Source) and Enclave3(Destination)
        status = Enclave1_test_create_session(e1_enclave_id, &ret_status, e1_enclave_id, e3_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave1_test_create_session Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nSecure Channel Establishment between Source (E1) and Destination (E3) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nSession establishment and key exchange failure between Source (E1) and Destination (E3): Error code is %x", ret_status);
                break;
            }
        }
        //Test Enclave to Enclave call between Enclave1(Source) and Enclave3(Destination)
        status = Enclave1_test_enclave_to_enclave_call(e1_enclave_id, &ret_status, e1_enclave_id, e3_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave1_test_enclave_to_enclave_call Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nEnclave to Enclave Call between Source (E1) and Destination (E3) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nEnclave to Enclave Call failure between Source (E1) and Destination (E3): Error code is %x", ret_status);
                break;
            }
        }
        //Test message exchange between Enclave1(Source) and Enclave3(Destination)
        status = Enclave1_test_message_exchange(e1_enclave_id, &ret_status, e1_enclave_id, e3_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave1_test_message_exchange Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nMessage Exchange between Source (E1) and Destination (E3) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nMessage Exchange failure between Source (E1) and Destination (E3): Error code is %x", ret_status);
                break;
            }
        }

        //Test Create session between Enclave2(Source) and Enclave3(Destination)
        status = Enclave2_test_create_session(e2_enclave_id, &ret_status, e2_enclave_id, e3_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave2_test_create_session Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nSecure Channel Establishment between Source (E2) and Destination (E3) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nSession establishment and key exchange failure between Source (E2) and Destination (E3): Error code is %x", ret_status);
                break;
            }
        }
        //Test Enclave to Enclave call between Enclave2(Source) and Enclave3(Destination)
        status = Enclave2_test_enclave_to_enclave_call(e2_enclave_id, &ret_status, e2_enclave_id, e3_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave2_test_enclave_to_enclave_call Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nEnclave to Enclave Call between Source (E2) and Destination (E3) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nEnclave to Enclave Call failure between Source (E2) and Destination (E3): Error code is %x", ret_status);
                break;
            }
        }
        //Test message exchange between Enclave2(Source) and Enclave3(Destination)
        status = Enclave2_test_message_exchange(e2_enclave_id, &ret_status, e2_enclave_id, e3_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave2_test_message_exchange Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nMessage Exchange between Source (E2) and Destination (E3) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nMessage Exchange failure between Source (E2) and Destination (E3): Error code is %x", ret_status);
                break;
            }
        }
    
        //Test Create session between Enclave3(Source) and Enclave1(Destination)
        status = Enclave3_test_create_session(e3_enclave_id, &ret_status, e3_enclave_id, e1_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave3_test_create_session Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nSecure Channel Establishment between Source (E3) and Destination (E1) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nSession establishment and key exchange failure between Source (E3) and Destination (E1): Error code is %x", ret_status);
                break;
            }
        }
        //Test Enclave to Enclave call between Enclave3(Source) and Enclave1(Destination)
        status = Enclave3_test_enclave_to_enclave_call(e3_enclave_id, &ret_status, e3_enclave_id, e1_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave3_test_enclave_to_enclave_call Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nEnclave to Enclave Call between Source (E3) and Destination (E1) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nEnclave to Enclave Call failure between Source (E3) and Destination (E1): Error code is %x", ret_status);
                break;
            }
        }
        //Test message exchange between Enclave3(Source) and Enclave1(Destination)
        status = Enclave3_test_message_exchange(e3_enclave_id, &ret_status, e3_enclave_id, e1_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave3_test_message_exchange Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nMessage Exchange between Source (E3) and Destination (E1) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nMessage Exchange failure between Source (E3) and Destination (E1): Error code is %x", ret_status);
                break;
            }
        }


        //Test Closing Session between Enclave1(Source) and Enclave2(Destination)
        status = Enclave1_test_close_session(e1_enclave_id, &ret_status, e1_enclave_id, e2_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave1_test_close_session Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nClose Session between Source (E1) and Destination (E2) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nClose session failure between Source (E1) and Destination (E2): Error code is %x", ret_status);
                break;
            }
        }
        //Test Closing Session between Enclave1(Source) and Enclave3(Destination)
        status = Enclave1_test_close_session(e1_enclave_id, &ret_status, e1_enclave_id, e3_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave1_test_close_session Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nClose Session between Source (E1) and Destination (E3) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nClose session failure between Source (E1) and Destination (E3): Error code is %x", ret_status);
                break;
            }
        }
        //Test Closing Session between Enclave2(Source) and Enclave3(Destination)
        status = Enclave2_test_close_session(e2_enclave_id, &ret_status, e2_enclave_id, e3_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave2_test_close_session Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nClose Session between Source (E2) and Destination (E3) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nClose session failure between Source (E2) and Destination (E3): Error code is %x", ret_status);
                break;
            }
        }
        //Test Closing Session between Enclave3(Source) and Enclave1(Destination)
        status = Enclave3_test_close_session(e3_enclave_id, &ret_status, e3_enclave_id, e1_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave3_test_close_session Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nClose Session between Source (E3) and Destination (E1) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nClose session failure between Source (E3) and Destination (E1): Error code is %x", ret_status);
                break;
            }
        }

#pragma warning (push)
#pragma warning (disable : 4127)    
    }while(0);
#pragma warning (pop)*/

    sgx_destroy_enclave(e1_enclave_id);
    sgx_destroy_enclave(e2_enclave_id);
    sgx_destroy_enclave(e3_enclave_id);

    waitForKeyPress();

    return 0;
}
