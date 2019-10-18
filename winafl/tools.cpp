#include "tools.h"

char *DICTPATH = "c:\\fuzz_config\\fuzz.dict";


int str_in_dictfile(const char *filepath, const char *str){
	FILE *fp;
	int ret = 0;

	fp = fopen(filepath, "r");
	if (!fp) return 0;

	fseek(fp, 0, SEEK_END);
    int filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *filecontent = (char *)malloc(filesize+1);
    fread(filecontent, filesize, 1, fp);
    
	fclose(fp);

	if (strstr(filecontent, str) != 0){
		ret = 1;
	}
	return ret;
}


int is_dictitem_valid(const char *str){
	int MIN_VALID_STR_LENGTH = 3;
	int length = strlen(str);

	if (length < MIN_VALID_STR_LENGTH) return 0;

	int i = 0;
	while (i < length){
		/*
		if (!(str[i] >= 'a' && str[i] <= 'z' ||
			  str[i] >= 'A' && str[i] <= 'Z' ||
			  str[i] >= '0' && str[i] <= '9' ||
			  str[i] == '_' || str[i] == '-'  )) {
				  return 0;
		}
		*/
		if (!(str[i] >= 0x21 && str[i] <=0x7e)){
			return 0;
		}
		i += 1;
	}

	return 1;
}

int add_one_dictitem(const char *dict, const char *filepath){

	int MAX_DICTITEMS = 100;

	FILE *fp;

	int i = 1;

	if (str_in_dictfile(DICTPATH, dict)) return 0;

	fp = fopen(filepath, "a");
	if (!fp) return 0;
	
	char line[128] = {0};
	char *prefix = "\"";
	strcpy(line, prefix);
	strcpy(line + strlen(line), dict);
	strcpy(line + strlen(line), "\"\n");


	fwrite(line, 1, strlen(line), fp);
	fclose(fp);
	return 1;
}
