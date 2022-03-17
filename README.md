# linux_system_programming

## myls.c
```-ls option``` 
- 옵션이 없는 경우 파일명을 이름순으로만 나열해준다. 
-  -l 옵션은 파일에 대한 정보, 수정 시간, 파일 이름 등 세부적인 내용을 알려준다. 
-  -i 옵션은 파일명과 함께 inode의 번호를 알려준다.
-  -t 옵션은 수정 시간을 기준으로 파일의 이름을 정렬하여 나열해준다.

### program start:
```./myls -option 파일명```

### output:
본래 ls 실행 결과

### function
- 옵션이 없는 경우는 optionNone 함수, l 옵션은 optionL 함수, i 옵션은 optionI, t 옵션은 optionT 함수를 통해 처리한다. <br/><br/>

## cell_matrix.c
순차처리, 병렬처리, thread 처리 중 하나를 선택하여 세포 게임을 진행한다  
- role1. 살아있는 세포는 살아있는 이웃이 2개 이하 혹은 7개 이상이면 죽는다.
- role2. 살아있는 세포는 살아있는 이웃이 3개 ~ 6개이면 산다.
- role3. 죽은 세포는 살아있는 이웃이 4개면 산다.
- role4. 죽은 세포는 살아있는 이웃이 4개가 아니면 죽는다.


### program start:
```./a.out input.matrix```
### output:
세대별 생성된 genN.matrix

