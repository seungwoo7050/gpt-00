# 24. STL 알고리즘 완벽 가이드 🔍

*LogCaster 프로젝트를 위한 사전지식 문서*

---

## 📋 문서 정보
- **난이도**: 🟡 중급 → 🔴 고급
- **예상 학습 시간**: 12-15시간
- **전제 조건**: 
  - C++ 기본 문법과 STL 컨테이너
  - Iterator 개념 이해
  - Lambda 함수 기본 지식
- **실습 환경**: C++11 이상 (C++17/20 권장), GCC 9+/Clang 10+

## 🎯 이 문서에서 배울 내용

**"STL 알고리즘으로 더 우아하고 효율적인 코드 작성하기! ⚡"**

STL 알고리즘은 C++의 **숨겨진 보물창고**입니다. 마치 **전문 요리사의 칼**처럼, 올바르게 사용하면 복잡한 데이터 처리를 간단하고 효율적으로 만들 수 있습니다.

---

## 📚 1. STL 알고리즘 기초

### 알고리즘 카테고리 이해

```cpp
#include <algorithm>
#include <numeric>
#include <functional>
#include <iterator>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>

// LogCaster에서 STL 알고리즘 기초 활용
struct LogEntry {
    std::string message;
    std::string level;
    int priority;
    std::chrono::system_clock::time_point timestamp;
    
    LogEntry(std::string msg, std::string lvl, int prio = 1)
        : message(std::move(msg)), level(std::move(lvl)), priority(prio)
        , timestamp(std::chrono::system_clock::now()) {}
        
    bool operator<(const LogEntry& other) const {
        return priority < other.priority;
    }
    
    bool operator==(const LogEntry& other) const {
        return message == other.message && level == other.level;
    }
};

class STLBasicsDemo {
private:
    std::vector<LogEntry> logs_;
    
public:
    void setupTestData() {
        logs_ = {
            LogEntry("서버 시작", "INFO", 1),
            LogEntry("데이터베이스 연결", "DEBUG", 0),
            LogEntry("사용자 로그인", "INFO", 1),
            LogEntry("메모리 부족", "WARN", 2),
            LogEntry("연결 실패", "ERROR", 3),
            LogEntry("시스템 다운", "CRITICAL", 4),
            LogEntry("복구 시작", "INFO", 1),
            LogEntry("백업 완료", "INFO", 1)
        };
    }
    
    void demonstrateAlgorithmCategories() {
        setupTestData();
        
        std::cout << "=== STL 알고리즘 카테고리별 예시 ===" << std::endl;
        
        // 1. 비수정 순차 연산 (Non-modifying sequence operations)
        demonstrateSearchOperations();
        
        // 2. 수정 순차 연산 (Modifying sequence operations)  
        demonstrateModifyingOperations();
        
        // 3. 분할 연산 (Partitioning operations)
        demonstratePartitioningOperations();
        
        // 4. 정렬 연산 (Sorting operations)
        demonstrateSortingOperations();
        
        // 5. 이진 검색 연산 (Binary search operations)
        demonstrateBinarySearchOperations();
        
        // 6. 집합 연산 (Set operations)
        demonstrateSetOperations();
        
        // 7. 누적 연산 (Numeric operations)
        demonstrateNumericOperations();
    }
    
private:
    void demonstrateSearchOperations() {
        std::cout << "\n--- 검색 연산 ---" << std::endl;
        
        // find: 조건에 맞는 첫 번째 요소 찾기
        auto error_it = std::find_if(logs_.begin(), logs_.end(),
            [](const LogEntry& log) { return log.level == "ERROR"; });
        
        if (error_it != logs_.end()) {
            std::cout << "첫 번째 ERROR 로그: " << error_it->message << std::endl;
        }
        
        // count_if: 조건에 맞는 요소 개수
        auto info_count = std::count_if(logs_.begin(), logs_.end(),
            [](const LogEntry& log) { return log.level == "INFO"; });
        std::cout << "INFO 로그 개수: " << info_count << std::endl;
        
        // all_of, any_of, none_of: 조건 검사
        bool all_have_message = std::all_of(logs_.begin(), logs_.end(),
            [](const LogEntry& log) { return !log.message.empty(); });
        std::cout << "모든 로그에 메시지 있음: " << all_have_message << std::endl;
        
        bool has_critical = std::any_of(logs_.begin(), logs_.end(),
            [](const LogEntry& log) { return log.level == "CRITICAL"; });
        std::cout << "CRITICAL 로그 존재: " << has_critical << std::endl;
        
        // for_each: 각 요소에 대해 작업 수행
        std::cout << "모든 로그 레벨: ";
        std::for_each(logs_.begin(), logs_.end(),
            [](const LogEntry& log) { std::cout << log.level << " "; });
        std::cout << std::endl;
    }
    
    void demonstrateModifyingOperations() {
        std::cout << "\n--- 수정 연산 ---" << std::endl;
        
        // 복사본으로 작업
        auto logs_copy = logs_;
        
        // transform: 각 요소를 변환
        std::vector<std::string> formatted_logs;
        std::transform(logs_copy.begin(), logs_copy.end(), 
                      std::back_inserter(formatted_logs),
            [](const LogEntry& log) {
                return "[" + log.level + "] " + log.message;
            });
        
        std::cout << "변환된 첫 번째 로그: " << formatted_logs[0] << std::endl;
        
        // fill: 특정 값으로 채우기
        std::vector<int> priorities(5);
        std::fill(priorities.begin(), priorities.end(), 1);
        std::cout << "기본 우선순위로 채워짐" << std::endl;
        
        // generate: 함수를 호출하여 값 생성
        int counter = 0;
        std::generate(priorities.begin(), priorities.end(),
            [&counter]() { return ++counter; });
        
        std::cout << "생성된 우선순위: ";
        for (int p : priorities) std::cout << p << " ";
        std::cout << std::endl;
        
        // replace_if: 조건에 맞는 요소 교체
        std::for_each(logs_copy.begin(), logs_copy.end(),
            [](LogEntry& log) {
                if (log.level == "DEBUG") {
                    log.level = "TRACE";  // DEBUG를 TRACE로 변경
                }
            });
        
        std::cout << "DEBUG -> TRACE 변경 완료" << std::endl;
    }
    
    void demonstratePartitioningOperations() {
        std::cout << "\n--- 분할 연산 ---" << std::endl;
        
        auto logs_copy = logs_;
        
        // partition: 조건에 따라 분할 (순서는 보장되지 않음)
        auto partition_point = std::partition(logs_copy.begin(), logs_copy.end(),
            [](const LogEntry& log) { return log.priority >= 2; });
        
        std::cout << "높은 우선순위 로그 개수: " 
                  << std::distance(logs_copy.begin(), partition_point) << std::endl;
        
        // stable_partition: 조건에 따라 분할 (원래 순서 유지)
        logs_copy = logs_;  // 복원
        auto stable_partition_point = std::stable_partition(logs_copy.begin(), logs_copy.end(),
            [](const LogEntry& log) { return log.level == "ERROR" || log.level == "CRITICAL"; });
        
        std::cout << "심각한 로그들:" << std::endl;
        for (auto it = logs_copy.begin(); it != stable_partition_point; ++it) {
            std::cout << "  " << it->level << ": " << it->message << std::endl;
        }
        
        // is_partitioned: 분할 상태 확인
        bool is_partitioned = std::is_partitioned(logs_copy.begin(), logs_copy.end(),
            [](const LogEntry& log) { return log.level == "ERROR" || log.level == "CRITICAL"; });
        std::cout << "심각한 로그들로 분할됨: " << is_partitioned << std::endl;
    }
    
    void demonstrateSortingOperations() {
        std::cout << "\n--- 정렬 연산 ---" << std::endl;
        
        auto logs_copy = logs_;
        
        // sort: 기본 정렬 (우선순위 기준)
        std::sort(logs_copy.begin(), logs_copy.end());
        
        std::cout << "우선순위별 정렬된 로그들:" << std::endl;
        for (const auto& log : logs_copy) {
            std::cout << "  " << log.priority << ": " << log.message << std::endl;
        }
        
        // sort with custom comparator: 사용자 정의 비교자
        std::sort(logs_copy.begin(), logs_copy.end(),
            [](const LogEntry& a, const LogEntry& b) {
                return a.level < b.level;  // 레벨별 알파벳 순
            });
        
        std::cout << "\n레벨별 정렬:" << std::endl;
        for (const auto& log : logs_copy) {
            std::cout << "  " << log.level << ": " << log.message << std::endl;
        }
        
        // partial_sort: 부분 정렬 (상위 N개만)
        logs_copy = logs_;  // 복원
        std::partial_sort(logs_copy.begin(), logs_copy.begin() + 3, logs_copy.end(),
            [](const LogEntry& a, const LogEntry& b) {
                return a.priority > b.priority;  // 높은 우선순위 순
            });
        
        std::cout << "\n상위 3개 우선순위 로그:" << std::endl;
        for (int i = 0; i < 3; ++i) {
            std::cout << "  " << logs_copy[i].priority << ": " << logs_copy[i].message << std::endl;
        }
        
        // nth_element: N번째 요소 찾기
        logs_copy = logs_;  // 복원
        std::nth_element(logs_copy.begin(), logs_copy.begin() + logs_copy.size()/2, 
                        logs_copy.end(),
            [](const LogEntry& a, const LogEntry& b) {
                return a.priority < b.priority;
            });
        
        std::cout << "\n중간값 우선순위: " << logs_copy[logs_copy.size()/2].priority << std::endl;
        
        // is_sorted: 정렬 상태 확인
        bool sorted_by_priority = std::is_sorted(logs_.begin(), logs_.end(),
            [](const LogEntry& a, const LogEntry& b) {
                return a.priority <= b.priority;
            });
        std::cout << "원본이 우선순위로 정렬됨: " << sorted_by_priority << std::endl;
    }
    
    void demonstrateBinarySearchOperations() {
        std::cout << "\n--- 이진 검색 연산 ---" << std::endl;
        
        // 먼저 정렬 (이진 검색 전제조건)
        auto sorted_logs = logs_;
        std::sort(sorted_logs.begin(), sorted_logs.end());
        
        // binary_search: 요소 존재 여부
        LogEntry search_target("연결 실패", "ERROR", 3);
        bool found = std::binary_search(sorted_logs.begin(), sorted_logs.end(), search_target);
        std::cout << "타겟 로그 존재: " << found << std::endl;
        
        // lower_bound: 첫 번째 가능한 위치
        LogEntry priority_2("", "", 2);
        auto lower_it = std::lower_bound(sorted_logs.begin(), sorted_logs.end(), priority_2,
            [](const LogEntry& log, const LogEntry& target) {
                return log.priority < target.priority;
            });
        
        if (lower_it != sorted_logs.end()) {
            std::cout << "우선순위 2 이상의 첫 번째 로그: " << lower_it->message << std::endl;
        }
        
        // upper_bound: 마지막 가능한 위치 다음
        auto upper_it = std::upper_bound(sorted_logs.begin(), sorted_logs.end(), priority_2,
            [](const LogEntry& target, const LogEntry& log) {
                return target.priority < log.priority;
            });
        
        std::cout << "우선순위 2인 로그 개수: " 
                  << std::distance(lower_it, upper_it) << std::endl;
        
        // equal_range: lower_bound와 upper_bound를 함께
        auto range = std::equal_range(sorted_logs.begin(), sorted_logs.end(), priority_2,
            [](const LogEntry& a, const LogEntry& b) {
                return a.priority < b.priority;
            });
        
        std::cout << "우선순위 2 범위의 로그들:" << std::endl;
        for (auto it = range.first; it != range.second; ++it) {
            std::cout << "  " << it->message << std::endl;
        }
    }
    
    void demonstrateSetOperations() {
        std::cout << "\n--- 집합 연산 ---" << std::endl;
        
        std::vector<std::string> levels1 = {"DEBUG", "INFO", "WARN"};
        std::vector<std::string> levels2 = {"INFO", "WARN", "ERROR", "CRITICAL"};
        
        std::sort(levels1.begin(), levels1.end());
        std::sort(levels2.begin(), levels2.end());
        
        // set_union: 합집합
        std::vector<std::string> union_result;
        std::set_union(levels1.begin(), levels1.end(),
                      levels2.begin(), levels2.end(),
                      std::back_inserter(union_result));
        
        std::cout << "레벨 합집합: ";
        for (const auto& level : union_result) std::cout << level << " ";
        std::cout << std::endl;
        
        // set_intersection: 교집합
        std::vector<std::string> intersection_result;
        std::set_intersection(levels1.begin(), levels1.end(),
                             levels2.begin(), levels2.end(),
                             std::back_inserter(intersection_result));
        
        std::cout << "레벨 교집합: ";
        for (const auto& level : intersection_result) std::cout << level << " ";
        std::cout << std::endl;
        
        // set_difference: 차집합
        std::vector<std::string> difference_result;
        std::set_difference(levels1.begin(), levels1.end(),
                           levels2.begin(), levels2.end(),
                           std::back_inserter(difference_result));
        
        std::cout << "레벨 차집합 (levels1 - levels2): ";
        for (const auto& level : difference_result) std::cout << level << " ";
        std::cout << std::endl;
        
        // includes: 부분집합 확인
        std::vector<std::string> subset = {"INFO", "WARN"};
        std::sort(subset.begin(), subset.end());
        
        bool is_subset = std::includes(levels2.begin(), levels2.end(),
                                      subset.begin(), subset.end());
        std::cout << "subset이 levels2에 포함됨: " << is_subset << std::endl;
    }
    
    void demonstrateNumericOperations() {
        std::cout << "\n--- 수치 연산 ---" << std::endl;
        
        std::vector<int> priorities;
        std::transform(logs_.begin(), logs_.end(), std::back_inserter(priorities),
            [](const LogEntry& log) { return log.priority; });
        
        // accumulate: 누적 계산
        int total_priority = std::accumulate(priorities.begin(), priorities.end(), 0);
        std::cout << "총 우선순위 합: " << total_priority << std::endl;
        
        // accumulate with custom operation
        int max_priority = std::accumulate(priorities.begin(), priorities.end(), 0,
            [](int max_val, int current) { return std::max(max_val, current); });
        std::cout << "최대 우선순위: " << max_priority << std::endl;
        
        // transform_reduce: 변환하면서 누적 (C++17)
#ifdef __cpp_lib_parallel_algorithm
        auto priority_sum = std::transform_reduce(logs_.begin(), logs_.end(), 0,
            std::plus<>(),  // reduce operation
            [](const LogEntry& log) { return log.priority; }  // transform operation
        );
        std::cout << "transform_reduce 우선순위 합: " << priority_sum << std::endl;
#endif
        
        // inner_product: 내적 계산
        std::vector<int> weights = {1, 2, 1, 3, 4, 5, 1, 1};
        if (weights.size() == priorities.size()) {
            int weighted_sum = std::inner_product(priorities.begin(), priorities.end(),
                                                weights.begin(), 0);
            std::cout << "가중치 적용된 우선순위 합: " << weighted_sum << std::endl;
        }
        
        // partial_sum: 부분합 계산
        std::vector<int> cumulative_priorities;
        std::partial_sum(priorities.begin(), priorities.end(),
                        std::back_inserter(cumulative_priorities));
        
        std::cout << "누적 우선순위: ";
        for (int p : cumulative_priorities) std::cout << p << " ";
        std::cout << std::endl;
        
        // adjacent_difference: 인접 요소 차이
        std::vector<int> differences;
        std::adjacent_difference(priorities.begin(), priorities.end(),
                               std::back_inserter(differences));
        
        std::cout << "인접 우선순위 차이: ";
        for (size_t i = 1; i < differences.size(); ++i) {  // 첫 번째는 원본값
            std::cout << differences[i] << " ";
        }
        std::cout << std::endl;
    }
};
```

