# Phase 6 설계: Order + ProductionQueueEntry — Create

관련 계획: [`docs/PLAN.md`](../PLAN.md#phase-6-order--productionqueueentry--create)
관련 요구사항: [`docs/feature/create.md`](../feature/create.md)

## 목표

Phase 5에서 만든 Order/ProductionQueueEntry 뼈대 위에 Create 기능을 추가한다.
Order는 Sample과 동일하게 id를 자동 부여하고, ProductionQueueEntry는
`orderId`를 자연키로 사용해 입력값을 그대로 받는다(자세한 배경은
[`docs/design/phase5-foundation.md`](phase5-foundation.md)의 "ProductionQueueEntry
키 설계" 참고).

## 리포지토리 확장

### OrderRepository

```cpp
// OrderRepository.h 에 추가
class OrderRepository {
public:
    // ... Phase 5에서 정의한 멤버 ...

    // 반환값: 생성된 Model::Order(부여된 id 포함)
    Model::Order create(const Model::Order& input);

private:
    int nextId() const;   // orderList_ 내 최댓값 id + 1
};
```

- `create()`는 Sample과 동일한 순서로 동작한다:
  1. `nextId()`로 고유 id를 부여한다(입력값의 id는 무시).
  2. `customerName`이 비어 있으면 `std::invalid_argument`.
  3. `orderedQuantity <= 0`이면 `std::invalid_argument`.
  4. `state`는 기본값 `OrderState::RESERVED`로 설정한다(입력받지 않는 한).
  5. `orderList_`에 추가하고 `save()`를 호출한다. 저장 실패 시 추가한 항목을
     제거해 롤백하고 예외를 상위로 전달한다.

### ProductionQueueEntryRepository

```cpp
// ProductionQueueEntryRepository.h 에 추가
class ProductionQueueEntryRepository {
public:
    // ... Phase 5에서 정의한 멤버 ...

    // 반환값: 생성된 Model::ProductionQueueEntry(orderId는 입력값 그대로)
    Model::ProductionQueueEntry create(const Model::ProductionQueueEntry& input);

private:
    bool exists(int orderId) const;
};
```

- `create()`는 다음 순서로 동작한다:
  1. `input.orderId <= 0`이면 `std::invalid_argument`.
  2. `exists(input.orderId)`가 true이면 `std::invalid_argument`("이미 존재하는
     orderId입니다").
  3. `orderId`는 입력값을 그대로 사용한다(자동 부여하지 않는다).
  4. `state`는 기본값 `ProductionState::WAITING`으로 설정한다.
  5. `entryList_`에 추가하고 `save()`를 호출한다. 저장 실패 시 추가한 항목을
     제거해 롤백하고 예외를 상위로 전달한다.

## 콘솔 UI 확장

### OrderConsoleApp

```cpp
// OrderConsoleApp.h
private:
    void handleCreate();   // 메뉴 "1. Create" 핸들러
```

`handleCreate()` 흐름은 Sample의 `handleCreate()`(Phase 1)와 동일하다: `sampleId`,
`customerName`, `orderedQuantity`를 순서대로 입력받고(`state`는 번호 선택 메뉴로
RESERVED/CONFIRMED/PRODUCING/RELEASE/REJECTED 중 선택받거나 기본값 RESERVED로
둔다), 형식 오류 시 재입력 또는 취소로 이어지며, `repo_.create(input)` 결과(부여된
id 포함)를 출력한다. 저장 중 예외 발생 시 오류 메시지를 출력하고 메뉴로 복귀한다.

### ProductionQueueEntryConsoleApp

```cpp
// ProductionQueueEntryConsoleApp.h
private:
    void handleCreate();   // 메뉴 "1. Create" 핸들러
```

`handleCreate()` 흐름: `orderId`, `sampleId`, `orderedQuantity`, `shortageQuantity`,
`actualProductionQuantity`, `totalProductionTime`을 순서대로 입력받고(`state`는
번호 선택 메뉴로 WAITING/PRODUCING/CONFIRMED 중 선택받거나 기본값 WAITING으로
둔다), `repo_.create(input)` 호출 후 결과를 출력한다. `orderId <= 0` 또는 중복
`orderId`로 인한 `std::invalid_argument` 발생 시 오류 메시지를 출력하고
재입력/취소로 이어진다.

## 메뉴 갱신 (양쪽 공통)

```
1. Create
0. 뒤로가기
```

## 완료 기준 (실행 확인)

- Order: 2건 연속 생성 시 부여된 id가 항상 유일함을 확인한다. `customerName` 누락
  또는 `orderedQuantity <= 0`으로 생성 시도 시 재입력/취소로 이어짐을 확인한다.
- ProductionQueueEntry: 유효한 `orderId`로 1건 생성이 성공함을 확인한다. 동일
  `orderId`로 재생성을 시도하면 거부됨을, `orderId <= 0`으로 생성을 시도하면
  거부됨을 확인한다.
- 프로그램을 재실행해도 `orders.json`/`production_queue.json`에 저장한 데이터가
  유지됨을 확인한다.
