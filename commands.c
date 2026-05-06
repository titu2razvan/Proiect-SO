#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "commands.h"
#include "helper.h"

static int appendActionLog(const char *district, const char *role, const char *user, const char *action){
    if(logAction(district, role, user, action) != 0){
        fprintf(stderr, "Warning: could not append to logged_district.\n");
        return -1;
    }

    return 0;
}

static int parseIntArgument(const char *text, const char *name, int *value){
    char *endPtr;
    long parsedValue;

    parsedValue = strtol(text, &endPtr, 10);
    if(text == endPtr || *endPtr != '\0'){
        fprintf(stderr, "Invalid %s.\n", name);
        return 0;
    }

    *value = (int)parsedValue;
    return 1;
}

static void notify_monitor(const char *district, const char *role, const char *user){
    char buf[32];
    ssize_t n;
    pid_t pid;
    char *endPtr;
    int fd;

    fd = open(".monitor_pid", O_RDONLY);
    if(fd == -1){
        appendActionLog(district, role, user,
            "monitor notification failed: .monitor_pid not found");
        return;
    }

    n = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if(n <= 0){
        appendActionLog(district, role, user,
            "monitor notification failed: could not read PID");
        return;
    }
    buf[n] = '\0';

    pid = (pid_t)strtol(buf, &endPtr, 10);
    if(endPtr == buf || pid <= 0){
        appendActionLog(district, role, user,
            "monitor notification failed: invalid PID in .monitor_pid");
        return;
    }

    if(kill(pid, SIGUSR1) == -1){
        appendActionLog(district, role, user,
            "monitor notification failed: kill() returned error");
    } else {
        appendActionLog(district, role, user,
            "monitor notified of new report via SIGUSR1");
    }
}

static int isSupportedFilterField(const char *field){
    return strcmp(field, "severity") == 0
        || strcmp(field, "category") == 0
        || strcmp(field, "inspector") == 0
        || strcmp(field, "timestamp") == 0;
}

void add(int argc, char **argv){
    char *role = getRole(argc, argv);
    char *user = getUser(argc, argv);
    char *district = getDistrict(argc, argv);
    char reportsPath[PATH_LEN];
    char action[ACTION_LEN];
    int threshold;
    int fd;
    struct stat st;
    Report r;

    if(role == NULL || user == NULL || district == NULL){
        fprintf(stderr, "Invalid arguments (role|user|district) for add function\n");
        return;
    }

    if(districtSetup(district) != 0){
        return;
    }

    buildReportsPath(reportsPath, district);

    if(stat(reportsPath, &st) == -1){
        perror("stat reports.dat");
        return;
    }

    if(!isValidRole(role)){
        fprintf(stderr, "Invalid role.\n");
        return;
    }
    if(!hasRolePermission(role, st.st_mode, S_IWUSR, S_IWGRP)){
        fprintf(stderr, "Permission denied: %s cannot write reports.dat\n", role);
        return;
    }

    r.report_id = getNextReportId(district);
    strncpy(r.inspector, user, INSPECTOR_LEN - 1);
    r.inspector[INSPECTOR_LEN - 1] = '\0';

    printf("Latitude: ");
    if(scanf("%f", &r.latitude) != 1){
        fprintf(stderr, "Invalid latitude.\n");
        return;
    }

    printf("Longitude: ");
    if(scanf("%f", &r.longitude) != 1){
        fprintf(stderr, "Invalid longitude.\n");
        return;
    }

    printf("Category: ");
    if(scanf("%29s", r.category) != 1){
        fprintf(stderr, "Invalid category.\n");
        return;
    }

    printf("Severity (1-3): ");
    if(scanf("%d", &r.severity) != 1 || r.severity < 1 || r.severity > 3){
        fprintf(stderr, "Severity must be an integer between 1 and 3.\n");
        return;
    }

    getchar();
    printf("Description: ");
    fgets(r.description, DESC_LEN, stdin);
    r.description[strcspn(r.description, "\n")] = '\0';

    r.timestamp = time(NULL);

    fd = open(reportsPath, O_WRONLY | O_APPEND);
    if(fd == -1){
        perror("open reports.dat");
        return;
    }

    if(write(fd, &r, sizeof(Report)) != sizeof(Report)){
        perror("write report");
        close(fd);
        return;
    }

    close(fd);
    printf("Report added.\n");

    notify_monitor(district, role, user);

    if(readSeverityThreshold(district, &threshold) == 0 && r.severity >= threshold){
        printf("Escalation alert: severity %d meets threshold %d.\n", r.severity, threshold);
    }

    snprintf(action, sizeof(action), "add report_id=%d", r.report_id);
    appendActionLog(district, role, user, action);
}

