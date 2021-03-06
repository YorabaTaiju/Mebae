/* coding: utf-8 */
/**
 * YorabaTaiju/Wakaba
 *
 * Copyright 2019-, Kaede Fujisaki
 */

#pragma once

#include <array>
#include <deque>
#include <memory>
#include <utility>
#include <algorithm>

#include "Optional.hpp"
#include "SubjectiveTime.hpp"

namespace donut {

template <typename Type, size_t length> class Value;

template <size_t length> class Clock {
public:
  explicit Clock() = default;

  Clock(Clock const&) = delete;
  Clock operator=(Clock const&) = delete;

  Clock(Clock<length>&&)  noexcept = default;
  Clock& operator=(Clock<length>&&)  noexcept = default;

  ~Clock() noexcept = default;

public:
  void tick() {
    subjectiveTime_.at_++;
  }

  SubjectiveTime const& leap(uint32_t const to) {
    subjectiveTime_.leap_++;
    subjectiveTime_.at_ = to;
    this->branches_.emplace_back(to);
    auto const currentLeap = subjectiveTime_.leap_;
    while((currentLeap - branchHorizon_) > length) {
      branchHorizon_++;
      branches_.pop_front();
    }
    for(auto it = this->branches_.begin(); it != branches_.end(); ++it) {
      *it = std::min(*it, to);
    }
    return subjectiveTime_;
  }

  [[nodiscard]] SubjectiveTime const& subjectiveTime() const {
    return this->subjectiveTime_;
  }

  [[nodiscard]] uint32_t leap() const {
    return this->subjectiveTime_.leap();
  }

  [[nodiscard]] uint32_t current() const {
    return this->subjectiveTime_.at();
  }

  template<typename T>
  [[nodiscard]] Value<T, length> newValue() {
    return Value<T, length>(*this);
  }

private:
  template <typename, size_t>
  friend class Value;

  [[nodiscard]] uint32_t timeToWatch(SubjectiveTime const& t) const {
    return t.leap() < this->branchHorizon_ ? t.at() : std::min(this->branches_[t.leap() - this->branchHorizon_], t.at());
  }
private:
  SubjectiveTime subjectiveTime_{};
private:
  std::deque<uint32_t> branches_ = {0};
  std::size_t branchHorizon_ = 0;
};

template <size_t length> class ShiftedClock final : Clock<length> {
public:
  explicit ShiftedClock(std::shared_ptr<Clock<length>> parent, SubjectiveTime const delta)
      : parent_(std::move(parent)), delta_(delta) {
  }
  ~ShiftedClock() = default;
public:
  [[nodiscard]] SubjectiveTime now() const {
    return this->parent_->subjectiveTime();// FIXME + this->delta_;
  }

private:
  SubjectiveTime delta_;
  std::shared_ptr<Clock<length>> parent_;
};

template<typename Type, std::size_t length>
class Value final {
public:
  Value() = delete;
  Value(Value const&) = delete;
  Value& operator=(Value const&) = delete;
  explicit Value(Clock<length>& clock)
  :clock_(clock)
  ,beg_(0)
  ,end_(0)
  ,lastModifiedLeap_(clock.subjectiveTime().leap())
  {
  }

public:
  Value<Type, length>& operator=(Type&& v) {
    this->set(std::forward<Type>(v));
    return *this;
  }

  Optional<Type const> get() const {
    return this->peek(this->clock_.subjectiveTime());
  }

  Optional<Type> get() {
    return this->peek(this->clock_.subjectiveTime());
  }

private:
  Value<Type, length>& set(Type&& v) {
    SubjectiveTime const t = clock_.subjectiveTime();
    auto& last = values_[end_-1];
    if (beg_ != end_ && std::get<0>(last) == t.at() && lastModifiedLeap_ == t.leap()) {
      std::get<1>(last) = std::forward<Type>(v);
    } else {
      auto const idx = std::get<1>(this->findEntry(t));
      end_ = idx + 1;
      values_[idx] = std::make_tuple(t.at(), std::forward<Type>(v));
      if((end_+1)%length == beg_) {
        beg_++;
      }
    }
    this->lastModifiedLeap_ = t.leap();
    return *this;
  }

  Optional<Type> peek(SubjectiveTime const& t) {
    auto& last = values_[end_-1];
    if (beg_ != end_ && std::get<0>(last) == t.at() && lastModifiedLeap_ == t.leap()) {
      return Optional<Type>(std::get<1>(last));
    } else {
      auto const idx = std::get<0>(this->findEntry(t));
      if (idx == end_) {
        return Optional<Type>();
      }
      return Optional<Type>(std::get<1>(values_[idx]));
    }
  }

  Optional<Type const> peek(SubjectiveTime const& t) const {
    auto const& last = values_[end_-1];
    if (beg_ != end_ && std::get<0>(last) == t.at() && lastModifiedLeap_ == t.leap()) {
      return Optional<Type const>(std::get<1>(last));
    } else {
      auto const idx = std::get<0>(this->findEntry(t));
      if (idx == end_) {
        return Optional<Type const>();
      }
      return Optional<Type const>(std::get<1>(values_[idx]));
    }
  }

  [[nodiscard]] std::tuple<size_t, size_t> findEntry(SubjectiveTime const& subjectiveTime) const {
    size_t beg = beg_;
    size_t end = end_;
    if(beg > end) {
      end = end + length;
    }
    uint32_t const t = clock_.timeToWatch(subjectiveTime);
    while((end-beg) > 1) {
      size_t mid = beg + (end-beg)/2;
      uint32_t const midTime = std::get<0>(values_[mid % length]);
      if (t <= midTime) {
        end = mid;
      } else {
        beg = mid;
      }
    }
    return std::make_tuple(beg, end);
  }

private:
  Clock<length>& clock_;
  std::array<std::tuple<uint32_t, Type>, length> values_;
  size_t beg_ = {};
  size_t end_ = {};
  uint32_t lastModifiedLeap_ = {};
};

}
