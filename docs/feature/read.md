# Read

> 이 문서는 Sample을 예시로 설명하며 Order/ProductionQueueEntry에도 동일 원칙이
> 적용된다(단, ProductionQueueEntry는 orderId를 키로 사용 —
> [`docs/design/phase5-foundation.md`](../design/phase5-foundation.md) 참고).

## 개요

저장된 시료 데이터를 조회하는 기능. 전체 목록 보기와 특정 ID 검색을 지원한다.

## 동작

### 전체 목록 보기

- [json-parsing](json-parsing.md)으로 로드된 시료 목록 전체를 콘솔에 출력한다.
- 데이터가 없을 경우 빈 목록임을 알린다(예외를 던지지 않는다).

### 특정 ID 검색

- 사용자로부터 검색할 ID를 입력받는다.
- 시료 목록에서 해당 ID와 일치하는 데이터를 선형 탐색해 반환한다.
- 일치하는 데이터가 없을 경우 `std::optional<Sample>`의 빈 값(`std::nullopt`)으로 결과를 알린다(예외를 던지지 않는다) — [update](update.md), [delete](delete.md)가 동일한 방식으로 대상 존재 여부를 판단한다.

## 관련 기능

- [json-parsing](json-parsing.md): 조회 대상 데이터를 파일에서 불러올 때 사용.
- [update](update.md), [delete](delete.md): 검색된 항목을 대상으로 후속 작업을 수행할 때 재사용.
