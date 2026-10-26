// src/tools/tool_registry.cpp — YOU IMPLEMENT (spec §4.3).
//
// A hand-written hash table. No std::unordered_map / std::map / std::set.
// Every function below is a stub so the starter compiles. Replace each body.

#include "tools/tool_registry.h"

//ToolRegistry::ToolRegistry() = default;    // TODO: create your initial buckets
ToolRegistry::ToolRegistry() : _buckets(16), _size(0) {};    // TODO: create your initial buckets
ToolRegistry::~ToolRegistry() = default;   // unique_ptrs in your buckets free the tools

std::size_t ToolRegistry::hash(std::string_view key) noexcept {
    std::uint64_t h = 1469598103934665603ULL;   // "offset basis"
    for (unsigned char c : key) 
    { 
        h ^= c; h *= 1099511628211ULL; // "FNV prime"
    }     
    return static_cast<std::size_t>(h);
}

bool ToolRegistry::add(std::unique_ptr<Tool> tool) {

    //if the tool does not exist
    if (tool == nullptr) 
    {
        return false;
    }
    else if (find(tool->name()) != nullptr) //if the tool is found in the buckets
    {
        return false;
    }
    else if (load_factor() > 0.9) //if the load factor is above 0.9, resize
    {
        //double the size and create a new, empty table
        std::size_t newSize = size() * 2;
        std::vector<std::vector<std::unique_ptr<Tool>>> newBuckets(newSize);

        //for each bucket...
        //it can't just use the for_each function because that only returns 
        //the stored pointer, not the unique_ptr
        for (std::size_t i = 0; i < bucket_count(); i++ )
        {
            std::vector<std::unique_ptr<Tool>>& bucket = _buckets[i];
            //look through chain for a match
            for (std::size_t j = 0; j < bucket.size(); j++) 
            {
                std::unique_ptr<Tool>& tool = bucket[j];
                //if it exists, just in case
                if (tool)
                {
                    //calculate a new bucket and move tool to it without destruction
                    std::size_t newId = hash(tool->name()) % newSize;
                    newBuckets[newId].push_back(std::move(tool));
                }
                
            }
        }
        //move new buckets to where the old buckets were
        _buckets = std::move(newBuckets);
    }
    
    //calculate bucket and add new tool to it
    std::size_t bucketId = hash(tool.get()->name()) % bucket_count();
    _buckets[bucketId].push_back(std::move(tool));
    _size++;
    return true;
}

//find a tool
Tool* ToolRegistry::find(std::string_view name) const {
    //calculate the bucket
    std::size_t bucketId = hash(name) % bucket_count();
    const std::vector<std::unique_ptr<Tool>>& bucket = _buckets[bucketId];

    //look through chain for a match
    for (size_t i = 0; i < bucket.size(); i++) 
    {
        //if it's a match, return the stored pointer
        if (bucket[i].get()->name() == name)
        {
            return bucket[i].get();
        }
    }
    return nullptr;
}

std::size_t ToolRegistry::size() const noexcept {
    return _size;
}

void ToolRegistry::for_each(const std::function<void(const Tool&)>& fn) const {
    //for each bucket...
    for (std::size_t i = 0; i < bucket_count(); i++ )
    {
        const std::vector<std::unique_ptr<Tool>>& bucket = _buckets[i];
        //look through chain for a match
        for (std::size_t j = 0; j < bucket.size(); j++) 
        {
            //execute provided function
            fn(*(bucket[j].get()));
        }
    }
}

std::size_t ToolRegistry::bucket_count() const noexcept {
   return _buckets.size();
}

double ToolRegistry::load_factor() const noexcept {
    //convert to doubles first so that calculation doesn't always give 0.0
    double lf = static_cast<double>(size()) / static_cast<double>(bucket_count());
    return lf;
}

std::size_t ToolRegistry::max_probe_length() const {
    std::size_t max = 0;
    //for each bucket...
    for (std::size_t i = 0; i < bucket_count(); i++)
    {
        //find the max chain length
        if (max < _buckets[i].size()) 
        {
            max = _buckets[i].size();
        }
    }
    return max;
}
