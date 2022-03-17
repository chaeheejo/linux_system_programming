#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

void optionNone(char *dir); //옵션이 아무것도 없는 경우
void printName(char * sort[],int i, int length, int column); //디렉토리의 이름을 정해진 form으로 출력해주는 함수
void printInode(char * sort[], ino_t inode[],int i, int length, int column); //디렉토리의 아이노드와 이름을 정해진 form으로 출력해주는 함수
void optionI(char *dir); //옵션이 i일 경우
void optionL(char *dir); //옵션이 l일 경우
void optionT(char *dir); //옵션이 t일 경우
void nameSorting(char * sort[], int i); //이름을 기반으로 정렬을 해주는 함수
char type(mode_t); //파일 타입을 리턴
char* perm(mode_t); //권한을 리턴
void printStat(char*, char*, struct stat*); //파일의 사용 권한을 리턴

int main(int argc, char **argv){
  char *dir;
  struct stat st;
    
  if(argc == 1){ //옵션을 입력하지 않음
    dir = ".";
    lstat(dir, &st);
    if(S_ISDIR(st.st_mode)){
      optionNone(dir);
    }
  } 
  else if(argc==2){ //파일 이름을 입력하지 않고, 옵션만 입력 한 경우와 옵션을 입력하지 않고 파일 이름만 입력한 경우
    dir = ".";
    if(strcmp(argv[1],"-i")==0){ //inode, 이름(정렬)
      optionI(dir);
    }
    else if(strcmp(argv[1],"-l")==0){ //합계, 권한, 생성자 이름 등, 이름(정렬)
      optionL(dir);
    }
    else if(strcmp(argv[1],"-t")==0){ //최종 수정 시간 순으로 정렬
      optionT(dir);
    }
    else {
      dir = argv[1]; //파일의 경로 설정
      lstat(dir, &st);
      if(S_ISDIR(st.st_mode)){ //옵션을 입력하지 않은 경우
        optionNone(dir);
      }
      else{ //입력한 파일의 경로가 디렉토리가 아닐 경우
        printf("%s\n", dir);
      } 
    }
  } 

  else if(argc>2){ //파일이름과 옵션을 입력한 경우
    dir = argv[2];
    if(strcmp(argv[1],"-i")==0){ //inode, 이름(정렬)
      optionI(dir);
    }
    else if(strcmp(argv[1],"-l")==0){ //합계, 권한, 생성자 이름 등, 이름(정렬)
      optionL(dir);
    }
    else if(strcmp(argv[1],"-t")==0){ //최종 수정 시간 순으로 정렬
      optionT(dir);
    }
    else {
      perror("옵션을 잘못 입력하셨습니다.");
    }
  }
}

//옵션이 아무것도 없을 때
void optionNone(char *dir){
  DIR *dp;
  struct dirent *d;
  char *sort[100];
  int total;

  if ((dp = opendir(dir)) == NULL){ // 디렉토리 열기
    perror(dir);
  }

  int i=0; //index
  int length=0; //가장 긴 문자열을 저장하기 위한 변수
  while((d = readdir(dp)) != NULL) {
    if(d->d_name[0]!='.'){ //.으로 시작하는 파일을 제외하고,
      sort[i]=d->d_name;  //정렬을 위해 디렉토리 이름을 순서대로 저장해준다
      i++;
      
      if(length<strlen(d->d_name)){  //가장 긴 문자열을 구한다.
        length=strlen(d->d_name);
      }
    }
  }

  nameSorting(sort, i);  //이름순으로 정렬

  if(length>=40){ //40개 이상일 경우, column의 개수는 3
    printName(sort, i, length, 3); //파일의 이름을 정해진 form으로 출력해줌
  } 
  else if(length<40 &&i>6){ //한 파일의 이름이 40이 안 넘고, 파일의 개수가 6개 초과인 경우 column의 개수는 6 
    printName(sort, i, length, 6); 
  }
  else if(length<40 && i<=6){ //한 파일의 이름이 40이 안 넘고, 파일의 개수가 6개 미만인 경우 column의 개수는 i 
    printName(sort, i, length, i);
  }
}

