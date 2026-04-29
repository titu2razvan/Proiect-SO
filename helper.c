#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

static int compareNumbers(long long left, const char *op, long long right){
    if(strcmp(op, "==") == 0) return left == right;
    if(strcmp(op, "!=") == 0) return left != right;
    if(strcmp(op, "<") == 0) return left < right;
    if(strcmp(op, "<=") == 0) return left <= right;
    if(strcmp(op, ">") == 0) return left > right;
    if(strcmp(op, ">=") == 0) return left >= right;
    return 0;
}

static int compareStrings(const char *left, const char *op, const char *right){
    int cmp = strcmp(left, right);

    if(strcmp(op, "==") == 0) return cmp == 0;
    if(strcmp(op, "!=") == 0) return cmp != 0;
    if(strcmp(op, "<") == 0) return cmp < 0;
    if(strcmp(op, "<=") == 0) return cmp <= 0;
    if(strcmp(op, ">") == 0) return cmp > 0;
    if(strcmp(op, ">=") == 0) return cmp >= 0;
    return 0;
}

static char *getArgValue(int argc, char **argv, const char *flag){
    for(int i = 1; i < argc - 1; i++)
        if(strcmp(argv[i], flag) == 0)
            return argv[i + 1];
    return NULL;
}

char *getRole(int argc, char **argv){ return getArgValue(argc, argv, "--role"); }
char *getUser(int argc, char **argv){ return getArgValue(argc, argv, "--user"); }

int getCommandIndex(int argc, char **argv){
    static const char *cmds[] = {
        "--add", "--list", "--view", "--remove_report",
        "--update_threshold", "--filter", "--remove_district", NULL
    };
    for(int i = 1; i < argc; i++)
        for(int j = 0; cmds[j] != NULL; j++)
            if(strcmp(argv[i], cmds[j]) == 0)
                return i;
    return -1;
}

char *getCommand(int argc, char **argv){
    int index=getCommandIndex(argc,argv);
    if(index==-1)
        return NULL;
    return argv[index];
}

char *getDistrict(int argc, char **argv){
    int index=getCommandIndex(argc,argv);
    if(index==-1 || index+1>=argc)
        return NULL;
    else return argv[index+1];
}

char *getExtraArg(int argc, char **argv){
    int index=getCommandIndex(argc,argv);
    if(index==-1 || index+2>=argc)
        return NULL;
    else return argv[index+2];
}

void buildDistrictPath(char *out, const char *district){
    snprintf(out, PATH_LEN, "./%s",district);
}

void buildReportsPath(char *out, const char *district){
    snprintf(out, PATH_LEN, "./%s/reports.dat",district);
}

void buildCfgPath(char *out, const char *district){
    snprintf(out, PATH_LEN, "./%s/district.cfg",district);
}

void buildLogPath(char *out, const char *district){
    snprintf(out, PATH_LEN, "./%s/logged_district",district);
}

void buildSymlinkPath(char *out, const char *district){
    snprintf(out, PATH_LEN, "./active_reports-%s",district);
}

int getNextReportId(const char *district){
    char reportsPath[PATH_LEN];
    buildReportsPath(reportsPath, district);
    Report r;
    int maxId = 0;
    int fd = open(reportsPath, O_RDONLY);
    if(fd == -1)
        return 1;
    while(read(fd, &r, sizeof(Report)) == sizeof(Report))
        if(r.report_id > maxId)
            maxId = r.report_id;
    close(fd);
    return maxId + 1;
}

static int createFile(const char *path, mode_t mode){
    int fd = open(path, O_RDWR | O_CREAT, mode);
    if(fd == -1){ perror(path); return -1; }
    close(fd);
    if(chmod(path, mode) == -1){ perror(path); return -1; }
    return 0;
}

