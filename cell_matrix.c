#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>

//role1. 살아있는 세포는 살아있는 이웃이 2개 이하 혹은 7개 이상이면 죽는다.
//role2. 살아있는 세포는 살아있는 이웃이 3개 ~ 6개이면 산다.
//role3. 죽은 세포는 살아있는 이웃이 4개면 산다.
//role4. 죽은 세포는 살아있는 이웃이 4개가 아니면 죽는다.

int M=0;
int N=0;

char threadMatrix[10000][10000]; 
typedef struct 
{
  char **matrix;
  int startRow;
  int endRow;
}threadEntry; //thread에서 사용할 구조체

//몫과 나머지를 기반으로 행의 크기를 균등하게 나눔
void rowNum(int num, int rowArray[][2]){ 
  int quotient=M/num; //몫 
  int remainder=M%num; //나머지
  int row=0;
  
  for(int i=0;i<num;i++){
    if(remainder!=0){
      rowArray[i][0]=row;
      row=row+quotient;
      rowArray[i][1]=row;
      remainder--;
      row++;
    }
    else{
      rowArray[i][0]=row;
      row=row+quotient-1;
      rowArray[i][1]=row;
      row++;
    }
  }
}

//i와 j가 M과 N 범위 안에 있는 유효한 값인지 판단
bool inRange(int i, int j){
  bool iInRange=true;
  bool jInRange=true;
  
  if(i<0 && i>=N) iInRange=false;
  if(j<0 && j>=M) iInRange=false;

  if(iInRange==true && jInRange==true) return true;
  else return false;
}

//matrix의 startRow~endRow의 범위에서 이웃한 값을 판별해 해당 세포가 살았는지 죽었는지 판별
void* lifeOrDead(int startRow, int endRow, char matrix[][N]){
  char temp[M][N]; //판별한 배열의 값을 임시로 저장할 배열
  int liveNeighbors=0;


  for(int i=startRow;i<=endRow;i++){
    for(int j=0;j<N;j++){
      liveNeighbors=0;

      //8개의 이웃을 확인함
      if((inRange(0, j-1)) && (matrix[i][j-1]=='1')) liveNeighbors++; //양옆의 두 이웃 
      if((inRange(0, j+1)) && (matrix[i][j+1]=='1')) liveNeighbors++; 
      if((inRange(i-1, 0)) && (matrix[i-1][j]=='1')) liveNeighbors++; //윗칸의 세 이웃 
      if((inRange(i-1, j-1)) && (matrix[i-1][j-1]=='1')) liveNeighbors++;
      if((inRange(i-1, j+1)) && (matrix[i-1][j+1]=='1')) liveNeighbors++;
      if((inRange(i+1, 0)) && (matrix[i+1][j]=='1')) liveNeighbors++; //아래칸의 세 이웃 
      if((inRange(i+1, j-1)) && (matrix[i+1][j-1]=='1')) liveNeighbors++; 
      if((inRange(i+1, j+1)) && (matrix[i+1][j+1]=='1')) liveNeighbors++;

      if(matrix[i][j]=='1' && (liveNeighbors<=2 || liveNeighbors>=7)){ //role1
        temp[i][j]='0';
      }
      else if(matrix[i][j]=='1' && (liveNeighbors>=3 && liveNeighbors<=6)){ //role2
        temp[i][j]='1';
      }
      else if(matrix[i][j]=='0' && liveNeighbors==4){ //role3
        temp[i][j]='1';
      }
      else if(matrix[i][j]=='0' && liveNeighbors!=4){ //role4
        temp[i][j]='0';
      }
    }
  }

  for(int i=startRow;i<=endRow;i++){ //판별 후 임시로 저장한 값을 matrix로 업데이트
    for(int j=0;j<N;j++){
      matrix[i][j]=temp[i][j];
    }
  }

  return matrix;
}

//matrix의 startRow~endRow의 행 부분을 공백과 개행 문자를 포함한 문자열로 변환
void matrixToBuf(int startRow, int endRow, char matrix[M][N], char* buf){
  int index=0;

  for(int i=startRow;i<=endRow;i++){
    for(int j=0;j<N;j++){
      buf[index++]=matrix[i][j];
      if(j!=N-1) buf[index++]=' '; //마지막 열에는 공백을 삽입해주지 않음 
    }
    buf[index++]='\n'; 
  }
  buf[--index]='\0'; //마지막 문자인 개행 문자를 삭제해주면서 null 문자를 넣어줌
  
}

