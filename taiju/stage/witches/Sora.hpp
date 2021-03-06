/* coding: utf-8 */
/**
 * wakaba
 *
 * Copyright 2020-, Kaede Fujisaki
 */
#pragma once

#include "Witch.hpp"

namespace taiju {

class Bullet;
class Sora final : Witch {
public:
  void hit(std::shared_ptr<Bullet> bullet);
};

}
