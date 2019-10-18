/*
   WinAFL - A simple test binary that crashes on certain inputs:
     - 'test1' with a normal write access violation at NULL
     - 'test2' with a /GS stack cookie violation
   -------------------------------------------------------------

   Written and maintained by Ivan Fratric <ifratric@google.com>

   Copyright 2016 Google Inc. All Rights Reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <string.h>


#include "qstring.h"
#include "qstringlist.h"
#include "time.h"
#include "cJSON.h"

#include "ini.c"

int get_random_button(int buttons){
	srand (time(NULL));

	int count=0;
	int bitcount = 0;
	int x = buttons;
	int idx;

	while (x){
		if (x & 1)
			count += 1;
		x = x >> 2;
	}

	idx = rand() % count;

	x = buttons;
	count = 0;

	while (x){
		if (x & 1){
			if (count == idx)
				return 1 << (2*bitcount);
			count += 1;
		}
		x = x >> 2;
		bitcount += 1;
	}
	return 0;
}

static inline uint julianDayFromGregorianDate(int year, int month, int day)
{
    // Gregorian calendar starting from October 15, 1582
    // Algorithm from Henry F. Fliegel and Thomas C. Van Flandern
    return (1461 * (year + 4800 + (month - 14) / 12)) / 4
           + (367 * (month - 2 - 12 * ((month - 14) / 12))) / 12
           - (3 * ((year + 4900 + (month - 14) / 12) / 100)) / 4
           + day - 32075;
}

static uint julianDayFromDate(int year, int month, int day)
{
    if (year < 0)
        ++year;

    if (year > 1582 || (year == 1582 && (month > 10 || (month == 10 && day >= 15)))) {
        return julianDayFromGregorianDate(year, month, day);
    } else if (year < 1582 || (year == 1582 && (month < 10 || (month == 10 && day <= 4)))) {
        // Julian calendar until October 4, 1582
        // Algorithm from Frequently Asked Questions about Calendars by Claus Toendering
        int a = (14 - month) / 12;
        return (153 * (month + (12 * a) - 3) + 2) / 5
               + (1461 * (year + 4800 - a)) / 4
               + day - 32083;
    } else {
        // the day following October 4, 1582 is October 15, 1582
        return 0;
    }
}

#include <time.h>

typedef struct times
{
	int Year;
	int Mon;
	int Day;
	int Hour;
	int Min;
	int Second;
}Times;

Times stamp_to_standard(int stampTime){
	time_t tick = (time_t)stampTime;
	struct tm tm;
	char s[100];
	Times t;

	tm = *localtime(&tick);
	strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tm);
	
	t.Year = atoi(s);
	t.Mon = atoi(s+5);
	t.Day = atoi(s+8);
	t.Hour = atoi(s+11);
	t.Min = atoi(s+14);
	t.Second = atoi(s+17);
	return t;

}





static char *read_file(const char *filename)
{
    FILE *file = NULL;
    long length = 0;
    char *content = NULL;
    size_t read_chars = 0;

    /* open in read binary mode */
    file = fopen(filename, "rb");
    if (file == NULL)
    {
        goto cleanup;
    }

    /* get the length */
    if (fseek(file, 0, SEEK_END) != 0)
    {
        goto cleanup;
    }
    length = ftell(file);
    if (length < 0)
    {
        goto cleanup;
    }
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        goto cleanup;
    }

    /* allocate content buffer */
    content = (char*)malloc((size_t)length + sizeof(""));
    if (content == NULL)
    {
        goto cleanup;
    }

    /* read the file into memory */
    read_chars = fread(content, sizeof(char), (size_t)length, file);
    if ((long)read_chars != length)
    {
        free(content);
        content = NULL;
        goto cleanup;
    }
    content[read_chars] = '\0';


cleanup:
    if (file != NULL)
    {
        fclose(file);
    }

    return content;
}


