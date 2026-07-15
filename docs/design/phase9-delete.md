# Phase 9 설계: Order + ProductionQueueEntry — Delete

관련 계획: [`docs/PLAN.md`](../PLAN.md#phase-9-order--productionqueueentry--delete)
관련 요구사항: [`docs/feature/delete.md`](../feature/delete.md)

## 목표

Order/ProductionQueueEntry 각각의 특정 데이터를 안전하게 삭제하는 기능을
추가한다. 이 phase가 끝나면 Sample/Order/ProductionQueueEntry 세 모델 모두
Create/Read/Update/Delete 전체 메뉴가 갖춰진 완성된 콘솔 CRUD 애플리케이션이
된다.

## 리포지토리 확장

### OrderRepository

```cpp
// OrderRepository.h 에 추가
class OrderRepository {
public:
    // ... Phase 5/6/7/8에서 정의한 멤버 ...

    // 반환값: 성공 여부(대상 미존재 시 false)
    bool remove(int id);
};
```

### ProductionQueueEntryRepository

```cpp
// ProductionQueueEntryRepository.h 에 추가
class ProductionQueueEntryRepository {
public:
    // ... Phase 5/6/7/8에서 정의한 멤버 ...

    // 반환값: 성공 여부(대상 미존재 시 false)
    bool remove(int orderId);
};
```

- 두 `remove()` 모두 Sample의 `remove()`(Phase 4)와 동일하게 동작한다: 해당
  키를 목록에서 제거하고 `save()`를 호출한다. 삭제 확인 절차(사용자 확인)는
  리포지토리가 아닌 각 `ConsoleApp`의 책임이다 — 리포지토리는 "확정된 삭제
  요청"만 처리한다.

## 콘솔 UI 확장

### OrderConsoleApp / ProductionQueueEntryConsoleApp 공통

```cpp
private:
    void handleDelete();   // 메뉴 "5. Delete" 핸들러
```

`handleDelete()` 흐름은 Sample의 `handleDelete()`(Phase 4)와 동일하다.

1. 삭제할 대상의 키(Order는 id, ProductionQueueEntry는 orderId)를 입력받는다.
2. `repo_.findById(id)`/`repo_.findByOrderId(orderId)`로 대상을 조회한다. 없으면
   안내 후 메뉴로 복귀한다.
3. 대상 정보를 출력하고 "정말 삭제하시겠습니까? (Y/N)" 확인을 받는다.
4. `Y`인 경우에만 `repo_.remove(...)`를 호출하고 결과를 출력한다. `N`이면 취소
   메시지를 출력하고 메뉴로 복귀한다(삭제 시도 없이).

## 메뉴 갱신 (최종, 양쪽 공통)

```
1. Create
2. 전체 목록 보기
3. ID(orderId)로 검색
4. Update
5. Delete
0. 뒤로가기
```

## 완료 기준 (실행 확인)

Phase 4와 동일한 관찰 항목을 Order/ProductionQueueEntry 각각에 대해 확인한다.

- 존재하는 대상에 대해 확인 절차에서 `N`을 선택하면 데이터가 삭제되지 않음을
  확인한다.
- `Y`를 선택하면 데이터가 삭제되고, 전체 목록 보기/검색으로 재조회 시 더 이상
  나타나지 않음을 확인한다.
- 존재하지 않는 대상에 대해 Delete 시도 시 안내 메시지와 함께 메뉴로 정상
  복귀함을 확인한다.
- Create → Read → Update → Delete를 한 실행 흐름 안에서 순서대로 수행해도 전체
  메뉴가 정상 동작함을 확인한다(CRUD 전체 통합 확인).
- 최상위 모델 선택 메뉴에서 Sample/Order/ProductionQueueEntry 세 모델을 오가며
  각 CRUD가 정상 동작함을 확인한다.
