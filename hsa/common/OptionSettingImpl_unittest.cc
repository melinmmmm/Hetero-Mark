/*
 * Hetero-mark
 *
 * Copyright (c) 2015 Northeastern University
 * All rights reserved.
 *
 * Developed by:
 *   Northeastern University Computer Architecture Research (NUCAR) Group
 *   Northeastern University
 *   http://www.ece.neu.edu/groups/nucar/
 *
 * Author: Yifan Sun (yifansun@coe.neu.edu)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal with the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 *   Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimers.
 *
 *   Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimers in the 
 *   documentation and/or other materials provided with the distribution.
 *
 *   Neither the names of NUCAR, Northeastern University, nor the names of 
 *   its contributors may be used to endorse or promote products derived 
 *   from this Software without specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS WITH THE SOFTWARE.
 */

#include "hsa/common/OptionSettingImpl.h"
#include "hsa/common/OptionSetting.h"
#include "hsa/common/Argument.h"
#include "gtest/gtest.h"

TEST(OptionSettingImpl, Iterator) {
  // Create environment
  std::unique_ptr<OptionSetting> optionSetting(new OptionSettingImpl());
  std::unique_ptr<Argument> arg1(new Argument("arg1"));
  std::unique_ptr<Argument> arg2(new Argument("arg2"));
  optionSetting->addArgument(std::move(arg1));
  optionSetting->addArgument(std::move(arg2));

  // Get the iterator
  std::unique_ptr<OptionSetting::Iterator> it = optionSetting->getIterator();

  // Iterator has next when it intialize
  EXPECT_TRUE(it->hasNext());

  // Two argument can be in any order
  Argument *arg = it->next();
  EXPECT_STREQ("arg1", arg->getName().c_str());
  arg = it->next();
  EXPECT_STREQ("arg2", arg->getName().c_str());

  // At this time, no next argument
  EXPECT_FALSE(it->hasNext());
}


