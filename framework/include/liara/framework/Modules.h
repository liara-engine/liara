/**
 * @file Modules.h
 * @brief ABI version negotiation across a set of loaded modules.
 */

#pragma once

#include <liara/abi_version.h>
#include <liara/modules.h>

#include <format>
#include <initializer_list>
#include <ostream>

namespace Liara::Framework
{
    /**
     * @brief Check that every module reports an ABI version this build can talk to.
     *
     * Every module is checked even after one fails, so that a host reporting the result names all
     * of the incompatible modules at once rather than the first one.
     *
     * @param modules The module descriptions to check. A null entry counts as a failure.
     * @param out The stream each verdict is written to.
     * @return True when every module is compatible, false otherwise.
     */
    inline bool ModulesAreCompatible(const std::initializer_list<const liara_module_info_t*> modules,
                                     std::ostream& out) {
        bool compatible = true;

        for (const auto* module : modules) {
            if (module == nullptr) {
                out << "Error: Failed to retrieve module information.\n";
                compatible = false;
                continue;
            }

            if (const liara_version_compat_t compat = liara_abi_is_compatible(module->abi_version);
                compat == LIARA_VERSION_COMPAT_EXACT || compat == LIARA_VERSION_COMPAT_COMPATIBLE) {
                out << std::format("{} {} is available and compatible (ABI {}).\n",
                                   module->module_name,
                                   module->module_version_str,
                                   module->abi_version_str);
            }
            else {
                out << std::format("Error: {} {} is not compatible with ABI {}.\n",
                                   module->module_name,
                                   module->module_version_str,
                                   module->abi_version_str);
                compatible = false;
            }
        }
        return compatible;
    }
}  // namespace Liara::Framework
