# JSON 파일 저장

> 이 문서는 Sample을 예시로 설명하며 Order/ProductionQueueEntry에도 동일 원칙이
> 적용된다(단, ProductionQueueEntry는 orderId를 키로 사용 —
> [`docs/design/phase5-foundation.md`](../design/phase5-foundation.md) 참고).

## 개요

메모리 상의 시료(`Sample`) 목록을 JSON 파일로 영속화하는 기능. [nlohmann/json](https://github.com/nlohmann/json)을 사용해 직렬화하며, 애플리케이션을 다시 실행해도 데이터가 유지되도록 한다(요구사항 문서상 "데이터 영속성"의 정의).

## 동작

- `Sample`은 `NLOHMANN_DEFINE_TYPE_INTRUSIVE` 매크로로 자기 자신을 `nlohmann::json`으로 직렬화할 수 있다(`Model/Sample.h` 참고).
- 저장 시 메모리 상의 시료 목록(`std::vector<Sample>`)을 `nlohmann::json` 배열로 변환하고, `dump(indent)`로 들여쓰기된 텍스트로 만든다.
- 변환된 내용을 곧바로 원본 파일에 쓰지 않는다. 다음 순서로 원자적 쓰기를 수행한다:
  1. 원본 경로에 `.tmp` 등의 접미사를 붙인 임시 파일에 `std::ofstream`으로 내용을 쓴다.
  2. 임시 파일 쓰기가 정상 완료되면 `std::filesystem::rename`으로 임시 파일을 원본 경로로 교체한다.
  3. 임시 파일 쓰기 중 실패하면(`ofstream`이 열리지 않는 등) 예외를 던지고, 원본 파일은 그대로 보존한다.
- Create/Update/Delete 등 데이터를 변경하는 모든 연산은 완료 직후 저장을 호출해, 메모리와 파일 상태를 항상 일치시킨다.
- 저장이 실패하면(예외 발생) 호출한 CRUD 연산은 메모리 상의 변경을 저장 전 상태로 롤백해야 한다(각 CRUD 문서 참고).

## 관련 기능

- [json-parsing](json-parsing.md): 저장된 파일을 다시 읽어 메모리로 복원할 때 사용.
- [create](create.md), [update](update.md), [delete](delete.md): 데이터 변경 후 저장을 호출하는 지점.
