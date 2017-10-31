#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<sys/time.h>
#include <time.h>
#include<fcntl.h>
#include"lz4.h"
#include"lz4hc.h"

#define REC_TIMES 5000

int rand_num(int max_n)
{
    struct timeval seed;
    int n;
    gettimeofday(&seed,NULL);
    srand(seed.tv_usec);
    n = (1+(int) ((float)max_n*rand()/(RAND_MAX+1.0)));
    return n;
}

int main()
{
    int fd = open("linux-4.4.36.tar", O_RDONLY);
    if(fd == -1){
        printf("open file faild\n");
        exit(-1);
    }
		
    for(int i = 1; i <= 64; i++) {
        int rd_size;
		rd_size = 512 * i;
		int src_size = rd_size;

		unsigned char *buf = (unsigned char *)malloc(rd_size*2+5);
		unsigned char *decom_buf = (unsigned char *)malloc(rd_size*2+5);
		unsigned char *dst_buf = (unsigned char *)malloc(rd_size*2+5);
		unsigned char *dst_buf_hc = (unsigned char *)malloc(rd_size*2+5);

		double seconds_avg;
		double seconds_avg_hc;
		double seconds_avg_decom;
		double seconds_avg_hc_decom;
		double temp;
        double MS_1000=1000;
        int tmp_size;
		unsigned long clen;
		unsigned long clen_decom;
		int dst_size = 0;
		int dst_size_hc = 0;
        int cnt1 = 0;
        int cnt2 = 0;
        int cnt1_decom = 0;
        int cnt2_decom = 0;
		struct timeval beginTime, endTime;
		for(int j = 0; j < REC_TIMES; j++) {
            int offset = rand_num(800000);
            lseek(fd, offset * 512, SEEK_SET);
            if(rd_size != read(fd, buf, rd_size)){
                printf("read error!\n");
                exit(-1);
            }
            
            //test lz4 comp
            clen = compressBound(rd_size);
			if (gettimeofday(&beginTime, NULL) == -1)
				return -1;
			//tmp_size = LZ4_compress_limitedOutput (buf, dst_buf, src_size, rd_size*2);
            //dst_size += tmp_size;
			compress(dst_buf, &clen, buf, rd_size);
    		if (gettimeofday(&endTime, NULL) == -1)
        		return -1;
			tmp_size = (int)clen;
            dst_size += tmp_size;
			temp = 1000*(endTime.tv_sec - beginTime.tv_sec) + (endTime.tv_usec - beginTime.tv_usec);
			temp /= 1000;
            if(temp > 0){
			    seconds_avg += temp;
                cnt1 += 1; 
            }

            //test lz4 decom
			clen_decom = rd_size;
			if (gettimeofday(&beginTime, NULL) == -1)
				return -1;
            //LZ4_decompress_safe (const char* source, char* dest, int compressedSize, int maxDecompressedSize);
			//int uncompress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen);
            //LZ4_decompress_safe (dst_buf, decom_buf, tmp_size, rd_size*2);
			uncompress(decom_buf, &clen_decom, dst_buf, clen);
    		if (gettimeofday(&endTime, NULL) == -1)
        		return -1;
            if(clen_decom != rd_size){
				printf("%d->%d decom error!\n", rd_size, clen_decom);
				exit(-1);
            }
            for(int k = 0; k < 20; k++){
                if(decom_buf[20*k]!=buf[20*k]){
                    printf("decom error!\n");
                    exit(-1);
                }
            }
			temp = 1000*(endTime.tv_sec - beginTime.tv_sec) + (endTime.tv_usec - beginTime.tv_usec);
			temp /= 1000;
            if(temp > 0){
			    seconds_avg_decom += temp;
                cnt1_decom += 1; 
            }


            //test lz4hc com
			if (gettimeofday(&beginTime, NULL) == -1)
				return -1;
			//tmp_size = LZ4_compressHC_limitedOutput (buf, dst_buf_hc, src_size, rd_size*2);
            //dst_size_hc += tmp_size;
			compress(dst_buf, &clen, buf, rd_size);
    		if (gettimeofday(&endTime, NULL) == -1)
        		return -1;
			tmp_size = (int)clen;
            dst_size_hc += tmp_size;
			temp = 1000*(endTime.tv_sec - beginTime.tv_sec) + (endTime.tv_usec - beginTime.tv_usec);
			temp /= 1000;
            if(temp > 0){
                seconds_avg_hc += temp;
                cnt2 += 1;
            }

            //test lz4hc decom
			clen_decom = rd_size;
			if (gettimeofday(&beginTime, NULL) == -1)
				return -1;
            //LZ4_decompress_safe (const char* source, char* dest, int compressedSize, int maxDecompressedSize);
            //LZ4_decompress_safe (dst_buf_hc, decom_buf, tmp_size, rd_size*2);
			uncompress(decom_buf, &clen_decom, dst_buf, clen);
    		if (gettimeofday(&endTime, NULL) == -1)
        		return -1;
            if(clen_decom != rd_size){
				printf("decom error!\n");
				exit(-1);
            }
            for(int k = 0; k < 20; k++){
                if(decom_buf[20*k]!=buf[20*k]){
                    printf("decom error!\n");
                    exit(-1);
                }
            }
			temp = 1000*(endTime.tv_sec - beginTime.tv_sec) + (endTime.tv_usec - beginTime.tv_usec);
			temp /= 1000;
            if(temp > 0){
			    seconds_avg_hc_decom += temp;
                cnt2_decom += 1; 
            }

		}

		float cr = (float)dst_size / REC_TIMES / src_size;
		float cr_hc = (float)dst_size_hc / REC_TIMES /src_size;
		seconds_avg /= cnt1;
		seconds_avg_hc /= cnt2;
		seconds_avg_decom /= cnt1_decom;
		seconds_avg_hc_decom /= cnt2_decom;
        /*
		printf("[%d\tBytes]   dst->[%0.1f %0.4f][%0.4fms %0.2fMB/s][%0.4fms %0.2fMB/s]  hc->[%0.1f %0.4f][%0.4fms %0.2fMB/s][%0.4fms %0.2fMB/s]\n", 
						src_size, (float)dst_size/REC_TIMES, cr, seconds_avg,MS_1000/(seconds_avg/i*2)/1024, seconds_avg_decom, MS_1000/(seconds_avg_decom/i*2)/1024, 	
								  (float)dst_size_hc/REC_TIMES, cr_hc, seconds_avg_hc, MS_1000/(seconds_avg_hc/i*2)/1024, seconds_avg_hc_decom,  MS_1000/(seconds_avg_hc_decom/i*2)/1024);
        */
		printf("%d\t%0.1f\t%0.4f\t%0.4f\t%0.2f\t%0.4f\t%0.2f\n", 
						src_size, (float)dst_size/REC_TIMES, cr, seconds_avg,MS_1000/(seconds_avg/i*2)/1024, seconds_avg_decom, MS_1000/(seconds_avg_decom/i*2)/1024); 	
		free(buf);
		free(decom_buf);
		free(dst_buf);
		free(dst_buf_hc);
    }
	close(fd);

}
