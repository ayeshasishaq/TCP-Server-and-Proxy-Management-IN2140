/*
 * This file implements two functions that read XML and binary information from a buffer,
 * respectively, and return pointers to Record or NULL.
 *
 * *** YOU MUST IMPLEMENT THESE FUNCTIONS ***
 *
 * The parameters and return values of the existing functions must not be changed.
 * You can add function, definition etc. as required.
 */
#include "recordFromFormat.h"
#include "record.h"
#include <string.h>
#include <stdint.h>
#include <netinet/in.h>  // for ntohl
#include <arpa/inet.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>


//The recordFromFormat.c file contains two functions for converting student records from different formats:

Record* XMLtoRecord( char* buffer, int bufSize, int* bytesread ){
    Record *r = newRecord();
    
    if (r == NULL) {
        perror("Failed to create new record");
        return NULL;
    }

    char *ptr = buffer;
    char *end;
    char *lastProcessedRecordEnd = NULL;

// checking for end tag so it doesnt crash when sleep 0
    char *recordEnd = strstr(ptr, "</record>");
    if(recordEnd == NULL){
        deleteRecord(r);
        return NULL;
    }
    recordEnd[0] = '\0';


    while ( ptr < buffer + bufSize && strncmp(ptr, "</record>", 9) != 0) {  // as long as it is within buffer and end tag not found

         ptr = strstr(ptr, "<record>") ;
            if (ptr == NULL) {
                printf("%s", "\nPointer to buffer is NULL");
  // process amount of bytes read
            if (lastProcessedRecordEnd != NULL) {
                *bytesread = lastProcessedRecordEnd - buffer;
            } else {
                *bytesread = ptr - buffer;
            }
            deleteRecord(r); //
            return NULL;
        }

            printf("%s", "\nPointer to buffer is not NULL");
            printf("\n------Pointer reading-----:\n %s\n--------------", ptr);
            ptr += strlen("<record>");
           
        // setting source
        char *sourcePtr = strstr(ptr, "<source=");
        if (sourcePtr != NULL) {
            sourcePtr += 9;
            setSource(r, *sourcePtr);
            ptr = sourcePtr + 2;
        }

        char *destPtr = strstr(ptr, "<dest=");
        if (destPtr != NULL) {
            destPtr += 7;
            setDest(r, *destPtr);
            ptr = destPtr + 2;
        }
        // checking length of username
        char *usernamePtr = strstr(ptr, "<username=");
        if (usernamePtr != NULL) {
            usernamePtr += 11;
            end = strchr(usernamePtr, '\"');
             if (end){
                *end = '\0';
             } 
             setUsername(r, usernamePtr);
            ptr = end + 1;
        }

        char *idPtr = strstr(ptr, "<id=");
        if (idPtr != NULL) {
            idPtr += 5;
            setId(r, atoi(idPtr)); // used to convert from ASCII to integer
            ptr = idPtr + 2;
        }

        char *groupPtr = strstr(ptr, "<group=");
        if (groupPtr != NULL) {
            groupPtr += 8;
            setGroup(r, atoi(groupPtr));
            ptr = groupPtr + 2;
        }

        char *semesterPtr = strstr(ptr, "<semester=");
        if (semesterPtr != NULL) {
            semesterPtr += 11;
            setSemester(r, atoi(semesterPtr));
            ptr = semesterPtr + 2;
        }
        // check for the flags of grade
        char *gradePtr = strstr(ptr, "<grade=");
        int teller;
        if (gradePtr != NULL) {
            gradePtr += 8;
            end = strchr(gradePtr, '\"');
            if (end){
                *end = '\0';
            } 
            
            if(strcmp(gradePtr, "None")== 0){
                teller = 0;
            }else if(strcmp(gradePtr, "Bachelor")== 0){
                teller =  1;
            }else if(strcmp(gradePtr, "Master")== 0){
                teller = 2;
            }else if(strcmp(gradePtr, "PhD")== 0){
                teller = 3; 
            }else{
                printf("Grade not found\n");
            }

            setGrade(r, teller);
            ptr = end + 1;
        }
        // check the flags og course
        char *coursePtr = strstr(ptr, "<course=");
        int courseTeller; 
        while (coursePtr != NULL) {
            coursePtr += 9;
            end = strchr(coursePtr, '\"');
            if (end){
                *end = '\0';
            }

            if(strcmp(coursePtr, "IN1000")== 0){
                courseTeller = Course_IN1000;
                setCourse(r, courseTeller);
            } else if(strcmp(coursePtr, "IN1010")== 0){
                courseTeller = Course_IN1010;
                setCourse(r, courseTeller);
            } else if(strcmp(coursePtr, "IN1020")== 0){
                courseTeller = Course_IN1020;
                setCourse(r, courseTeller);            
            } else if(strcmp(coursePtr, "IN1030")== 0){
                courseTeller = Course_IN1030;
                setCourse(r, courseTeller);      
            } else if(strcmp(coursePtr, "IN1050")== 0){
                courseTeller = Course_IN1050;
                setCourse(r, courseTeller);      
            } else if(strcmp(coursePtr, "IN1060")== 0){
                courseTeller = Course_IN1060;
                setCourse(r, courseTeller);      
            } else if(strcmp(coursePtr, "IN1080")== 0){
                courseTeller = Course_IN1080;
                setCourse(r, courseTeller);      
            } else if(strcmp(coursePtr, "IN1140")== 0){
                courseTeller = Course_IN1140;
                setCourse(r, courseTeller);      
            } else if(strcmp(coursePtr, "IN1150")== 0){
                courseTeller = Course_IN1150;
                setCourse(r, courseTeller);      
            } else if(strcmp(coursePtr, "IN1900")== 0){
                courseTeller = Course_IN1900;
                setCourse(r, courseTeller);      
            } else if(strcmp(coursePtr, "IN1910")== 0){
                courseTeller = Course_IN1910;
                setCourse(r, courseTeller);      
            } else {
                printf("Course not found\n");
            }

            printf("\nCourses:%s", coursePtr);
            coursePtr = strstr(end + 1, "<course=");

        }
        // when end tag found update bytesread
        *bytesread = recordEnd + strlen("</record>") - buffer;
        return r;
    }
}




