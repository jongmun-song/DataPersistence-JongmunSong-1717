# CLAUDE.md

이 파일은 이 저장소에서 작업할 때 Claude Code(claude.ai/code)에게 제공되는 가이드입니다.

## 저장소 목적

이 저장소는 `../ref/requirements.pdf`에 기술된 "반도체 시료 생산주문관리 시스템" 과제의 **"데이터 영속성 처리 PoC"**입니다.

`SSemiProductManager/` 아래의 5개 형제 PoC 저장소 중 하나입니다.
- `ConsoleMVC` — Model/Controller/View 구조 및 역할 분리
- `DataPersistence` (본 저장소) — 저장소/CRUD 처리 (파일, JSON, 또는 DB)
- `DataMonitor` — 저장된 데이터를 실시간으로 조회하는 콘솔 도구
- `DummyDataGenerator` — 영속성 계층에 더미 데이터를 채워 넣는 도구
- `SampleOrderSystem` — 실제 비즈니스 애플리케이션(주문/생산/재고 흐름)으로, 이 저장소가 검증한 영속성 메커니즘을 참고하여 실제 도메인 엔티티(Sample, Order 등)를 저장할 것으로 예상됨

**PoC로서의 성격**: 이 저장소는 "JSON 파일 기반 CRUD 영속성"이라는 메커니즘 자체를 검증하는 것이 목적입니다. 참고 구현([jongmun-song/JSON_Console](https://github.com/jongmun-song/JSON_Console))에 있는 것과 동일하게, 하나의 구체적인 엔티티를 대상으로 한 리포지토리를 직접 구현합니다. 다만 엔티티 자체는 `Member` 같은 임의의 예시 대신, `ConsoleMVC/Tests/Example`의 PoC 검증용 도메인 모델(`Sample`, `Order`, `ProductionQueueEntry`)을 참고하여 실제로 사용할 데이터 모델로 정의합니다 — 자세한 내용은 아래 "데이터 모델(Model/)" 절을 참고하세요.

## 데이터 모델 (`Model/`)

`Model/` 디렉터리에 이 저장소가 실제로 영속화할 데이터 모델을 정의합니다. 각 모델은 `ConsoleMVC/Example/Model/`의 대응 타입을 참고했지만, ConsoleMVC의 핵심 타입(`IEntity`, `QuantityGuard`, `StatefulModel`)에 의존하지 않는 **순수 데이터 구조체**입니다 — 이 저장소는 ConsoleMVC에 의존하지 않으며, 각 구조체는 `NLOHMANN_DEFINE_TYPE_INTRUSIVE`(또는 열거형은 `NLOHMANN_JSON_SERIALIZE_ENUM`)로 스스로 JSON과 상호 변환할 수 있는 것 외에는 아무 책임도 지지 않습니다. 재고 증감 가드나 상태 전이 검증 같은 비즈니스 규칙은 이 모델의 책임이 아니며, 이를 사용하는 프로젝트(`SampleOrderSystem`)의 책임입니다.

| 파일 | 참고한 ConsoleMVC 예시 | 필드 |
|---|---|---|
| `Model/Sample.h` | `Example/Model/Sample.h` | `id`, `name`, `averageProductionTimePerUnit`, `yieldRatio`, `stockQuantity` |
| `Model/Order.h` | `Example/Model/Order.h` | `id`, `sampleId`, `customerName`, `orderedQuantity`, `state`(`OrderState`: RESERVED/CONFIRMED/PRODUCING/RELEASE/REJECTED) |
| `Model/ProductionQueueEntry.h` | `Example/Model/ProductionQueueEntry.h` | `orderId`, `sampleId`, `orderedQuantity`, `shortageQuantity`, `actualProductionQuantity`, `totalProductionTime`, `state`(`ProductionState`: WAITING/PRODUCING/CONFIRMED, 기본값 WAITING) |

이 중 `Sample`을 Phase 0~4(아래 "개발 단계" 참고)에서 리포지토리 CRUD를 시연하는 1차 대상으로 삼습니다. `Order`, `ProductionQueueEntry`는 동일한 리포지토리 패턴(`docs/feature/json-parsing.md`, `docs/feature/json-file-storage.md`)을 그대로 적용할 수 있는 형태로 이미 정의해 두었으며, 필요 시 `SampleRepository`와 같은 구조로 `OrderRepository`/`ProductionQueueEntryRepository`를 추가하면 됩니다.

## 개발 단계(Phase)로 진행 상황을 확인합니다

이 PoC는 `docs/PLAN.md`에 정의된 phase 순서대로 진행하며, **각 phase가 끝날 때마다 빌드·실행 가능한 콘솔 애플리케이션**을 산출물로 남깁니다. 다음 phase는 이전 phase가 남긴 실행 가능한 상태 위에 기능을 덧붙이는 방식으로 진행하므로, 언제든 "지금 몇 번째 phase까지 실행 가능한 상태인지"로 진행 상황을 확인할 수 있습니다.

- Phase 0: 프로젝트 뼈대 + JSON 라이브러리 연동 — [`docs/design/phase0-foundation.md`](docs/design/phase0-foundation.md)
- Phase 1: Create 구현 — [`docs/design/phase1-create.md`](docs/design/phase1-create.md)
- Phase 2: Read 구현 — [`docs/design/phase2-read.md`](docs/design/phase2-read.md)
- Phase 3: Update 구현 — [`docs/design/phase3-update.md`](docs/design/phase3-update.md)
- Phase 4: Delete 구현 — [`docs/design/phase4-delete.md`](docs/design/phase4-delete.md)

각 phase 문서에는 목표, 리포지토리/콘솔 UI에 추가할 내용, 메뉴 구성, 그리고 **실행으로 확인해야 하는 완료 기준**이 함께 정의되어 있습니다. 어떤 phase든 완료 기준(빌드 성공 + 실제 실행 확인)을 만족하지 못하면 다음 phase로 넘어가지 않습니다.

## 현재 상태

`Model/Sample.h`, `Model/Order.h`, `Model/ProductionQueueEntry.h`가 정의되어 있고 `DataPersistence.vcxproj`에 등록되어 있습니다. 그 외 리포지토리/콘솔 UI 소스는 아직 없습니다 — Phase 0(`docs/design/phase0-foundation.md`)에서 `SampleRepository`/`ConsoleApp`/`main.cpp`를 추가합니다. 새 소스 파일을 추가할 때는 `DataPersistence.vcxproj`(`<ClCompile>`/`<ClInclude>`가 포함된 `<ItemGroup>`)와 `DataPersistence.vcxproj.filters`(“소스 파일” / “헤더 파일”로 분류) 양쪽 모두에 등록하세요.

## 빌드

Visual Studio C++ 프로젝트(`DataPersistence.vcxproj` / `DataPersistence.slnx`)이며, C++20(`stdcpp20`)을 대상으로 `x86`/`x64`, Debug/Release 구성의 Win32 `Application`(콘솔 서브시스템)으로 빌드됩니다. 툴셋은 `v145`, 문자 집합은 Unicode입니다. `AdditionalIncludeDirectories`에 `external/`이 등록되어 있어 `#include <nlohmann/json.hpp>`를 바로 사용할 수 있습니다.

- `DataPersistence.slnx`를 Visual Studio(v145 툴셋)에서 열어 빌드/실행하거나,
- 다음과 같이 명령줄에서 MSBuild로 빌드할 수 있습니다:
  ```
  msbuild DataPersistence.vcxproj /p:Configuration=Debug /p:Platform=x64
  ```
- 빌드된 실행 파일은 `x64\Debug\DataPersistence.exe`에서 실행하세요(경로는 구성/플랫폼에 따라 다름).

`packages.config`에 googletest(`Microsoft.googletest.v140.windesktop.msvcstl.static.rt-dyn`)가 NuGet으로 등록되어 있습니다. 테스트 프로젝트/러너 구성은 아직 없으며, 첫 테스트를 추가할 때 함께 정비하세요. CMake는 사용하지 않습니다.

## 개발 요구사항 (requirements.pdf Chapter 3 — PoC 범위 기준)

### 1. 영속화 방식 선택

파일, JSON, DB 중 하나의 방식을 선택하여 구현합니다(요구사항 문서상 자유 선택 항목). 다음을 기본값으로 채택합니다:
- **JSON 파일 기반 영속화**, 파서/직렬화 라이브러리는 [nlohmann/json](https://github.com/nlohmann/json) 사용 (자체 JSON 파서를 구현하지 않음).
- 헤더 온리 라이브러리 원본이 `external/nlohmann/json.hpp`에 이미 포함되어 있습니다.

### 2. CRUD 지원

- Create / Read(단건 조회, 전체 목록 조회) / Update / Delete를 모두 지원해야 합니다.
- 각 데이터는 고유 ID로 식별되며, ID는 리포지토리가 자동 부여합니다(최댓값+1 방식).
- 존재하지 않는 ID에 대한 조회/수정/삭제는 예외를 던지지 않고 `실패`(예: `bool` 반환 또는 `std::optional`)로 표현합니다. 반면 잘못된 입력값(필수 필드 누락 등)이나 저장 실패(파일 쓰기 오류 등)는 예외로 전달합니다.

### 3. 데이터 영속성 보장

- 애플리케이션을 다시 실행해도 데이터가 유지되어야 합니다(요구사항 문서의 "데이터 영속성" 정의).
- 파일 쓰기는 원자적으로 처리하세요: 임시 파일에 먼저 쓰고 `std::filesystem::rename`으로 원본 파일을 교체하는 방식을 기본으로 합니다. 쓰기 도중 실패 시 메모리 상의 상태를 변경 전으로 롤백해야 합니다.

## 참고 구현 — jongmun-song/JSON_Console

유사한 목적(회원 정보 CRUD, DB 없이 JSON 파일 영속화)의 PoC가 https://github.com/jongmun-song/JSON_Console 에 구현되어 있습니다. 이 저장소는 대상 엔티티만 `Sample`로 바꾸고 그 구조를 그대로 따라갑니다:

- **계층 분리**: 데이터 모델(`Model/Sample.h`) / JSON 파일 입출력을 담당하는 리포지토리(`SampleRepository.h`·`.cpp`) / 콘솔 UI(`ConsoleApp`)를 별도 translation unit으로 분리.
- **엔티티 직렬화**: `NLOHMANN_DEFINE_TYPE_INTRUSIVE` 매크로로 `Sample` 구조체 자체에 JSON 변환 책임을 위임(참고 구현의 `Member`에 대응).
- **리포지토리 인터페이스**: `load()`(파일→메모리, 파일 없으면 빈 목록), `save()`(메모리→파일, 임시 파일 작성 후 rename), `all()`, `create()`, `findById()`, `update(id, mutator)`, `remove(id)`.
- **오류 처리 원칙**: ID 미존재는 `std::nullopt`/`false`로 표현(예외 아님), 입력 검증 실패는 `std::invalid_argument`, 저장 실패는 예외 전달 + 메모리 상태 롤백.
- **ID 부여**: 리포지토리 내부에서 현재 목록의 최댓값 ID + 1로 자동 부여, 입력값의 ID는 무시.
- **phase 단위 진행**: 참고 구현의 `docs/PLAN.md`/`docs/design/phase*.md` 구성을 그대로 가져와, Phase 0(뼈대) → Phase 1(Create) → Phase 2(Read) → Phase 3(Update) → Phase 4(Delete) 순서로 개발한다.

## 문서 안내

- `docs/PRE.md`: 이 저장소의 전체 요구사항(배경/목적/범위/기술 스택/완료 기준)을 정리한 문서. 요구사항의 단일 출처(source of truth)로 삼습니다.
- `docs/PLAN.md`: phase별 목표·완료 기준을 정리한 개발 계획. 지금 어느 phase까지 진행됐는지 확인할 때 이 문서를 먼저 봅니다.
- `docs/design/`: phase별 상세 설계 문서(`phase0-foundation.md` ~ `phase4-delete.md`). `Sample`/`SampleRepository`/`ConsoleApp`의 각 phase별 구체적인 인터페이스와 흐름을 정의합니다.
- `docs/feature/`: 기능별(JSON 파싱/저장, Create/Read/Update/Delete) 상세 명세.
  - [`json-parsing.md`](docs/feature/json-parsing.md) — JSON 파일 → 메모리 시료 목록 변환
  - [`json-file-storage.md`](docs/feature/json-file-storage.md) — 메모리 시료 목록 → JSON 파일 저장(원자적 쓰기)
  - [`create.md`](docs/feature/create.md) / [`read.md`](docs/feature/read.md) / [`update.md`](docs/feature/update.md) / [`delete.md`](docs/feature/delete.md) — CRUD 각 기능의 동작 명세
- `docs/CODE_CONVENTION.md`: 로버트 C. 마틴의 클린 코드 원칙에 기반한 코드 컨벤션. `reviewer` subagent의 권고 사항 검토 기준(가독성, SRP, 중복 제거 등)이 여기에 정의되어 있습니다.
- `.claude/agents/`: manager(총감독)/developer/reviewer/tester subagent 정의. phase별 구현은 이 파이프라인(요구사항 확인 → 구현 → 리뷰 → 테스트)을 통해 진행합니다.
- `external/nlohmann/json.hpp`: 헤더 온리 JSON 라이브러리(vendored). 별도 패키지 관리자 없이 이 헤더를 그대로 참조합니다.
