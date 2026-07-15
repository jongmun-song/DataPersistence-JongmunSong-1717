# JSON 파싱

## 개요

디스크에 저장된 JSON 파일을 읽어 메모리 상의 시료(`Sample`) 목록으로 변환하는 기능. 외부 라이브러리인 [nlohmann/json](https://github.com/nlohmann/json)을 사용해 파일 내용을 파싱한다(직접 파서를 구현하지 않는다).

## 라이브러리 연동

- 헤더 온리 라이브러리인 `nlohmann/json`(`nlohmann/json.hpp`)을 사용한다. 이 저장소에는 `external/nlohmann/json.hpp`에 이미 포함되어 있다.
- `DataPersistence.vcxproj`의 `AdditionalIncludeDirectories`에 `external/`이 이미 연결되어 있으며, 새 소스에서는 `#include <nlohmann/json.hpp>` 후 `nlohmann::json` 타입으로 JSON 값을 다룬다.
- vcpkg/NuGet으로 별도 설치할 필요 없이, 저장소에 포함된 헤더를 그대로 참조하는 방식을 기본으로 한다.

## 동작

- 지정된 경로의 JSON 파일을 `std::ifstream`으로 읽는다.
- `nlohmann::json::parse()`로 파일 스트림을 파싱하여 `nlohmann::json` 값(배열)으로 변환한다.
- JSON 배열의 각 요소를 `Sample` 모델(`Model/Sample.h`)로 매핑한다. `nlohmann::json`의 `get<Sample>()` 또는 `NLOHMANN_DEFINE_TYPE_INTRUSIVE`가 생성한 변환 함수를 활용해 모델 구조체와 상호 변환한다.
- 파일이 존재하지 않을 경우 빈 목록으로 취급한다(예외를 던지지 않는다).
- 파싱 중 문법 오류가 발생하면(`nlohmann::json::parse_error` 예외) 오류를 상위로 전달하여 CRUD 동작(Create/Read/Update/Delete)이 손상된 데이터로 잘못된 처리를 이어가지 않도록 중단시킨다.

## 관련 기능

- [json-file-storage](json-file-storage.md): 파싱된 데이터를 다시 파일로 저장할 때 사용.
- [read](read.md): 파싱 결과를 목록 조회/검색에 사용.