void list(int argc, char **argv){
    char *role = getRole(argc, argv);
    char *user = getUser(argc, argv);
    char *district = getDistrict(argc, argv);
    char reportsPath[PATH_LEN];
    int fd;
    struct stat st;
    char permString[10];
    char *timeString;
    Report r;

    if(role == NULL || user == NULL || district == NULL){
        fprintf(stderr, "Invalid arguments for list function\n");
        return;
    }

    buildReportsPath(reportsPath, district);

    if(stat(reportsPath, &st) == -1){
        perror("stat reports.dat");
        return;
    }

    if(!isValidRole(role)){
        fprintf(stderr, "Invalid role.\n");
        return;
    }
    if(!hasRolePermission(role, st.st_mode, S_IRUSR, S_IRGRP)){
        fprintf(stderr, "Permission denied: %s cannot read reports.dat\n", role);
        return;
    }

    modeToString(st.st_mode, permString);
    timeString = ctime(&st.st_mtime);

    printf("reports.dat info:\n");
    printf("Permissions: %s\n", permString);
    printf("Size: %ld bytes\n", (long)st.st_size);
    printf("Last modified: %s", timeString);

    fd = open(reportsPath, O_RDONLY);
    if(fd == -1){
        perror("open reports.dat");
        return;
    }

    while(read(fd, &r, sizeof(Report)) == sizeof(Report)){
        printReport(&r);
    }

    close(fd);
    appendActionLog(district, role, user, "list");
}

void view(int argc, char **argv){
    char *role = getRole(argc, argv);
    char *user = getUser(argc, argv);
    char *district = getDistrict(argc, argv);
    char *idText = getExtraArg(argc, argv);
    char reportsPath[PATH_LEN];
    char action[ACTION_LEN];
    int fd;
    int wantedId;
    int found = 0;
    struct stat st;
    Report r;

    if(role == NULL || user == NULL || district == NULL || idText == NULL){
        fprintf(stderr, "Usage: --view <district> <report_id>\n");
        return;
    }

    buildReportsPath(reportsPath, district);

    if(stat(reportsPath, &st) == -1){
        perror("stat reports.dat");
        return;
    }

    if(!isValidRole(role)){
        fprintf(stderr, "Invalid role.\n");
        return;
    }
    if(!hasRolePermission(role, st.st_mode, S_IRUSR, S_IRGRP)){
        fprintf(stderr, "Permission denied: %s cannot read reports.dat\n", role);
        return;
    }

    if(!parseIntArgument(idText, "report id", &wantedId)){
        return;
    }

    fd = open(reportsPath, O_RDONLY);
    if(fd == -1){
        perror("open reports.dat");
        return;
    }

    while(read(fd, &r, sizeof(Report)) == sizeof(Report)){
        if(r.report_id == wantedId){
            printReport(&r);
            found = 1;
            break;
        }
    }

    close(fd);

    if(found == 0){
        fprintf(stderr, "Report not found.\n");
        return;
    }

    snprintf(action, sizeof(action), "view report_id=%d", wantedId);
    appendActionLog(district, role, user, action);
}

