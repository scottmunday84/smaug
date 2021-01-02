/* Copyright by Gregor Stipicic, 2001 */

typedef struct mdice_data MDICE_DATA;

struct mdice_data
{
 int level;
 char *hitd;
 int ac;
 char *damd;
};

extern const MDICE_DATA mdice_table[MAX_MOB_LEVEL];

