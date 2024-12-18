# =====================================================================
# Makefile for UDS_fuzzer
# =====================================================================

# 컴파일러 설정
CC = gcc
CFLAGS = -Wall -Wextra -g -I. -Iattack-functions      # 모든 경고 활성화, 디버깅 정보 포함, 헤더 파일 경로 추가
LDFLAGS = -lpthread                                  # 필요한 라이브러리 링크 (예: pthread)

# 타겟 실행 파일 이름
TARGET = uds_fuzzer

# 소스 파일 목록
SRCS = attack_packet_sender.c \
       attack-functions/mode_f.c \
       attack-functions/mode_fs.c \
       attack-functions/dos.c \
       attack-functions/fuzzing.c \
       attack-functions/replay.c \
       attack-functions/suspension.c \
       attack-functions/msq.c \
       can_sender.c \
       nonblocking.c \
       payloads.c \
       main.c

# 객체 파일 목록 (.c를 .o로 변환)
OBJS = $(SRCS:.c=.o)
$(info OBJS=$(OBJS))

# 헤더 파일 목록
HEADERS = payloads.h all_headers.h attack-functions/attack_functions.h can_sender.h nonblocking.h attack-functions/*.h

# 기본 타겟
.PHONY: all clean

all: $(TARGET)

# 실행 파일을 생성하는 규칙
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# .c 파일을 .o 파일로 컴파일하는 규칙
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# 정리 규칙
clean:
	rm -f $(OBJS) $(TARGET)

# =====================================================================