//옵션이 아무것도 없는 케이스에서, 파일의 이름을 출력해주는 함수
void printName(char * sort[],int i, int length, int column){
  int line=i/column; //출력할 라인의 개수는 i를 column으로 나눈 값이다.
  int remainder=i%column; //배열의 인덱스를 먼저 출력할 형태로 지정

  if(i%column!=0){ //column으로 나눠 나머지가 있는 경우에는,
      line++; //한 라인이 더 필요하다. 
  }

  int index=0;
  int location[line][column]; //배열의 인덱스를 출력할 형태로 저장함
  if(remainder!=0){ //나머지가 있다면
    for(int j=0;j<column;j++){
      for(int m=0;m<line;m++){
        if(remainder<=j && m==(line-1)){ //마지막 줄의 나머지의 개수가 넘는 column에는 999999를 넣어줌
          location[line-1][j]=99999;
        }
        else{
          location[m][j]=index;
          index++;
        }
      }
    }
  }else{ //나머지가 없다면
    for(int j=0;j<column;j++){
      for(int m=0;m<line;m++){
        location[m][j]=index;
        index++;
      }
    }
  }

  int loca=0;
  for(int m=0;m<line;m++){ //line의 횟수만큼 for문을 반복한다.
    for(int j=0;j<column;j++){ //배열의 index에 접근하기 위한 for문
      loca=location[m][j];
      if(loca!=99999){ 
        printf("%s", sort[loca]); //이름을 출력해주고,
        int blank=strlen(sort[loca])-2; //빈칸을 출력해 출력하는 form을 설정한다. 이름과 이름 사이의 구분을 2칸씩 지어준다.
        while(true){ //빈칸 수를 정렬해주기 위해 가장 긴 문자열의 개수, length를 기준으로
          if(blank<length){ //나머지 공간에 빈칸을 채워준다.
            printf(" ");
            blank++;
          }
          else{
            break;
          }
        }
      }
      else{
        break;
      }
    }
    printf("\n");
  }
}

//옵션이 i인 케이스에서, 파일의 이름과 inode를 출력해주는 함수
void printInode(char * sort[], ino_t inode[],int i, int length, int column){
  int line=i/column; //출력할 라인의 개수는 i를 column으로 나눈 값이다.
  int remainder=i%column; //배열의 인덱스를 먼저 출력할 형태로 지정

  if(i%column!=0){ //column으로 나눠 나머지가 있는 경우에는,
      line++; //한 라인이 더 필요하다. 
  }

  int index=0;
  int location[line][column]; //배열의 인덱스를 출력할 형태로 저장함
  if(remainder!=0){ //나머지가 있다면
    for(int j=0;j<column;j++){
      for(int m=0;m<line;m++){
        if(remainder<=j && m==(line-1)){ //마지막 줄의 나머지의 개수가 넘는 column에는 999999를 넣어줌
          location[line-1][j]=99999;
        }
        else{
          location[m][j]=index;
          index++;
        }
      }
    }
  }else{ //나머지가 없다면
    for(int j=0;j<column;j++){
      for(int m=0;m<line;m++){
        location[m][j]=index;
        index++;
      }
    }
  }

  int loca=0;
  for(int m=0;m<line;m++){ //line의 횟수만큼 for문을 반복한다.
    for(int j=0;j<column;j++){ //배열의 index에 접근하기 위한 for문
      loca=location[m][j];
      if(loca!=99999){ 
        printf("%lu %s", inode[loca], sort[loca]); //이름을 출력해주고,
        int blank=strlen(sort[loca])-2; //빈칸을 출력해 출력하는 form을 설정한다. 이름과 이름 사이의 구분을 2칸씩 지어준다.
        while(true){ //빈칸 수를 정렬해주기 위해 가장 긴 문자열의 개수, length를 기준으로
          if(blank<length){ //나머지 공간에 빈칸을 채워준다.
            printf(" ");
            blank++;
          }
          else{
            break;
          }
        }
      }
      else{
        break;
      }
    }
    printf("\n");
  }
}

