// src/tools/workspace.cpp — YOU IMPLEMENT (spec §4.4).
//
// Workspace::resolve: a purely lexical check, steps 1–6 of the spec.
// Do not touch the filesystem here (no exists(), canonical(), ...).
// The constructor, root() and check_symlinks() are provided in
// workspace_provided.cpp.

#include "tools/workspace.h"

namespace fs = std::filesystem;



bool Workspace::resolve(std::string_view user_path, fs::path* out,
                        std::string* reason) const {

    if (user_path.empty())
    {
     
        *reason = "user_path has no value";
        return false;
    }

    if (user_path.find('\0') != std::string_view::npos) //there are no null characters before the terminating character
    {
        *reason = "user_path contains a null character";
        return false;
    }
    
    std::filesystem::path p = std::string(user_path);
    if (p.is_absolute())
    {
        *reason = "user_path is an absolute path. Only use paths relative to the workspace";
        return false;
    }
    if (p.has_root_path())
    {
        *reason = "user_path contains the root path. Only use paths relative to the workspace";   
        return false;
    }
    
    std::filesystem::path n = p.lexically_normal();
    if (*n.begin() == "..")
    {
        *reason = "user_path goes above root";
        return false;
    }

    if (n == ".")
    {
        *out = root();
        return true;
    }
    else
    {
        *out = root() / n;
        return true;
    }
}