---

## 🔥 2. 고급 알고리즘 패턴

### 함수 객체와 람다 조합

```cpp
#include <functional>
#include <random>
#include <unordered_map>

// LogCaster에서 고급 알고리즘 패턴 활용
class AdvancedAlgorithmPatterns {
private:
    std::vector<LogEntry> logs_;
    
public:
    void setupLargeTestData() {
        logs_.clear();
        
        std::vector<std::string> levels = {"DEBUG", "INFO", "WARN", "ERROR", "CRITICAL"};
        std::vector<std::string> sources = {"web", "db", "cache", "auth", "network"};
        std::vector<std::string> actions = {"start", "stop", "connect", "disconnect", "process", "fail"};
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> level_dist(0, levels.size() - 1);
        std::uniform_int_distribution<> source_dist(0, sources.size() - 1);
        std::uniform_int_distribution<> action_dist(0, actions.size() - 1);
        std::uniform_int_distribution<> priority_dist(0, 4);
        
        for (int i = 0; i < 10000; ++i) {
            std::string message = sources[source_dist(gen)] + " " + actions[action_dist(gen)] + 
                                " #" + std::to_string(i);
            logs_.emplace_back(message, levels[level_dist(gen)], priority_dist(gen));
        }
    }
    
    void demonstrateFunctionObjectPatterns() {
        setupLargeTestData();
        
        std::cout << "=== 함수 객체와 람다 조합 패턴 ===" << std::endl;
        
        // 1. Predicate 조합
        auto is_high_priority = [](const LogEntry& log) { return log.priority >= 3; };
        auto is_error_or_critical = [](const LogEntry& log) { 
            return log.level == "ERROR" || log.level == "CRITICAL"; 
        };
        
        // AND 조합
        auto high_priority_and_critical = [=](const LogEntry& log) {
            return is_high_priority(log) && is_error_or_critical(log);
        };
        
        // OR 조합
        auto high_priority_or_critical = [=](const LogEntry& log) {
            return is_high_priority(log) || is_error_or_critical(log);
        };
        
        // NOT 조합
        auto not_high_priority = [=](const LogEntry& log) {
            return !is_high_priority(log);
        };
        
        auto critical_logs = std::count_if(logs_.begin(), logs_.end(), high_priority_and_critical);
        std::cout << "높은 우선순위 + 심각한 로그: " << critical_logs << std::endl;
        
        // 2. 함수 객체 클래스 활용
        struct LogLevelCounter {
            std::unordered_map<std::string, int> counts;
            
            void operator()(const LogEntry& log) {
                counts[log.level]++;
            }
        };
        
        auto counter = std::for_each(logs_.begin(), logs_.end(), LogLevelCounter{});
        std::cout << "레벨별 개수:" << std::endl;
        for (const auto& [level, count] : counter.counts) {
            std::cout << "  " << level << ": " << count << std::endl;
        }
        
        // 3. 템플릿 함수 객체
        template<typename Predicate>
        struct PredicateNegator {
            Predicate pred;
            
            PredicateNegator(Predicate p) : pred(p) {}
            
            template<typename T>
            bool operator()(const T& value) const {
                return !pred(value);
            }
        };
        
        auto negate_pred = [](auto predicate) {
            return PredicateNegator{predicate};
        };
        
        auto non_debug_logs = std::count_if(logs_.begin(), logs_.end(),
            negate_pred([](const LogEntry& log) { return log.level == "DEBUG"; }));
        std::cout << "DEBUG가 아닌 로그: " << non_debug_logs << std::endl;
        
        // 4. std::function을 활용한 동적 조합
        using LogPredicate = std::function<bool(const LogEntry&)>;
        
        std::vector<LogPredicate> filters = {
            [](const LogEntry& log) { return log.priority > 1; },
            [](const LogEntry& log) { return log.message.find("fail") != std::string::npos; },
            [](const LogEntry& log) { return log.level != "DEBUG"; }
        };
        
        // 모든 필터를 만족하는 로그 찾기
        auto passes_all_filters = [&filters](const LogEntry& log) {
            return std::all_of(filters.begin(), filters.end(),
                [&log](const LogPredicate& filter) { return filter(log); });
        };
        
        auto filtered_logs = std::count_if(logs_.begin(), logs_.end(), passes_all_filters);
        std::cout << "모든 필터 통과 로그: " << filtered_logs << std::endl;
    }
    
    void demonstrateTransformationChains() {
        std::cout << "\n--- 변환 체인 패턴 ---" << std::endl;
        
        // 복잡한 데이터 변환 파이프라인
        std::vector<std::string> processed_messages;
        
        // 1단계: 필터링 -> 2단계: 변환 -> 3단계: 수집
        std::transform_if(logs_.begin(), logs_.end(), 
                         std::back_inserter(processed_messages),
            [](const LogEntry& log) { return log.priority >= 2; },  // 필터
            [](const LogEntry& log) {  // 변환
                return "[" + log.level + "] " + log.message + 
                       " (P:" + std::to_string(log.priority) + ")";
            });
        
        std::cout << "처리된 메시지 수: " << processed_messages.size() << std::endl;
        if (!processed_messages.empty()) {
            std::cout << "첫 번째: " << processed_messages[0] << std::endl;
        }
        
        // Map-Reduce 스타일 처리
        std::unordered_map<std::string, std::vector<int>> level_priorities;
        
        // Map 단계: 레벨별로 우선순위 그룹화
        std::for_each(logs_.begin(), logs_.end(),
            [&level_priorities](const LogEntry& log) {
                level_priorities[log.level].push_back(log.priority);
            });
        
        // Reduce 단계: 레벨별 평균 우선순위 계산
        std::unordered_map<std::string, double> level_avg_priority;
        std::transform(level_priorities.begin(), level_priorities.end(),
                      std::inserter(level_avg_priority, level_avg_priority.end()),
            [](const auto& pair) {
                const auto& [level, priorities] = pair;
                double avg = std::accumulate(priorities.begin(), priorities.end(), 0.0) / priorities.size();
                return std::make_pair(level, avg);
            });
        
        std::cout << "레벨별 평균 우선순위:" << std::endl;
        for (const auto& [level, avg] : level_avg_priority) {
            std::cout << "  " << level << ": " << avg << std::endl;
        }
    }

private:
    // transform_if는 표준에 없으므로 구현
    template<typename InputIt, typename OutputIt, typename Predicate, typename Transform>
    void transform_if(InputIt first, InputIt last, OutputIt out, 
                     Predicate pred, Transform trans) {
        while (first != last) {
            if (pred(*first)) {
                *out++ = trans(*first);
            }
            ++first;
        }
    }
};
```