//옵션이 -i일 때
void optionI(char *dir){
  DIR *dp;
  struct dirent *d;
  struct stat st;
  char *name[100]; 
  ino_t inode[100];
  char path[1024];

  if ((dp = opendir(dir)) == NULL){ // 디렉토리 열기
    perror(dir);
  }

  int i=0; //index
  int length=0; //가장 긴 문자열을 저장하기 위한 변수
  while((d = readdir(dp)) != NULL) {
    if(d->d_name[0]!='.'){ //.으로 시작하는 파일을 제외하고,
      name[i]=d->d_name;  //정렬을 위해 디렉토리 이름을 순서대로 저장해준다
      i++;
      
      if(length<strlen(d->d_name)){  
        length=strlen(d->d_name)+1; //inode가 출력될 길이까지 포함해서 가장 긴 문자열을 구한다.
      }
    }
  }

  //stat 구조체의 inode 번호 저장
  for(int j=0;j<i;j++){ // 디렉토리의 각 파일에 대해
    sprintf(path, "%s/%s", dir, name[j]); // 파일경로명 만들기

    if (lstat(path, &st) < 0){ //파일 상태 정보 가져오기
      perror(path);
    }

    inode[j]=st.st_ino; //inode 번호 저장
  }

  //이름을 기반으로 inode[]와 name[] 정렬
  char *temp=" ";
  ino_t tempInode;
  for(int m=0;m<i;m++){
    for(int n=m;n<i;n++){
      if(strcmp(name[m],name[n])>0){ //이름 순으로 정렬 
        temp=name[m];  //name 배열의 이름을 정렬해준다.
        name[m]=name[n];
        name[n]=temp;

        tempInode=inode[m];  //inode 배열 또한 이름을 기반으로 정렬해준다. 
        inode[m]=inode[n];
        inode[n]=tempInode;
      }
    }
  }

  if(length>=40){ //40개 이상일 경우, column의 개수는 2
    printInode(name, inode, i, length, 2);
  } 
  else if(length<40 &&i>6){ //한 파일의 이름이 40이 안 넘고, 파일의 개수가 6개 초과인 경우 column의 개수는 4 
    printInode(name,inode, i, length, 4);
  }
  else if(length<40 && i<=6){ //한 파일의 이름이 40이 안 넘고, 파일의 개수가 6개 미만인 경우 column의 개수는 i 
    printInode(name, inode, i, length, i);
  }

}

//옵션이 -l로 지정됐을 때
void optionL(char *dir){
  DIR *dp;
  struct stat st;
  struct dirent *d;
  char path[1024];
  char *sort[100];
  int total=0; //ls -l 명령어의 맨 첫번째 줄의 합계

  if ((dp = opendir(dir)) == NULL){ // 디렉토리 열기
    perror(dir);
  }

  int i=0; //index
  while((d = readdir(dp)) != NULL) {
    sprintf(path, "%s/%s", dir, d->d_name); // 파일경로명 만들기
    if (lstat(path, &st) < 0){ //total 변수를 구하기 위해 파일 상태 정보 가져오기
      perror(path);
    }
    
    if(d->d_name[0]!='.'){ //.으로 시작하는 파일을 제외하고,
      sort[i]=d->d_name;  //정렬을 위해 디렉토리 이름을 순서대로 저장해준다.
      i++;
      total+=st.st_blocks; //block의 개수를 더해줌
    }
  }

  printf("합계 %d\n", total/2); //나누기 2 해주면 total을 구할 수 있음
  
  nameSorting(sort, i); //이름을 정렬해줌

  for(int j=0;j<i;j++){ // 디렉토리의 각 파일에 대해
    sprintf(path, "%s/%s", dir, sort[j]); // 파일경로명 만들기

    if (lstat(path, &st) < 0){ //파일 상태 정보 가져오기
      perror(path);
    }

    printStat(path, sort[j], &st); // 상태 정보 출력
    putchar('\n');
  }

  closedir(dp);
  exit(0);
}

