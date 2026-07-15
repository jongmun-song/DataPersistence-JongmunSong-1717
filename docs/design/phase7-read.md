# Phase 7 설계: Order + ProductionQueueEntry — Read

관련 계획: [`docs/PLAN.md`](../PLAN.md#phase-7-order--productionqueueentry--read)
관련 요구사항: [`docs/feature/read.md`](../feature/read.md)

## 목표

전체 목록 보기와 단건 검색을 Order/ProductionQueueEntry 각각에 추가한다.
Phase 6(Create)으로 저장한 데이터를 실제로 조회·검색할 수 있어야 한다.
ProductionQueueEntry는 자연키인 `orderId`로 검색한다.

## 리포지토리 확장

### OrderRepository

```cpp
// OrderRepository.h 에 추가
class OrderRepository {
public:
    // ... Phase 5/6에서 정의한 멤버 ...

    // 이미 Phase 5에 존재: const std::vector<Model::Order>& all() const;

    std::optional<Model::Order> findById(int id) const;
};
```

- `findById()`는 Sample의 `findById()`(Phase 2)와 동일하게 선형 탐색해 일치하는
  항목을 반환하고, 없으면 `std::nullopt`을 반환한다(예외를 던지지 않는다).

### ProductionQueueEntryRepository

```cpp
// ProductionQueueEntryRepository.h 에 추가
class ProductionQueueEntryRepository {
public:
    // ... Phase 5/6에서 정의한 멤버 ...

    // 이미 Phase 5에 존재: const std::vector<Model::ProductionQueueEntry>& all() const;

    std::optional<Model::ProductionQueueEntry> findByOrderId(int orderId) const;
};
```

- `findByOrderId()`는 `entryList_`를 선형 탐색해 `orderId`가 일치하는 항목을
  반환하고, 없으면 `std::nullopt`을 반환한다(예외를 던지지 않는다).

## 콘솔 UI 확장

### OrderConsoleApp

```cpp
// OrderConsoleApp.h
private:
    void handleReadAll();     // 메뉴 "2. 전체 목록 보기" 핸들러
    void handleReadById();    // 메뉴 "3. ID로 검색" 핸들러
```

Sample의 `handleReadAll()`/`handleReadById()`(Phase 2)와 동일한 흐름: 목록이
비어 있으면 안내 메시지를 출력하고, 비어 있지 않으면 각 주문을 한 줄씩 출력한다
(고객명/주문수량/상태 포함). 검색은 id를 입력받아 `repo_.findById(id)` 결과를
출력한다.

### ProductionQueueEntryConsoleApp

```cpp
// ProductionQueueEntryConsoleApp.h
private:
    void handleReadAll();             // 메뉴 "2. 전체 목록 보기" 핸들러
    void handleReadByOrderId();       // 메뉴 "3. orderId로 검색" 핸들러
```

`handleReadAll()`은 Order와 동일한 흐름이다. `handleReadByOrderId()`는 검색할
`orderId`를 입력받아 `repo_.findByOrderId(orderId)` 결과를 출력한다(없으면
"해당 orderId의 생산 큐 엔트리를 찾을 수 없습니다" 안내).

## 메뉴 갱신

### OrderConsoleApp

```
1. Create
2. 전체 목록 보기
3. ID로 검색
0. 뒤로가기
```

### ProductionQueueEntryConsoleApp

```
1. Create
2. 전체 목록 보기
3. orderId로 검색
0. 뒤로가기
```

## 완료 기준 (실행 확인)

Phase 2와 동일한 관찰 항목을 Order/ProductionQueueEntry 각각에 대해 확인한다.

- Create로 2건 이상 등록 후 전체 목록 보기로 모두 조회됨을 확인한다.
- 존재하는 키(Order는 id, ProductionQueueEntry는 orderId)로 검색 시 해당 데이터가,
  존재하지 않는 키로 검색 시 "찾을 수 없음" 메시지가 출력됨을 확인한다.
- 데이터가 하나도 없는 상태에서 전체 목록 보기를 실행해도 크래시 없이 빈 목록
  메시지가 출력됨을 확인한다.