int districtSetup(const char *district){
    char districtPath[PATH_LEN];
    char reportsPath[PATH_LEN];
    char cfgPath[PATH_LEN];
    char logPath[PATH_LEN];
    char symlinkPath[PATH_LEN];
    const char *defaultCfg = "severity_threshold=2\n";
    struct stat st;
    int fd;

    buildDistrictPath(districtPath, district);
    buildReportsPath(reportsPath, district);
    buildCfgPath(cfgPath, district);
    buildLogPath(logPath, district);
    buildSymlinkPath(symlinkPath, district);

    if(stat(districtPath, &st) == -1 && mkdir(districtPath, DIR_MODE) == -1){
        perror("mkdir"); return -1;
    }
    if(chmod(districtPath, DIR_MODE) == -1){ 
        perror("chmod district"); return -1; 
    }

    if(createFile(reportsPath, REPORTS_MODE) == -1)     
        return -1;

    fd = open(cfgPath, O_RDWR | O_CREAT, CFG_MODE);
    if(fd == -1){ 
        perror("open district.cfg"); return -1; 
    }
    if(fstat(fd, &st) == -1){ 
        perror("fstat district.cfg"); 
        close(fd); 
        return -1; 
    }
    if(st.st_size == 0 && write(fd, defaultCfg, strlen(defaultCfg)) == -1){
        perror("write district.cfg"); close(fd); return -1;
    }
    close(fd);
    if(chmod(cfgPath, CFG_MODE) == -1){ perror("chmod district.cfg"); return -1; }

    if(createFile(logPath, LOG_MODE) == -1) return -1;

    if(lstat(symlinkPath, &st) == 0 && unlink(symlinkPath) == -1){
        perror("unlink symlink"); return -1;
    }
    if(symlink(reportsPath, symlinkPath) == -1){ perror("symlink"); return -1; }

    return 0;
}

void modeToString(mode_t mode, char *out){
    out[0]= (mode & S_IRUSR) ? 'r' :'-';
    out[1]= (mode & S_IWUSR) ? 'w' :'-';
    out[2]= (mode & S_IXUSR) ? 'x' :'-';
    
    out[3]= (mode & S_IRGRP) ? 'r' :'-';
    out[4]= (mode & S_IWGRP) ? 'w' :'-';
    out[5]= (mode & S_IXGRP) ? 'x' :'-';
    
    out[6]= (mode & S_IROTH) ? 'r' :'-';
    out[7]= (mode & S_IWOTH) ? 'w' :'-';
    out[8]= (mode & S_IXOTH) ? 'x' :'-';
    out[9]='\0';
}

void printReport(const Report *r){
    char timeBuffer[64];
    struct tm *timeInfo = localtime(&r->timestamp);

    if(timeInfo != NULL){
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", timeInfo);
    }
    else{
        snprintf(timeBuffer, sizeof(timeBuffer), "%lld", (long long)r->timestamp);
    }

    printf("Report ID: %d\n",r->report_id);
    printf("Inspector: %s\n",r->inspector);
    printf("Latitude: %.2f\n",r->latitude);
    printf("Longitude: %.2f\n",r->longitude);
    printf("Category: %s\n",r->category);
    printf("Severity: %d\n",r->severity);
    printf("Timestamp: %s\n", timeBuffer);
    printf("Description: %s\n",r->description);
    printf("-------------------------------------\n");
}

int isValidRole(const char *role){
    if(role == NULL) return 0;
    return strcmp(role, "manager") == 0 || strcmp(role, "inspector") == 0;
}

int hasRolePermission(const char *role, mode_t mode, mode_t managerMask, mode_t inspectorMask){
    if(strcmp(role, "manager") == 0){
        return (mode & managerMask) == managerMask;
    }

    if(strcmp(role, "inspector") == 0){
        return (mode & inspectorMask) == inspectorMask;
    }

    return 0;
}

