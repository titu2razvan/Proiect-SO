#ifndef HELPER_H
#define HELPER_H

#include <time.h>
#include <sys/types.h>

#define INSPECTOR_LEN 50
#define CATEGORY_LEN 30
#define DESC_LEN 128
#define PATH_LEN 256
#define FIELD_LEN 32
#define OP_LEN 3
#define VALUE_LEN 128
#define ACTION_LEN 256

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
int getCommandIndex(int argc, char **argv);
char *getDistrict(int argc, char **argv);
char *getExtraArg(int argc, char **argv);

//path helpers
void buildDistrictPath(char *out, const char *district);
void buildReportsPath(char *out, const char *district);
void buildCfgPath(char *out, const char *district);
void buildLogPath(char *out, const char *district);
void buildSymlinkPath(char *out, const char *district);

int getNextReportId(const char *district);
int districtSetup(const char *district);

void modeToString(mode_t mode, char *out);
void printReport(const Report *r);
int isValidRole(const char *role);
int hasRolePermission(const char *role, mode_t mode, mode_t managerMask, mode_t inspectorMask);
int logAction(const char *district, const char *role, const char *user, const char *action);
int readSeverityThreshold(const char *district, int *threshold);
int writeSeverityThreshold(const char *district, int threshold);
int parse_condition(const char *input, char *field, char *op, char *value);
int match_condition(const Report *r, const char *field, const char *op, const char *value);

#endif