### 병렬 알고리즘 (C++17)

```cpp
#include <execution>
#include <future>
#include <thread>

// LogCaster에서 병렬 알고리즘 활용
class ParallelAlgorithmsDemo {
private:
    std::vector<LogEntry> large_logs_;
    
public:
    void setupMassiveTestData() {
        large_logs_.clear();
        large_logs_.reserve(100000);
        
        std::vector<std::string> levels = {"DEBUG", "INFO", "WARN", "ERROR", "CRITICAL"};
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> level_dist(0, 4);
        std::uniform_int_distribution<> priority_dist(0, 4);
        
        for (int i = 0; i < 100000; ++i) {
            std::string message = "로그 메시지 #" + std::to_string(i);
            large_logs_.emplace_back(message, levels[level_dist(gen)], priority_dist(gen));
        }
    }
    
    void demonstrateParallelAlgorithms() {
        setupMassiveTestData();
        
        std::cout << "=== 병렬 알고리즘 성능 비교 ===" << std::endl;
        
        auto measure_time = [](const std::string& name, auto&& func) {
            auto start = std::chrono::high_resolution_clock::now();
            auto result = func();
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << name << ": " << duration.count() << " ms, 결과: " << result << std::endl;
            return result;
        };
        
#ifdef __cpp_lib_parallel_algorithm
        // 1. 순차 vs 병렬 count_if 비교
        auto count_error_logs_sequential = [this]() {
            return std::count_if(large_logs_.begin(), large_logs_.end(),
                [](const LogEntry& log) { return log.level == "ERROR"; });
        };
        
        auto count_error_logs_parallel = [this]() {
            return std::count_if(std::execution::par, 
                                large_logs_.begin(), large_logs_.end(),
                [](const LogEntry& log) { return log.level == "ERROR"; });
        };
        
        measure_time("순차 count_if", count_error_logs_sequential);
        measure_time("병렬 count_if", count_error_logs_parallel);
        
        // 2. 순차 vs 병렬 transform 비교
        std::vector<std::string> formatted_sequential(large_logs_.size());
        std::vector<std::string> formatted_parallel(large_logs_.size());
        
        auto transform_sequential = [&]() {
            std::transform(large_logs_.begin(), large_logs_.end(),
                          formatted_sequential.begin(),
                [](const LogEntry& log) {
                    return "[" + log.level + "] " + log.message;
                });
            return formatted_sequential.size();
        };
        
        auto transform_parallel = [&]() {
            std::transform(std::execution::par,
                          large_logs_.begin(), large_logs_.end(),
                          formatted_parallel.begin(),
                [](const LogEntry& log) {
                    return "[" + log.level + "] " + log.message;
                });
            return formatted_parallel.size();
        };
        
        measure_time("순차 transform", transform_sequential);
        measure_time("병렬 transform", transform_parallel);
        
        // 3. 순차 vs 병렬 sort 비교
        auto logs_copy_seq = large_logs_;
        auto logs_copy_par = large_logs_;
        
        auto sort_sequential = [&]() {
            std::sort(logs_copy_seq.begin(), logs_copy_seq.end(),
                [](const LogEntry& a, const LogEntry& b) {
                    return a.priority > b.priority;
                });
            return logs_copy_seq.size();
        };
        
        auto sort_parallel = [&]() {
            std::sort(std::execution::par,
                     logs_copy_par.begin(), logs_copy_par.end(),
                [](const LogEntry& a, const LogEntry& b) {
                    return a.priority > b.priority;
                });
            return logs_copy_par.size();
        };
        
        measure_time("순차 sort", sort_sequential);
        measure_time("병렬 sort", sort_parallel);
        
        // 4. reduce 연산 (C++17 병렬)
        auto sum_priorities_parallel = [this]() {
            return std::transform_reduce(std::execution::par,
                                       large_logs_.begin(), large_logs_.end(),
                                       0,  // 초기값
                                       std::plus<>(),  // reduce 연산
                                       [](const LogEntry& log) { return log.priority; }  // transform
            );
        };
        
        measure_time("병렬 transform_reduce", sum_priorities_parallel);
        
#else
        std::cout << "병렬 알고리즘을 지원하지 않는 환경입니다." << std::endl;
        
        // 수동 병렬화 예시
        demonstrateManualParallelization();
#endif
    }
    
    void demonstrateManualParallelization() {
        std::cout << "\n--- 수동 병렬화 예시 ---" << std::endl;
        
        // 작업을 여러 스레드로 분할
        const size_t num_threads = std::thread::hardware_concurrency();
        const size_t chunk_size = large_logs_.size() / num_threads;
        
        auto count_in_range = [this](size_t start, size_t end) {
            return std::count_if(large_logs_.begin() + start, 
                               large_logs_.begin() + end,
                [](const LogEntry& log) { return log.level == "ERROR"; });
        };
        
        // 비동기 작업 시작
        std::vector<std::future<size_t>> futures;
        for (size_t i = 0; i < num_threads; ++i) {
            size_t start = i * chunk_size;
            size_t end = (i == num_threads - 1) ? large_logs_.size() : (i + 1) * chunk_size;
            
            futures.push_back(std::async(std::launch::async, count_in_range, start, end));
        }
        
        // 결과 수집
        size_t total_errors = 0;
        for (auto& future : futures) {
            total_errors += future.get();
        }
        
        std::cout << "수동 병렬화 ERROR 로그 개수: " << total_errors << std::endl;
        
        // 검증을 위한 순차 계산
        auto sequential_errors = std::count_if(large_logs_.begin(), large_logs_.end(),
            [](const LogEntry& log) { return log.level == "ERROR"; });
        
        std::cout << "순차 계산 ERROR 로그 개수: " << sequential_errors << std::endl;
        std::cout << "결과 일치: " << (total_errors == sequential_errors) << std::endl;
    }
};
```

