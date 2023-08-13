#include"Printer.h"

std::string PrinterInnerFn::cut_quotas(const std::string& str)
{
    return str.substr(0, str.size());
}
void PrinterInnerFn::erase_sub(std::string& str, const std::string& sub)
{
    auto pos = str.find(sub);
    if (pos != std::string::npos)
    {
        str.erase(pos, sub.length());
    }
}
void PrinterInnerFn::iterate_over_dir(const Options const* options,
                  const fn& iterate,
                  const fn1& iterate_sorted)
{
    std::list<std::string> dirs, files;

    for (const fs::directory_entry& dir_entry : fs::directory_iterator(options->dir))
    {
        iterate(dir_entry, dirs, files);
    }

    if (options->sort)
        iterate_sorted(dirs, files);
}
void PrinterInnerFn::iterate_over_dir_recursively(const Options const* options,
                    const fn& iterate,
                    const fn1& iterate_sorted)
{
    std::list<std::string> dirs, files;

    for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(options->dir))
    {
        iterate(dir_entry, dirs, files);
    }

    if (options->sort)
        iterate_sorted(dirs, files);
}
size_t PrinterInnerFn::get_max_dir_str_size(const std::string& dir,const Options const* options)
{
    std::vector<size_t> sizes;
    for (const fs::directory_entry& dir_entry : fs::directory_iterator(dir))
    {
        sizes.push_back(PrinterInnerFn::prepare_entry_val(dir_entry, options).size());
    }

    return *std::max_element(sizes.begin(), sizes.end());
}
size_t PrinterInnerFn::get_max_dir_str_size_recursivly(const std::string& dir, const Options const* options)
{
    std::vector<size_t> sizes;
    for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(dir))
    {
        sizes.push_back(PrinterInnerFn::prepare_entry_val(dir_entry, options).size());
    }

    return *std::max_element(sizes.begin(), sizes.end());
}

void Printer::print_as_list(const Options const* options)
{
    auto iterate = [&](const fs::directory_entry& dir_entry,
        std::list<std::string>& dirs,
        std::list<std::string>& files)
    {
        namespace in = PrinterInnerFn;

        size_t max_size = options->show_file_size ?
            options->recursive ? in::get_max_dir_str_size_recursivly(options->dir, options) :
            in::get_max_dir_str_size(options->dir, options) : 1;

        auto entry_val = in::prepare_entry_val(dir_entry, options);
        //if shouldn't sort just print
        if (options->sort)
        {
            if (dir_entry.is_directory())
                dirs.push_back(colorize(in::cut_quotas(entry_val) + "/",options->dir_color,options->dir_bg_color));
            else
                files.push_back(colorize(in::cut_quotas(entry_val), options->file_color, options->file_bg_color));
                
        }
        else
        {
            if (options->show_only_dirs and dir_entry.is_directory())
            {
                std::cout << colorize(entry_val + "/",options->dir_color,options->dir_bg_color) << std::endl;
            }
            else if (options->show_only_files and !dir_entry.is_directory())
            {
                auto mult_val = max_size == 1 ? 1 : (max_size - entry_val.size()) + 1;
                std::cout << colorize(entry_val, 
                                      options->file_color, 
                                      options->file_bg_color) 
                     << in::mult_str(" ",mult_val);

                if (options->show_file_size)
                    std::cout << in::HumanReadable{fs::file_size(dir_entry)} << std::endl;
                else
                    std::cout << std::endl;
            }
        }
    };
    
    auto iterate_sorted = [&](std::list<std::string>& dirs,
                             std::list<std::string>& files)
    {
        namespace in = PrinterInnerFn;
        size_t max_size = options->show_file_size ?
            options->recursive ? in::get_max_dir_str_size_recursivly(options->dir, options) :
            in::get_max_dir_str_size(options->dir, options) : 1;
       
        bool show_size = false;
        auto print = [&](const auto& arg) 
        { 
            auto mult_val = max_size == 1 ? 1 : (max_size - arg.size()) + 1;
            std::cout <<arg << PrinterInnerFn::mult_str(" ",max_size);
            if (options->show_file_size and show_size)
                std::cout << PrinterInnerFn::HumanReadable{fs::file_size(arg)} << std::endl;
            else std::cout << std::endl;
        };
        for (auto& ch : options->sorting_order)
        {
            if (ch == 'd')
                std::for_each(dirs.begin(), dirs.end(), print);
            if (ch == 'f')
            {
                show_size = true;
                std::for_each(files.begin(), files.end(), print);
            }
        }
    };

    if (options->recursive)
    {
        PrinterInnerFn::iterate_over_dir_recursively(options, iterate, iterate_sorted);
    }
    else
        PrinterInnerFn::iterate_over_dir(options, iterate, iterate_sorted);


}
void Printer::print_as_table(const Options const* options)
{
    size_t counter = 0;
    auto iterate = [&](const fs::directory_entry& dir_entry,
        std::list<std::string>& dirs,
        std::list<std::string>& files)
    {
        namespace in = PrinterInnerFn;
        auto entry_val = in::prepare_entry_val(dir_entry, options);

        if (options->sort)
        {
            if (dir_entry.is_directory())
                dirs.push_back(colorize(in::cut_quotas(entry_val) + "/",options->dir_color,options->dir_bg_color));
            else
                files.push_back(colorize(in::cut_quotas(entry_val),options->file_color,options->file_bg_color));
        }
        else
        {
            auto separator = counter == options->table_output_width ? '\n' : ' ';
            if (separator == '\n')counter = 0;

            if (options->show_only_dirs and dir_entry.is_directory())
                std::cout << colorize(entry_val,options->dir_color,options->dir_bg_color) + "/" << separator;
            else if (options->show_only_files and !dir_entry.is_directory())
                std::cout << colorize(entry_val,options->file_color,options->file_bg_color) << separator;

            counter++;
        }
    };

    auto iterate_sorted = [&](std::list<std::string>& dirs,
        std::list<std::string>& files)
    {
        counter = 0;

        char separator = ' ';
        auto print = [&](const auto& arg)
        {
            separator = counter == options->table_output_width ? '\n' : ' ';
            if (separator == '\n')counter = 0;

            std::cout << arg << separator;
            counter++;
        };
        for (auto& ch : options->sorting_order)
        {
            if (ch == 'd')
                std::for_each(dirs.begin(), dirs.end(), print);
            if (ch == 'f')
                std::for_each(files.begin(), files.end(), print);
        }
    };
   
    if (options->recursive)
    {
        PrinterInnerFn::iterate_over_dir_recursively(options, iterate, iterate_sorted);
    }
    else
        PrinterInnerFn::iterate_over_dir(options, iterate, iterate_sorted);
}