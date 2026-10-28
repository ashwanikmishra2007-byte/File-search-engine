/*#include <iostream>
#include <filesystem>
#include <string>
#include <system_error>
#include <windows.h>

namespace fs = std::filesystem;*/

/*int main()
{
    const fs::path target = R"(C:\Documents and Settings)";
    std::error_code ec; // To handle any potential errors during filesystem operations

    std::cout << "Target: " << target.string() << '\n';
    std::cout << "Exists: " << fs::exists(target, ec) << '\n';

    if (ec)
    {
        std::cout << "Exists error: "
                  << ec.value() << " - "
                  << ec.message() << '\n';
        ec.clear(); // Clear the message after finding the error
    }

    // fs:: recursive_directory_iterator it(target,ec);

    std::cout << "Is directory: "
              << fs::is_directory(target, ec) << '\n';

    if (ec)
    {
        std::cout << "Directory error: "
                  << ec.value() << " - "
                  << ec.message() << '\n';
        ec.clear();
    }

    // fs::recursive_directory_iterator end;
    const auto status = fs::symlink_status(target, ec);  //check if it is a symbolic link if an error report through ec

    /*while (it != end)
    {
        std::cout << it->path().string() << '\n';
        it.increment(ec);

        if (ec)
        {
            std::cerr << "Increment failed: "
                      << ec.value() << " - "
                      << ec.category().name() << " - "
                      << ec.message() << '\n';
            return 2;
        }
    }
    if (ec)
    {
        std::cout << "Symlink status error: "
                  << ec.value() << " - "
                  << ec.message() << '\n';
    }
    else
    {
        std::cout << "Is symlink: "
                  << fs::is_symlink(status) << '\n';
        std::cout << "Is directory (symlink status): "
                  << fs::is_directory(status) << '\n';
    }
}*/

/*int main()
{
    const char *path = R"(C:\Documents and Settings)";

    DWORD attributes = GetFileAttributesA(path); // Get file attributes by passing the address of path
    // The A suffix means this version accepts a narrow-character string.
    Windows may report several attributes together as individual bits in an integer.
    For example, the result can indicate that the path represents a directory and is a reparse point.

    if (attributes == INVALID_FILE_ATTRIBUTES) The Windows API uses the special constant INVALID_FILE_ATTRIBUTES to indicate failure.
   checks whether Windows failed to retrieve the attributes.
    {
        std::cout << "Failed: " << GetLastError() << '\n'; //If it failed retrieves the Windows error code associated with the failure.
        return 1;
    }

    if (attributes & FILE_ATTRIBUTE_REPARSE_POINT)
    { First, Windows defines the constant:
        FILE_ATTRIBUTE_REPARSE_POINT
        It represents a particular bit flag in the attributes value.
        The expression:
        attributes & FILE_ATTRIBUTE_REPARSE_POINT
        uses the bitwise AND operator (&) to check whether that flag is present.
        std::cout << "This path is a reparse point.\n";
    }
    else
    {
        std::cout << "This path is not a reparse point.\n";
    }

    if (attributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        std::cout << "This path is a directory.\n";
    }
}

/*int main() {
    const fs::path target = R"(C:\Documents and Settings)";

    std::error_code ec;

    fs::path resolved = fs::canonical(target,ec);

    if(ec){
        std::cout << "Resolution failed: "
                  << ec.message() << '\n';
        return 1;
    }

    std::cout<<"Original path"<<target<<"\n";
    std::cout<<"Resolved path"<<resolved<<"\n";

}*/