---

## 🎯 3. 커스텀 알고리즘 구현

### 자주 사용하는 패턴들

```cpp
// LogCaster 전용 커스텀 알고리즘들
namespace LogCasterAlgorithms {
    
    // 1. find_all: 조건에 맞는 모든 요소의 iterator 반환
    template<typename Iterator, typename Predicate>
    std::vector<Iterator> find_all(Iterator first, Iterator last, Predicate pred) {
        std::vector<Iterator> results;
        while (first != last) {
            first = std::find_if(first, last, pred);
            if (first != last) {
                results.push_back(first);
                ++first;
            }
        }
        return results;
    }
    
    // 2. group_by: 키 함수를 사용해서 그룹화
    template<typename Iterator, typename KeyFunc>
    auto group_by(Iterator first, Iterator last, KeyFunc key_func) {
        using ValueType = typename std::iterator_traits<Iterator>::value_type;
        using KeyType = decltype(key_func(*first));
        
        std::map<KeyType, std::vector<ValueType>> groups;
        
        std::for_each(first, last, [&groups, &key_func](const ValueType& item) {
            groups[key_func(item)].push_back(item);
        });
        
        return groups;
    }
    
    // 3. sliding_window: 슬라이딩 윈도우로 처리
    template<typename Iterator, typename Func>
    void sliding_window(Iterator first, Iterator last, size_t window_size, Func func) {
        if (std::distance(first, last) < static_cast<long>(window_size)) {
            return;
        }
        
        for (auto it = first; it <= last - window_size; ++it) {
            func(it, it + window_size);
        }
    }
    
    // 4. batch_process: 배치 단위로 처리
    template<typename Iterator, typename Func>
    void batch_process(Iterator first, Iterator last, size_t batch_size, Func func) {
        while (first != last) {
            auto batch_end = std::next(first, std::min(static_cast<size_t>(std::distance(first, last)), 
                                                      batch_size));
            func(first, batch_end);
            first = batch_end;
        }
    }
    
    // 5. top_k: 상위 K개 요소 찾기
    template<typename Iterator, typename Compare = std::less<>>
    std::vector<typename std::iterator_traits<Iterator>::value_type> 
    top_k(Iterator first, Iterator last, size_t k, Compare comp = {}) {
        using ValueType = typename std::iterator_traits<Iterator>::value_type;
        
        if (k == 0) return {};
        
        std::vector<ValueType> result(first, last);
        if (result.size() <= k) {
            std::sort(result.begin(), result.end(), comp);
            return result;
        }
        
        std::partial_sort(result.begin(), result.begin() + k, result.end(), comp);
        result.resize(k);
        return result;
    }
    
    // 6. unique_by: 사용자 정의 키로 중복 제거
    template<typename Iterator, typename KeyFunc>
    Iterator unique_by(Iterator first, Iterator last, KeyFunc key_func) {
        if (first == last) return last;
        
        using KeyType = decltype(key_func(*first));
        std::unordered_set<KeyType> seen;
        
        return std::remove_if(first, last, [&seen, &key_func](const auto& item) {
            KeyType key = key_func(item);
            if (seen.find(key) != seen.end()) {
                return true;  // 이미 본 키, 제거
            } else {
                seen.insert(key);
                return false;  // 새로운 키, 유지
            }
        });
    }
    
    // 7. chain: 여러 컨테이너를 연결해서 순회
    template<typename... Containers>
    class ChainIterator {
    private:
        std::tuple<Containers&...> containers_;
        size_t current_container_ = 0;
        // 복잡한 구현 생략...
    public:
        explicit ChainIterator(Containers&... containers) 
            : containers_(containers...) {}
        // iterator interface 구현...
    };
    
    template<typename... Containers>
    auto chain(Containers&... containers) {
        return ChainIterator<Containers...>(containers...);
    }
}

class CustomAlgorithmsDemo {
private:
    std::vector<LogEntry> logs_;
    
public:
    void setupTestData() {
        logs_ = {
            LogEntry("서버 시작", "INFO", 1),
            LogEntry("DB 연결", "INFO", 1), 
            LogEntry("사용자 로그인", "INFO", 1),
            LogEntry("메모리 경고", "WARN", 2),
            LogEntry("연결 실패", "ERROR", 3),
            LogEntry("DB 연결", "INFO", 1),  // 중복
            LogEntry("시스템 오류", "ERROR", 3),
            LogEntry("복구 완료", "INFO", 1),
            LogEntry("메모리 경고", "WARN", 2),  // 중복
            LogEntry("종료 신호", "INFO", 1)
        };
    }
    
    void demonstrateCustomAlgorithms() {
        setupTestData();
        
        std::cout << "=== 커스텀 알고리즘 데모 ===" << std::endl;
        
        // 1. find_all 사용
        auto info_iterators = LogCasterAlgorithms::find_all(logs_.begin(), logs_.end(),
            [](const LogEntry& log) { return log.level == "INFO"; });
        
        std::cout << "INFO 로그 위치들: " << info_iterators.size() << "개" << std::endl;
        for (auto it : info_iterators) {
            std::cout << "  " << std::distance(logs_.begin(), it) << ": " << it->message << std::endl;
        }
        
        // 2. group_by 사용
        auto logs_by_level = LogCasterAlgorithms::group_by(logs_.begin(), logs_.end(),
            [](const LogEntry& log) { return log.level; });
        
        std::cout << "\n레벨별 그룹화:" << std::endl;
        for (const auto& [level, group_logs] : logs_by_level) {
            std::cout << "  " << level << ": " << group_logs.size() << "개" << std::endl;
            for (const auto& log : group_logs) {
                std::cout << "    - " << log.message << std::endl;
            }
        }
        
        // 3. top_k 사용
        auto top_priority_logs = LogCasterAlgorithms::top_k(logs_.begin(), logs_.end(), 3,
            [](const LogEntry& a, const LogEntry& b) {
                return a.priority > b.priority;  // 내림차순
            });
        
        std::cout << "\n상위 3개 우선순위 로그:" << std::endl;
        for (const auto& log : top_priority_logs) {
            std::cout << "  P" << log.priority << ": " << log.message << std::endl;
        }
        
        // 4. unique_by 사용 (메시지로 중복 제거)
        auto logs_copy = logs_;
        auto unique_end = LogCasterAlgorithms::unique_by(logs_copy.begin(), logs_copy.end(),
            [](const LogEntry& log) { return log.message; });
        
        logs_copy.erase(unique_end, logs_copy.end());
        
        std::cout << "\n중복 제거 후 로그 수: " << logs_copy.size() << std::endl;
        
        // 5. sliding_window 사용
        std::cout << "\n슬라이딩 윈도우 (크기 3) 분석:" << std::endl;
        LogCasterAlgorithms::sliding_window(logs_.begin(), logs_.end(), 3,
            [](auto first, auto last) {
                int total_priority = 0;
                std::for_each(first, last, [&total_priority](const LogEntry& log) {
                    total_priority += log.priority;
                });
                
                double avg_priority = static_cast<double>(total_priority) / std::distance(first, last);
                std::cout << "  윈도우 평균 우선순위: " << avg_priority 
                          << " (메시지: " << first->message << " ~ " << (last-1)->message << ")" << std::endl;
            });
        
        // 6. batch_process 사용
        std::cout << "\n배치 처리 (크기 4):" << std::endl;
        int batch_number = 1;
        LogCasterAlgorithms::batch_process(logs_.begin(), logs_.end(), 4,
            [&batch_number](auto first, auto last) {
                std::cout << "  배치 " << batch_number++ << " (" << std::distance(first, last) << "개): ";
                
                std::unordered_map<std::string, int> level_counts;
                std::for_each(first, last, [&level_counts](const LogEntry& log) {
                    level_counts[log.level]++;
                });
                
                for (const auto& [level, count] : level_counts) {
                    std::cout << level << "=" << count << " ";
                }
                std::cout << std::endl;
            });
    }
    
    void demonstrateRealWorldScenarios() {
        std::cout << "\n=== 실제 시나리오 적용 ===" << std::endl;
        
        // 시나리오 1: 로그 레벨별 최근 N개 추출
        auto get_recent_logs_by_level = [this](const std::string& level, size_t count) {
            // 해당 레벨 로그들 찾기
            std::vector<LogEntry> level_logs;
            std::copy_if(logs_.begin(), logs_.end(), std::back_inserter(level_logs),
                [&level](const LogEntry& log) { return log.level == level; });
            
            // 타임스탬프 기준으로 정렬 (최신 순)
            std::sort(level_logs.begin(), level_logs.end(),
                [](const LogEntry& a, const LogEntry& b) {
                    return a.timestamp > b.timestamp;
                });
            
            // 상위 N개 반환
            if (level_logs.size() > count) {
                level_logs.resize(count);
            }
            
            return level_logs;
        };
        
        auto recent_errors = get_recent_logs_by_level("ERROR", 2);
        std::cout << "최근 ERROR 로그 " << recent_errors.size() << "개:" << std::endl;
        for (const auto& log : recent_errors) {
            std::cout << "  " << log.message << std::endl;
        }
        
        // 시나리오 2: 로그 패턴 분석 (연속된 같은 레벨 로그 감지)
        auto detect_consecutive_patterns = [this]() {
            std::vector<std::pair<std::string, int>> patterns;
            
            if (logs_.empty()) return patterns;
            
            std::string current_level = logs_[0].level;
            int count = 1;
            
            for (size_t i = 1; i < logs_.size(); ++i) {
                if (logs_[i].level == current_level) {
                    count++;
                } else {
                    if (count >= 2) {  // 2개 이상 연속일 때만 기록
                        patterns.emplace_back(current_level, count);
                    }
                    current_level = logs_[i].level;
                    count = 1;
                }
            }
            
            // 마지막 패턴 처리
            if (count >= 2) {
                patterns.emplace_back(current_level, count);
            }
            
            return patterns;
        };
        
        auto patterns = detect_consecutive_patterns();
        std::cout << "\n연속된 로그 패턴:" << std::endl;
        for (const auto& [level, count] : patterns) {
            std::cout << "  " << level << " 연속 " << count << "회" << std::endl;
        }
        
        // 시나리오 3: 로그 품질 점수 계산
        auto calculate_log_quality_score = [this]() {
            double total_score = std::transform_reduce(logs_.begin(), logs_.end(), 0.0,
                std::plus<>(),  // reduce: 합계
                [](const LogEntry& log) {  // transform: 각 로그의 점수 계산
                    double score = 0.0;
                    
                    // 메시지 길이 점수 (적당한 길이가 좋음)
                    if (log.message.length() > 10 && log.message.length() < 100) {
                        score += 1.0;
                    }
                    
                    // 레벨별 가중치
                    if (log.level == "ERROR" || log.level == "CRITICAL") {
                        score += 0.5;  // 오류 로그는 중요하지만 너무 많으면 안 좋음
                    } else if (log.level == "INFO") {
                        score += 1.0;  // 정보 로그는 좋음
                    } else if (log.level == "DEBUG") {
                        score += 0.3;  // 디버그 로그는 적당히
                    }
                    
                    return score;
                });
            
            return total_score / logs_.size();  // 평균 점수
        };
        
        double quality_score = calculate_log_quality_score();
        std::cout << "\n로그 품질 점수: " << quality_score << "/1.0" << std::endl;
    }
};
```

