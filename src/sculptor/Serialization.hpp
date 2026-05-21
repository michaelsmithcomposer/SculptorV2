#pragma once

#include <Geode/Geode.hpp>
#include "../lib/Manager.hpp"

using namespace geode::prelude;
using namespace Sculptor;

template <>
struct matjson::Serialize<CCPoint> {
    static geode::Result<CCPoint> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const CCPoint& point);
};

template <>
struct matjson::Serialize<Sequence> {
    static geode::Result<Sequence> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const Sequence& sequence);
};

template <>
struct matjson::Serialize<BezierCurve> {
    static geode::Result<BezierCurve> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const BezierCurve& curve);
};

template <>
struct matjson::Serialize<Sculptor::Modulator*> {
    static geode::Result<Sculptor::Modulator*> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const Sculptor::Modulator* modulator);
};

template <>
struct matjson::Serialize<Sculptor::Layer*> {
    static geode::Result<Sculptor::Layer*> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const Sculptor::Layer* layer);
};

template <>
struct matjson::Serialize<Form*> {
    static geode::Result<Form*> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const Form* form);
};

template <>
struct matjson::Serialize<Manager*> {
    static geode::Result<Manager*> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const Manager* manager);
};

