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
 *  @file JPetFilterRidgelet.h
 */

#ifndef _JPetFilterRidgelet_H_
#define _JPetFilterRidgelet_H_
#include "./JPetFilterInterface.h"
#include <cmath>

/*! \brief Filter F(x) = sqrt(x)
*/
class JPetFilterRidgelet : public JPetFilterInterface
{
public:
  JPetFilterRidgelet() {}
  explicit JPetFilterRidgelet(double maxCutOff) : fCutOff(maxCutOff) {}
  virtual double operator()(double pos) override {
    return 0.;
    /*if(n == 0) {
      return 0.02;
    }
    double ramlak = (double)(n) / (double) (m);
    if (ramlak >= fCutOff) return 0.;
    return ramlak * std::sqrt((double)(n) / ((double)(m) + fCutOff));*/
  }

private:
  JPetFilterRidgelet(const JPetFilterRidgelet &) = delete;
  JPetFilterRidgelet &operator=(const JPetFilterRidgelet &) = delete;

  double fCutOff = 1.0f;
};

#endif /*  !_JPetFilterRidgelet_H_ */
