# Phase 4 설계: Delete 구현

관련 계획: [`docs/PLAN.md`](../PLAN.md#phase-4-delete-구현)
관련 요구사항: [`docs/feature/delete.md`](../feature/delete.md)

## 목표

특정 데이터를 안전하게 삭제하는 기능을 추가한다. 이 phase가 끝나면
Create/Read/Update/Delete 전체 메뉴가 갖춰진 완성된 콘솔 CRUD 애플리케이션이 된다.

## 리포지토리 확장

```cpp
// SampleRepository.h 에 추가
class SampleRepository {
public:
    // ... Phase 0/1/2/3에서 정의한 멤버 ...

    // 반환값: 성공 여부(대상 미존재 시 false)
    bool remove(int id);
};
```

- `remove()`는 `sampleList_`에서 해당 id를 제거하고 `save()`를 호출한다. 삭제
  확인 절차(사용자 확인)는 리포지토리가 아닌 `ConsoleApp`의 책임이다 — 리포지토리는
  "확정된 삭제 요청"만 처리한다.

## 콘솔 UI 확장

```cpp
// ConsoleApp.h
private:
    void handleDelete();   // 메뉴 "5. Delete" 핸들러
```

`handleDelete()` 흐름:

1. 삭제할 id를 입력받는다.
2. `repo_.findById(id)`로 대상을 조회한다. 없으면 안내 후 메뉴로 복귀한다.
3. 대상 정보를 출력하고 "정말 삭제하시겠습니까? (Y/N)" 확인을 받는다.
4. `Y`인 경우에만 `repo_.remove(id)`를 호출하고 결과를 출력한다. `N`이면 취소
   메시지를 출력하고 메뉴로 복귀한다(삭제 시도 없이).

## 메뉴 갱신 (최종)

```
1. Create
2. 전체 목록 보기
3. ID로 검색
4. Update
5. Delete
0. 종료
```

## 완료 기준 (실행 확인)

- 존재하는 id에 대해 확인 절차에서 `N`을 선택하면 데이터가 삭제되지 않음을
  확인한다.
- `Y`를 선택하면 데이터가 삭제되고, 전체 목록 보기/ID 검색으로 재조회 시 더 이상
  나타나지 않음을 확인한다.
- 존재하지 않는 id에 대해 Delete 시도 시 안내 메시지와 함께 메뉴로 정상 복귀함을
  확인한다.
- Create → Read → Update → Delete를 한 실행 흐름 안에서 순서대로 수행해도 전체
  메뉴가 정상 동작함을 확인한다(CRUD 전체 통합 확인).
