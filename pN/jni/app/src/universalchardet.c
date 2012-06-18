#include "universalchardet.h"

#include <stdio.h>
#include <android/log.h>

char broov_subtitle_encoding[CHARDET_MAX_ENCODING_NAME];
int  broov_encoding_valid;

static void doit(FILE* fp)
{
    int ret=0;
    char buf[4096];
    size_t len;
    int res = 0;
    chardet_t det = NULL;

    chardet_create(&det);

    do {
	len = fread(buf, 1, sizeof(buf), fp);
	res = chardet_handle_data(det, buf, len);
    } while (res==CHARDET_RESULT_OK && feof(fp)==0);

    chardet_data_end(det);

    ret = chardet_get_charset(det, broov_subtitle_encoding, CHARDET_MAX_ENCODING_NAME);
    if (ret== CHARDET_RESULT_OK) {
        broov_encoding_valid=1;
    } 

    //printf("Charset = %s\n", encoding);
    __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Charset = %s\n", broov_subtitle_encoding);

    chardet_destroy(det);
}

int universalchardet_main(int argc, char* argv[])
{
    FILE* fp = NULL;

    if (argc > 1) {
	fp = fopen(argv[1], "rb");
	if (fp) {
	    doit(fp);
	    fclose(fp);
	} else {
	    //printf("Can't open %s\n", argv[1]);
            __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Can't open %s\n", argv[1]);
	    return 1;
	}
    } else {
	//printf("USAGE: chardet filename\n");
        __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "USAGE: chardet filename\n");
	return 1;
    }

    return 0;
}

