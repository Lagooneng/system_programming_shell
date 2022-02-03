/* mysh: make a shell, by sys32204236 이름: 정현우 학번: 32204236, 최초 작성일: 10월 24일, 마지막 수정일: 2021/11/3 nuna90@naver.com*/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <error.h>

// 명령을 토큰으로 분리
int tokenizer(char *line, char *tokens[]) {
	int count = 0;
	char *token;

	token = strtok(line, " ");
        while( token != NULL && count < 99 ) {
        	tokens[count] = token;
               	token = strtok(NULL, " ");
                count++;
      	}
	if( strcmp(tokens[count - 1], "&") == 0 ) {
		count = count - 1;
	}
	tokens[count] = '\0';
	return count;
}

// 리다이렉션 처리
int redirection(char *tokens[], int token_size) {
	int fd, count = 0, idx = -1;

	while( count < token_size ) {
		if( strcmp(tokens[count], ">") == 0 ) { 
			idx = count;
			break;
		}
		count = count + 1;	
	}
	if( idx == 0 ) {
		printf("USAGE: input > output\n");
		return -1;
	} if( idx < 0 ) {
		return 0;
	} else if( idx > 0 ) {
		if( (count + 1) == token_size ) {
			printf("USAGE: input > output\n");
			return -1;
		}
		fd = open(tokens[idx + 1], O_RDWR | O_CREAT | O_TRUNC, 0641);
		if( fd < 0 ) {
			perror("open error");
			exit(1);
		}
		
		dup2(fd, STDOUT_FILENO);
		close(fd);
		tokens[idx] = '\0';
	}
	return 0;
}

//exit 내부 명령: 쉘 종료
int command_exit(char *token) {
	return strcmp(token, "exit");
}

// cd 내부 명령: 디렉토리 변경
int command_cd(char *tokens[]) {
	int result;
	if( strcmp(tokens[0], "cd") != 0 ) {
		return 0;
	}
	
	result = chdir(tokens[1]);
	if( result < 0 ) {
		return -1;
	} else {
		return 1;
	}
}

// help, ? 내부 명령: help 나 ? 입력 시 도움말 출력
int command_help(char *help) {
	if( strcmp(help, "help") == 0 || strcmp(help, "?") == 0 ) {
		printf("----------MY Shell----------\n");
		printf("This shell can be used like conventional shell\n\n");
		printf("Internal commands\n");
		printf("cd\t: change directory\n");
		printf("exit\t: exit this shell\n");
		printf("help\t: show this help\n");
		printf("?\t: show this help\n");
		printf("----------------------------\n");
		return 0;
	}
	return -1;
}

// fork(), execvp(), redirection 확인, 백그라운드 프로세싱 확인, 내부 명령어 사용
int run(char *tokens[], int token_size, int bgCheck) {
	pid_t fork_return;
	int rd_check, cd_check;	
	pid_t w;
	if( command_exit(tokens[0]) == 0 ) exit(0);

	cd_check = command_cd(tokens);
	if( cd_check > 0) {
		return 0;
	} else if ( cd_check < 0 ) {
		printf("이동 실패\n");
		return 0;
	}

	if( command_help(tokens[0]) == 0 ) {
		return 0;
	}

	fork_return = fork();
	if( fork_return < 0 ) {
		printf("fork error\n");
          	return -1;   
        } else if( fork_return == 0 ) {
		rd_check = redirection(tokens, token_size);
                if( rd_check < 0 ) exit(1);

                execvp(tokens[0], tokens);
                printf("명령이 적절하지 않습니다.\n");
                exit(1);
        } else {
		if( bgCheck == 0 ) {
			waitpid(fork_return, NULL, 0);
		}
	}	

	return 0;
}

// 메인 함수
int main(int argc, char *argv[]) {
	int bgCheck, token_size;
	char line[1024];
	char *tokens[100];

	if(argc != 1) {
		printf("USAGE: %s\n", argv[0]);
		return 0;
	}
	
	while(1) {
		bgCheck = 0;
		printf("%s $ ", get_current_dir_name());
        	fgets(line, sizeof(line), stdin);	
	
		if( strcmp(line, "\n") == 0 ) continue;

		line[strlen(line) - 1] = '\0';	
				
		if( line[strlen(line) - 1] == '&' ) {
			bgCheck = 1;
		}
		
		token_size = tokenizer(line, tokens);
		
		if( run(tokens, token_size, bgCheck) < 0 ) break;
	}

	return 0;
}