---

## ⚡ 4. 성능 최적화와 복잡도 분석

### 알고리즘 복잡도 이해

```cpp
#include <chrono>
#include <iomanip>

class AlgorithmComplexityAnalysis {
private:
    std::vector<LogEntry> small_logs_;   // 1K entries
    std::vector<LogEntry> medium_logs_;  // 10K entries  
    std::vector<LogEntry> large_logs_;   // 100K entries
    
public:
    void setupBenchmarkData() {
        std::cout << "벤치마크 데이터 생성 중..." << std::endl;
        
        auto generate_logs = [](size_t count) {
            std::vector<LogEntry> logs;
            logs.reserve(count);
            
            std::vector<std::string> levels = {"DEBUG", "INFO", "WARN", "ERROR", "CRITICAL"};
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> level_dist(0, 4);
            std::uniform_int_distribution<> priority_dist(0, 4);
            
            for (size_t i = 0; i < count; ++i) {
                std::string message = "로그 메시지 #" + std::to_string(i);
                logs.emplace_back(message, levels[level_dist(gen)], priority_dist(gen));
            }
            
            return logs;
        };
        
        small_logs_ = generate_logs(1000);      // 1K
        medium_logs_ = generate_logs(10000);    // 10K  
        large_logs_ = generate_logs(100000);    // 100K
        
        std::cout << "데이터 생성 완료!" << std::endl;
    }
    
    void analyzeBigOComplexity() {
        std::cout << "\n=== Big-O 복잡도 분석 ===" << std::endl;
        
        auto benchmark_algorithm = [](const std::string& name, auto&& func, 
                                     const std::vector<LogEntry>& data) {
            auto start = std::chrono::high_resolution_clock::now();
            auto result = func(data);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            return std::make_tuple(duration.count(), result);
        };
        
        // O(n) 알고리즘들 - Linear
        std::cout << "\n--- O(n) 알고리즘들 ---" << std::endl;
        std::cout << std::setw(20) << "알고리즘" << std::setw(15) << "1K (μs)" 
                  << std::setw(15) << "10K (μs)" << std::setw(15) << "100K (μs)" << std::endl;
        std::cout << std::string(65, '-') << std::endl;
        
        auto linear_algorithms = {
            std::make_pair("find_if", [](const std::vector<LogEntry>& logs) {
                return std::find_if(logs.begin(), logs.end(),
                    [](const LogEntry& log) { return log.level == "ERROR"; }) != logs.end();
            }),
            
            std::make_pair("count_if", [](const std::vector<LogEntry>& logs) {
                return std::count_if(logs.begin(), logs.end(),
                    [](const LogEntry& log) { return log.priority > 2; });
            }),
            
            std::make_pair("accumulate", [](const std::vector<LogEntry>& logs) {
                return std::accumulate(logs.begin(), logs.end(), 0,
                    [](int sum, const LogEntry& log) { return sum + log.priority; });
            }),
            
            std::make_pair("for_each", [](const std::vector<LogEntry>& logs) {
                int dummy = 0;
                std::for_each(logs.begin(), logs.end(),
                    [&dummy](const LogEntry& log) { dummy += log.message.length(); });
                return dummy;
            })
        };
        
        for (const auto& [name, func] : linear_algorithms) {
            auto [time_1k, result_1k] = benchmark_algorithm(name, func, small_logs_);
            auto [time_10k, result_10k] = benchmark_algorithm(name, func, medium_logs_);
            auto [time_100k, result_100k] = benchmark_algorithm(name, func, large_logs_);
            
            std::cout << std::setw(20) << name 
                      << std::setw(15) << time_1k
                      << std::setw(15) << time_10k  
                      << std::setw(15) << time_100k << std::endl;
        }
        
        // O(n log n) 알고리즘들 - Linearithmic
        std::cout << "\n--- O(n log n) 알고리즘들 ---" << std::endl;
        std::cout << std::setw(20) << "알고리즘" << std::setw(15) << "1K (μs)" 
                  << std::setw(15) << "10K (μs)" << std::setw(15) << "100K (μs)" << std::endl;
        std::cout << std::string(65, '-') << std::endl;
        
        auto nlongn_algorithms = {
            std::make_pair("sort", [](std::vector<LogEntry> logs) {
                std::sort(logs.begin(), logs.end(),
                    [](const LogEntry& a, const LogEntry& b) {
                        return a.priority < b.priority;
                    });
                return logs.size();
            }),
            
            std::make_pair("stable_sort", [](std::vector<LogEntry> logs) {
                std::stable_sort(logs.begin(), logs.end(),
                    [](const LogEntry& a, const LogEntry& b) {
                        return a.level < b.level;
                    });
                return logs.size();
            }),
            
            std::make_pair("partial_sort", [](std::vector<LogEntry> logs) {
                size_t k = std::min(logs.size(), size_t(10));  // 상위 10개만
                std::partial_sort(logs.begin(), logs.begin() + k, logs.end(),
                    [](const LogEntry& a, const LogEntry& b) {
                        return a.priority > b.priority;
                    });
                return k;
            })
        };
        
        for (const auto& [name, func] : nlongn_algorithms) {
            auto [time_1k, result_1k] = benchmark_algorithm(name, func, small_logs_);
            auto [time_10k, result_10k] = benchmark_algorithm(name, func, medium_logs_);
            auto [time_100k, result_100k] = benchmark_algorithm(name, func, large_logs_);
            
            std::cout << std::setw(20) << name 
                      << std::setw(15) << time_1k
                      << std::setw(15) << time_10k
                      << std::setw(15) << time_100k << std::endl;
        }
        
        // O(log n) 알고리즘들 - Logarithmic (정렬된 데이터에서)
        std::cout << "\n--- O(log n) 알고리즘들 (정렬된 데이터) ---" << std::endl;
        
        auto prepare_sorted_data = [](std::vector<LogEntry> logs) {
            std::sort(logs.begin(), logs.end(),
                [](const LogEntry& a, const LogEntry& b) {
                    return a.priority < b.priority;
                });
            return logs;
        };
        
        auto sorted_small = prepare_sorted_data(small_logs_);
        auto sorted_medium = prepare_sorted_data(medium_logs_);
        auto sorted_large = prepare_sorted_data(large_logs_);
        
        auto logarithmic_algorithms = {
            std::make_pair("binary_search", [](const std::vector<LogEntry>& logs) {
                LogEntry target("", "", 2);  // priority 2 찾기
                return std::binary_search(logs.begin(), logs.end(), target,
                    [](const LogEntry& a, const LogEntry& b) {
                        return a.priority < b.priority;
                    });
            }),
            
            std::make_pair("lower_bound", [](const std::vector<LogEntry>& logs) {
                LogEntry target("", "", 2);
                auto it = std::lower_bound(logs.begin(), logs.end(), target,
                    [](const LogEntry& a, const LogEntry& b) {
                        return a.priority < b.priority;
                    });
                return std::distance(logs.begin(), it);
            })
        };
        
        std::cout << std::setw(20) << "알고리즘" << std::setw(15) << "1K (μs)" 
                  << std::setw(15) << "10K (μs)" << std::setw(15) << "100K (μs)" << std::endl;
        std::cout << std::string(65, '-') << std::endl;
        
        for (const auto& [name, func] : logarithmic_algorithms) {
            auto [time_1k, result_1k] = benchmark_algorithm(name, func, sorted_small);
            auto [time_10k, result_10k] = benchmark_algorithm(name, func, sorted_medium);
            auto [time_100k, result_100k] = benchmark_algorithm(name, func, sorted_large);
            
            std::cout << std::setw(20) << name 
                      << std::setw(15) << time_1k
                      << std::setw(15) << time_10k
                      << std::setw(15) << time_100k << std::endl;
        }
    }
    
    void demonstrateOptimizationTechniques() {
        std::cout << "\n=== 최적화 기법들 ===" << std::endl;
        
        // 1. Reserve vs No Reserve
        auto benchmark_vector_operations = []() {
            const size_t COUNT = 100000;
            
            // Reserve 없이
            auto without_reserve = []() {
                std::vector<LogEntry> logs;
                for (size_t i = 0; i < COUNT; ++i) {
                    logs.emplace_back("메시지 " + std::to_string(i), "INFO", 1);
                }
                return logs.size();
            };
            
            // Reserve 사용
            auto with_reserve = []() {
                std::vector<LogEntry> logs;
                logs.reserve(COUNT);
                for (size_t i = 0; i < COUNT; ++i) {
                    logs.emplace_back("메시지 " + std::to_string(i), "INFO", 1);
                }
                return logs.size();
            };
            
            auto measure = [](const std::string& name, auto func) {
                auto start = std::chrono::high_resolution_clock::now();
                auto result = func();
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                std::cout << name << ": " << duration.count() << " μs" << std::endl;
                return result;
            };
            
            measure("Reserve 없이", without_reserve);
            measure("Reserve 사용", with_reserve);
        };
        
        benchmark_vector_operations();
        
        // 2. Iterator vs Index
        std::cout << "\n--- Iterator vs Index 성능 비교 ---" << std::endl;
        
        auto sum_with_iterator = [this]() {
            int sum = 0;
            for (auto it = large_logs_.begin(); it != large_logs_.end(); ++it) {
                sum += it->priority;
            }
            return sum;
        };
        
        auto sum_with_index = [this]() {
            int sum = 0;
            for (size_t i = 0; i < large_logs_.size(); ++i) {
                sum += large_logs_[i].priority;
            }
            return sum;
        };
        
        auto sum_with_range_for = [this]() {
            int sum = 0;
            for (const auto& log : large_logs_) {
                sum += log.priority;
            }
            return sum;
        };
        
        auto measure = [](const std::string& name, auto func) {
            auto start = std::chrono::high_resolution_clock::now();
            auto result = func();
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << name << ": " << duration.count() << " μs, 결과: " << result << std::endl;
        };
        
        measure("Iterator", sum_with_iterator);
        measure("Index", sum_with_index);
        measure("Range-based for", sum_with_range_for);
        
        // 3. 알고리즘 선택에 따른 성능 차이
        std::cout << "\n--- 알고리즘 선택 최적화 ---" << std::endl;
        
        // 정렬된 데이터에서 특정 값 찾기: find vs binary_search
        auto sorted_logs = large_logs_;
        std::sort(sorted_logs.begin(), sorted_logs.end(),
            [](const LogEntry& a, const LogEntry& b) {
                return a.priority < b.priority;
            });
        
        auto find_linear = [&sorted_logs]() {
            return std::find_if(sorted_logs.begin(), sorted_logs.end(),
                [](const LogEntry& log) { return log.priority == 3; });
        };
        
        auto find_binary = [&sorted_logs]() {
            LogEntry target("", "", 3);
            return std::binary_search(sorted_logs.begin(), sorted_logs.end(), target,
                [](const LogEntry& a, const LogEntry& b) {
                    return a.priority < b.priority;
                });
        };
        
        measure("Linear find", [&]() { return find_linear() != sorted_logs.end(); });
        measure("Binary search", find_binary);
    }
};
```