//buf의 값을 배열 matrix 값으로 변환
void bufToMatrix(char* buf, char matrix[M][N]){
  int index=0, check=0;
  for(int i=0;i<M;i++){
    for(int j=0;j<N;j++){
      while(1){
        if(buf[index]=='\0'){
          break;
        }
        else if(buf[index]=='0' || buf[index]=='1'){
            matrix[i][j]=buf[index++];
            break;
        }
        else{
          index++;
        }
      }
    }
  }
}

void single(char matrix[M][N], clock_t inputfile){

  int num=0;

  printf("세대 수를 입력해주세요 : ");
  scanf("%d",&num);
  printf("\n");

  clock_t start=clock();

  FILE *file[num];
  char buf[M*N*2-1]; //파일에 입력할 문자열
  char path[16];

  if(num<1){
    fprintf(stderr, "error : enter the number of generation (>1) \n");
    exit(0);
  }

  for(int i=0;i<num;i++){ 
    matrix=lifeOrDead(0, M-1, matrix); //1세대 matrix를 생성
    matrixToBuf(0, M-1, matrix, buf); //배열의 내용을 공백과 개행문자를 포함한 문자열로 변환

    if(i==num-1){ //마지막 파일이면
      sprintf(path, "output.matrix");
    }
    else{
      sprintf(path,"gen%d.matrix", i+1);
    }
    if((file[i]=fopen(path, "w"))<0){
      fprintf(stderr, "error : file open at %s\n", path);
      return;
    }
    if((fwrite(buf, sizeof(char), sizeof(buf), file[i]))<0){
      fprintf(stderr, "error : file write at %s\n", path);
    }
    
    printf("%s 파일을 생성하였습니다.\n", path);
    fclose(file[i]); 
  }
  printf("\n");

  clock_t end=clock();
  clock_t total = end - start + inputfile;

  printf("수행시간은 %lu (ms)입니다. \n", total);
}

void process(char matrix[M][N], clock_t inputfile){
  int childNum=0, num=0, status=0;
  char origin[M][N];

  for(int i=0;i<M;i++){ //파라미터로 넘어온 matrix 원본 내용을 origin에 보존
    for(int j=0;j<N;j++){
      origin[i][j]=matrix[i][j];
    }
  }

  printf("child process 수를 입력해주세요 : ");
  scanf("%d", &childNum);
  printf("세대 수를 입력해주세요 : ");
  scanf("%d", &num);
  printf("\n");

  if(num<1 || childNum<0){
    fprintf(stderr, "error : enter the number of generation (>1) \n");
    exit(0);
  }

  clock_t start=clock();

  pid_t pid[num][childNum];
  int rowArray[childNum][2]; //균등하게 나눈 행에 대한 정보를 저장
  char childBuf[childNum][M*N*2-1]; //child process로 작업한 배열 저장
  char* shmaddr; //부모 프로세스와 자식 프로세스를 이어줄 공유 메모리
  char buf[M*N*2-1]; //child process들의 배열을 취합한 배열

  int shmid=0;
  rowNum(childNum, rowArray); //child process의 개수에 따라 균등하게 행을 나누고 그 정보를 rowArray에 저장

  FILE *file[num];
  char path[16];
  int index=0; 

  for(int t=0;t<num;t++){ //세대 개수만큼 반복
    index=0; 
    shmid=shmget((key_t)2223+t, sizeof(char)*M*N*2-1, 0600|IPC_CREAT); //공유 메모리 생성
    shmaddr=(char*)shmat(shmid, (void*)0, 0); //공유 메모리 접속
    
    for(int i=0;i<childNum;i++){  //child process 개수만큼 반복
      pid[t][i]=fork();

      for(int i=0;i<M;i++){ //matrix 배열의 값을 origin 배열의 값으로 초기화
          for(int j=0;j<N;j++){
            matrix[i][j]=origin[i][j];
          }
        }

      if(pid[t][i]==0){ //child
        //matrix의 rowArray[0]~rowArray[1]의 행을 lifeOrDead로 판별한 후, 반환되는 배열을 문자열 형태로 childBuf[i]에 저장
        matrixToBuf(rowArray[i][0], rowArray[i][1], lifeOrDead(rowArray[i][0], rowArray[i][1], matrix), childBuf[i]);
        strcpy((char*)shmaddr, childBuf[i]); //childBuf[i]의 내용을 공유 메모리에 저장
        shmdt(shmaddr); //공유 메모리 접속 해제 
        exit(1); //자식 프로세스 종료
      }

      if(waitpid(pid[t][i], &status, 0)==pid[t][i]){ //parent 자식 모두 회수
        for(int m=0;(m<M*N*2-1 && shmaddr[m]!='\0');m++){
          buf[index++]=shmaddr[m]; //회수하면서 공유 메모리에 저장된 내용을 취합
        }
        buf[index++]='\n';
      }
    }
    buf[--index]='\0'; //buf의 마지막 문자에 null을 삽입

    shmctl(shmid, IPC_RMID, NULL);
    shmdt(shmaddr); //공유 메모리 접속 해제 

    if(t==num-1){ //마지막 파일이면
      sprintf(path, "output.matrix");
    }
    else{
      sprintf(path,"gen%d.matrix", t+1);
    }
    if((file[t]=fopen(path, "w"))<0){
      fprintf(stderr, "error : file open at %s\n", path);
      return;
    }
    if((fwrite(buf, sizeof(char), sizeof(buf), file[t]))<0){
      fprintf(stderr, "error : file write at %s\n", path);
    }
    fclose(file[t]); 
    
    printf("%s 파일을 생성하였습니다.\n", path);
    bufToMatrix(buf, origin); //finalBuf의 내용을 origin에 업데이트함 
  }

  printf("\n총 생성한 프로세스 ID\n");
  for(int i=0;i<num;i++){
    printf("%d 세대\n", i+1);
    for(int j=0;j<childNum;j++){
      printf("pid : %d\n", pid[i][j]);
    }
  }
  printf("\n");

  clock_t end=clock();
  clock_t total = end - start + inputfile;

  printf("수행시간은 %lu (ms)입니다. \n", total);
}

