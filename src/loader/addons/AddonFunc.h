#pragma once
#include "../stdafx.h"
#include "exceptions.h"
#include "../diagnostics/HistoricTimeMetric.h"

namespace loader::addons {

    template<typename R, typename... Args>
    class AddonFunc {
        friend class Addon;
        friend class NativeAddon;

    public:
        AddonFunc() = default;
        AddonFunc(const std::string& funcName, std::function<R(Args...)> func) :
            FuncName(funcName),
            Func(func) { }

        R operator()(Args... args) {
            return this->Call(args...);
        }

        explicit operator bool() const {
            return this->IsValid();
        }

        bool operator!() const {
            return !this->IsValid();
        }

        bool IsValid() const {
            return this->Func != nullptr;
        }


        diagnostics::HistoricTimeMetric<1000000, 2>& GetMetric() {
            return this->metric;
        }

        template<typename T = R, typename = typename std::enable_if_t<!std::is_void_v<T>>>
        T Call(Args... args) {
            if (!this->IsValid()) {
                return {};
            }

            this->StartMetric();
            T result;
            try {
                try {
                    result = this->Func(args...);
                }
                catch (const std::exception& ex) {
                    throw exceptions::AddonFuncException(this->FuncName, ex.what());
                }
                catch (const char* err) {
                    throw exceptions::AddonFuncException(this->FuncName, err);
                }
                catch (const wchar_t* err) {
                    throw exceptions::AddonFuncException(this->FuncName, err);
                }
                catch (...) {
                    throw exceptions::AddonFuncException(this->FuncName, "Unknown error");
                }
            }
            catch (const exceptions::AddonFuncException& ex) {
                this->EndMetric();
                throw ex;
            }

            this->EndMetric();
            return result;
        }

        template<typename T = R, typename = typename std::enable_if_t<std::is_void_v<T>>>
        void Call(Args... args) {
            if (!this->IsValid()) {
                return;
            }

            this->StartMetric();
            try {
                try {
                    this->Func(args...);
                }
                catch (const std::exception& ex) {
                    throw exceptions::AddonFuncException(this->FuncName, ex.what());
                }
                catch (const char* err) {
                    throw exceptions::AddonFuncException(this->FuncName, err);
                }
                catch (const wchar_t* err) {
                    throw exceptions::AddonFuncException(this->FuncName, err);
                }
                catch (...) {
                    throw exceptions::AddonFuncException(this->FuncName, "Unknown error");
                }
            }
            catch (const exceptions::AddonFuncException& ex) {
                this->EndMetric();
                throw ex;
            }

            this->EndMetric();
        }


    protected:
        std::function<R(Args...)> Func;
        std::string FuncName;

        virtual void StartMetric() = 0;
        virtual void EndMetric() = 0;

        diagnostics::HistoricTimeMetric<1000000, 2> metric;
    };

}
