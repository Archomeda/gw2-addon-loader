#pragma once
#include <functional>
#include <string>
#include "AddonMetric.h"
#include "exceptions.h"
#include "../Config.h"

namespace loader {
    namespace addons {

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


            AddonMetric& GetMetric() {
                return this->metric;
            }

            void SetGlobalMetric(AddonMetric* globalMetric) {
                this->globalMetric = globalMetric;
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


        private:
            void StartMetric() {
                if (AppConfig.GetShowDebugFeatures()) {
                    if (this->globalMetric != nullptr) {
                        this->globalMetric->StartMeasurement();
                    }
                    this->metric.StartMeasurement();
                }
            }

            void EndMetric() {
                if (AppConfig.GetShowDebugFeatures()) {
                    if (this->globalMetric != nullptr) {
                        this->globalMetric->EndMeasurement();
                    }
                    this->metric.EndMeasurement();
                }
            }


            AddonMetric metric;
            AddonMetric* globalMetric = nullptr;
        };

    }
}