void* threadStart(void* arg){
  threadEntry *threadData = (threadEntry *)arg; 
  int start = threadData->startRow;
  int end = threadData->endRow;

  char matrix[M][N];
  for(int i=0;i<M;i++){
    for(int j=0;j<N;j++){
      matrix[i][j]=(threadData->matrix)[i][j];
    }
  }

  int liveNeighbors;
  char temp[M][N];
  for(int i=start;i<=end;i++){
    for(int j=0;j<N;j++){
      liveNeighbors=0;

      //8개의 이웃을 확인함
      if((inRange(0, j-1)) && (matrix[i][j-1]=='1')) liveNeighbors++; //양옆의 두 이웃 
      if((inRange(0, j+1)) && (matrix[i][j+1]=='1')) liveNeighbors++; 
      if((inRange(i-1, 0)) && (matrix[i-1][j]=='1')) liveNeighbors++; //윗칸의 세 이웃 
      if((inRange(i-1, j-1)) && (matrix[i-1][j-1]=='1')) liveNeighbors++;
      if((inRange(i-1, j+1)) && (matrix[i-1][j+1]=='1')) liveNeighbors++;
      if((inRange(i+1, 0)) && (matrix[i+1][j]=='1')) liveNeighbors++; //아래칸의 세 이웃 
      if((inRange(i+1, j-1)) && (matrix[i+1][j-1]=='1')) liveNeighbors++; 
      if((inRange(i+1, j+1)) && (matrix[i+1][j+1]=='1')) liveNeighbors++;

      if(matrix[i][j]=='1' && (liveNeighbors<=2 || liveNeighbors>=7)){ //role1
        temp[i][j]='0'; 
      }
      else if(matrix[i][j]=='1' && (liveNeighbors>=3 && liveNeighbors<=6)){ //role2
        temp[i][j]='1'; 
      }
      else if(matrix[i][j]=='0' && liveNeighbors==4){ //role3
        temp[i][j]='1'; 
      }
      else if(matrix[i][j]=='0' && liveNeighbors!=4){ //role4
        temp[i][j]='0'; 
      }
    }
  }
  for(int i=start;i<=end;i++){
    for(int j=0;j<N;j++){
      threadMatrix[i][j]=temp[i][j];
    }
  }

  return (void*)0;
}

