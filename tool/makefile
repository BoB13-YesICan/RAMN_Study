# Makefile for the CAN packet sender tool

# 컴파일러와 플래그 설정
CC = gcc
CFLAGS = -Wall -Wextra -g
INCLUDES = -I.

# 소스 파일 목록
SRC = main.c \
      attack_packet_sender.c \
      can_sender.c \
      payloads.c \
      attack-functions/dos.c \
      attack-functions/fuzzing.c \
      attack-functions/msq.c \
      attack-functions/replay.c \
      attack-functions/suspension.c

# 오브젝트 파일 목록
OBJ = $(SRC:.c=.o)

# 실행 파일 이름
TARGET = tool

# 기본 빌드 규칙
all: $(TARGET)

# 실행 파일 생성 규칙
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# 각 소스 파일에 대한 컴파일 규칙
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# attack-functions 디렉토리 내의 파일 컴파일 규칙
attack-functions/%.o: attack-functions/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 클린업 규칙
clean:
	rm -f $(OBJ) $(TARGET)

# 디렉토리 내의 파일 변경을 추적
.PHONY: all clean
