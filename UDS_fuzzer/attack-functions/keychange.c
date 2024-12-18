/*================================================================
keychange.c
This function gathers key changing seeds.
It also gets keys from the diagnostic device and writes up a dictionary if possible.
=================================================================*/

#include "all_headers.h"

// 링크드 리스트의 헤드 (전역 변수)
PayloadNode *mainListHead = NULL;

// 새로운 노드 생성 함수
PayloadNode* createNode(uint8_t *data, size_t length) {
    PayloadNode *newNode = (PayloadNode*)malloc(sizeof(PayloadNode));
    if (!newNode) {
        printf("메모리 할당 실패\n");
        exit(1);
    }
    memcpy(newNode->payload.data, data, length > 8 ? 8 : length); // 최대 8바이트
    newNode->payload.length = length;
    newNode->next = NULL;
    newNode->child = NULL;
    return newNode;
}

// 주 리스트에 노드 추가 함수
void appendNext(PayloadNode **head, PayloadNode *newNode) {
    if (*head == NULL) {
        *head = newNode;
        return;
    }
    PayloadNode *temp = *head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = newNode;
}

// 서브 리스트에 노드 추가 함수
void appendChild(PayloadNode *parent, PayloadNode *newNode) {
    if (parent->child == NULL) {
        parent->child = newNode;
        return;
    }
    PayloadNode *temp = parent->child;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = newNode;
}

// 페이로드 크기 순으로 정렬된 상태로 링크드 리스트에 노드 추가 함수
void insertSorted(PayloadNode **head, PayloadNode *newNode) {
    if (*head == NULL || newNode->payload.length < (*head)->payload.length) {
        newNode->next = *head;
        *head = newNode;
    } else {
        PayloadNode *current = *head;
        while (current->next != NULL && current->next->payload.length < newNode->payload.length) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }
}

// CAN 프레임 전송 함수 (플레이스홀더)
int sendCANFrame(uint32_t canid, uint8_t *data, size_t length) {
    // 실제 CAN 전송 코드로 대체 필요
    // 예: can_send(canid, data, length);
    printf("Sending CANID: 0x%X, Data: ", canid);
    for(size_t i = 0; i < length; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    return 0; // 성공적으로 전송했다고 가정
}

// CAN 프레임 수신 함수 (플레이스홀더)
// 반환값: 0 - 성공, -1 - 실패
int receiveCANFrame(uint32_t expected_canid, uint8_t *buffer, size_t *length) {
    // 실제 CAN 수신 코드로 대체 필요
    // 예: can_receive(&recv_canid, buffer, length);
    // 여기서는 예시로 긍정 응답을 가정
    uint32_t recv_canid = expected_canid; // 예시: 예상한 CANID와 동일
    if (recv_canid == expected_canid) {
        // 긍정 응답 페이로드 예시 (UDS 긍정 응답 형식)
        uint8_t response_payload[8] = {0x40, 0x27, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
        memcpy(buffer, response_payload, 8);
        *length = 8;
        printf("Received positive response for CANID: 0x%X\n", recv_canid);
        return 0;
    }
    return -1; // 응답 없음
}

// 메인 노드를 채우는 함수
void keychange_mainnode(uint32_t canid, uint8_t *payload, size_t payload_len, int iterations) {
    for(int i = 0; i < iterations; i++) {
        // 페이로드 전송
        if (sendCANFrame(canid, payload, payload_len) != 0) {
            printf("페이로드 전송 실패\n");
            continue;
        }

        uint32_t response_canid = canid + 0x008;

        // 응답 수신 대기 (여기서는 간단히 호출)
        uint8_t response_payload[8];
        size_t response_length;
        if (receiveCANFrame(response_canid, response_payload, &response_length) == 0) {
            // 긍정 응답인지 확인 (예시: 첫 바이트가 0x40인 경우 긍정 응답으로 간주)
            if (response_payload[0] == 0x40) {
                // 새로운 노드 생성
                PayloadNode *newNode = createNode(response_payload, response_length);
                // 링크드 리스트에 정렬된 상태로 추가
                insertSorted(&mainListHead, newNode);
                printf("응답 페이로드을 링크드 리스트에 추가: Length = %zu\n", response_length);
            } else {
                printf("부정 응답을 받았습니다.\n");
            }
        } else {
            printf("응답을 받지 못했습니다.\n");
        }

        // 5초 대기
        sleep(5);
    }
}

// 링크드 리스트 출력 함수 (선택 사항)
void printMainList() {
    PayloadNode *temp = mainListHead;
    printf("Main Linked List:\n");
    while (temp != NULL) {
        printf("Payload (Length: %zu): ", temp->payload.length);
        for(size_t i = 0; i < temp->payload.length; i++) {
            printf("%02X ", temp->payload.data[i]);
        }
        printf("\n");
        temp = temp->next;
    }
}

// 링크드 리스트 해제 함수
void freeMainList(PayloadNode *head) {
    PayloadNode *temp;
    while (head != NULL) {
        if (head->child != NULL) {
            freeMainList(head->child);
        }
        temp = head;
        head = head->next;
        free(temp);
    }
}

// 테스트용 main 함수 (필요시 제거하거나 수정)
int main() {
    // 예시 CANID 및 페이로드
    uint32_t example_canid = 0x100; // 전송할 CANID
    uint8_t example_payload[8] = {0x02, 0x27, 0x01}; // 전송할 페이로드 (길이 3바이트)
    // keychange_mainnode 함수 호출 (예: 5번 반복)
    keychange_mainnode(example_canid, example_payload, 3, 5);
    // 링크드 리스트 출력
    printMainList();
    // 메모리 해제
    freeMainList(mainListHead);
    return 0;
}