int logAction(const char *district, const char *role, const char *user, const char *action){
    char logPath[PATH_LEN];
    char line[512];
    char timeBuffer[64];
    time_t now = time(NULL);
    struct tm *timeInfo = localtime(&now);
    int fd;
    size_t lineLen;

    buildLogPath(logPath, district);

    if(timeInfo != NULL){
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", timeInfo);
    }
    else{
        snprintf(timeBuffer, sizeof(timeBuffer), "%lld", (long long)now);
    }

    snprintf(
        line,
        sizeof(line),
        "%s | role=%s | user=%s | action=%s\n",
        timeBuffer,
        role,
        user,
        action
    );

    fd = open(logPath, O_WRONLY | O_APPEND);
    if(fd == -1){
        perror("open logged_district");
        return -1;
    }

    lineLen = strlen(line);
    if(write(fd, line, lineLen) != (ssize_t)lineLen){
        perror("write logged_district");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int readSeverityThreshold(const char *district, int *threshold){
    char cfgPath[PATH_LEN];
    char buffer[64];
    ssize_t bytesRead;
    int fd;

    buildCfgPath(cfgPath, district);

    fd = open(cfgPath, O_RDONLY);
    if(fd == -1){
        perror("open district.cfg");
        return -1;
    }

    bytesRead = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if(bytesRead <= 0){
        fprintf(stderr, "Could not read severity threshold.\n");
        return -1;
    }

    buffer[bytesRead] = '\0';

    if(sscanf(buffer, "severity_threshold=%d", threshold) != 1){
        fprintf(stderr, "Invalid district.cfg format.\n");
        return -1;
    }

    return 0;
}

int writeSeverityThreshold(const char *district, int threshold){
    char cfgPath[PATH_LEN];
    char buffer[64];
    int fd;
    int len;

    buildCfgPath(cfgPath, district);

    fd = open(cfgPath, O_WRONLY | O_TRUNC);
    if(fd == -1){
        perror("open district.cfg");
        return -1;
    }

    len = snprintf(buffer, sizeof(buffer), "severity_threshold=%d\n", threshold);
    if(write(fd, buffer, len) != len){
        perror("write district.cfg");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int parse_condition(const char *input, char *field, char *op, char *value){
    const char *firstColon;
    const char *rest;
    size_t fieldLen;
    size_t valueLen;
    size_t opLen;
    size_t i;

    static const struct { const char *token; const char *sym; } ops[] = {
        /* text operators — shell-safe, no quoting needed */
        {"lte", "<="},
        {"gte", ">="},
        {"neq", "!="},
        {"eq",  "=="},
        {"ne",  "!="},
        {"lt",  "<"},
        {"gt",  ">"},
        /* symbolic operators — require quoting in the shell when using > or < */
        {"<=",  "<="},
        {">=",  ">="},
        {"==",  "=="},
        {"!=",  "!="},
        {"<",   "<"},
        {">",   ">"},
    };

    if(input == NULL || field == NULL || op == NULL || value == NULL)
        return 0;

    firstColon = strchr(input, ':');
    if(firstColon == NULL || firstColon == input)
        return 0;

    fieldLen = (size_t)(firstColon - input);
    if(fieldLen >= FIELD_LEN)
        return 0;

    memcpy(field, input, fieldLen);
    field[fieldLen] = '\0';

    rest = firstColon + 1;
    for(i = 0; i < sizeof(ops) / sizeof(ops[0]); i++){
        opLen = strlen(ops[i].token);
        if(strncmp(rest, ops[i].token, opLen) == 0 && rest[opLen] == ':'){
            strcpy(op, ops[i].sym);
            valueLen = strlen(rest + opLen + 1);
            if(valueLen == 0 || valueLen >= VALUE_LEN)
                return 0;
            strcpy(value, rest + opLen + 1);
            return 1;
        }
    }

    return 0;
}

int match_condition(const Report *r, const char *field, const char *op, const char *value){
    char *endPtr;
    long long numericValue;

    if(r == NULL || field == NULL || op == NULL || value == NULL){
        return 0;
    }

    if(strcmp(field, "severity") == 0){
        numericValue = strtoll(value, &endPtr, 10);
        if(*endPtr != '\0'){
            return 0;
        }
        return compareNumbers(r->severity, op, numericValue);
    }

    if(strcmp(field, "timestamp") == 0){
        numericValue = strtoll(value, &endPtr, 10);
        if(*endPtr != '\0'){
            return 0;
        }
        return compareNumbers((long long)r->timestamp, op, numericValue);
    }

    if(strcmp(field, "category") == 0){
        return compareStrings(r->category, op, value);
    }

    if(strcmp(field, "inspector") == 0){
        return compareStrings(r->inspector, op, value);
    }

    return 0;
}





