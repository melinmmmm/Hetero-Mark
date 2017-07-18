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

#include "src/be/hc/be_hc_benchmark.h"
#include <cstdio>
#include <cstdlib>
#include <hc.hpp>

void BeHcBenchmark::Initialize() { BeBenchmark::Initialize(); }

void BeHcBenchmark::Run() {
  if (collaborative_execution_) {
    CollaborativeRun();
  } else {
    NormalRun();
  }
}

void BeHcBenchmark::CollaborativeRun() {
  printf("Collaborative run\n");
  uint32_t num_pixels = width_ * height_ * channel_;
  std::vector<uint8_t *> frames;
  uint8_t *decoding = NULL;
  uint8_t *extracting = NULL;
  uint8_t *encoding = NULL;

  std::vector<uint8_t> foreground;
  foreground.resize(num_pixels);

  // Initialize background
  decoding = nextFrame();
  frames.push_back(decoding);
  for (int i = 0; i < num_pixels; i++) {
    background_[i] = static_cast<float>(decoding[i]);
  }
  extracting = decoding;

  float alpha = alpha_;
  uint8_t threshold = threshold_;

  hc::array_view<float, 1> background(num_pixels, background_);
  hc::accelerator_view acc_view = hc::accelerator().get_default_view();
  hc::array_view<uint8_t, 1> av_foreground(num_pixels, foreground);

  for (uint64_t i = 0; i < num_frames_; i++) {
    printf("Frame %ld\n", i);

    // Extracting
    if (extracting != NULL) {
      hc::array_view<uint8_t, 1> av_frame(num_pixels, extracting);
      av_foreground.discard_data();
      hc::parallel_for_each(
          acc_view, hc::extent<1>(num_pixels), [=](hc::index<1> j)[[hc]] {
            uint8_t diff = 0;
            if (av_frame[j] > background[j]) {
              diff = av_frame[j] - background[j];
            } else {
              diff = -av_frame[j] + background[j];
            }

            if (diff > threshold) {
              av_foreground[j] = av_frame[j];
            } else {
              av_foreground[j] = 0;
            }
            background[j] = background[j] * (1 - alpha) + av_frame[j] * alpha;
          });
    }

    // encoding
    if (encoding != NULL) {
      cv::Mat output_frame(cv::Size(width_, height_), CV_8UC3, encoding,
                           cv::Mat::AUTO_STEP);
      video_writer_ << output_frame;
    }

    // Decoding
    decoding = nextFrame();
    frames.push_back(decoding);

    // Extracting done
    av_foreground.synchronize();
    encoding = foreground.data();
    extracting = decoding;
  }

  // Last frame
  if (encoding != NULL) {
    cv::Mat output_frame(cv::Size(width_, height_), CV_8UC3, encoding,
                         cv::Mat::AUTO_STEP);
    video_writer_ << output_frame;
  }

  video_writer_.release();

  for (auto frame : frames) {
    delete[] frame;
  }
}

void BeHcBenchmark::NormalRun() {
  printf("Normal run\n");
  uint32_t num_pixels = width_ * height_ * channel_;
  std::vector<uint8_t *> frames;
  std::vector<uint8_t> foreground;
  foreground.resize(num_pixels);

  // Initialize background
  uint8_t *frame = nextFrame();
  frames.push_back(frame);
  for (int i = 0; i < num_pixels; i++) {
    background_[i] = static_cast<float>(frame[i]);
  }

  float alpha = alpha_;
  uint8_t threshold = threshold_;

  hc::array_view<float, 1> background(num_pixels, background_);
  hc::accelerator_view acc_view = hc::accelerator().get_default_view();

  for (uint64_t i = 0; i < num_frames_; i++) {
    hc::array_view<uint8_t, 1> av_foreground(num_pixels, foreground);
    hc::array_view<uint8_t, 1> av_frame(num_pixels, frame);

    av_foreground.discard_data();
    hc::parallel_for_each(
        acc_view, hc::extent<1>(num_pixels), [=](hc::index<1> j)[[hc]] {
          uint8_t diff = 0;
          if (av_frame[j] > background[j]) {
            diff = av_frame[j] - background[j];
          } else {
            diff = -av_frame[j] + background[j];
          }

          if (diff > threshold) {
            av_foreground[j] = av_frame[j];
          } else {
            av_foreground[j] = 0;
          }
          background[j] = background[j] * (1 - alpha) + av_frame[j] * alpha;
        });
    av_foreground.synchronize();

    cv::Mat output_frame(cv::Size(width_, height_), CV_8UC3, foreground.data(),
                         cv::Mat::AUTO_STEP);
    video_writer_ << output_frame;

    frame = nextFrame();
    frames.push_back(frame);
  }

  video_writer_.release();

  for (auto frame : frames) {
    delete[] frame;
  }
}

void BeHcBenchmark::Cleanup() { BeBenchmark::Cleanup(); }
