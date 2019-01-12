#pragma once
#include "../stdafx.h"
#include "AddonFunc.h"
#include "../Config.h"
#include "../diagnostics/HistoricTimeMetric.h"

namespace loader::addons {

    template<typename R, typename... Args>
    class AddonFrameFunc : AddonFunc<R, Args...> {
        friend class Addon;
        friend class NativeAddon;

    public:
        AddonFrameFunc() : AddonFunc<R, Args...>() { }
        AddonFrameFunc(const std::string& funcName, std::function<R(Args...)> func) : AddonFunc<R, Args...>(funcName, func) { }

        AddonFunc<R, Args...>::operator();
        AddonFunc<R, Args...>::operator bool;
        AddonFunc<R, Args...>::operator!;

        AddonFunc<R, Args...>::IsValid;

        AddonFunc<R, Args...>::GetMetric;

        void SetGlobalMetric(diagnostics::HistoricTimeMetric<1000000, 2> *globalMetric) {
            this->globalMetric = globalMetric;
        }

        AddonFunc<R, Args...>::Call;

    protected:
        virtual void StartMetric() override {
            if (AppConfig.GetDiagnostics()) {
                if (this->globalMetric != nullptr) {
                    this->globalMetric->Continue();
                }
                this->metric.Continue();
            }
        }

        virtual void EndMetric() override {
            if (AppConfig.GetDiagnostics()) {
                if (this->globalMetric != nullptr) {
                    this->globalMetric->Pause();
                }
                this->metric.Pause();
            }
        }

        diagnostics::HistoricTimeMetric<1000000, 2>* globalMetric = nullptr;
    };

}

