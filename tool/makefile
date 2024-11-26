# 컴파일러와 플래그 설정
CC = gcc
CFLAGS = -Wall -Wextra -g
INCLUDES = -I.

# 소스 파일 목록
SRC = main.c \
      attack_packet_sender.c \
      can_sender.c \
      payloads.c \
      nonblocking.c \
      attack-functions/dos.c \
      attack-functions/fuzzing.c \
      attack-functions/msq.c \
      attack-functions/replay.c \
      attack-functions/suspension.c

# 오브젝트 파일 목록
OBJ = $(SRC:.c=.o)

# 실행 파일 이름
TARGET = attackRAMN

# 기본 빌드 규칙
all: $(TARGET)
	@$(MAKE) clean_objs  # 빌드 완료 후 .o 파일 삭제

# 실행 파일 생성 규칙
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# 각 소스 파일에 대한 컴파일 규칙
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# attack-functions 디렉토리 내의 파일 컴파일 규칙
attack-functions/%.o: attack-functions/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 오브젝트 파일만 삭제하는 규칙
clean_objs:
	rm -f $(OBJ)

# 전체 클린업 규칙
clean:
	rm -f $(OBJ) $(TARGET)

# 디렉토리 내의 파일 변경을 추적
.PHONY: all clean clean_objs