void mytest(){
	QString str = "c:\\out\\afl.input";
	QStringList strs;
	strs.append(str);
	printf("strs size:%d\n", strs.size());


	srand(1000);
	printf("rand: %d\n", rand());



	int x = 0x10101000;

	//x = 0xb0303030;

	int y = get_random_button(x);
	
	printf("random is:%x\n", y);


	int st = 1569309315; //2019-09-24 15:15:15

	st =  0x70303030;
	Times t = stamp_to_standard(st);

	uint number_day = julianDayFromDate(t.Year, t.Mon, t.Day);
	uint number_mseconds = (t.Hour * 3600 + t.Min * 60 + t.Second)*1000;

	printf("time %d-%d-%d %d:%d:%d\n" , t.Year, t.Mon, t.Day, t.Hour, t.Min, t.Second);

	printf("%08x %08x\n", number_day, number_mseconds);



	int myx = 0;


	__asm{
		lea eax, myx
		mov [eax], 0x22

	}

	printf("x %d\n", myx);




	exit(0);
}



int __declspec(noinline) test_target(char* input_file_path, char* argv_0)
{
	char *crash = NULL;
	FILE *fp = fopen(input_file_path, "rb");
	char c;


	mytest();


	if (!fp) {
		printf("Error opening file\n");
		return 0;
	}
	if (fread(&c, 1, 1, fp) != 1) {
		printf("Error reading file\n");
		fclose(fp);
		return 0;
	}
	if (c != 't') {
		printf("Error 1\n");
		fclose(fp);
		return 0;
	}
	if (fread(&c, 1, 1, fp) != 1) {
		printf("Error reading file\n");
		fclose(fp);
		return 0;
	}
	if (c != 'e') {
		printf("Error 2\n");
		fclose(fp);
		return 0;
	}
	if (fread(&c, 1, 1, fp) != 1) {
		printf("Error reading file\n");
		fclose(fp);
		return 0;
	}
	if (c != 's') {
		printf("Error 3\n");
		fclose(fp);
		return 0;
	}
	if (fread(&c, 1, 1, fp) != 1) {
		printf("Error reading file\n");
		fclose(fp);
		return 0;
	}
	if (c != 't') {
		printf("Error 4\n");
		fclose(fp);
		return 0;
	}
	printf("!!!!!!!!!!OK!!!!!!!!!!\n");

	if (fread(&c, 1, 1, fp) != 1) {
		printf("Error reading file\n");
		fclose(fp);
		return 0;
	}
	if (c == '1') {
		// cause a crash
		crash[0] = 1;
	}
	else if (c == '2') {
		char buffer[5] = { 0 };
		// stack-based overflow to trigger the GS cookie corruption
		for (int i = 0; i < 5; ++i)
			strcat(buffer, argv_0);
		printf("buffer: %s\n", buffer);
	}
	else {
		printf("Error 5\n");
	}
	fclose(fp);
	return 0;
	//return test_target(input_file_path, argv_0);
}



int test_strcmp(const char *filepath){
	
	FILE *fp;

	char *password =  "1111111111111111111111111111111111";
	char *password2 = "2222222222222222222222222222222222";
	char *password3 = "3333333333333333333333333333333333";

	if (!filepath) return 0;
	fp = fopen(filepath, "r");
	if (!fp) return 0;

	fseek(fp, 0, SEEK_END);
    int filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buffer = (char *)malloc(filesize+1);
    fread(buffer, filesize, 1, fp);

	buffer[filesize] = '\0';
    
	fclose(fp);


	if (strlen(buffer) == 0) return 0;
	

	if (strcmp(buffer, password) == 0){
		puts("password strcmp");
		abort();
	}else if(strncmp(buffer, password2, strlen(password2)) == 0 ){
		puts("passwrod2 strncmp");
		abort();
	}else if( memcmp(buffer, password3, strlen(password3)) == 0 ) {
		puts("password3 memcmp");
		abort();
	}
	
	if (strstr(password3, buffer) != 0){
		if (strlen(buffer) > 10){

			puts("password3 strstr");
			abort();
		}
	}

	return 0;

}
int test_ini(){
	ini_t *peini = ini_load("c:\\fuzz_config\\pe.ini");
	int start = ini_get_int(peini, "", ".text_start1");
	printf("%x\n", start);

	return 0;
}

int main(int argc, char** argv)
{
	

	test_strcmp(argv[1]);

	//test_ini();

	return 0;


    if(argc < 2) {
        printf("Usage: %s <input file>\n", argv[0]);
        return 0;
    }

	if (argc == 3 && !strcmp(argv[2], "loop"))
	{
		//loop inside application and call target infinitey
		while (true)
		{
			test_target(argv[1], argv[0]);
		}
	}
	else
	{
		//regular single target call
		return test_target(argv[1], argv[0]);
	}
}
