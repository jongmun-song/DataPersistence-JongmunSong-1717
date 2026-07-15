# Phase 2 설계: Read 구현

관련 계획: [`docs/PLAN.md`](../PLAN.md#phase-2-read-구현)
관련 요구사항: [`docs/feature/read.md`](../feature/read.md)

## 목표

전체 목록 보기와 ID 검색을 추가한다. Phase 1(Create)로 저장한 데이터를
실제로 조회·검색할 수 있어야 한다.

## 리포지토리 확장

```cpp
// SampleRepository.h 에 추가
class SampleRepository {
public:
    // ... Phase 0/1에서 정의한 멤버 ...

    // 이미 Phase 0에 존재: const std::vector<Sample>& all() const;

    std::optional<Sample> findById(int id) const;
};
```

- `all()`은 Phase 0에서 이미 정의했으므로 재사용한다(전체 목록 보기에 그대로 사용).
- `findById()`는 `sampleList_`를 선형 탐색해 일치하는 항목을 반환하고, 없으면
  `std::nullopt`을 반환한다(예외를 던지지 않는다 — "찾을 수 없음"은 정상 흐름).

## 콘솔 UI 확장

```cpp
// ConsoleApp.h
private:
    void handleReadAll();     // 메뉴 "2. 전체 목록 보기" 핸들러
    void handleReadById();    // 메뉴 "3. ID로 검색" 핸들러
```

`handleReadAll()` 흐름:

1. `repo_.all()`이 비어 있으면 "등록된 시료가 없습니다" 등을 출력한다.
2. 비어 있지 않으면 각 시료를 한 줄씩 표 형태로 출력한다(이름/평균 생산시간/수율/
   재고 수량 포함).

`handleReadById()` 흐름:

1. 검색할 id를 입력받는다(숫자 변환 실패 시 오류 메시지 후 메뉴로 복귀).
2. `repo_.findById(id)` 호출.
3. 값이 있으면 상세 정보 출력, 없으면 "해당 ID의 시료를 찾을 수 없습니다" 출력.

## 메뉴 갱신

```
1. Create
2. 전체 목록 보기
3. ID로 검색
0. 종료
```

## 완료 기준 (실행 확인)

- Create로 2건 이상 등록 후 전체 목록 보기로 모두 조회됨을 확인한다.
- 존재하는 id로 검색 시 해당 데이터가, 존재하지 않는 id로 검색 시 "찾을 수 없음"
  메시지가 출력됨을 확인한다.
- 데이터가 하나도 없는 상태에서 전체 목록 보기를 실행해도 크래시 없이 빈 목록
  메시지가 출력됨을 확인한다.
