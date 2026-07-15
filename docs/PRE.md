# PRE — DataPersistence

## 1. 배경

`SSemiProductManager` 과제는 "반도체 시료 생산주문관리 시스템"(`SampleOrderSystem`)을 구현하기 전, 4개의 PoC(`ConsoleMVC`, `DataPersistence`, `DataMonitor`, `DummyDataGenerator`)로 핵심 기능을 검증하도록 요구한다(`../ref/requirements.pdf` Chapter 3, [미션1]).

`DataPersistence`는 그중 **"데이터 영속성 처리"** PoC로, "팀별로 선택한 방식(파일, JSON, DB 등)으로 데이터를 저장·불러오는 구조 구현, CRUD 포함"을 목표로 한다. 저장소 역할 구분과 개발 방식은 `../CLAUDE.md`에 정의되어 있으며, 본 문서는 그 내용을 바탕으로 프로젝트 전체 요구사항을 정리한다.

## 2. 목적

콘솔 애플리케이션을 위한 **JSON 파일 기반 CRUD 영속성 메커니즘**을 PoC로 검증한다. 애플리케이션을 다시 실행해도 데이터가 유지되어야 하며(요구사항 문서상 "데이터 영속성"의 정의), 참고 구현([jongmun-song/JSON_Console](https://github.com/jongmun-song/JSON_Console))과 동일한 구조로 하나의 구체적인 엔티티에 대한 리포지토리를 직접 구현함으로써 메커니즘 자체의 동작을 검증하는 것이 이 PoC의 목적이다. 대상 엔티티는 `ConsoleMVC/Tests/Example`의 PoC 검증용 도메인 모델(`Sample`, `Order`, `ProductionQueueEntry`)을 참고하여 `Model/`에 정의한 실제 데이터 모델이다.

## 3. 범위

### 포함 (In Scope)
- JSON 파일을 저장 매체로 하는 영속화 계층(파싱/저장)
- `Model/`에 정의된 `Sample`, `Order`, `ProductionQueueEntry` 데이터 모델
- `Sample`에 대한 CRUD 리포지토리(`SampleRepository`) — Phase 0~4로 시연
- 저장 실패 시 데이터가 훼손되지 않도록 하는 안전한 파일 쓰기(임시 파일 → rename)
- ID 자동 부여, 존재하지 않는 대상에 대한 예외 없는 실패 처리 등 리포지토리 공통 규칙
- `docs/PLAN.md`에 정의된 phase(0~4) 순서에 따른 단계별 개발 및 각 phase의 실행 가능한 산출물

### 제외 (Out of Scope)
- `Sample`/`Order`/`ProductionQueueEntry`의 비즈니스 규칙(재고 증감 가드, 상태 전이 검증, 생산량/시간 계산 등) — `Model/`의 데이터 모델은 순수 구조체이며, 이러한 규칙은 이를 사용하는 프로젝트(`SampleOrderSystem`)의 책임이다.
- Model/View/Controller 골격(`ConsoleMVC` 저장소 담당)
- 실시간 데이터 모니터링 도구(`DataMonitor` 저장소 담당)
- 더미 데이터 생성(`DummyDataGenerator` 저장소 담당)

## 4. 참고할 소비 시나리오

| 저장소 | 이 PoC를 참고하는 방식 |
|---|---|
| `SampleOrderSystem` | `SampleRepository`와 동일한 구조로 `OrderRepository`(및 필요 시 `ProductionQueueEntryRepository`)를 직접 작성 |
| `DataMonitor` | 동일한 JSON 파일 읽기 방식(Read 경로)을 참고해 실시간 조회 도구를 구현 |
| `DummyDataGenerator` | 동일한 Create 경로를 참고해 더미 데이터 삽입 도구를 구현 |

## 5. 기술 스택

- 언어/표준: C++20 (`stdcpp20`)
- 빌드: Visual Studio (`DataPersistence.slnx`, `DataPersistence.vcxproj`), MSBuild, Win32 `Application`(콘솔 서브시스템), x86/x64, Debug/Release
- JSON 파싱/직렬화: [nlohmann/json](https://github.com/nlohmann/json) (헤더 온리, `external/nlohmann/json.hpp`로 저장소에 포함, `AdditionalIncludeDirectories`에 `external/` 등록됨)
- CMake 미사용, googletest가 `packages.config`로 등록되어 있으나 테스트 프로젝트/러너 구성은 아직 없음(추가 시 함께 정비)

## 6. 데이터 모델

`Model/`에 정의된 세 모델은 `ConsoleMVC/Example/Model/`의 대응 타입을 참고했으나, ConsoleMVC의 핵심 타입(`IEntity`, `QuantityGuard`, `StatefulModel`)에는 의존하지 않는 순수 데이터 구조체다.

| 모델 | 필드 | 참고 |
|---|---|---|
| `Sample` | `id`, `name`, `averageProductionTimePerUnit`, `yieldRatio`, `stockQuantity` | `Example/Model/Sample.h` |
| `Order` | `id`, `sampleId`, `customerName`, `orderedQuantity`, `state`(`OrderState`) | `Example/Model/Order.h` |
| `ProductionQueueEntry` | `orderId`, `sampleId`, `orderedQuantity`, `shortageQuantity`, `actualProductionQuantity`, `totalProductionTime`, `state`(`ProductionState`) | `Example/Model/ProductionQueueEntry.h` |

`OrderState`(RESERVED/CONFIRMED/PRODUCING/RELEASE/REJECTED)와 `ProductionState`(WAITING/PRODUCING/CONFIRMED)는 각각 `NLOHMANN_JSON_SERIALIZE_ENUM`으로 문자열 직렬화된다. `ProductionState`는 생산 큐에 막 들어와 아직 생산 라인이 처리하지 않은 항목을 나타내는 `WAITING`을 기본값으로 한다(요구사항 문서의 "대기 주문 확인" 참고).

## 7. 핵심 설계 원칙

1. **단계별 실행 가능성** — `docs/PLAN.md`의 phase 순서(0: 뼈대 → 1: Create → 2: Read → 3: Update → 4: Delete)를 따르며, 각 phase는 빌드·실행 가능한 콘솔 애플리케이션을 산출물로 남긴다.
2. **계층 분리** — 데이터 모델(직렬화 책임, `Model/`) / JSON 파일 입출력을 담당하는 리포지토리 계층 / 콘솔 UI를 별도 translation unit으로 유지한다(자세한 내용은 `docs/feature/json-parsing.md`, `docs/feature/json-file-storage.md` 참고).
3. **데이터 안전성** — 저장 도중 실패해도 기존 파일이 훼손되지 않아야 하며(임시 파일 작성 후 rename), 메모리 상태는 실패 시 변경 전으로 롤백한다.
4. **예측 가능한 오류 처리** — 존재하지 않는 ID 조회/수정/삭제는 예외가 아닌 실패 반환(`std::optional`/`bool`)으로, 입력 검증 실패나 저장 실패는 예외로 구분해 전달한다.

## 8. CRUD 기능 요구사항

- **Create**: 새로운 시료(`Sample`) 데이터를 입력받아 고유 ID를 자동 부여하고 JSON 파일에 저장한다. → [상세](feature/create.md)
- **Read**: 전체 목록 조회 및 특정 ID로 단건 검색을 제공한다. → [상세](feature/read.md)
- **Update**: 기존 시료 데이터를 찾아 특정 필드를 수정하고 파일에 반영한다. → [상세](feature/update.md)
- **Delete**: 특정 시료 데이터를 확인 절차를 거쳐 안전하게 삭제하고 파일에 반영한다. → [상세](feature/delete.md)
- **JSON 라이브러리 연동**: CRUD 전 계층이 공통으로 의존하는 파싱/저장 방식.
  - [JSON 파싱](feature/json-parsing.md)
  - [JSON 파일 저장](feature/json-file-storage.md)

## 9. 개발 단계 (Phase)

이 PoC는 참고 구현의 `docs/PLAN.md`/`docs/design/phase*.md` 구성을 그대로 가져와, 아래 순서로 진행 상황을 확인한다. 자세한 phase별 목표·완료 기준은 `docs/PLAN.md`, 각 phase의 구체적인 설계는 `docs/design/`를 참고한다.

| Phase | 목표 | 설계 문서 |
|---|---|---|
| 0 | 프로젝트 뼈대 + JSON 라이브러리 연동 | `docs/design/phase0-foundation.md` |
| 1 | Create 구현 | `docs/design/phase1-create.md` |
| 2 | Read 구현 | `docs/design/phase2-read.md` |
| 3 | Update 구현 | `docs/design/phase3-update.md` |
| 4 | Delete 구현 | `docs/design/phase4-delete.md` |

## 10. 참고 구현

유사한 목적(회원 정보 CRUD, DB 없이 JSON 파일 영속화)의 PoC가 [jongmun-song/JSON_Console](https://github.com/jongmun-song/JSON_Console)에 구현되어 있다. 이 저장소는 대상 엔티티를 `Member`에서 `Sample`로 바꿔, `Sample`/`SampleRepository` 구조, `NLOHMANN_DEFINE_TYPE_INTRUSIVE` 기반 직렬화, 임시 파일 → rename 저장 패턴, phase별 개발 계획을 그대로 따른다. 자세한 대응 관계는 `../CLAUDE.md`의 "참고 구현" 절 참고.

## 11. 산출물

- 데이터 모델: `Model/Sample.h`, `Model/Order.h`, `Model/ProductionQueueEntry.h`
- JSON 파싱/저장 설계 문서: `docs/feature/json-parsing.md`, `docs/feature/json-file-storage.md`
- CRUD 설계 문서: `docs/feature/create.md`, `docs/feature/read.md`, `docs/feature/update.md`, `docs/feature/delete.md`
- phase별 개발 계획/설계 문서: `docs/PLAN.md`, `docs/design/phase0-foundation.md` ~ `phase4-delete.md`
- `SampleRepository`/`ConsoleApp` 소스 코드

## 12. 완료 기준 (Definition of Done)

- `Sample` 데이터 모델 / 리포지토리(JSON 파일 입출력) / 콘솔 UI가 명확히 분리된 파일 구조로 존재
- Create/Read/Update/Delete가 모두 동작하고, 애플리케이션 재시작 후에도 데이터가 유지됨
- 저장 실패 시 기존 파일이 훼손되지 않음(임시 파일 → rename 검증)
- Phase 0~4가 각각 정의된 완료 기준(빌드 성공 + 실제 실행 확인)을 순서대로 만족함
