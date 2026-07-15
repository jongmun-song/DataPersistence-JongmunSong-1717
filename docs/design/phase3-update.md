# Phase 3 설계: Update 구현

관련 계획: [`docs/PLAN.md`](../PLAN.md#phase-3-update-구현)
관련 요구사항: [`docs/feature/update.md`](../feature/update.md)

## 목표

Read로 조회한 대상을 선택해 특정 필드를 수정하고, JSON 파일에 반영한다. 수정
후 재조회 시 변경 사항이 확인되어야 한다.

## 리포지토리 확장

```cpp
// SampleRepository.h 에 추가
class SampleRepository {
public:
    // ... Phase 0/1/2에서 정의한 멤버 ...

    // 반환값: 성공 여부(대상 미존재 시 false)
    bool update(int id, const std::function<void(Sample&)>& mutator);
};
```

- `update()`는 `findById`와 동일한 방식으로 대상을 찾아 참조를 얻고, `mutator`로
  필드를 변경한 뒤 `save()`를 호출한다.
- 필드 단위 검증(Create와 동일한 규칙)은 `ConsoleApp`에서 새 값을 입력받는
  시점에 수행하고, 검증을 통과한 값만 `mutator`에 담아 전달한다.

## 콘솔 UI 확장

```cpp
// ConsoleApp.h
private:
    void handleUpdate();   // 메뉴 "4. Update" 핸들러
```

`handleUpdate()` 흐름:

1. 수정할 id를 입력받는다.
2. `repo_.findById(id)`로 대상 존재 여부를 먼저 확인하고, 없으면 안내 후 메뉴로
   복귀한다(리포지토리의 `update()`를 호출하기 전에 조회해, 사용자에게 대상 정보를
   먼저 보여줄 수 있게 한다).
3. 수정할 필드를 선택받는다(예: 1) 이름 2) 평균 생산시간 3) 수율 4) 재고 수량).
4. 새 값을 입력받아 Create와 동일한 형식/필수 항목 검증을 수행한다.
5. `repo_.update(id, ...)` 호출 후 결과를 출력한다.

## 메뉴 갱신

```
1. Create
2. 전체 목록 보기
3. ID로 검색
4. Update
0. 종료
```

## 완료 기준 (실행 확인)

- 존재하는 id에 대해 필드 하나를 수정한 뒤, 전체 목록 보기/ID 검색으로 재조회해
  변경 사항이 반영됨을 확인한다.
- 존재하지 않는 id에 대해 Update 시도 시 안내 메시지와 함께 메뉴로 정상 복귀함을
  확인한다.
- 잘못된 형식의 새 값 입력 시 저장되지 않고 재입력/취소로 이어짐을 확인한다.