---

## 📊 5. 실전 LogCaster 알고리즘 활용

### 완전한 로그 분석 시스템

```cpp
#include <unordered_set>
#include <regex>
#include <fstream>

// LogCaster 실전 알고리즘 종합 활용
namespace LogCaster::Analytics {

struct LogPattern {
    std::string name;
    std::regex pattern;
    int severity;
    
    LogPattern(std::string n, std::string p, int s) 
        : name(std::move(n)), pattern(p), severity(s) {}
};

struct LogAlert {
    std::string message;
    std::string level;
    std::chrono::system_clock::time_point timestamp;
    int count;
    
    LogAlert(std::string msg, std::string lvl, int cnt = 1)
        : message(std::move(msg)), level(std::move(lvl)), 
          timestamp(std::chrono::system_clock::now()), count(cnt) {}
};

class AdvancedLogAnalyzer {
private:
    std::vector<LogEntry> logs_;
    std::vector<LogPattern> patterns_;
    std::vector<LogAlert> alerts_;
    
    // 성능을 위한 인덱스들
    std::unordered_map<std::string, std::vector<size_t>> level_index_;
    std::multimap<std::chrono::system_clock::time_point, size_t> time_index_;
    
public:
    AdvancedLogAnalyzer() {
        setupPatterns();
    }
    
    void addLogs(const std::vector<LogEntry>& new_logs) {
        size_t start_idx = logs_.size();
        logs_.insert(logs_.end(), new_logs.begin(), new_logs.end());
        
        // 인덱스 업데이트
        updateIndexes(start_idx);
        
        // 새 로그들에 대해 실시간 분석 수행
        analyzeNewLogs(start_idx);
    }
    
    // 1. 로그 레벨별 통계 분석
    std::map<std::string, size_t> getLevelStatistics() const {
        std::map<std::string, size_t> stats;
        
        std::for_each(logs_.begin(), logs_.end(),
            [&stats](const LogEntry& log) {
                stats[log.level]++;
            });
        
        return stats;
    }
    
    // 2. 시간대별 로그 패턴 분석
    std::vector<std::pair<std::string, std::vector<size_t>>> analyzeTimePatterns() const {
        std::map<std::string, std::vector<size_t>> hourly_counts;
        
        std::for_each(logs_.begin(), logs_.end(),
            [&hourly_counts](const LogEntry& log) {
                auto time_t = std::chrono::system_clock::to_time_t(log.timestamp);
                auto tm = *std::localtime(&time_t);
                std::string hour_key = std::to_string(tm.tm_hour);
                
                if (hourly_counts.find(hour_key) == hourly_counts.end()) {
                    hourly_counts[hour_key].resize(24, 0);
                }
                hourly_counts[log.level].resize(24, 0);
                hourly_counts[log.level][tm.tm_hour]++;
            });
        
        return std::vector<std::pair<std::string, std::vector<size_t>>>(
            hourly_counts.begin(), hourly_counts.end());
    }
    
    // 3. 이상 패턴 감지
    std::vector<LogAlert> detectAnomalies() {
        std::vector<LogAlert> anomalies;
        
        // 에러율 급증 감지
        auto error_spike_alerts = detectErrorSpikes();
        anomalies.insert(anomalies.end(), error_spike_alerts.begin(), error_spike_alerts.end());
        
        // 반복 패턴 감지  
        auto repetitive_alerts = detectRepetitivePatterns();
        anomalies.insert(anomalies.end(), repetitive_alerts.begin(), repetitive_alerts.end());
        
        // 정규식 패턴 매칭
        auto pattern_alerts = detectKnownPatterns();
        anomalies.insert(anomalies.end(), pattern_alerts.begin(), pattern_alerts.end());
        
        return anomalies;
    }
    
    // 4. 로그 품질 분석
    struct LogQualityReport {
        double completeness_score;    // 완성도 점수
        double consistency_score;     // 일관성 점수
        double informativeness_score; // 정보성 점수
        std::vector<std::string> recommendations;
    };
    
    LogQualityReport analyzeLogQuality() const {
        LogQualityReport report;
        
        // 완성도 분석 (빈 메시지, 타임스탬프 등)
        auto empty_messages = std::count_if(logs_.begin(), logs_.end(),
            [](const LogEntry& log) { return log.message.empty(); });
        report.completeness_score = 1.0 - (static_cast<double>(empty_messages) / logs_.size());
        
        // 일관성 분석 (로그 형식의 일관성)
        std::map<std::string, int> format_patterns;
        std::for_each(logs_.begin(), logs_.end(),
            [&format_patterns](const LogEntry& log) {
                // 간단한 패턴 분석 (실제로는 더 복잡한 로직 필요)
                bool has_brackets = log.message.find('[') != std::string::npos;
                bool has_timestamp = log.message.find(':') != std::string::npos;
                
                std::string pattern = "";
                if (has_brackets) pattern += "brackets_";
                if (has_timestamp) pattern += "timestamp_";
                if (pattern.empty()) pattern = "plain";
                
                format_patterns[pattern]++;
            });
        
        // 가장 일반적인 패턴의 비율을 일관성 점수로 사용
        auto max_pattern = std::max_element(format_patterns.begin(), format_patterns.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });
        
        if (max_pattern != format_patterns.end()) {
            report.consistency_score = static_cast<double>(max_pattern->second) / logs_.size();
        }
        
        // 정보성 분석 (메시지 길이, 다양성 등)
        auto avg_length = std::transform_reduce(logs_.begin(), logs_.end(), 0.0,
            std::plus<>(),
            [](const LogEntry& log) { return static_cast<double>(log.message.length()); }
        ) / logs_.size();
        
        // 적절한 길이(20-100자)를 기준으로 점수 계산
        report.informativeness_score = std::min(1.0, avg_length / 50.0);
        
        // 권장사항 생성
        if (report.completeness_score < 0.9) {
            report.recommendations.push_back("빈 로그 메시지를 줄이세요");
        }
        if (report.consistency_score < 0.7) {
            report.recommendations.push_back("로그 형식을 일관성 있게 유지하세요");
        }
        if (report.informativeness_score < 0.5) {
            report.recommendations.push_back("로그 메시지를 더 자세하게 작성하세요");
        }
        
        return report;
    }
    
    // 5. 고급 검색 및 필터링
    template<typename... Predicates>
    std::vector<LogEntry> advancedSearch(Predicates... predicates) const {
        std::vector<LogEntry> results;
        
        std::copy_if(logs_.begin(), logs_.end(), std::back_inserter(results),
            [&predicates...](const LogEntry& log) {
                return (predicates(log) && ...);  // C++17 fold expression
            });
        
        return results;
    }
    
    // 시간 범위별 검색
    std::vector<LogEntry> searchByTimeRange(
        std::chrono::system_clock::time_point start,
        std::chrono::system_clock::time_point end) const {
        
        std::vector<LogEntry> results;
        
        // 시간 인덱스 활용한 효율적 검색
        auto start_it = time_index_.lower_bound(start);
        auto end_it = time_index_.upper_bound(end);
        
        std::transform(start_it, end_it, std::back_inserter(results),
            [this](const auto& time_idx_pair) {
                return logs_[time_idx_pair.second];
            });
        
        return results;
    }
    
    // 복잡한 조건 검색
    std::vector<LogEntry> searchComplex(const std::string& level_filter,
                                       const std::string& text_filter,
                                       int min_priority,
                                       std::chrono::minutes time_window) const {
        auto now = std::chrono::system_clock::now();
        auto cutoff_time = now - time_window;
        
        return advancedSearch(
            [&level_filter](const LogEntry& log) {
                return level_filter.empty() || log.level == level_filter;
            },
            [&text_filter](const LogEntry& log) {
                return text_filter.empty() || 
                       log.message.find(text_filter) != std::string::npos;
            },
            [min_priority](const LogEntry& log) {
                return log.priority >= min_priority;
            },
            [cutoff_time](const LogEntry& log) {
                return log.timestamp >= cutoff_time;
            }
        );
    }
    
    // 6. 성능 모니터링
    void printPerformanceMetrics() const {
        std::cout << "\n=== LogCaster Analytics 성능 지표 ===" << std::endl;
        
        std::cout << "총 로그 수: " << logs_.size() << std::endl;
        std::cout << "메모리 사용량: " << 
            (logs_.size() * sizeof(LogEntry) + 
             level_index_.size() * sizeof(std::pair<std::string, std::vector<size_t>>) +
             time_index_.size() * sizeof(std::pair<std::chrono::system_clock::time_point, size_t>))
            / 1024 << " KB" << std::endl;
        
        // 인덱스 효율성
        size_t total_indexed = 0;
        for (const auto& [level, indices] : level_index_) {
            total_indexed += indices.size();
        }
        std::cout << "레벨 인덱스 커버리지: " 
                  << (static_cast<double>(total_indexed) / logs_.size() * 100) << "%" << std::endl;
        
        std::cout << "시간 인덱스 엔트리: " << time_index_.size() << std::endl;
    }

private:
    void setupPatterns() {
        patterns_ = {
            LogPattern("Memory Leak", R"(memory.*leak|out of memory)", 4),
            LogPattern("Database Error", R"(database.*error|sql.*exception|connection.*failed)", 3),
            LogPattern("Network Timeout", R"(timeout|connection.*timed.*out|network.*unreachable)", 2),
            LogPattern("Authentication Failure", R"(auth.*failed|invalid.*credentials|unauthorized)", 2),
            LogPattern("File Not Found", R"(file.*not.*found|no such file)", 1)
        };
    }
    
    void updateIndexes(size_t start_idx) {
        for (size_t i = start_idx; i < logs_.size(); ++i) {
            const auto& log = logs_[i];
            
            // 레벨 인덱스 업데이트
            level_index_[log.level].push_back(i);
            
            // 시간 인덱스 업데이트
            time_index_.emplace(log.timestamp, i);
        }
    }
    
    void analyzeNewLogs(size_t start_idx) {
        // 새로 추가된 로그들에 대해 실시간 분석 수행
        for (size_t i = start_idx; i < logs_.size(); ++i) {
            const auto& log = logs_[i];
            
            // 패턴 매칭
            for (const auto& pattern : patterns_) {
                if (std::regex_search(log.message, pattern.pattern)) {
                    alerts_.emplace_back("패턴 감지: " + pattern.name, 
                                       log.level, pattern.severity);
                }
            }
        }
    }
    
    std::vector<LogAlert> detectErrorSpikes() const {
        std::vector<LogAlert> alerts;
        
        // 최근 5분간의 에러 로그 수 계산
        auto now = std::chrono::system_clock::now();
        auto five_minutes_ago = now - std::chrono::minutes(5);
        
        auto recent_errors = std::count_if(logs_.begin(), logs_.end(),
            [&](const LogEntry& log) {
                return log.level == "ERROR" && log.timestamp > five_minutes_ago;
            });
        
        // 임계값 초과 시 알림
        if (recent_errors > 10) {  // 5분간 10개 초과
            alerts.emplace_back("에러 급증 감지", "CRITICAL", recent_errors);
        }
        
        return alerts;
    }
    
    std::vector<LogAlert> detectRepetitivePatterns() const {
        std::vector<LogAlert> alerts;
        std::unordered_map<std::string, int> message_counts;
        
        // 메시지별 빈도 계산
        std::for_each(logs_.begin(), logs_.end(),
            [&message_counts](const LogEntry& log) {
                message_counts[log.message]++;
            });
        
        // 과도하게 반복되는 메시지 찾기
        for (const auto& [message, count] : message_counts) {
            if (count > 100) {  // 100회 초과
                alerts.emplace_back("반복 메시지: " + message.substr(0, 50), 
                                   "WARN", count);
            }
        }
        
        return alerts;
    }
    
    std::vector<LogAlert> detectKnownPatterns() const {
        std::vector<LogAlert> alerts;
        
        for (const auto& pattern : patterns_) {
            auto matching_logs = std::count_if(logs_.begin(), logs_.end(),
                [&pattern](const LogEntry& log) {
                    return std::regex_search(log.message, pattern.pattern);
                });
            
            if (matching_logs > 0) {
                std::string level = (pattern.severity >= 3) ? "ERROR" : "WARN";
                alerts.emplace_back("알려진 패턴: " + pattern.name, 
                                   level, matching_logs);
            }
        }
        
        return alerts;
    }
};

} // namespace LogCaster::Analytics

// 사용 예시
void demonstrateAdvancedAnalytics() {
    using namespace LogCaster::Analytics;
    
    AdvancedLogAnalyzer analyzer;
    
    // 테스트 데이터 생성
    std::vector<LogEntry> sample_logs = {
        LogEntry("서버 시작 완료", "INFO", 1),
        LogEntry("database connection failed", "ERROR", 3),
        LogEntry("memory leak detected in module X", "CRITICAL", 4),
        LogEntry("사용자 로그인 성공", "INFO", 1),
        LogEntry("network timeout occurred", "WARN", 2),
        LogEntry("database connection failed", "ERROR", 3),  // 중복
        LogEntry("auth failed for user admin", "ERROR", 3),
        LogEntry("database connection failed", "ERROR", 3),  // 중복
        LogEntry("파일 처리 완료", "INFO", 1),
        LogEntry("out of memory error", "CRITICAL", 4)
    };
    
    // 대량의 중복 로그 추가 (패턴 감지 테스트)
    for (int i = 0; i < 150; ++i) {
        sample_logs.emplace_back("database connection failed", "ERROR", 3);
    }
    
    analyzer.addLogs(sample_logs);
    
    // 분석 결과 출력
    std::cout << "=== 고급 로그 분석 결과 ===" << std::endl;
    
    // 1. 레벨별 통계
    auto level_stats = analyzer.getLevelStatistics();
    std::cout << "\n레벨별 통계:" << std::endl;
    for (const auto& [level, count] : level_stats) {
        std::cout << "  " << level << ": " << count << "개" << std::endl;
    }
    
    // 2. 이상 패턴 감지
    auto anomalies = analyzer.detectAnomalies();
    std::cout << "\n감지된 이상 패턴:" << std::endl;
    for (const auto& alert : anomalies) {
        std::cout << "  [" << alert.level << "] " << alert.message 
                  << " (빈도: " << alert.count << ")" << std::endl;
    }
    
    // 3. 로그 품질 분석
    auto quality = analyzer.analyzeLogQuality();
    std::cout << "\n로그 품질 분석:" << std::endl;
    std::cout << "  완성도: " << (quality.completeness_score * 100) << "%" << std::endl;
    std::cout << "  일관성: " << (quality.consistency_score * 100) << "%" << std::endl;
    std::cout << "  정보성: " << (quality.informativeness_score * 100) << "%" << std::endl;
    
    if (!quality.recommendations.empty()) {
        std::cout << "  권장사항:" << std::endl;
        for (const auto& rec : quality.recommendations) {
            std::cout << "    - " << rec << std::endl;
        }
    }
    
    // 4. 복합 검색 테스트
    auto complex_results = analyzer.searchComplex("ERROR", "database", 2, std::chrono::hours(1));
    std::cout << "\n복합 검색 결과: " << complex_results.size() << "개" << std::endl;
    
    // 5. 성능 지표
    analyzer.printPerformanceMetrics();
}
```

