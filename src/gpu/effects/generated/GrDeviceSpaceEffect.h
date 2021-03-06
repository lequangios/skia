/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrDeviceSpaceEffect.fp; do not modify.
 **************************************************************************************************/
#ifndef GrDeviceSpaceEffect_DEFINED
#define GrDeviceSpaceEffect_DEFINED
#include "include/core/SkTypes.h"
#include "include/core/SkM44.h"

#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrFragmentProcessor.h"
class GrDeviceSpaceEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> fp,
                                                     const SkMatrix& matrix = SkMatrix::I()) {
        return std::unique_ptr<GrFragmentProcessor>(new GrDeviceSpaceEffect(std::move(fp), matrix));
    }
    GrDeviceSpaceEffect(const GrDeviceSpaceEffect& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "DeviceSpaceEffect"; }
    int fp_index = -1;
    SkMatrix matrix;

private:
    GrDeviceSpaceEffect(std::unique_ptr<GrFragmentProcessor> fp, SkMatrix matrix)
            : INHERITED(kGrDeviceSpaceEffect_ClassID, kNone_OptimizationFlags), matrix(matrix) {
        SkASSERT(fp);
        fp_index = this->numChildProcessors();
        fp->setSampledWithExplicitCoords();
        this->registerChildProcessor(std::move(fp));
    }
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    typedef GrFragmentProcessor INHERITED;
};
#endif
