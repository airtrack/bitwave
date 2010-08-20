#ifndef _UNIT_TEST_H_
#define _UNIT_TEST_H_

#include <algorithm>
#include <string>
#include <vector>
#include <ostream>
#include <sstream>
#include <iostream>
#include <iterator>

class TestResultReporter;

class TestErrorsCollector
{
public:
    void Register(TestResultReporter *reporter)
    {
        reporters_.push_back(reporter);
    }

    void RunCases(std::ostream& os);

private:
    typedef std::vector<TestResultReporter *> Reporters;
    Reporters reporters_;
}TestCollector;

class TestResultReporter
{
protected:
    TestResultReporter()
    {
        TestCollector.Register(this);
    }

public:
    virtual void ReportErrors(
            std::string& casename,
            std::vector<std::string>& errors) = 0;
};

void TestErrorsCollector::RunCases(std::ostream& os = std::cout)
{
    std::size_t failed = 0;
    for (Reporters::iterator it = reporters_.begin(); it != reporters_.end(); ++it)
    {
        std::string casename;
        std::vector<std::string> errors;
        (*it)->ReportErrors(casename, errors);
        if (errors.empty())
        {
            os << "[" << casename << "]" << " passed!" << std::endl;
        }
        else
        {
            os << "[" << casename << "]" << " failed:" << std::endl;
            std::copy(errors.begin(), errors.end(),
                    std::ostream_iterator<std::string>(os, "\n"));
            ++failed;
        }
    }

    os << reporters_.size() << " cases has been run, passed: "
       << reporters_.size() - failed << ", failed: " << failed << std::endl;
}

#define TEST_CASE(case_name)                                    \
    class test_##case_name : public TestResultReporter          \
    {                                                           \
    public:                                                     \
        test_##case_name();                                     \
        virtual void ReportErrors(                              \
                std::string& casename,                          \
                std::vector<std::string>& errors)               \
        {                                                       \
            casename = #case_name;                              \
            errors = errors_;                                   \
        }                                                       \
        void RecordError(const std::string& expression,         \
                const std::string& file, int line)              \
        {                                                       \
            std::ostringstream oss;                             \
            oss << "\"" + expression +  "\" fail at file: "     \
                << file + " line: " << line;                    \
            errors_.push_back(oss.str());                       \
        }                                                       \
        std::vector<std::string> errors_;                       \
    }test_##case_name##_object;                                 \
                                                                \
    test_##case_name::test_##case_name()

#define CHECK_TRUE(expression)                                  \
    (expression) ? static_cast<void>(0) :                       \
    RecordError(#expression, __FILE__, __LINE__)

#endif // _UNIT_TEST_H_
