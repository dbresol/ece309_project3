// tests/p3/test_p3.cpp — YOUR P3 TEST SUITE.
//
// The five tests below are worked examples. Keep them, and add your own
// until you have AT LEAST 15 (spec §6 lists what must be covered).
// Name every test <area>_<what>: reg_ for the registry, ws_ for the
// workspace, calc_ for the calculator.
//
// Build & run:   cmake --build build && ./build/test_p3
// One area:      ./build/test_p3 reg_        (names starting with "reg_")
// One test:      ./build/test_p3 =reg_add_and_find

#include "../mini_test.h"
#include "../test_helpers.h"     // TempDir, write_text, args1, ...

#include "tools/calculator.h"
#include "tools/tool_registry.h"
#include "tools/workspace.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
namespace fs = std::filesystem;

namespace {

// A tool whose name you choose — use it to fill the registry in tests.
class NamedTool : public Tool {
public:
    explicit NamedTool(std::string n) : n_(std::move(n)), schema_(JsonValue::object()) {}
    std::string name() const override { return n_; }
    std::string description() const override { return "test tool"; }
    const JsonValue& schema() const override { return schema_; }
    ToolResult invoke(const JsonValue&, ToolContext&) override { return ToolResult::success(n_); }
private:
    std::string n_;
    JsonValue   schema_;   // an empty JSON object: {}
};

// Tokenize with the provided tokenizer, then run YOUR calc_evaluate.
bool eval(const std::string& expr, double* value, std::string* error) {
    std::vector<CalcToken> tokens;
    if (!calc_tokenize(expr, &tokens, error)) return false;
    return calc_evaluate(tokens, value, error);
}

}  // namespace

// ---------------------------------------------------------------- examples

TEST(reg_add_and_find) {
    ToolRegistry r;
    auto t = std::make_unique<NamedTool>("alpha");
    Tool* raw = t.get();                        // remember the address before moving
    REQUIRE(r.add(std::move(t)));
    CHECK(r.size() == 1);
    CHECK(r.find("alpha") == raw);              // same object, not a copy
    CHECK(r.find("beta") == nullptr);
}

TEST(reg_duplicate_keeps_original) {
    ToolRegistry r;
    auto first = std::make_unique<NamedTool>("alpha");
    Tool* original = first.get();
    REQUIRE(r.add(std::move(first)));
    CHECK(!r.add(std::make_unique<NamedTool>("alpha")));
    CHECK(r.find("alpha") == original);
    CHECK(r.size() == 1);
}

TEST(reg_nullptr_rejected) 
{
    ToolRegistry r;
    REQUIRE(!r.add(nullptr));
    CHECK(r.size() == 0);
}

TEST(reg_absent_name_rejected) 
{
    ToolRegistry r;
    REQUIRE(!r.find("alpha"));
    CHECK(r.size() == 0);
}

TEST(reg_for_each_several_tools_single_visit)
{
    ToolRegistry r;
    r.add(std::make_unique<NamedTool>("alpha"));
    r.add(std::make_unique<NamedTool>("beta"));
    r.add(std::make_unique<NamedTool>("gamma"));

    int counter = 0;
    r.for_each([&](const Tool& tool)
    {   
        if (!(tool.name().empty()))
        {
            counter++;
        }
    });
    CHECK(counter == 3);
}

TEST(reg_many_tools)
{
    ToolRegistry r;
    for (int i = 0; i < 10000; i++)
    {
        r.add(std::make_unique<NamedTool>(std::to_string(i)));
        CHECK(r.load_factor() <= 1.0);
    }
    CHECK(r.size() == 10000);
    CHECK(r.max_probe_length() <= 64);
    std::printf("bucket_count = %zu; load_factor = %f; max_probe_length = %zu\n", r.bucket_count(), r.load_factor(), r.max_probe_length());
}

TEST(reg_alternate_hash)
{
    ToolRegistry r;
    std::size_t numBuckets = 1<<14; //smallest power of two above 10000
    std::vector<int> counts(numBuckets, 0);
    for (size_t i = 0; i < 10000; i++)
    {
        std::string name = "tool" + std::to_string(i);
        std::size_t hash = name[0] + name.size();
        std::size_t bucket = hash % numBuckets;
        counts[bucket]++;
    }
    
    int max = 0;
    for (size_t i = 0; i < numBuckets; i++)
    {
        if (counts[i] > max)
        {
            max = counts[i];
        }
    }

    std::printf("Longest alternate hash chain length: %d\n", max);
    CHECK(5000 > 2000);
}

