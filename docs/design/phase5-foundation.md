# Phase 5 설계: 모델 선택 메뉴 + Order/ProductionQueueEntry 리포지토리 뼈대

관련 계획: [`docs/PLAN.md`](../PLAN.md#phase-5-모델-선택-메뉴--orderproductionqueueentry-리포지토리-뼈대)
관련 요구사항: [`docs/feature/json-parsing.md`](../feature/json-parsing.md),
[`docs/feature/json-file-storage.md`](../feature/json-file-storage.md)

## 목표

Phase 0~4에서 `Sample`을 대상으로 검증한 모듈 구조(모델 / 리포지토리 / 콘솔 UI
분리, load/save/all, 원자적 쓰기)를 `Order`, `ProductionQueueEntry`에도 그대로
적용할 수 있는 뼈대를 마련한다. 동시에 세 모델(Sample/Order/ProductionQueueEntry)
중 하나를 먼저 선택하는 최상위 메뉴(`MainConsoleApp`)를 도입해, 이후 phase마다
Order/ProductionQueueEntry 각각의 CRUD 메뉴를 독립적으로 채워 나갈 수 있는
구조를 만든다. 이 시점의 실행 결과물은 Order/ProductionQueueEntry 메뉴에
"뒤로가기"만 있는 상태다.

## ProductionQueueEntry 키 설계

`ProductionQueueEntry`(`Model/ProductionQueueEntry.h`)는 자체 `id` 필드가 없고
`orderId`가 자연키다(1주문 = 1큐 엔트리 가정). `Sample`/`Order`처럼 인조 id를
추가하지 않는다.

- 생성 시 `orderId`를 자동 부여(max+1)하지 않고 입력값을 그대로 사용한다.
  `orderId <= 0`이면 `std::invalid_argument`.
- 이미 동일 `orderId`를 가진 엔트리가 존재하면 `std::invalid_argument`("이미
  존재하는 orderId입니다").
- 리포지토리 메서드명은 키가 `orderId`임을 드러내도록 `findByOrderId(int orderId)`,
  `update(int orderId, mutator)`, `remove(int orderId)`로 명명한다(Sample의
  `findById`/`update(id, ...)`/`remove(id)`에 대응).
- `sampleId`/`orderId`의 참조 무결성(실존 여부) 검증은 이 저장소의 책임이 아니다
  — `SampleOrderSystem`의 책임이다.

## 모듈 구성

| 파일 | 책임 |
|---|---|
| `Model/Order.h` | 주문 데이터 모델(구조체) 정의, JSON 변환 — 이미 작성되어 있음 |
| `Model/ProductionQueueEntry.h` | 생산 큐 엔트리 데이터 모델 정의, JSON 변환 — 이미 작성되어 있음 |
| `OrderRepository.h` / `.cpp` | `orders.json` 읽기/쓰기, 메모리 상 목록 관리 |
| `ProductionQueueEntryRepository.h` / `.cpp` | `production_queue.json` 읽기/쓰기, 메모리 상 목록 관리 |
| `OrderConsoleApp.h` / `.cpp` | Order 메뉴 출력, 입력 처리, CRUD 핸들러 호출 |
| `ProductionQueueEntryConsoleApp.h` / `.cpp` | ProductionQueueEntry 메뉴 출력, 입력 처리, CRUD 핸들러 호출 |
| `MainConsoleApp.h` / `.cpp` | 최상위 모델 선택 메뉴, 하위 ConsoleApp 구동 |
| `ConsoleApp.h` / `.cpp` | (기존) Sample 메뉴 — "0. 종료" 문구만 "0. 뒤로가기"로 수정 |
| `main.cpp` | 엔트리 포인트: 리포지토리 3개 생성/load, 서브 ConsoleApp 3개 구성, `MainConsoleApp` 구동 |

각 신규 파일은 `DataPersistence.vcxproj`(및 `.filters`)에 등록한다.

## 리포지토리 계층

```cpp
// OrderRepository.h
class OrderRepository {
public:
    explicit OrderRepository(std::filesystem::path jsonPath);

    void load();                 // 파일 -> orderList_ (파일 없으면 빈 목록)
    void save() const;           // orderList_ -> 파일 (임시 파일 -> rename)

    const std::vector<Model::Order>& all() const;

private:
    std::filesystem::path jsonPath_;
    std::vector<Model::Order> orderList_;
};
```

```cpp
// ProductionQueueEntryRepository.h
class ProductionQueueEntryRepository {
public:
    explicit ProductionQueueEntryRepository(std::filesystem::path jsonPath);

    void load();                 // 파일 -> entryList_ (파일 없으면 빈 목록)
    void save() const;           // entryList_ -> 파일 (임시 파일 -> rename)

    const std::vector<Model::ProductionQueueEntry>& all() const;

private:
    std::filesystem::path jsonPath_;
    std::vector<Model::ProductionQueueEntry> entryList_;
};
```

- `load()`/`save()`는 `SampleRepository`(Phase 0)와 동일한 규칙을 따른다: 파일이
  없으면 빈 목록, 파싱 오류는 예외 전달, 저장은 임시 파일 작성 후
  `std::filesystem::rename`.
- Create/Read/Update/Delete에 필요한 메서드(`create`, `findById`/
  `findByOrderId`, `update`, `remove`)는 Phase 6~9에서 점진적으로 추가한다 —
  Phase 5에서는 `load`/`save`/`all`만 구현한다.
- 파일명은 `orders.json`, `production_queue.json`을 사용한다.

## 콘솔 UI 재구성

### 기존 `ConsoleApp`(Sample 전용)

거의 그대로 유지하되 `printMenu()`의 "0. 종료" 문구를 "0. 뒤로가기"로 수정한다
(최상위 종료는 이제 `MainConsoleApp`의 책임이므로).

### 신규 `OrderConsoleApp` / `ProductionQueueEntryConsoleApp`

`ConsoleApp`과 동일한 구조(메뉴 출력 → 입력 처리 → 핸들러 호출 반복)를 각각
독립적으로 만든다. 공통 베이스 클래스는 두지 않는다 — PoC 성격상 세 모델 사이의
조기 일반화를 지양하고, Sample에서 검증된 패턴을 그대로 복제하는 것으로 충분하다.

```cpp
// OrderConsoleApp.h
class OrderConsoleApp {
public:
    explicit OrderConsoleApp(OrderRepository& repo);
    void run();   // 메뉴 출력 -> 입력 처리 -> "뒤로가기" 시까지 반복

private:
    void printMenu() const;
    OrderRepository& repo_;
};
```

```cpp
// ProductionQueueEntryConsoleApp.h
class ProductionQueueEntryConsoleApp {
public:
    explicit ProductionQueueEntryConsoleApp(ProductionQueueEntryRepository& repo);
    void run();

private:
    void printMenu() const;
    ProductionQueueEntryRepository& repo_;
};
```

Phase 5 시점의 메뉴 항목은 둘 다 `0. 뒤로가기` 하나뿐이다.

### 신규 `MainConsoleApp`

최상위 모델 선택 메뉴를 담당한다.

```cpp
// MainConsoleApp.h
class MainConsoleApp {
public:
    MainConsoleApp(ConsoleApp& sampleApp, OrderConsoleApp& orderApp,
        ProductionQueueEntryConsoleApp& entryApp);
    void run();   // 모델 선택 메뉴 -> 하위 앱 run() 호출 -> 복귀 반복, "0" 선택 시 종료

private:
    void printMenu() const;
    ConsoleApp& sampleApp_;
    OrderConsoleApp& orderApp_;
    ProductionQueueEntryConsoleApp& entryApp_;
};
```

```
1. Sample 관리
2. Order 관리
3. ProductionQueueEntry 관리
0. 종료
```

1/2/3을 선택하면 해당 서브 `ConsoleApp`의 `run()`을 호출하고, 그 `run()`이
반환되면(하위 메뉴에서 "뒤로가기") 다시 모델 선택 메뉴로 복귀한다. 최상위에서
`0`을 눌러야 프로그램이 진짜 종료된다.

`main.cpp`는 `SampleRepository`/`OrderRepository`/`ProductionQueueEntryRepository`
3개를 생성하고 각각 `load()`한 뒤, `ConsoleApp`/`OrderConsoleApp`/
`ProductionQueueEntryConsoleApp` 3개를 만들어 `MainConsoleApp`에 전달하고
`MainConsoleApp::run()`을 호출하는 구조로 교체한다.

## 완료 기준 (실행 확인)

- 빌드: `msbuild DataPersistence.vcxproj /p:Configuration=Debug /p:Platform=x64` 성공.
- 실행: `orders.json`/`production_queue.json`이 없는 상태로 실행해도 크래시 없이
  모델 선택 메뉴가 출력된다.
- `1`(Sample)을 선택하면 기존 Sample CRUD 메뉴가 정상 동작하고, `0`(뒤로가기)으로
  모델 선택 메뉴로 복귀한다.
- `2`(Order), `3`(ProductionQueueEntry)를 선택하면 "뒤로가기"만 있는 메뉴가
  표시되고, `0`으로 모델 선택 메뉴로 복귀한다.
- 최상위 모델 선택 메뉴에서 `0`을 선택했을 때만 프로그램이 종료된다.