---

## 📋 6. 체크리스트와 정리

### STL 알고리즘 마스터 체크리스트

#### 기본 알고리즘
- [ ] **검색**: find, find_if, search, count, count_if
- [ ] **조건 검사**: all_of, any_of, none_of
- [ ] **변환**: transform, for_each
- [ ] **누적**: accumulate, reduce, transform_reduce

#### 시퀀스 조작
- [ ] **복사**: copy, copy_if, copy_n
- [ ] **이동**: move, move_backward  
- [ ] **채우기**: fill, fill_n, generate, generate_n
- [ ] **교체**: replace, replace_if, replace_copy

#### 정렬과 분할
- [ ] **정렬**: sort, stable_sort, partial_sort
- [ ] **분할**: partition, stable_partition
- [ ] **N번째 요소**: nth_element
- [ ] **정렬 확인**: is_sorted, is_sorted_until

#### 이진 검색 (정렬된 범위)
- [ ] **검색**: binary_search, lower_bound, upper_bound
- [ ] **범위**: equal_range
- [ ] **병합**: merge, inplace_merge

#### 집합 연산 (정렬된 범위)
- [ ] **연산**: set_union, set_intersection, set_difference
- [ ] **포함**: includes
- [ ] **대칭차집합**: set_symmetric_difference

