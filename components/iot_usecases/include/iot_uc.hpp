#ifndef IOT_UC_HPP
#define IOT_UC_HPP

#include "iot_uc.h"
#include <string>
#include <vector>

namespace iot {

/** C++ catalog over the C use-case engine (same behavior, typed listing). */
class UseCaseEngine {
public:
    static iot_err_t init()
    {
        return iot_uc_init();
    }

    static size_t count()
    {
        return iot_uc_count();
    }

    static const char *name(iot_uc_id_t id)
    {
        return iot_uc_name(id);
    }

    static const char *summary(iot_uc_id_t id)
    {
        return iot_uc_summary(id);
    }

    static iot_uc_id_t idFromName(const std::string &n)
    {
        return iot_uc_id_from_name(n.c_str());
    }

    static std::vector<iot_uc_id_t> all()
    {
        std::vector<iot_uc_id_t> v;
        v.reserve(IOT_UC_COUNT);
        for (int i = 1; i < (int)IOT_UC_COUNT; i++) {
            v.push_back((iot_uc_id_t)i);
        }
        return v;
    }

    iot_err_t run(iot_uc_id_t id, const iot_uc_in_t &in, iot_uc_out_t &out) const
    {
        return iot_uc_run(id, &in, &out);
    }

    iot_err_t runJson(const std::string &json, iot_uc_out_t &out) const
    {
        return iot_uc_run_json(json.c_str(), json.size(), &out);
    }
};

} /* namespace iot */

#endif /* IOT_UC_HPP */