Record* BinaryToRecord(char* buffer, int bufSize, int* bytesread) {
    Record *r = newRecord();
    if (r == NULL) {
        perror("Failed to create new record");
        return NULL;
    }

    char *ptr = buffer;
    uint8_t flags = *ptr;
    ptr++;

    if (flags & FLAG_SRC) {  // source flag
        setSource(r, *ptr);
        ptr++;
    }

    if (flags & FLAG_DST) {  // dest flag
        setDest(r, *ptr);
        ptr++;
    }
    // finding lentgh of username
    if (flags & FLAG_USERNAME) {  // username flag
        uint32_t* username_ptr = (uint32_t*) ptr;
        uint32_t username_length = ntohl(*username_ptr);
        ptr += 4;

        char* username = malloc(username_length + 1);
        memcpy(username, ptr, username_length);
        username[username_length] = '\0';
        setUsername(r, username);
        free(username);  
        ptr += username_length;
    }

    if (flags & FLAG_ID) {  // id flag
        uint32_t* id_ptr = (uint32_t*) ptr;
        uint32_t id = ntohl(*id_ptr);
        
        ptr += 4;
        setId(r, id);
    }

    if (flags & FLAG_GROUP) {  // group flag
         // Cast the pointer to uint32_t* 
        uint32_t* group_ptr = (uint32_t*) ptr;
        uint32_t group = ntohl(*group_ptr);
        ptr += 4;
        setGroup(r, group);
    }

    if (flags & FLAG_SEMESTER) {  // semester flag // leser semester feeil i XML
        uint8_t semester = *ptr;
        ptr++;
        setSemester(r, semester);
    }

    if (flags & FLAG_GRADE) {  // grade flag
        uint8_t grade = *ptr;
        ptr++;
        setGrade(r, (Grade) grade);
    }

    if (flags & FLAG_COURSES) {  // course flag
        uint16_t* course_ptr = (uint16_t*) ptr;
        uint16_t course = ntohs(*course_ptr);
        ptr += 2;
        setCourse(r, course);
        
    }
// if incomplete record
    *bytesread = ptr - buffer;
    if(*bytesread > bufSize){
        printf("Incomplete record received.\n");
       // deleteRecord(r);
        return NULL;
    }
    return r;
}


