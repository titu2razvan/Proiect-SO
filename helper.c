#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
char *getRole(int argc, char **argv){
    for(int i=1; i<argc-1; i++){
        if (strcmp(argv[i], "--role")==0) {
            return argv[i + 1];
        }
    }
    return NULL;
}

char *getUser(int argc, char **argv){
    for(int i=1; i<argc-1; i++){
        if (strcmp(argv[i], "--user")==0) {
            return argv[i + 1];
        }
    }
    return NULL;
}

int getCommandIndex(int argc, char **argv){
    for(int i=1; i<argc; i++){
        if (strcmp(argv[i], "--add")==0) return i;
        if (strcmp(argv[i], "--list")==0) return i;
        if (strcmp(argv[i], "--view")==0) return i;
        if (strcmp(argv[i], "--remove_report")==0) return i;
        if (strcmp(argv[i], "--update_threshold") == 0) return i;
        if (strcmp(argv[i], "--filter")==0) return i;
    }
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

void buildDistrictPath(char *out, char *district){
    snprintf(out, PATH_LEN, "./%s",district);
}

void buildReportsPath(char *out, char *district){
    snprintf(out, PATH_LEN, "./%s/reports.dat",district);
}

void buildCfgPath(char *out, char *district){
    snprintf(out, PATH_LEN, "./%s/district.cfg",district);
}

void buildLogPath(char *out, char *district){
    snprintf(out, PATH_LEN, "./%s/logged_district",district);
}

void buildSymlinkPath(char *out, char *district){
    snprintf(out, PATH_LEN, "./active_reports-%s",district);
}

int getNextReportId(char *district){
    char reportsPath[PATH_LEN];
    buildReportsPath(reportsPath,district);
    Report r;
    int maxId=0;
    int fd=open(reportsPath,O_RDONLY);
    if(fd==-1){
        //nu am putut deschide si ii dam indexul 1
        return 1;
    }
    //citim din fisier sizeof(Report) ca sa gasim care ii indexu care sa il dam la Reportul nou
    while(read(fd,&r,sizeof(Report))== sizeof(Report)){
        if(r.report_id > maxId)
            maxId=r.report_id;

        }
    close(fd);
    return maxId+1;
}

int districtSetup(char *district){
    char districtPath[PATH_LEN];
    char reportsPath[PATH_LEN];
    char cfgPath[PATH_LEN];
    char logPath[PATH_LEN];
    char symlinkPath[PATH_LEN];
    buildDistrictPath(districtPath, district);
    buildReportsPath(reportsPath, district);
    buildCfgPath(cfgPath, district);
    buildLogPath(logPath, district);
    buildSymlinkPath(symlinkPath, district);
    struct stat st;
    char *defaultCfg ="severity_threshold=2\n";
    
    if(stat(districtPath, &st) == -1){
        if(mkdir(districtPath, DIR_MODE) == -1){
            perror("mkdir error");
            return -1;
        }
    }
    if(chmod(districtPath, DIR_MODE) == -1){
        perror("chmod district");
        return -1;
    }

    int fd = open(reportsPath, O_RDWR | O_CREAT, REPORTS_MODE);
    if(fd == -1){
        perror("open reports.dat");
        return -1;
    }
    close(fd);
    if(chmod(reportsPath, REPORTS_MODE) == -1){
        perror("chmod reports.dat");
        return -1;
    }
    fd = open(cfgPath, O_RDWR | O_CREAT, CFG_MODE);
    if(fd == -1){
        perror("open district.cfg");
        return -1;
    }
    if(fstat(fd, &st) == -1){
        perror("fstat district.cfg");
        close(fd);
        return -1;
    }
    if(st.st_size == 0){
        if(write(fd, defaultCfg, strlen(defaultCfg)) == -1){
            perror("write district.cfg");
            close(fd);
            return -1;
        }
    }

    close(fd);

    if(chmod(cfgPath, CFG_MODE) == -1){
        perror("chmod district.cfg");
        return -1;
    }

    fd = open(logPath, O_RDWR | O_CREAT, LOG_MODE);
    if(fd == -1){
        perror("open logged_district");
        return -1;
    }
    close(fd);

    if(chmod(logPath, LOG_MODE) == -1){
        perror("chmod logged_district");
        return -1;
    }

    if(lstat(symlinkPath, &st) == 0){
        if(unlink(symlinkPath) == -1){
            perror("unlink symlink");
            return -1;
        }
    }

    if(symlink(reportsPath, symlinkPath) == -1){
        perror("symlink");
        return -1;
    }

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
    printf("Report ID: %d\n",r->report_id);
    printf("Inspector: %s\n",r->inspector);
    printf("Latitude: %.2f\n",r->latitude);
    printf("Longitude: %.2f\n",r->longitude);
    printf("Category: %s\n",r->category);
    printf("Severity: %ld\n",r->severity);
    printf("Timestamp:",r->timestamp);
    printf("Description: %s\n",r->description);
    printf("-------------------------------------\n");
}