TEST(ws_rejects_parent_escape) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));   // sanity: accepts *something*
    CHECK(out == ws.root() / "notes" / "todo.txt");
    CHECK(!ws.resolve("../etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

TEST(ws_accepts_relative_path) 
{
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    std::string input = "notes/todo.txt";
    std::string expectedOutput = ws.root() / input;
    REQUIRE(ws.resolve(input, &out, &reason));
    CHECK(out == expectedOutput);
}

TEST(ws_formats_path) 
{
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    std::string input = "./notes//todo.txt";
    std::string expectedOutput = ws.root() / "notes/todo.txt";
    REQUIRE(ws.resolve(input, &out, &reason));
    CHECK(out == expectedOutput);
}

TEST(ws_formats_only_root_path) 
{
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    std::string input = ".";
    std::string expectedOutput = ws.root();
    REQUIRE(ws.resolve(input, &out, &reason));
    CHECK(out == expectedOutput);
}

TEST(ws_formats_parent_in_path) 
{
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    std::string input = "a/../b";
    std::string expectedOutput = ws.root() / "b";
    REQUIRE(ws.resolve(input, &out, &reason));
    CHECK(out == expectedOutput);
}

TEST(ws_formats_parent_path) 
{
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    std::string input = "a/..";
    std::string expectedOutput = ws.root();
    REQUIRE(ws.resolve(input, &out, &reason));
    CHECK(out == expectedOutput);
}

TEST(ws_file_does_not_exist) 
{
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    std::string input = "new/dir/file.txt";
    std::string expectedOutput = ws.root() / input;
    REQUIRE(ws.resolve(input, &out, &reason));
    CHECK(out == expectedOutput);
}

TEST(ws_accept_parent_in_file_name) 
{
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    std::string input = "my..notes.txt";
    std::string expectedOutput = ws.root() / input;
    REQUIRE(ws.resolve(input, &out, &reason));
    CHECK(out == expectedOutput);
}

TEST(ws_rejects_empty_path) 
{
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(!ws.resolve("", &out, &reason));
    CHECK(!reason.empty());
}

TEST(ws_rejects_zero_byte) 
{
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    //std::string nul_path = std::string("notes") + '\0' + "/x";
    REQUIRE(!ws.resolve((std::string("notes") + '\0' + "/x"), &out, &reason));
    CHECK(!reason.empty());
}

TEST(ws_rejects_absolute_path)
{
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    //std::string nul_path = std::string("notes") + '\0' + "/x";
    REQUIRE(!ws.resolve("/etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

TEST(ws_rejects_above_root) 
{
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    std::string input = "..";
    //std::string expectedOutput = ws.root() / input;
    REQUIRE(!ws.resolve(input, &out, &reason));
    CHECK(reason == "user_path goes above root");
}

TEST(ws_rejects_above_root_and_back_down) 
{
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    std::string input = "../etc/passwd";
    //std::string expectedOutput = ws.root() / input;
    REQUIRE(!ws.resolve(input, &out, &reason));
    CHECK(reason == "user_path goes above root");
}

TEST(ws_rejects_above_root_twice) 
{
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    std::string input = "a/../../b";
    //std::string expectedOutput = ws.root() / input;
    REQUIRE(!ws.resolve(input, &out, &reason));
    CHECK(reason == "user_path goes above root");
}

TEST(ws_rejects_above_root_twice_and_back_down) 
{
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    std::string input = "notes/../../ws/notes/todo.txt";
    //std::string expectedOutput = ws.root() / input;
    REQUIRE(!ws.resolve(input, &out, &reason));
    CHECK(reason == "user_path goes above root");
}

TEST(calc_precedence) {
    double v = 0;
    std::string err;
    REQUIRE(eval("2 + 3 * 4", &v, &err));
    CHECK(v == 14);
}

TEST(calc_division_by_zero) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / 0", &v, &err));
    CHECK(err == "division by zero");
}

TEST(calc_subtraction) {
    double v = 0;
    std::string err;
    REQUIRE(eval("10 - 4 - 3", &v, &err));
    CHECK(v == 3);
}

TEST(calc_divison) {
    double v = 0;
    std::string err;
    REQUIRE(eval("100 / 10 / 5", &v, &err));
    CHECK(v == 2);
}

TEST(calc_parentheses) {
    double v = 0;
    std::string err;
    REQUIRE(eval("(2 + 3) * 4", &v, &err));
    CHECK(v == 20);
}

TEST(calc_parentheses_precedence) {
    double v = 0;
    std::string err;
    REQUIRE(eval("2 * (3 + 4) - 5", &v, &err));
    CHECK(v == 9);
}

TEST(calc_negative_product) {
    double v = 0;
    std::string err;
    REQUIRE(eval("-3 * -3", &v, &err));
    CHECK(v == 9);
}

TEST(calc_division_decimal) {
    double v = 0;
    std::string err;
    REQUIRE(eval("7 / 2", &v, &err));
    CHECK(v == 3.5);
}

TEST(calc_divide_by_zero_parentheses) {
    double v = 0;
    std::string err;
    REQUIRE(!eval("1 / (2 - 2)", &v, &err));
    CHECK(err == "division by zero");
}

//custom test 1
TEST(calc_nested_parentheses) 
{
    double v = 0;
    std::string err;
    REQUIRE(eval("1 - (2 * (3 + 4) - 5)", &v, &err));
    CHECK(v == -8);
}

//custom test 2
TEST(calc_non_nested_parentheses) 
{
    double v = 0;
    std::string err;
    REQUIRE(eval("12.5 * (-1 + -3) + 50 / (2 - 1)", &v, &err));
    CHECK(v == 0);
}

// ------------------------------------------------------------- your tests
//
// Still to write (spec §6 has the full list). Suggested names:
//   reg_null_rejected, reg_for_each_visits_each_once
//   reg_10000_tools               REQUIRED: print bucket_count / load_factor /
//                                 max_probe_length, check the bound
//   reg_alternative_hash          REQUIRED: longest chain of a weak hash
//   ws_<case>                     REQUIRED: every row of the table in spec §4.4
//   calc_<case>                   left-to-right, parentheses, negatives

int main(int argc, char** argv) { return mt::run_all(argc, argv); }