//-t 옵션
void optionT(char *dir){
  DIR *dp;
  struct dirent *d;
  struct stat st;
  char *name[100]; 
  time_t mtime[100];
  char path[1024];

  if ((dp = opendir(dir)) == NULL){ // 디렉토리 열기
    perror(dir);
  }

  int i=0; //index
  int length=0; 

  //디렉토리의 이름 저장
  while((d = readdir(dp)) != NULL) { 
    if(d->d_name[0]!='.'){ //.으로 시작하는 파일을 제외하고,
      name[i]=d->d_name;  //정렬을 위해 이름을 순서대로 저장해준다
      i++;
      
      if(length<strlen(d->d_name)){  //가장 긴 문자열을 구한다.
        length=strlen(d->d_name);
      }
    }
  }

  //디렉토리의 수정 시간 저장
  for(int j=0;j<i;j++){ // 디렉토리의 각 파일에 대해
    sprintf(path, "%s/%s", dir, name[j]); // 파일경로명 만들기

    if (lstat(path, &st) < 0){ //파일 상태 정보 가져오기
      perror(path);
    }

    mtime[j]=st.st_mtime; //수정 시간 저장
  }

  //수정 시간을 기반으로 이름을 정렬
  char *temp=" ";
  time_t tempTime;
  for(int m=0;m<i;m++){
    for(int n=m;n<i;n++){
      if(mtime[m]<mtime[n]){ //인덱스가 더 작은 것의 시간 값이 더 커야 한다.
        tempTime=mtime[m];  //mtime 배열을 정렬
        mtime[m]=mtime[n];
        mtime[n]=tempTime;

        temp=name[m];  //수정 시간에 기반해 name 배열의 이름을 정렬해준다.
        name[m]=name[n];
        name[n]=temp;
      }
    }
  }

  if(length>=40){ //40개 이상일 경우, column의 개수는 3
    printName(name, i, length, 3); //파일의 이름을 정해진 form에 맞춰 출력해주는 함수
  } 
  else if(length<40 &&i>6){ //한 파일의 이름이 40이 안 넘고, 파일의 개수가 6개 초과인 경우 column의 개수는 6 
    printName(name, i, length, 6);
  }
  else if(length<40 && i<=6){ //한 파일의 이름이 40이 안 넘고, 파일의 개수가 6개 미만인 경우 column의 개수는 i 
    printName(name, i, length, i);
  }

}

//이름을 기반으로 정렬해주는 함수
void nameSorting(char * sort[], int i){ 
  char *temp=" ";

  for (int m=0;m<i;m++){ //이름 순으로 정렬
    for(int n=m;n<i;n++){
      if(strcmp(sort[m],sort[n])>0){
        temp=sort[m];
        sort[m]=sort[n];
        sort[n]=temp;
      }
    }
  }
}

/* 파일 상태 정보를 출력*/
void printStat(char *pathname, char *file, struct stat *st) {
  printf("%c%s ", type(st->st_mode), perm(st->st_mode));
  printf("%lu ", st->st_nlink);
  printf("%s %s ", getpwuid(st->st_uid)->pw_name, getgrgid(st->st_gid)->gr_name);
  printf("%5ld ", st->st_size);
  printf("%.12s ", ctime(&st->st_mtime)+4);
  printf("%s", file);
}

/* 파일 타입을 리턴*/
char type(mode_t mode) {
  if (S_ISREG(mode))
    return('-');
  if (S_ISDIR(mode))
    return('d');
  if (S_ISCHR(mode))
    return('c');
  if (S_ISBLK(mode))
    return('b');
  if (S_ISLNK(mode))
    return('l');
  if (S_ISFIFO(mode))
    return('p');
  if (S_ISSOCK(mode))
    return('s');
  return 0;
}

/* 파일 사용권한을 리턴 */
char* perm(mode_t mode) {
  static char perms[9];
  for(int i=0;i<9;i++){
    perms[i]='-';
  }
  for (int i=0; i < 3; i++) {
    if (mode & (S_IRUSR >> i*3))
      perms[i*3] = 'r';
    if (mode & (S_IWUSR >> i*3))
      perms[i*3+1] = 'w';
    if (mode & (S_IXUSR >> i*3))
      perms[i*3+2] = 'x';
  }
  if(mode & S_ISUID){  //setUerID 확인
    if(perms[2]=='x'){ //x가 허용되었으면
      perms[2]='s'; //small s로 바꿔준다.
    }
    else{ //x가 허용되지 않았으면
      perms[2]='S'; //big s로 바꿔준다.
    }
  }
  if(mode & S_ISGID){  //setGroupID 확인
    if(perms[5]=='x'){ //x가 허용되었으면
      perms[5]='s';
    }
    else{
      perms[5]='S';
    }
  }
  if(mode & S_ISVTX){ //sticky bit 확인
    if(perms[8]=='x'){
      perms[8]='t';
    }
    else{
      perms[8]='T';
    }
  }
  return perms;
}