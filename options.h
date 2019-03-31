#pragma once
#include <string>
#include <map>
#include <vector>

namespace options {
    struct option {
        bool required;
        bool multiple;
        bool value_required;
        std::string name;
        std::string short_name;
    };

    struct parse_result {
        std::map<std::string, std::vector<std::string>> options;
        std::vector<std::string> extra_args;

        bool has_opt(const std::string& opt) const {
            return options.count(opt) != 0;
        }
    };

    struct parse_result parse(const std::vector<option>& opts, int argc, const char** argv) 
    {
        struct parse_result results;
        for(int i=1; i<argc; i++) {
            bool found = false;
            for(auto& o : opts) {
                if("--" + o.name == argv[i] || "-" + o.short_name == argv[i]) {
                    if(results.options.count(o.name) != 0 && !o.multiple) throw std::invalid_argument(o.name + " can only be specified once");
                    if(o.value_required) {
                        if(i == argc -1) throw std::invalid_argument("expected argument for " + o.name);
                        i++;
                        results.options[o.name].push_back(argv[i]);
                    } else results.options[o.name].push_back(argv[i]);
                    found = true;
                    break;
                }
            }
            if(!found) results.extra_args.push_back(argv[i]);
        }
        for(auto& o : opts) {
            if(o.required && results.options.count(o.name) == 0) throw std::invalid_argument("required argument " + o.name + " not found");
        }
        return results;
    }
}
