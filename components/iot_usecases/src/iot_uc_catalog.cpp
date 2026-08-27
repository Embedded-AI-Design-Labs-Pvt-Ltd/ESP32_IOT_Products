/**
 * @file iot_uc_catalog.cpp
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief C++ compilation unit for UseCaseEngine (ensures C++ linkage on IDF and host).
 */
#include "iot_uc.hpp"

extern "C" size_t iot_uc_cpp_catalog_size(void)
{
    return iot::UseCaseEngine::all().size();
}

extern "C" iot_err_t iot_uc_cpp_run_json(const char *json, iot_uc_out_t *out)
{
    iot::UseCaseEngine eng;
    if ((json == NULL) || (out == NULL)) {
        return IOT_ERR_INVALID_ARG;
    }
    return eng.runJson(json, *out);
}
