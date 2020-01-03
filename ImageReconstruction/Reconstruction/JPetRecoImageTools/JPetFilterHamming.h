/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  @file JPetFilterHamming.h
 */

#ifndef _JPetFilterHamming_H_
#define _JPetFilterHamming_H_
#include "./JPetFilterInterface.h"
#include <cmath>

/*! \brief Filter F(x) = x * (alpha + (1. - alpha * cos (2. * pi * x)))
 * default alpha = 0.54
 */
class JPetFilterHamming : public JPetFilterInterface {
public:
  JPetFilterHamming() {}
  explicit JPetFilterHamming(double maxCutOff) : fCutOff(maxCutOff) {}
  virtual double operator()(double pos) override
  {
    return pos < fCutOff ? 0.54 + 0.46 * std::cos((2. * M_PI * pos) / fCutOff) : 0.;
  }

private:
  double fCutOff = 1.0f;
  JPetFilterHamming(const JPetFilterHamming&) = delete;
  JPetFilterHamming& operator=(const JPetFilterHamming&) = delete;
};

#endif /*  !_JPetFilterHamming_H_ */
