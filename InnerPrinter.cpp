#include"InnerPrinter.h"

std::string InnerPrinter::cut_quotas(const std::string& str)
{
    //erase " " at the beginning and the end of string
    return str.substr(0, str.size());
}
void InnerPrinter::erase_sub(std::string& str, const std::string& sub)
{
    //erase substring from str

    auto pos = str.find(sub);
    if (pos != std::string::npos)
    {
        str.erase(pos, sub.length());
    }
}
std::string InnerPrinter::prepare_entry_val(const fs::directory_entry& dir_entry, unmutable options)
{
    auto entry_val = dir_entry.path().filename().string();
    entry_val = RECURSIVE? dir_entry.path().parent_path().string() + "/" + entry_val : entry_val;
    if (RECURSIVE)
    {
        erase_sub(entry_val, options->dir);
        if (entry_val[0] == '/' || entry_val[0] == '\\')
            entry_val = entry_val.substr(1); //erase weird / or \ at the beginning of str
    }

    return fs::path{entry_val}.string();
};

uintmax_t InnerPrinter::ipow(uintmax_t base, uintmax_t exp)
{
    uintmax_t result = 1;
    for (;;)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }

    return result;
}
std::string InnerPrinter::convert_bytes(uintmax_t file_size)
{
    const char* prefix = "BKMGTPE";

    auto format = [](const std::string& str)
    {
        auto p = str.find('.');

        //str contains integer number
        if (p == std::string::npos)
        {
            return str;
        }
        else
        {
            return str.substr(0, p + 2);
        }
    };
    for (size_t i = 0; i < 8; i++)
    {
        if (file_size <= sizes[i])
        {
            if (i != 0)
            {
                auto size = (double)file_size / sizes[i - 1];
                return format(std::to_string(size)) + prefix[i] + "iB";
            }
            else
                return std::to_string(file_size) + "B";
        }
    }
}
void InnerPrinter::iterate_over_dir(unmutable options,
    const fn& iterate,
    const fn1& iterate_sorted)
{
    std::list<fs::directory_entry> dirs, files;
    for (const fs::directory_entry& dir_entry : fs::directory_iterator(options->dir))
    {
        iterate(dir_entry, dirs, files);
    }

    if (SORT)
        iterate_sorted(dirs, files);
}
void InnerPrinter::iterate_over_dir_recursively(unmutable options,
    const fn& iterate,
    const fn1& iterate_sorted)
{
    std::list<fs::directory_entry> dirs, files;

    for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(options->dir))
    {
        iterate(dir_entry, dirs, files);
    }

    if (SORT)
        iterate_sorted(dirs, files);
}
void InnerPrinter::print_d(const std::string& str, unmutable options)
{
    //print passed directory without colors if print_pure option is on,else print with colors from config file

    if (PRINT_PURE)
    {
        fmt::print("{}", str);
    }
    else
    {
        fmt::print(FG(options->dir_color) | BG(options->dir_bg_color), str);
    }
}
void InnerPrinter::print_f(const std::string& str, unmutable options)
{
    //print passed file without colors if print_pure option is on,else print with colors from config file

    if (PRINT_PURE)
    {
        fmt::print("{}", str);
    }
    else
    {
        fmt::print(FG(options->file_color) | BG(options->file_bg_color), str);
    }
}

size_t InnerPrinter::max_dir_size(const std::string& dir, unmutable options)
{
    std::vector<size_t> sizes;
    for (const fs::directory_entry& dir_entry : fs::directory_iterator(dir))
    {
        sizes.push_back(InnerPrinter::prepare_entry_val(dir_entry, options).size());
    }

    return *std::max_element(sizes.begin(), sizes.end());
}
size_t InnerPrinter::max_dir_size_rec(const std::string& dir, unmutable options)
{
    std::vector<size_t> sizes;
    for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(dir))
    {
        sizes.push_back(InnerPrinter::prepare_entry_val(dir_entry, options).size());
    }

    return *std::max_element(sizes.begin(), sizes.end());
}
size_t InnerPrinter::max_file_in_dir(const std::string& dir, unmutable options)
{
    std::vector<size_t> sizes;
    for (const fs::directory_entry& dir_entry : fs::directory_iterator(dir))
    {
        if (!fs::is_directory(dir_entry))
        {
            try
            {
                auto size = convert_bytes(fs::file_size(dir_entry));
                sizes.push_back(size.size());
            }
            catch (const std::exception& e)
            {

                fmt::print("{}", e.what());
                sizes.push_back(0);
            }
        }
    }

    return *std::max_element(sizes.begin(), sizes.end());
}
size_t InnerPrinter::max_file_in_dir_rec(const std::string& dir, unmutable options)
{
    std::vector<size_t> sizes;
    for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(dir))
    {
        if (!fs::is_directory(dir_entry))
        {
            try
            {
                auto size = convert_bytes(fs::file_size(dir_entry));
                sizes.push_back(size.size());
            }
            catch (const std::exception& e)
            {

                fmt::print("{}", e.what());
                sizes.push_back(0);
            }
        }
    }

    return *std::max_element(sizes.begin(), sizes.end());
}

