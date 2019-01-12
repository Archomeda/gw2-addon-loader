#pragma once
#include "../stdafx.h"
#include "AddonFunc.h"
#include "../Config.h"

namespace loader::addons {

    template<typename R, typename... Args>
    class AddonCallFunc : AddonFunc<R, Args...> {
        friend class Addon;
        friend class NativeAddon;

    public:
        AddonCallFunc() : AddonFunc<R, Args...>() { }
        AddonCallFunc(const std::string& funcName, std::function<R(Args...)> func) : AddonFunc<R, Args...>(funcName, func) { }

        AddonFunc<R, Args...>::operator();
        AddonFunc<R, Args...>::operator bool;
        AddonFunc<R, Args...>::operator!;

        AddonFunc<R, Args...>::IsValid;

        AddonFunc<R, Args...>::GetMetric;

        AddonFunc<R, Args...>::Call;

    protected:
        virtual void StartMetric() override {
            if (AppConfig.GetDiagnostics()) {
                this->metric.Start();
            }
        }

        virtual void EndMetric() override {
            if (AppConfig.GetDiagnostics()) {
                this->metric.Stop();
            }
        }
    };

}