void thread(char origin[M][N], clock_t inputfile){
  int threadNum=0, num=0;

  char** matrix; //thread 함수에 파라미터로 넣어줄 구조체 멤버

  //matrix의 메모리 할당
  matrix=(char**)malloc(sizeof(char*)*N);
  for(int i=0;i<M;i++){
    matrix[i]=(char*)malloc(sizeof(char)*N);
  }

  for(int i=0;i<M;i++){
    for(int j=0;j<N;j++){
      matrix[i][j]=origin[i][j]; //파라미터로 넘어온 origin 원본 배열을 matrix 초기화
    }
  }
  
  printf("thread 수를 입력해주세요 : ");
  scanf("%d", &threadNum);
  printf("세대 수를 입력해주세요 : ");
  scanf("%d", &num);
  printf("\n");

  if(num<1 || threadNum<0){
    fprintf(stderr, "error : enter the number of generation (>1) \n");
    exit(0);
  }

  clock_t start=clock();

  pthread_t tid[num][threadNum];
  threadEntry threadArray[threadNum]; //구조체 배열 선언
  int rowArray[threadNum][2]; //균등하게 나눈 행에 대한 정보를 저장
  char buf[M*N*2-1]; //thread로 작업 후 취합한 최종 배열을 문자열로

  rowNum(threadNum, rowArray); //thread의 개수에 따라 균등하게 행을 나누고 그 정보를 rowArray에 저장
  
  FILE *file[num];
  char path[16];
  int index=0;

  for(int t=0;t<num;t++){
    index=0; //세대별로 변수 초기화

    for(int i=0;i<threadNum;i++){ //thread 개수만큼 thread 생성

     for(int m=0;m<M;m++){ //matrix의 내용은 origin 내용으로 항상 업데이트
        for(int n=0;n<N;n++){
         matrix[m][n]=origin[m][n];
        }
      }

      //threadEntry 구조체 초기화
      threadArray[i].startRow=rowArray[i][0];
      threadArray[i].endRow=rowArray[i][1];
      threadArray[i].matrix=matrix;
      
      pthread_create(&tid[t][i], NULL, threadStart, (void*)&threadArray[i]); //thread 생성 
    }

    for(int i=0;i<threadNum;i++){ //thread join
      pthread_join(tid[t][i], NULL);
    }

    char temp[M][N]; //전역변수 배열의 값을 잠시 저장할 배열
      for(int m=0;m<M;m++){
         for(int j=0;j<N;j++){
           temp[m][j]=threadMatrix[m][j];
         }
    }
    matrixToBuf(0, M-1, temp , buf); //thread들이 수행한 작업을 matrix 배열에 저장

    if(t==num-1){ //마지막 파일이면
      sprintf(path, "output.matrix");
    }
    else{
      sprintf(path,"gen%d.matrix", t+1);
    }
    if((file[t]=fopen(path, "w"))<0){
      fprintf(stderr, "error : file open at %s\n", path);
      return;
    }
    if((fwrite(buf, sizeof(char), sizeof(buf), file[t]))<0){
      fprintf(stderr, "error : file write at %s\n", path);
    }
    fclose(file[t]); 
    
    printf("%s 파일을 생성하였습니다.\n", path);
    bufToMatrix(buf, origin); //buf의 내용을 origin에 업데이트
  }

  printf("\n총 생성한 스레드 ID\n");
  for(int i=0;i<num;i++){
    printf("%d 세대\n", i+1);
    for(int j=0;j<threadNum;j++){
      printf("tid : %d\n",(int)tid[i][j]);
    }
  }
  printf("\n");

  clock_t end=clock();
  clock_t total = end - start + inputfile;

  printf("수행시간은 %lu (ms)입니다. \n", total);
}

int main(int argc, char* argv[]){
  clock_t start, end;
  double mstime;

  FILE *inputFile;
  int length=0;
  char buf[10000];

  start = clock();

  if((inputFile=fopen(argv[1], "r"))<0){
    fprintf(stderr, "error : file open at %s\n", argv[0]);
  }

  if((length=fread(buf, 1, 10000, inputFile))<0){
    fprintf(stderr, "error : file read at %s\n", argv[0]);
  }
  buf[length]='\0';

  //m x n 에서 m, n 의 값을 파악
  int index=0;
  while(length>=0){
    if(buf[index]=='\n') M++; 
    else if(M==1 && (buf[index]=='0' || buf[index]=='1')) N++;
    index++; 
    length--;
  }

  //input.maxtirx의 내용을 maxtrix 배열에 저장
  index=0;
  char matrix[M][N];
  for(int i=0;i<M;i++){
    for(int j=0;j<N;j++){ 
      while(buf[index]!='\0'){
        if(buf[index]=='0' || buf[index]=='1') {
          matrix[i][j]=buf[index];
          index++;
          break;
        }
        else index++;
      }
    }
  }

  end = clock();
  clock_t timeInput = end-start;

  while(1){
    printf("(1)프로그램 종료 (2)순차처리 (3)Process 병렬처리 (4)Thread 병렬처리 \n");
    int choice=-1;
    scanf("%d", &choice);

    switch (choice)
    {
    case 1:
      return 0;
    case 2:
      single(matrix, timeInput);
      break;
    case 3:
      process(matrix, timeInput);      
      break;
    case 4:
      thread(matrix, timeInput);
      break;
    default:
      printf("error : 1부터 4까지의 수를 입력해주세요. \n");
      break;
    }
  }
  fclose(inputFile);
}