void InnerPrinter::print_time(const std::string& time, unmutable options, const std::string& space)
{
    if ((SHOW_PERMISSION && !SHOW_FILE_SIZE) ||
        (SHOW_PERMISSION && SHOW_FILE_SIZE))
        fmt::print("  ");
    else
        fmt::print("{}", space);

    if (PRINT_PURE)
        fmt::print("{}", time);
    else if (time.find("mod") != std::string::npos)
        fmt::print(FG(options->mod_time_color) | BG(options->mod_time_bg_color), "{}", time);
    else
        fmt::print(FG(options->creat_time_color) | BG(options->creat_time_bg_color), "{}", time);
}
void InnerPrinter::show_permissions(const std::string& entry, unmutable options)
{
    auto p = fs::status(entry).permissions();
    using std::filesystem::perms;
    auto show = [=](char op, perms perm)
    {
        auto _p = perms::none == (perm & p) ? '-' : op;
        if (PRINT_PURE)
            fmt::print("{}", _p);
        else
            fmt::print(FG(options->perm_color) | BG(options->perm_bg_color), "{}", _p);
    };

    fmt::print(" ");
    show('r', perms::owner_read);
    show('w', perms::owner_write);
    show('x', perms::owner_exec);
    show('r', perms::group_read);
    show('w', perms::group_write);
    show('x', perms::group_exec);
    show('r', perms::others_read);
    show('w', perms::others_write);
    show('x', perms::others_exec);
}
void InnerPrinter::printDirectoryTree(unmutable options, const fs::path& path, size_t level)
{
    for (const auto& entry : fs::directory_iterator(path))
    {
        auto entry_val = cut_quotas(entry.path().filename().string());

        if (fs::is_directory(entry))
        {
            fmt::print("{}", std::string(level, '-'));
            print_d(entry_val, options);
            fmt::println("");
            printDirectoryTree(options, entry, level + 1);
        }
        else
        {
            fmt::print("{}", std::string(level, ' '));
            print_f(entry_val, options);
            fmt::println("");
        }
    }
}
size_t InnerPrinter::compute_dir_elements_number(const std::string& path, bool rec)
{
    using std::filesystem::directory_iterator;
    using std::filesystem::recursive_directory_iterator;

    using fp = bool (*)(const std::filesystem::path&);

    //returns the sum of counted number of files and directories
    if (!rec)// the sum of counted number of files and subdirectories of current dir
        return std::count_if(directory_iterator(path), directory_iterator{}, (fp)std::filesystem::is_regular_file) +
        std::count_if(directory_iterator(path), directory_iterator{}, (fp)std::filesystem::is_directory);
    else // the total number of elements of directory tree with path as top dir
        return std::count_if(recursive_directory_iterator(path), recursive_directory_iterator{}, (fp)std::filesystem::is_regular_file) +
        std::count_if(recursive_directory_iterator(path), recursive_directory_iterator{}, (fp)std::filesystem::is_directory);
}
std::string InnerPrinter::print_size(const std::string& dir_entry,unmutable options)
{
    auto f = fs::file_size(dir_entry);
    auto size = convert_bytes(f);
    if(PRINT_PURE)
        fmt::print("{}", size);
    else
        fmt::print(FG(options->file_size_color) | BG(options->file_size_bg_color),"{}", size);

    return size;
}
