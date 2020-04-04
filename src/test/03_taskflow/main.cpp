#include <taskflow/taskflow.hpp>  // Cpp-Taskflow is header-only

int main() {
    tf::Executor executor;
    tf::Taskflow taskflow;
    auto [A, B, C, D] = taskflow.emplace(
        []() { std::cout << "TaskA\n"; },               //  task dependency graph
        []() { std::cout << "TaskB\n"; },               // 
        []() { std::cout << "TaskC\n"; },               //          +---+          
        []() { std::cout << "TaskD\n"; }                //    +---->| B |-----+   
    );                                                 //    |     +---+     |
                                                       //  +---+           +-v-+ 
    A.precede(B);  // A runs before B                  //  | A |           | D | 
    A.precede(C);  // A runs before C                  //  +---+           +-^-+ 
    B.precede(D);  // B runs before D                  //    |     +---+     |    
    C.precede(D);  // C runs before D                  //    +---->| C |-----+    
                                                       //          +---+          
    executor.run(taskflow).wait();

    return 0;
}
