#include <cstdlib>
#include <cstring> 
#include <array>
#include <vector>
#include <chrono>
#include <iostream>
#include <memory>
#include <numeric>

template<std::size_t N>
void fillData (std::array<double, N>& data)
{
    for (auto& d : data)
        d = (rand() / (double) RAND_MAX);
}

//==============================================================================
struct ValueBase
{
    virtual ~ValueBase() {}

    using DataRange = std::pair<const double*, std::size_t>;
    virtual DataRange getData() const = 0;

    double count() const
    {
        auto data = getData();

        return std::accumulate (data.first, data.first + data.second, 0.0);
    }
};

template<std::size_t N>
struct ValueDerived : public ValueBase
{
    ValueDerived() { fillData (data); }

    DataRange getData() const override { return { data.data(), data.size() }; }
    std::array<double, N> data;
};

/** Use some 'real-world-like' struct sizes. */
static constexpr std::size_t struct1 = 81;
static constexpr std::size_t struct2 = 162;
static constexpr std::size_t struct3 = 243;

using ValueType0 = ValueDerived<struct1>;
using ValueType1 = ValueDerived<struct2>;
using ValueType2 = ValueDerived<struct3>;

//==============================================================================
/** My dodgy variant - no std::variant in my version of Xcode, plus it's nice
    to have all the machinary visible, because I'm interested in the perf of
    the memory access patterns, not std::variant specifically.
    Obviously this is a super-dangerous footgun, not how you'd do this in reality!
*/
struct TaggedUnion
{
    TaggedUnion (ValueType0 t) { value.t0 = t; tag = Tag::type0; }
    TaggedUnion (ValueType1 t) { value.t1 = t; tag = Tag::type1; }
    TaggedUnion (ValueType2 t) { value.t2 = t; tag = Tag::type2; }

    double count() const
    {
        switch (tag)
        {
            case Tag::type0:
                return std::accumulate (value.t0.data.begin(), value.t0.data.end(), 0.0);

            case Tag::type1:
                return std::accumulate (value.t1.data.begin(), value.t1.data.end(), 0.0);

            case Tag::type2:
                return std::accumulate (value.t2.data.begin(), value.t2.data.end(), 0.0);

            default: return 0.0;
        }
    }

    enum class Tag
    {
        type0,
        type1,
        type2
    };

    union Value
    {
        Value() { new (&t0) ValueType0(); }
        Value (const Value& o) { std::memcpy (this, &o, sizeof (Value)); }
        ~Value() { }

        ValueType0 t0;
        ValueType1 t1;
        ValueType2 t2;
    };

    Value value;
    Tag tag;
};

//==============================================================================
int main (int argc, char* argv[])
{
    std::vector<std::unique_ptr<ValueBase>> ptrVec;
    std::vector<std::unique_ptr<ValueBase>> heapFragmenter;
    std::vector<TaggedUnion> unionVec;

    constexpr std::size_t vecLength = 1000000;
    constexpr int runThroughs = 5;

    ptrVec.reserve (vecLength);
    heapFragmenter.reserve (vecLength / 2);
    unionVec.reserve (vecLength);

    for (std::size_t i = 0; i < vecLength; ++i)
    {
        const auto rand1 = rand() % 3;
        const auto rand2 = rand() % 2;

        switch (rand1)
        {
            case 0:
                ptrVec.emplace_back (std::make_unique<ValueType0>());
                unionVec.push_back (ValueType0());
                break;

            case 1:
                ptrVec.emplace_back (std::make_unique<ValueType1>());
                unionVec.push_back (ValueType1());
                break;

            case 2:
                ptrVec.emplace_back (std::make_unique<ValueType2>());
                unionVec.push_back (ValueType2());
                break;

            default:
                break;
        }

        if (rand2 == 0)
            heapFragmenter.emplace_back (std::make_unique<ValueType2>());
    }

    double ptrThrowawayResult = 0.0;
    double unionThrowawayResult = 0.0;
    double ptrTimeUs = 0.0;
    double unionTimeUs = 0.0;

    /** VECTOR OF POINTERS */
    for (int i = 0; i < runThroughs; ++i)
    {
        const auto start = std::chrono::high_resolution_clock::now();

        for (auto& ptr : ptrVec)
            ptrThrowawayResult += ptr->count();

        const auto end = std::chrono::high_resolution_clock::now();
        const auto elapsed = std::chrono::duration<double, std::micro>(end - start).count();

        ptrTimeUs = (i == 0) ? elapsed : (ptrTimeUs + elapsed) * 0.5;
    }

    /** VECTOR OF TAGGED UNIONS */
    for (int i = 0; i < runThroughs; ++i)
    {
        const auto start = std::chrono::high_resolution_clock::now();

        for (auto& un : unionVec)
            unionThrowawayResult += un.count();

        const auto end = std::chrono::high_resolution_clock::now();
        const auto elapsed = std::chrono::duration<double, std::micro>(end - start).count();

        unionTimeUs = (i == 0) ? elapsed : (unionTimeUs + elapsed) * 0.5;
    }

    std::cout << vecLength << " iterations. \n"
        << "struct sizes: " << struct1 << ", "<< struct2 << ", " << struct3 << " (doubles)\n-------- \n"
        << "vec of pointers took: " << ptrTimeUs << "us \n"
        << "vec of unions took: " << unionTimeUs << "us \n";

    std::cout << "ptrs result: " << ptrThrowawayResult << ", union result: " << unionThrowawayResult << "\n";

    return 0;
}
