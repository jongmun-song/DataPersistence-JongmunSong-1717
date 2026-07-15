# Phase 8 설계: Order + ProductionQueueEntry — Update

관련 계획: [`docs/PLAN.md`](../PLAN.md#phase-8-order--productionqueueentry--update)
관련 요구사항: [`docs/feature/update.md`](../feature/update.md)

## 목표

Read로 조회한 Order/ProductionQueueEntry 대상을 선택해 특정 필드를 수정하고,
각 JSON 파일에 반영한다. ProductionQueueEntry는 자연키인 `orderId`로 대상을
지정한다.

## 리포지토리 확장

### OrderRepository

```cpp
// OrderRepository.h 에 추가
class OrderRepository {
public:
    // ... Phase 5/6/7에서 정의한 멤버 ...

    // 반환값: 성공 여부(대상 미존재 시 false)
    bool update(int id, const std::function<void(Model::Order&)>& mutator);
};
```

### ProductionQueueEntryRepository

```cpp
// ProductionQueueEntryRepository.h 에 추가
class ProductionQueueEntryRepository {
public:
    // ... Phase 5/6/7에서 정의한 멤버 ...

    // 반환값: 성공 여부(대상 미존재 시 false)
    bool update(int orderId, const std::function<void(Model::ProductionQueueEntry&)>& mutator);
};
```

- 두 `update()` 모두 Sample의 `update()`(Phase 3)와 동일한 방식으로 동작한다:
  대상을 찾아 참조를 얻고, `mutator`로 필드를 변경한 뒤 `save()`를 호출한다.
  필드 단위 검증(Create와 동일한 규칙)은 `ConsoleApp` 계층에서 새 값을 입력받는
  시점에 수행하고, 검증을 통과한 값만 `mutator`에 담아 전달한다. 저장 실패 시
  변경 전 상태로 롤백하고 예외를 상위로 전달한다.

## 콘솔 UI 확장

### OrderConsoleApp

```cpp
// OrderConsoleApp.h
private:
    void handleUpdate();   // 메뉴 "4. Update" 핸들러
```

`handleUpdate()` 흐름은 Sample의 `handleUpdate()`(Phase 3)와 동일하다: 수정할
id를 입력받아 `repo_.findById(id)`로 존재를 먼저 확인하고, 없으면 안내 후 메뉴로
복귀한다. 수정할 필드를 다음 중에서 선택받는다.

```
1) customerName
2) orderedQuantity
3) state
```

(`sampleId` 수정 허용 여부는 구현 시 developer가 결정한다.) 새 값을 입력받아
Create와 동일한 형식/필수 항목 검증을 수행한 뒤 `repo_.update(id, ...)`를
호출하고 결과를 출력한다.

### ProductionQueueEntryConsoleApp

```cpp
// ProductionQueueEntryConsoleApp.h
private:
    void handleUpdate();   // 메뉴 "4. Update" 핸들러
```

`handleUpdate()` 흐름: 수정할 `orderId`를 입력받아
`repo_.findByOrderId(orderId)`로 존재를 먼저 확인하고, 없으면 안내 후 메뉴로
복귀한다. 수정할 필드를 다음 중에서 선택받는다(`orderId`/`sampleId`는 수정
대상에서 제외한다).

```
1) shortageQuantity
2) actualProductionQuantity
3) totalProductionTime
4) state
```

새 값을 입력받아 형식 검증을 수행한 뒤 `repo_.update(orderId, ...)`를 호출하고
결과를 출력한다.

## 메뉴 갱신 (양쪽 공통)

```
1. Create
2. 전체 목록 보기
3. ID(orderId)로 검색
4. Update
0. 뒤로가기
```

## 완료 기준 (실행 확인)

Phase 3과 동일한 관찰 항목을 Order/ProductionQueueEntry 각각에 대해 확인한다.

- 존재하는 대상에 대해 필드 하나를 수정한 뒤, 전체 목록 보기/검색으로 재조회해
  변경 사항이 반영됨을 확인한다.
- 존재하지 않는 대상(Order는 id, ProductionQueueEntry는 orderId)에 대해 Update
  시도 시 안내 메시지와 함께 메뉴로 정상 복귀함을 확인한다.
- 잘못된 형식의 새 값 입력 시 저장되지 않고 재입력/취소로 이어짐을 확인한다.
