#ifndef HELPER_H
#define HELPER_H

#include <time.h>
#include <sys/types.h>

#define INSPECTOR_LEN 50
#define CATEGORY_LEN 30
#define DESC_LEN 128
#define PATH_LEN 256

//permission bits
#define DIR_MODE 0750
#define REPORTS_MODE 0664
#define CFG_MODE 0640
#define LOG_MODE 0644

typedef struct {
    int report_id;
    char inspector[INSPECTOR_LEN];
    float latitude;
    float longitude;
    char category[CATEGORY_LEN];
    int severity;
    time_t timestamp;
    char description[DESC_LEN];
} Report;

//argument helpers 
char *getRole(int argc, char **argv);
char *getUser(int argc, char **argv);
char *getCommand(int argc, char **argv);
int getCommand_index(int argc, char **argv);
char *getDistrict(int argc, char **argv);
char *getExtraArg(int argc, char **argv);

//path helpers
void buildDistrictPath(char *out, char *district);
void buildReportsPath(char *out, char *district);
void buildCfgPath(char *out, char *district);
void buildLogPath(char *out,  char *district);
void buildSymlinkPath(char *out, char *district);

int getNextReportId(char *district);
int districtSetup(char *district);

void modeToString(mode_t mode, char *out);
void printReport(const Report *r);

#endif