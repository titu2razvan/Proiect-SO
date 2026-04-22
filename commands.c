#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "commands.h"
#include "helper.h"

void add(int argc, char **argv){
   char *role=getRole(argc,argv);
   char *user = getUser(argc, argv);
   char *district = getDistrict(argc, argv);
   int fd;
   struct stat st;
   Report r;
   char reportsPath[PATH_LEN];
   
   if(role == NULL || user == NULL || district == NULL){
        fprintf(stderr, "Invalid arguments (role|user|district) for add function\n");
        return;
    }

    if(districtSetup(district) != 0){
        return;
    }
   buildReportsPath(reportsPath, district);

   if(stat(reportsPath, &st)== -1){
    perror("report.dat stat error");
    return;
   }
   //here we verifiy if the manager and the inspector have write permission
   if(strcmp(role, "manager") == 0){
        if((st.st_mode & S_IWUSR) == 0){
            fprintf(stderr, "Manager has no write permission on reports.dat\n");
            return;
        }
    }
    else if(strcmp(role, "inspector") == 0){
        if((st.st_mode & S_IWGRP) == 0){
            fprintf(stderr, "Inspector has no write permission on reports.dat\n");
            return;
        }
    }
    else{
        fprintf(stderr,"Invalid role");
        return;
    }
    r.report_id=getNextReportId(district);
    strncpy(r.inspector, user, INSPECTOR_LEN - 1);
    r.inspector[INSPECTOR_LEN-1]='\0';
    
    printf("Latitude: ");
    scanf("%f", &r.latitude);

    printf("Longitude: ");
    scanf("%f", &r.longitude);

    printf("Category: ");
    scanf("%s", r.category);

    printf("Severity (1-3): ");
    scanf("%d", &r.severity);

    getchar();
    printf("Description: ");
    fgets(r.description, DESC_LEN, stdin);
    r.description[strcspn(r.description, "\n")] = '\0';

    r.timestamp=time(NULL);

    fd = open(reportsPath, O_WRONLY | O_APPEND);
    if(fd==-1){
        perror("error opening report.dat");
    }

    if(write(fd, &r, sizeof(Report)) != sizeof(Report)){
        perror("write report");
        close(fd);
        return;
    }
    close(fd);
    printf("Repord added!");
}

void list(int argc, char **argv){
    char *role=getRole(argc,argv);
    char *district = getDistrict(argc, argv);
    int fd;
    char reportsPath[PATH_LEN];
    buildReportsPath(reportsPath,district);
    struct stat st;
    char permString[10];
    char *timeString;
    Report r;
    if(role==NULL || district ==NULL){
        fprintf(stderr,"Inavalid arguments for list function");
        return;
    }
    if(stat(reportsPath, &st)== -1){
    perror("report.dat stat error");
    return;
    }

    if(strcmp(role, "manager") == 0){
        if((st.st_mode & S_IRUSR) == 0){
            fprintf(stderr, "Manager has no write permission on reports.dat\n");
            return;
        }
    }
    else if(strcmp(role, "inspector") == 0){
        if((st.st_mode & S_IRGRP) == 0){
            fprintf(stderr, "Inspector has no write permission on reports.dat\n");
            return;
        }
    }
    else{
        fprintf(stderr,"Invalid role");
        return;
    }

    modeToString(st.st_mode, permString);
    timeString=ctime(&st.st_mtime);
    printf("reports.dat info:\n");
    printf("Permissions: %s\n",permString);
    printf("Size: %ld bytes\n",(long)st.st_size);
    printf("Last modified: %s", timeString);

    fd = open(reportsPath, O_RDONLY);
    if(fd==-1){
        perror("error opening report.dat");
    }

    while(read(fd, &r,sizeof(Report))==sizeof(Report)){
        printReport(&r);
    }
    close(fd);

}

void view(int argc, char **argv){
    printf("view\n");
}

void remove_report(int argc, char **argv){
    printf("remove_report\n");
}

void update_threshold(int argc, char **argv){
    printf("update_threshold\n");
}

void filter(int argc, char **argv){
    printf("filter\n");
}