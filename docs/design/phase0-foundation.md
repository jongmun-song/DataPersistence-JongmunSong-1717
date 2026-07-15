# Phase 0 설계: 프로젝트 뼈대 + JSON 라이브러리 연동

관련 계획: [`docs/PLAN.md`](../PLAN.md#phase-0-프로젝트-뼈대--json-라이브러리-연동)
관련 요구사항: [`docs/feature/json-parsing.md`](../feature/json-parsing.md),
[`docs/feature/json-file-storage.md`](../feature/json-file-storage.md)

## 목표

이후 모든 phase가 공유할 모듈 구조(모델 / 리포지토리 / 콘솔 UI)와 nlohmann/json
연동 방식을 확정한다. 이 시점의 실행 결과물은 CRUD 메뉴 없이 "메뉴 표시 → 종료"만
동작하는 최소 콘솔 앱이다.

## 모듈 구성

`CLAUDE.md`의 계층 분리 방향(모델 / 리포지토리 / UI를 별도 translation unit으로)에
따라 다음과 같이 나눈다.

| 파일 | 책임 |
|---|---|
| `Model/Sample.h` | 시료 데이터 모델(구조체) 정의, JSON 변환 — 이미 작성되어 있음 |
| `SampleRepository.h` / `.cpp` | JSON 파일 읽기/쓰기, 메모리 상 목록 관리 |
| `ConsoleApp.h` / `.cpp` | 메뉴 출력, 사용자 입력 처리, 각 CRUD 핸들러 호출 |
| `main.cpp` | 엔트리 포인트: 리포지토리 생성, `ConsoleApp` 구동 |

각 파일은 `DataPersistence.vcxproj`(및 `.filters`)에 등록한다.

## 데이터 모델

```cpp
// Model/Sample.h (이미 작성되어 있음 — CLAUDE.md "데이터 모델(Model/)" 참고)
struct Sample {
    int id = 0;
    std::string name;
    double averageProductionTimePerUnit = 0.0;
    double yieldRatio = 0.0;
    int stockQuantity = 0;
};

// nlohmann/json 변환 (NLOHMANN_DEFINE_TYPE_INTRUSIVE 매크로 사용)
NLOHMANN_DEFINE_TYPE_INTRUSIVE(Sample, id, name, averageProductionTimePerUnit, yieldRatio, stockQuantity)
```

- `Sample`은 `ConsoleMVC/Example/Model/Sample.h`를 참고해 이미 정의되어 있다. Phase 0는
  이 모델을 그대로 사용하고, 필드를 추가/변경하지 않는다.

## 리포지토리 계층

```cpp
// SampleRepository.h
class SampleRepository {
public:
    explicit SampleRepository(std::filesystem::path jsonPath);

    void load();                 // 파일 -> sampleList_ (파일 없으면 빈 목록)
    void save() const;           // sampleList_ -> 파일 (임시 파일 -> rename)

    const std::vector<Sample>& all() const;

private:
    std::filesystem::path jsonPath_;
    std::vector<Sample> sampleList_;
};
```

- `load()`: 파일이 없으면 `sampleList_`를 빈 벡터로 두고 반환한다. 파일은 있으나
  파싱 오류(`nlohmann::json::parse_error`)면 예외를 상위로 전달해 `ConsoleApp`이
  사용자에게 오류를 알리고 종료하도록 한다.
- `save()`: `nlohmann::json` 배열로 직렬화 후 임시 파일에 쓰고 원본으로 교체한다
  (`docs/feature/json-file-storage.md` 참고).
- Create/Read/Update/Delete에 필요한 메서드(`create`, `findById`, `update`, `remove`
  등)는 이후 phase에서 이 클래스에 점진적으로 추가한다 — Phase 0에서는 `load`/`save`/
  `all`만 구현한다.

## 콘솔 UI 골격

```cpp
// ConsoleApp.h
class ConsoleApp {
public:
    explicit ConsoleApp(SampleRepository& repo);
    void run();   // 메뉴 출력 -> 입력 처리 -> 종료 시까지 반복

private:
    void printMenu() const;
    SampleRepository& repo_;
};
```

- Phase 0의 메뉴 항목은 `0. 종료` 하나뿐이다. 이후 phase마다 항목이 하나씩 늘어난다.

## 라이브러리 연동 방식

- `external/nlohmann/json.hpp`에 헤더 온리 라이브러리가 이미 포함되어 있고,
  `DataPersistence.vcxproj`의 `AdditionalIncludeDirectories`에 `external/`이 이미
  등록되어 있다. vcpkg나 NuGet으로 새로 설치할 필요가 없다.
- 소스에서는 `#include <nlohmann/json.hpp>` 하나만으로 사용 가능하다(헤더 온리).

## 완료 기준 (실행 확인)

- 빌드: `msbuild DataPersistence.vcxproj /p:Configuration=Debug /p:Platform=x64` 성공.
- 실행: JSON 파일이 없는 상태로 실행 시 크래시 없이 메뉴가 출력되고, `0` 입력 시
  정상 종료된다.
