#ifndef SUBREADER_H
#define SUBREADER_H

//extern int suboverlap_enabled;
//extern int sub_no_text_pp;  // disable text post-processing
//extern int sub_match_fuzziness;

// subtitle formats
#define SUB_INVALID   -1
#define SUB_MICRODVD  0
#define SUB_SUBRIP    1
#define SUB_SUBVIEWER 2
#define SUB_SAMI      3
#define SUB_VPLAYER   4
#define SUB_RT        5
#define SUB_SSA       6
#define SUB_PJS       7
#define SUB_MPSUB     8
#define SUB_AQTITLE   9
#define SUB_SUBVIEWER2 10
#define SUB_SUBRIP09 11
#define SUB_JACOSUB  12
#define SUB_MPL2     13

// One of the SUB_* constant above
//extern int sub_format;

#define MAX_SUBTITLE_FILES 128

#define SUB_MAX_TEXT 10
#define SUB_ALIGNMENT_HLEFT 1
#define SUB_ALIGNMENT_HCENTER   0
#define SUB_ALIGNMENT_HRIGHT    2

typedef struct 
{

    int lines;

    unsigned long start;
    unsigned long end;

    char *text[SUB_MAX_TEXT];
    unsigned char alignment;
} subtitle;

typedef struct 
{
    subtitle *subtitles;
    char *filename;
    int sub_uses_time;
    int sub_num;          // number of subtitle structs
    int sub_errs;
} sub_data;

sub_data* sub_read_file (char *filename, float pts);
//subtitle* subcp_recode1 (subtitle *sub);
//void subcp_open (char *current_sub_cp); /* for demux_ogg.c */
//void subcp_open_noenca (); /* for demux_ogg.c */
//void subcp_close (void); /* for demux_ogg.c */
char ** sub_filenames(char *path, char *fname);
void list_sub_file(sub_data* subd);
//void dump_srt(sub_data* subd, float fps);
//void dump_mpsub(sub_data* subd, float fps);
//void dump_microdvd(sub_data* subd, float fps);
//void dump_jacosub(sub_data* subd, float fps);
//void dump_sami(sub_data* subd, float fps);
void sub_free( sub_data * subd );
void find_sub(sub_data* subd,int key);
void step_sub(sub_data *subd, float pts, int movement);
int find_sub1(sub_data* subs, float pts, int *millisec, float fps);

int  subInit(char* pcszFileName, float fps);
int  subFree();
int  subInTime(ULONG ulTime);
void subFindNext(ULONG ulTime);
void subDisplay(SDL_Surface *screen, int type);
void subClearDisplay(SDL_Surface *screen);

#define SUBTITLE_ENCODING_AUTO_DETECT 0
#define SUBTITLE_ENCODING_ISO_2022_JP 1
#define SUBTITLE_ENCODING_ISO_2022_CN 2
#define SUBTITLE_ENCODING_ISO_2022_KR 3
#define SUBTITLE_ENCODING_ISO_8859_5  4
#define SUBTITLE_ENCODING_ISO_8859_7  5
#define SUBTITLE_ENCODING_ISO_8859_8  6
#define SUBTITLE_ENCODING_BIG5        7
#define SUBTITLE_ENCODING_GB18030     8
#define SUBTITLE_ENCODING_EUC_JP      9
#define SUBTITLE_ENCODING_EUC_KR      10
#define SUBTITLE_ENCODING_EUC_TW      11
#define SUBTITLE_ENCODING_SHIFT_JIS   12
#define SUBTITLE_ENCODING_IBM855      13
#define SUBTITLE_ENCODING_IBM866      14
#define SUBTITLE_ENCODING_KOI8_R      15
#define SUBTITLE_ENCODING_MACCYRILLIC 16
#define SUBTITLE_ENCODING_WINDOWS_1251 17
#define SUBTITLE_ENCODING_WINDOWS_1252 18
#define SUBTITLE_ENCODING_WINDOWS_1253 19
#define SUBTITLE_ENCODING_WINDOWS_1255 20
#define SUBTITLE_ENCODING_UTF_8        21
#define SUBTITLE_ENCODING_UTF_16BE     22
#define SUBTITLE_ENCODING_UTF_16LE     23
#define SUBTITLE_ENCODING_UTF_32BE     24
#define SUBTITLE_ENCODING_UTF_32LE     25
#define SUBTITLE_ENCODING_HZ_GB_2312   26

#define MAX_SUBTITLE_ENCODING_TYPES    26

#endif
