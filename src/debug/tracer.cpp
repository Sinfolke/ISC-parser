#include <debug/tracer.h>

#ifdef ENABLE_TRACER
# ifdef _WIN32
// windows code
    static void stack_trace_t::initSymbolHandler() {
        SymInitialize(GetCurrentProcess(), nullptr, TRUE);
    }
    static void stack_trace_t::cleanupSymbolHandler() {
        SymCleanup(GetCurrentProcess());
    }
    static std::string stack_trace_t::getSymbolName(void* addr) {
        // alloc buffer
        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO symbol = reinterpret_cast<PSYMBOL_INFO>(buffer);

        // add buffer properties
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        // get the symbol name
        DWORD64 displacement = 0;
        HANDLE process = GetCurrentProcess();
        return symbol->Name;
    }
    const char* stack_trace_t::getCallerName() {
        return getSymbolName(caller);
    }
    const char* stack_trace_t::getFuncName() {
        return getSymbolName(func);
    }
# else
// linux code
    const char* stack_trace_t::getCallerName() {
        Dl_info caller_info;
        dladdr(func, &caller_info);
        return caller_info.dli_sname;
    }
    const char* stack_trace_t::getFuncName() {
        Dl_info func_info;
        dladdr(func, &func_info);
        return func_info.dli_sname;
    }
# endif
std::vector<stack_trace_t> call_trace {};
extern "C" {
    void __attribute__((no_instrument_function))
    __cyg_profile_func_enter(void *func, void *caller) {
        stack_trace_t stack(func, caller);
        call_trace.push_back(stack);
    }

    void __attribute__((no_instrument_function))
    __cyg_profile_func_exit(void *func, void *caller) {
        call_trace.pop_back();
    }
}

#endif // ENABLE_TRACER