void remove_report(int argc, char **argv){
    char *role = getRole(argc, argv);
    char *user = getUser(argc, argv);
    char *district = getDistrict(argc, argv);
    char *idText = getExtraArg(argc, argv);
    char reportsPath[PATH_LEN];
    char action[ACTION_LEN];
    Report r;
    ssize_t bytesRead;
    off_t removeOffset = -1;
    off_t writeOffset;
    int wantedId;
    int fd;
    struct stat st;

    if(role == NULL || user == NULL || district == NULL || idText == NULL){
        fprintf(stderr, "Usage: --remove_report <district> <report_id>\n");
        return;
    }

    if(strcmp(role, "manager") != 0){
        fprintf(stderr, "Only the manager role may remove reports.\n");
        return;
    }

    if(!parseIntArgument(idText, "report id", &wantedId)){
        return;
    }
    buildReportsPath(reportsPath, district);

    if(stat(reportsPath, &st) == -1){
        perror("stat reports.dat");
        return;
    }

    if((st.st_mode & (S_IRUSR | S_IWUSR)) != (S_IRUSR | S_IWUSR)){
        fprintf(stderr, "Manager needs read and write permission on reports.dat.\n");
        return;
    }

    fd = open(reportsPath, O_RDWR);
    if(fd == -1){
        perror("open reports.dat");
        return;
    }

    while((bytesRead = read(fd, &r, sizeof(Report))) == sizeof(Report)){
        if(r.report_id == wantedId){
            removeOffset = lseek(fd, 0, SEEK_CUR) - (off_t)sizeof(Report);
            break;
        }
    }

    if(bytesRead == -1){
        perror("read reports.dat");
        close(fd);
        return;
    }

    if(removeOffset == -1){
        fprintf(stderr, "Report not found.\n");
        close(fd);
        return;
    }

    writeOffset = removeOffset;

    while((bytesRead = read(fd, &r, sizeof(Report))) == sizeof(Report)){
        off_t nextOffset = lseek(fd, 0, SEEK_CUR);

        if(lseek(fd, writeOffset, SEEK_SET) == -1){
            perror("lseek write position");
            close(fd);
            return;
        }

        if(write(fd, &r, sizeof(Report)) != sizeof(Report)){
            perror("write shifted report");
            close(fd);
            return;
        }

        writeOffset += sizeof(Report);

        if(lseek(fd, nextOffset, SEEK_SET) == -1){
            perror("lseek read position");
            close(fd);
            return;
        }
    }

    if(bytesRead == -1){
        perror("read reports.dat");
        close(fd);
        return;
    }

    if(ftruncate(fd, st.st_size - (off_t)sizeof(Report)) == -1){
        perror("ftruncate reports.dat");
        close(fd);
        return;
    }

    close(fd);
    printf("Report removed.\n");

    snprintf(action, sizeof(action), "remove_report report_id=%d", wantedId);
    appendActionLog(district, role, user, action);
}

void update_threshold(int argc, char **argv){
    char *role = getRole(argc, argv);
    char *user = getUser(argc, argv);
    char *district = getDistrict(argc, argv);
    char *valueText = getExtraArg(argc, argv);
    char cfgPath[PATH_LEN];
    char action[ACTION_LEN];
    long threshold;
    char *endPtr;
    struct stat st;

    if(role == NULL || user == NULL || district == NULL || valueText == NULL){
        fprintf(stderr, "Usage: --update_threshold <district> <value>\n");
        return;
    }

    if(strcmp(role, "manager") != 0){
        fprintf(stderr, "Only the manager role may update the threshold.\n");
        return;
    }

    threshold = strtol(valueText, &endPtr, 10);
    if(*endPtr != '\0' || threshold < 1 || threshold > 3){
        fprintf(stderr, "Threshold must be an integer between 1 and 3.\n");
        return;
    }

    buildCfgPath(cfgPath, district);

    if(stat(cfgPath, &st) == -1){
        perror("stat district.cfg");
        return;
    }

    if((st.st_mode & 0777) != CFG_MODE){
        fprintf(stderr, "district.cfg permissions are %o, expected %o.\n", st.st_mode & 0777, CFG_MODE);
        return;
    }

    if(writeSeverityThreshold(district, (int)threshold) != 0){
        return;
    }

    printf("Severity threshold updated to %ld.\n", threshold);

    snprintf(action, sizeof(action), "update_threshold value=%ld", threshold);
    appendActionLog(district, role, user, action);
}

