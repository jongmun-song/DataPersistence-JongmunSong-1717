# PLAN

`docs/PRE.md`와 `docs/feature/*.md`에 정의된 요구사항을 구현하기 위한 phase별 계획.
각 phase는 manager → developer → reviewer → tester 파이프라인(`.claude/agents/`)을
거쳐 완료하며, **phase가 끝날 때마다 빌드·실행 가능한 콘솔 애플리케이션**을
산출물로 남긴다. 다음 phase는 이전 phase가 남긴 실행 가능한 상태 위에 기능을
덧붙이는 방식으로 진행한다.

## Phase 0: 프로젝트 뼈대 + JSON 라이브러리 연동

- 목표: 콘솔 메뉴 루프의 뼈대를 세우고, [nlohmann/json](https://github.com/nlohmann/json)을
  프로젝트에 연동한다.
- 참고 문서: `docs/feature/json-parsing.md`, `docs/feature/json-file-storage.md`
- 산출물: `Model/Sample.h`(데이터 모델, 이미 정의됨)를 기반으로 빈 JSON 배열을 읽고
  쓰는 `SampleRepository` 계층.
- **실행 가능한 결과물**: 프로그램을 실행하면 메뉴(종료 옵션 포함)가 뜨고, JSON
  파일이 없으면 빈 목록으로 시작해 정상 종료된다. CRUD 메뉴 항목은 아직 없어도 된다.
- 완료 기준: `msbuild`로 빌드가 성공하고, 실행 시 크래시 없이 메뉴 → 종료 흐름이
  동작한다.

## Phase 1: Create 구현

- 목표: 새로운 시료(`Sample`) 데이터를 입력받아 JSON 파일에 저장한다.
- 참고 문서: `docs/feature/create.md`
- **실행 가능한 결과물**: 메뉴에 Create 항목이 추가되어, 실행 중 데이터를 입력하면
  JSON 파일에 저장되고 프로그램을 재시작해도 저장된 데이터가 유지된다.
- 완료 기준: 신규 데이터 입력, ID 자동 부여, 필수 항목 검증, 저장까지 실제 실행으로
  확인된다.

## Phase 2: Read 구현

- 목표: 전체 목록 조회와 ID 검색을 구현한다.
- 참고 문서: `docs/feature/read.md`
- **실행 가능한 결과물**: 메뉴에 Read 항목(전체 목록 보기, ID 검색)이 추가되어,
  Phase 1에서 Create로 저장한 데이터를 실제로 조회·검색할 수 있다.
- 완료 기준: 목록이 비어 있을 때, 검색 대상이 없을 때를 포함해 실제 실행으로
  확인된다.

## Phase 3: Update 구현

- 목표: 기존 데이터를 선택해 특정 필드를 수정한다.
- 참고 문서: `docs/feature/update.md`
- **실행 가능한 결과물**: 메뉴에 Update 항목이 추가되어, Read로 조회한 대상을
  선택해 필드를 수정하면 JSON 파일에 반영되고 재조회 시 변경 사항이 확인된다.
- 완료 기준: 대상 미존재 처리, 필드 검증, 저장 반영까지 실제 실행으로 확인된다.

## Phase 4: Delete 구현

- 목표: 특정 데이터를 안전하게 삭제한다.
- 참고 문서: `docs/feature/delete.md`
- **실행 가능한 결과물**: 메뉴에 Delete 항목이 추가되어, 이 시점에서 Create/Read/
  Update/Delete 전체 메뉴가 갖춰진 완성된 콘솔 CRUD 애플리케이션이 된다.
- 완료 기준: 삭제 전 확인 절차, 대상 미존재 처리, 저장 반영까지 실제 실행으로
  확인된다.

## Phase 5: 모델 선택 메뉴 + Order/ProductionQueueEntry 리포지토리 뼈대

- 목표: Phase 0과 동일 수준의 뼈대를 `Order`/`ProductionQueueEntry`에 갖추고,
  `Sample`/`Order`/`ProductionQueueEntry` 중 다룰 모델을 먼저 고르는
  `MainConsoleApp`의 모델 선택 메뉴를 도입한다.
- 참고 문서: `docs/design/phase5-foundation.md`
- **실행 가능한 결과물**: 실행하면 모델 선택 메뉴(1.Sample 2.Order
  3.ProductionQueueEntry 0.종료)가 뜨고, 1을 선택하면 기존 Sample CRUD 메뉴가
  그대로 동작하며 0으로 복귀한다. 2/3을 선택하면 아직 "뒤로가기"만 있는
  Order/ProductionQueueEntry 메뉴가 뜬다.
- 완료 기준: 빌드 성공, JSON 파일이 없어도 크래시 없이 모델 선택 메뉴가 표시되고,
  각 모델 메뉴 진입/복귀, 최상위 0 선택 시에만 프로그램이 종료됨을 실행으로
  확인한다.

## Phase 6: Order + ProductionQueueEntry — Create

- 목표: `Order`, `ProductionQueueEntry` 데이터를 입력받아 각각
  `orders.json`/`production_queue.json`에 저장한다.
- 참고 문서: `docs/design/phase6-create.md`
- **실행 가능한 결과물**: Order/ProductionQueueEntry 메뉴에 Create 항목이 추가되어,
  실행 중 데이터를 입력하면 각 JSON 파일에 저장되고 재시작해도 유지된다.
- 완료 기준: Order는 Sample과 동일한 방식(ID 자동 부여, 필수 항목 검증)으로,
  ProductionQueueEntry는 `orderId`를 자연키로 사용하는 방식(중복 orderId 거부,
  `orderId<=0` 거부)으로 생성이 실제 실행으로 확인된다.

## Phase 7: Order + ProductionQueueEntry — Read

- 목표: Order/ProductionQueueEntry 각각에 대해 전체 목록 조회와 단건 검색을
  구현한다.
- 참고 문서: `docs/design/phase7-read.md`
- **실행 가능한 결과물**: 각 메뉴에 전체 목록 보기/검색 항목이 추가되어, Phase 6에서
  Create로 저장한 데이터를 조회·검색할 수 있다. ProductionQueueEntry는 orderId로
  검색한다.
- 완료 기준: Phase 2와 동일한 관찰 항목(2건 이상 조회, 존재/미존재 검색, 빈 목록
  처리)이 Order/ProductionQueueEntry 각각에 대해 실제 실행으로 확인된다.

## Phase 8: Order + ProductionQueueEntry — Update

- 목표: 기존 Order/ProductionQueueEntry 데이터를 선택해 특정 필드를 수정한다.
- 참고 문서: `docs/design/phase8-update.md`
- **실행 가능한 결과물**: 각 메뉴에 Update 항목이 추가되어, 조회한 대상을 선택해
  필드를 수정하면 각 JSON 파일에 반영되고 재조회 시 변경 사항이 확인된다.
  ProductionQueueEntry는 orderId로 대상을 지정한다.
- 완료 기준: Phase 3과 동일한 관찰 항목(대상 미존재 처리, 필드 검증, 저장 반영)이
  Order/ProductionQueueEntry 각각에 대해 실제 실행으로 확인된다.

## Phase 9: Order + ProductionQueueEntry — Delete

- 목표: 특정 Order/ProductionQueueEntry 데이터를 안전하게 삭제한다.
- 참고 문서: `docs/design/phase9-delete.md`
- **실행 가능한 결과물**: 각 메뉴에 Delete 항목이 추가되어, 이 시점에서 Sample/
  Order/ProductionQueueEntry 세 모델 모두 Create/Read/Update/Delete 전체 메뉴를
  갖춘 완성된 콘솔 CRUD 애플리케이션이 된다.
- 완료 기준: Phase 4와 동일한 관찰 항목(삭제 전 확인 절차, 대상 미존재 처리, 저장
  반영)이 Order/ProductionQueueEntry 각각에 대해 실제 실행으로 확인되고, 최상위
  모델 선택 메뉴에서 세 모델을 오가며 정상 동작함을 확인한다.

## Phase 진행 방식

- 각 phase는 순서대로 진행하되, 이전 phase가 개발자(developer) → 리뷰어(reviewer) →
  테스터(tester) 검증을 모두 통과해 실행 가능한 상태로 확인된 뒤 다음 phase로
  넘어간다.
- phase 완료 기준(빌드 성공 + 실제 실행 확인)을 만족하지 못하면 해당 phase 내에서
  재작업하며, 다음 phase로 넘어가지 않는다.
- 지금 어느 phase까지 진행됐는지는 이 문서와 `docs/design/`의 대응 phase 문서,
  그리고 실제로 빌드·실행해본 결과로 확인한다.