#### 힙 연산
- [ ] **힙 생성**: make_heap, push_heap, pop_heap
- [ ] **정렬**: sort_heap
- [ ] **확인**: is_heap, is_heap_until

#### 수치 알고리즘
- [ ] **누적**: accumulate, inner_product
- [ ] **부분합**: partial_sum, adjacent_difference
- [ ] **최소/최대**: min_element, max_element, minmax_element

### 복잡도 이해
- [ ] **O(1)**: 상수 시간 - 직접 접근
- [ ] **O(log n)**: 로그 시간 - 이진 검색
- [ ] **O(n)**: 선형 시간 - 순차 탐색, 변환
- [ ] **O(n log n)**: 로그선형 시간 - 정렬, 병합
- [ ] **O(n²)**: 제곱 시간 - 중첩 루프 (피해야 함)

### 성능 최적화
- [ ] **컨테이너 선택**: vector vs list vs deque
- [ ] **반복자 타입**: random_access vs bidirectional
- [ ] **메모리 지역성**: 캐시 친화적 접근
- [ ] **병렬화**: C++17 execution policies

### 실전 활용
- [ ] **조건부 로직**: 람다와 predicate 조합
- [ ] **파이프라인**: 알고리즘 체이닝
- [ ] **커스텀 비교자**: 정렬과 검색 최적화
- [ ] **함수 객체**: 상태 유지 및 재사용

---

*🔍 STL 알고리즘은 C++ 프로그래밍의 핵심 도구입니다. 올바른 알고리즘 선택과 적절한 사용법을 익혀서 효율적이고 읽기 쉬운 코드를 작성하세요. LogCaster 프로젝트에서 이러한 알고리즘들을 적극 활용하여 데이터 처리 전문가가 되어보세요!*