# Phase 1 설계: Create 구현

관련 계획: [`docs/PLAN.md`](../PLAN.md#phase-1-create-구현)
관련 요구사항: [`docs/feature/create.md`](../feature/create.md)

## 목표

Phase 0에서 만든 뼈대 위에 Create 기능을 추가한다. 실행 중 입력한 데이터가
JSON 파일에 저장되고, 프로그램을 재시작해도 유지되어야 한다.

## 리포지토리 확장

```cpp
// SampleRepository.h 에 추가
class SampleRepository {
public:
    // ... Phase 0에서 정의한 멤버 ...

    // 반환값: 생성된 Sample(부여된 id 포함)
    Sample create(const Sample& input);

private:
    int nextId() const;   // sampleList_ 내 최댓값 id + 1
};
```

- `create()`는 다음 순서로 동작한다:
  1. `nextId()`로 고유 id를 부여한다(입력값의 id는 무시한다 — 리포지토리가 id를
     부여하는 방식을 기본으로 한다).
  2. 필수 필드(`name` 등)가 비어 있으면 `std::invalid_argument`를 던진다.
  3. `sampleList_`에 추가한다.
  4. `save()`를 호출해 JSON 파일에 반영한다.

## 콘솔 UI 확장

```cpp
// ConsoleApp.h
private:
    void handleCreate();   // 메뉴 "1. Create" 핸들러
```

`handleCreate()` 흐름:

1. `name`, `averageProductionTimePerUnit`, `yieldRatio`, `stockQuantity` 등 필드를
   순서대로 입력받는다(`std::getline`/숫자 변환).
2. 빈 값 등 형식 오류가 있으면 오류 메시지를 출력하고 해당 필드 재입력을 요구하거나
   Create 자체를 취소한다(둘 중 하나로 확정 — 구현 시 developer가 선택하되
   `docs/feature/create.md`의 "형식/필수 항목 검증" 요건을 충족해야 한다).
3. `repo_.create(input)` 호출 후 결과(부여된 id 포함)를 콘솔에 출력한다.
4. 저장 중 예외가 발생하면 오류 메시지를 출력하고 메뉴로 복귀한다(프로그램이
   죽지 않아야 한다).

## 메뉴 갱신

```
1. Create
0. 종료
```

## 완료 기준 (실행 확인)

- 실행 후 Create로 데이터 1건 입력 → 프로그램 종료 → 재실행 시 JSON 파일에 해당
  데이터가 남아 있음을 (Phase 0 수준에서는) 파일 내용 확인으로 검증한다. Phase 2
  이후에는 Read 메뉴로 직접 조회해 검증한다.
- 필수 항목 누락 시 저장되지 않고 재입력/취소로 이어짐을 확인한다.
- 여러 건을 연속 생성했을 때 부여된 id가 항상 유일함을 확인한다.
