#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


#define LOG_NAME  "UMSATS_ROCKET.log"
#define OUTPUT_NAME "flightComputer.csv"

#define ACC_TYPE    0x8000
#define GYRO_TYPE   0x4000
#define PRES_TYPE   0x2000
#define TEMP_TYPE   0x1000

#define ACC_OFFSET  0
#define GYRO_OFFSET 6
#define PRES_OFFSET 12
#define TEMP_OFFSET 15

typedef struct{

	int16_t x;
	int16_t y;
	int16_t z;
    uint16_t time;

} acc_measure;

typedef struct{

	int16_t x;
	int16_t y;
	int16_t z;
    uint16_t time;

} gyro_measure;

typedef struct{

    uint32_t pres;
    uint16_t time;


} pres_measure;

typedef struct{

	int32_t temp;
    uint16_t time;


} temp_measure;


typedef struct{

    uint16_t header;
    uint8_t data[18];
    
}measure;

void printArray(char arr[], int n){


        int i;
        for(i=0;i<n;i++){
        
            printf("%x ",arr[i]);
        }
        printf("\n");
}

int main(){

    FILE *fp;
    FILE *fp_out;

    char buffer[100];
    uint8_t m_buff[20];
    measure m;
   int m_count =0;
    uint32_t bytesRead=0;
    int numLong=0;
    int numShort=0;
    fp = fopen(LOG_NAME,"r");
    if(fp == NULL){
        printf("Could not open log file.:%s\n",LOG_NAME);
    }
    fp_out = fopen(OUTPUT_NAME,"w");
    if(fp_out == NULL){
        printf("Could not open log file:%s.\n",OUTPUT_NAME);
    }


    
    //Skip the first two lines.
    fgets(buffer,100,fp);
   printf("Skipped: %s\n",buffer); 
    fgets(buffer,100,fp);
   printf("Skipped: %s\n",buffer);

    uint8_t  n_char = '\0';
    int count=0;
    while(n_char == '\0'){
        n_char = fgetc(fp);
        count++;
    }
    printf("Skiped %d null chars.\n",count);
    fseek(fp,-1,SEEK_CUR);
   
    uint32_t prevTime =0;
    int done = 0;
   int stat = 0;
    
   
   pres_measure p;
   temp_measure t;
   acc_measure a;
   gyro_measure g;
    uint32_t running_count = 0; 
  uint32_t buff_sz = 256;
    
    while(!done){

    stat = fread(&m,1,2,fp);
    if(stat != 2){
            done =1;
            break;
    }
    /*while(m.header ==0){
     
    stat = fread(&m,1,2,fp);
    if(stat != 2){
            done =1;
            break;
    }

    }
    */
/*
    if(m.header == 0){
    
    fseek(fp,256-running_count,SEEK_CUR);
    continue;
    }
 */   uint8_t length =0;
    m_count ++;
    printf("%d Header value: %x\n",m_count,m.header);
    
    
    if(m.header& ACC_TYPE){
        printf("This measurement contains accelerometer data.\n");
        length += 6;
        
    }
     if(m.header& GYRO_TYPE){
        printf("This measurement contains gyroscope data.\n");
        length +=6;
     }
    if(m.header& PRES_TYPE){
        printf("This measurement contains pressure data.\n");
        length+=3;
    }
     if(m.header& TEMP_TYPE){
        printf("This measurement contains temperature data.\n");
        length+=3;
        numLong++;
     }else{
     numShort++;
     }
    
    running_count +=length;
     stat = fread(&m.data,1,length,fp);
     if(stat != length){
     
             done =1;
             break;
     }
       bytesRead += 2+length; 
      printArray(m.data,12); 
     
     uint32_t time_delta = m.header &(0x0FFF);
     uint32_t time_abs = time_delta+prevTime;
     printf("At time t=%d: \n",time_delta+prevTime);

     prevTime += time_delta;
     if(m.header& ACC_TYPE){
        
        a.x = (m.data[0+ACC_OFFSET]<<8) + m.data[1+ACC_OFFSET];
        a.y = (m.data[2+ACC_OFFSET]<<8) + m.data[3+ACC_OFFSET];
        a.z = (m.data[4+ACC_OFFSET]<<8) + m.data[5+ACC_OFFSET];

        
        printf("A.x = %d A.y = %d A.z = %d \n",a.x,a.y,a.z);
    }
     if(m.header& GYRO_TYPE){
        
        g.x = (m.data[0+GYRO_OFFSET]<<8) + m.data[1+GYRO_OFFSET];
        g.y = (m.data[2+GYRO_OFFSET]<<8) + m.data[3+GYRO_OFFSET];
        g.z = (m.data[4+GYRO_OFFSET]<<8) + m.data[5+GYRO_OFFSET];
        
        printf("G.x = %d G.y = %d G.z = %d \n",g.x,g.y,g.z);


     }
    if(m.header& PRES_TYPE){
        
        p.pres = (m.data[0+PRES_OFFSET]<<16) + (m.data[1+PRES_OFFSET]<<8) + m.data[2+PRES_OFFSET];
        
        printf("P:%d\n",p.pres);


    
    }
     if(m.header& TEMP_TYPE){
     
        t.temp = (m.data[0+TEMP_OFFSET]<<16) + (m.data[1+TEMP_OFFSET]<<8) + m.data[2+TEMP_OFFSET];
        
        printf("T:%d\n",t.temp);

     }

        char str[50];
        if(m.header & PRES_TYPE){
            sprintf(str,"%d,%d,%d,%d,%d,%d,%d,%d,%d\n",time_abs,a.x,a.y,a.z,g.x,g.y,g.z,p.pres,t.temp);
        }
        else{
            sprintf(str,"%d,%d,%d,%d,%d,%d,%d,%d,%d\n",time_abs,a.x,a.y,a.z,g.x,g.y,g.z,p.pres,t.temp);
            //sprintf(str,"%d,%d,%d,%d,%d,%d,%d\n",time_abs,a.x,a.y,a.z,g.x,g.y,g.z);


        }

        fputs(str,fp_out);

    }
    fclose(fp);
    fclose(fp_out);
    printf("Num of long: %d num ofd short: %d\n",numLong,numShort) ;                                  
    printf("Bytes read: %d",bytesRead);

    return 0;
}