void filter(int argc, char **argv){
    char *role = getRole(argc, argv);
    char *user = getUser(argc, argv);
    char *district = getDistrict(argc, argv);
    char reportsPath[PATH_LEN];
    char field[FIELD_LEN], op[OP_LEN], value[VALUE_LEN];
    int commandIndex = getCommandIndex(argc, argv);
    int conditionStart = commandIndex + 2;
    int found = 0;
    int fd;
    struct stat st;
    Report r;

    if(role == NULL || user == NULL || district == NULL){
        fprintf(stderr, "Usage: --filter <district> <condition> [condition ...]\n");
        return;
    }

    if(commandIndex == -1 || conditionStart >= argc){
        fprintf(stderr, "Filter requires at least one condition.\n");
        return;
    }

    for(int i = conditionStart; i < argc; i++){
        if(!parse_condition(argv[i], field, op, value) || !isSupportedFilterField(field)){
            fprintf(stderr, "Invalid condition: %s\n", argv[i]);
            return;
        }
    }

    buildReportsPath(reportsPath, district);

    if(stat(reportsPath, &st) == -1){
        perror("stat reports.dat");
        return;
    }

    if(!isValidRole(role)){
        fprintf(stderr, "Invalid role.\n");
        return;
    }
    if(!hasRolePermission(role, st.st_mode, S_IRUSR, S_IRGRP)){
        fprintf(stderr, "Permission denied: %s cannot read reports.dat\n", role);
        return;
    }

    fd = open(reportsPath, O_RDONLY);
    if(fd == -1){
        perror("open reports.dat");
        return;
    }

    while(read(fd, &r, sizeof(Report)) == sizeof(Report)){
        int matchesAll = 1;
        for(int i = conditionStart; i < argc; i++){
            parse_condition(argv[i], field, op, value);
            if(!match_condition(&r, field, op, value)){
                matchesAll = 0;
                break;
            }
        }
        if(matchesAll){ printReport(&r); found = 1; }
    }

    close(fd);

    if(!found) printf("No reports matched the filter.\n");
    appendActionLog(district, role, user, "filter");
}

void remove_district(int argc, char **argv){
    char *role = getRole(argc, argv);
    char *user = getUser(argc, argv);
    char *district = getDistrict(argc, argv);
    char districtPath[PATH_LEN];
    char symlinkPath[PATH_LEN];
    struct stat st;
    pid_t pid;
    int status;

    if(role == NULL || user == NULL || district == NULL){
        fprintf(stderr, "Usage: --remove_district <district>\n");
        return;
    }

    if(strcmp(role, "manager") != 0){
        fprintf(stderr, "Only the manager role may remove districts.\n");
        return;
    }

    buildDistrictPath(districtPath, district);
    buildSymlinkPath(symlinkPath, district);

    if(stat(districtPath, &st) == -1){
        fprintf(stderr, "District '%s' does not exist.\n", district);
        return;
    }

    pid = fork();
    if(pid == -1){ perror("fork"); return; }

    if(pid == 0){
        execlp("rm", "rm", "-rf", districtPath, NULL);
        perror("execlp");
        exit(1);
    }

    if(waitpid(pid, &status, 0) == -1){ 
        perror("waitpid"); return; 
    }

    if(!WIFEXITED(status) || WEXITSTATUS(status) != 0){
        fprintf(stderr, "Failed to remove district directory.\n");
        return;
    }

    if(lstat(symlinkPath, &st) == 0) unlink(symlinkPath);

    printf("District '%s' removed.\n", district);
    appendActionLog(district, role, user, "remove_district");
}
