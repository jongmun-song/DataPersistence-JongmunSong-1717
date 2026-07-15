// Entry point (see docs/design/phase0-foundation.md).

#include <exception>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

#include "ConsoleApp.h"
#include "SampleRepository.h"

int main()
{
#ifdef _WIN32
    // Source files are UTF-8; align the console output code page so
    // Korean menu text renders correctly instead of as mojibake.
    SetConsoleOutputCP(CP_UTF8);
#endif

    DataPersistence::SampleRepository repository("samples.json");

    try
    {
        repository.load();
    }
    catch (const std::exception& e)
    {
        std::cerr << "samples.json 파일을 읽는 중 오류가 발생했습니다: " << e.what() << '\n';
        return 1;
    }

    DataPersistence::ConsoleApp app(repository);
    app.run();

    return 0;
}
