// *************************************************************************
//
// Licensed under the MIT License (see accompanying LICENSE file).
//
// The authors of this code are: Gabriel Studer, Gerardo Tauriello
// 
// Based on mmtf_c developed by Julien Ferte (http://www.julienferte.com/),
// Anthony Bradley, Thomas Holder with contributions from Yana Valasatava,
// Gazal Kalyan, Alexander Rose.
//
// *************************************************************************

#ifndef MMTF_STRUCTURE_DATA_H
#define MMTF_STRUCTURE_DATA_H

#include <string>
#include <vector>
#include <stdint.h>
#include <sstream>
#include <limits>

namespace mmtf {

/**
 * @brief MMTF spec version which this library implements
 */
#define MMTF_SPEC_VERSION_MAJOR 1
#define MMTF_SPEC_VERSION_MINOR 0

/**
 * @brief Get string representation of MMTF spec version implemented here
 */
inline std::string getVersionString();

/**
 * @brief Check if version is supported (minor revisions ok, major ones not)
 * @return true if supported, false if not
 */
inline bool isVersionSupported(const std::string& version_string);

/**
 * @brief Group (residue) level data store
 *
 * https://github.com/rcsb/mmtf/blob/HEAD/spec.md#group-data
 */
struct GroupType {
    std::vector<int32_t>      formalChargeList;
    std::vector<std::string>  atomNameList;
    std::vector<std::string>  elementList;
    std::vector<int32_t>      bondAtomList;
    std::vector<int8_t>       bondOrderList;
    std::string               groupName;
    char                      singleLetterCode;
    std::string               chemCompType;
};

/**
 * @brief Entity type.
 *
 * https://github.com/rcsb/mmtf/blob/HEAD/spec.md#entitylist
 */
struct Entity {
    std::vector<int32_t> chainIndexList;
    std::string          description;
    std::string          type;
    std::string          sequence;
};

/**
 * @brief Transformation definition for a set of chains.
 *
 * https://github.com/rcsb/mmtf/blob/HEAD/spec.md#bioassemblylist
 */
struct Transform {
    std::vector<int32_t> chainIndexList;
    float                matrix[16];
};

/**
 * @brief Data store for the biological assembly annotation.
 *
 * https://github.com/rcsb/mmtf/blob/HEAD/spec.md#bioassemblylist
 */
struct BioAssembly {
    std::vector<Transform> transformList;
    std::string            name;
};

/**
 * @brief Top level MMTF data container.
 *
 * Default values (mmtf::isDefaultValue, mmtf::setDefaultValue) are set in
 * constructor and can be used to check if value was never set (only relevant
 * for optional values):
 * - default for vectors and strings: empty
 * - default for numeric types (incl. char): max. value of that type
 * - default for numXX = 0
 *
 * https://github.com/rcsb/mmtf/blob/HEAD/spec.md#fields
 */
struct StructureData {
    std::string                      mmtfVersion;
    std::string                      mmtfProducer;
    std::vector<float>               unitCell;
    std::string                      spaceGroup;
    std::string                      structureId;
    std::string                      title;
    std::string                      depositionDate;
    std::string                      releaseDate;
    std::vector<std::vector<float> > ncsOperatorList;
    std::vector<BioAssembly>         bioAssemblyList;
    std::vector<Entity>              entityList;
    std::vector<std::string>         experimentalMethods;
    float                            resolution;
    float                            rFree;
    float                            rWork;
    int32_t                          numBonds;
    int32_t                          numAtoms;
    int32_t                          numGroups;
    int32_t                          numChains;
    int32_t                          numModels;
    std::vector<GroupType>           groupList;
    std::vector<int32_t>             bondAtomList;
    std::vector<int8_t>              bondOrderList;
    std::vector<float>               xCoordList;
    std::vector<float>               yCoordList;
    std::vector<float>               zCoordList;
    std::vector<float>               bFactorList;
    std::vector<int32_t>             atomIdList;
    std::vector<char>                altLocList;
    std::vector<float>               occupancyList;
    std::vector<int32_t>             groupIdList;
    std::vector<int32_t>             groupTypeList;
    std::vector<int8_t>              secStructList;
    std::vector<char>                insCodeList;
    std::vector<int32_t>             sequenceIndexList;
    std::vector<std::string>         chainIdList;
    std::vector<std::string>         chainNameList;
    std::vector<int32_t>             groupsPerChain;
    std::vector<int32_t>             chainsPerModel;

    /**
     * @brief Construct object with default values set.
     */
    StructureData();

    /**
     * @brief Check consistency of structural data.
     * @return True if all required fields are set and vector sizes and indices
     *         are consistent.
     */
    bool hasConsistentData();
};

/**
 * @brief Get default value for given type.
 */
template <typename T>
inline T getDefaultValue();

/**
 * @return True if given value is default.
 * @tparam T Can be any numeric type, vector of string
 */
template <typename T>
inline bool isDefaultValue(const T& value);
template <typename T>
inline bool isDefaultValue(const std::vector<T>& value);
template <>
inline bool isDefaultValue(const std::string& value);

/**
 * @brief Set default value to given type.
 * @tparam T Can be any numeric type (no need for vector or strings here)
 */
template <typename T>
inline void setDefaultValue(T& value);


// *************************************************************************
// IMPLEMENTATION
// *************************************************************************

// helpers in anonymous namespace (only visible in this file)
namespace {

// check optional date string
// -> either default or "YYYY-MM-DD" (only string format checked, not date)
bool isValidDateFormatOptional(const std::string& s) {
    // default?
    if (isDefaultValue(s)) return true;
    // check length
    if (s.length() != 10) return false;
    // check delimiters
    if (s[4] != '-' || s[7] != '-') return false;
    // check format
    std::istringstream is(s);
    int d, m, y;
    char dash1, dash2;
    if (is >> y >> dash1 >> m >> dash2 >> d) {
        return (dash1 == '-' && dash2 == '-');
    } else {
        return false;
    }
}

// check if optional vector has right size
template<typename T>
bool hasRightSizeOptional(const std::vector<T>& v, int exp_size) {
    return (isDefaultValue(v) || v.size() == exp_size);
}

// check if all indices in vector are in [0, num-1] (T = integer type)
template<typename T, typename Tnum>
bool hasValidIndices(const T* v, size_t size, Tnum num) {
    T tnum = T(num);
    for (size_t i = 0; i < size; ++i) {
        if (v[i] < T(0) || v[i] >= tnum) return false;
    }
    return true;
}
template<typename T, typename Tnum>
bool hasValidIndices(const std::vector<T>& v, Tnum num) {
    if (v.empty()) return true;
    else           return hasValidIndices(&v[0], v.size(), num);
}

} // anon ns

// VERSIONING

inline std::string getVersionString() {
    std::stringstream version;
    version << MMTF_SPEC_VERSION_MAJOR << "." << MMTF_SPEC_VERSION_MINOR;
    return version.str();
}

inline bool isVersionSupported(const std::string& version_string) {
    std::stringstream ss(version_string);
    int major = -1;
    return ((ss >> major) && (major <= MMTF_SPEC_VERSION_MAJOR));
}

// CLASS StructureData

StructureData::StructureData() {
    // no need to do anything with strings and vectors
    setDefaultValue(resolution);
    setDefaultValue(rFree);
    setDefaultValue(rWork);
    // numXX set to 0 to have consistent data
    numBonds = 0;
    numAtoms = 0;
    numGroups = 0;
    numChains = 0;
    numModels = 0;
    // set version and producer
    mmtfVersion = getVersionString();
    mmtfProducer = "mmtf-cpp library (github.com/rcsb/mmtf-cpp)";
}

bool StructureData::hasConsistentData() {
    // check unitCell: if given, must be of length 6
    if (!hasRightSizeOptional(unitCell, 6)) return false;
    // check dates
    if (!isValidDateFormatOptional(depositionDate)) return false;
    if (!isValidDateFormatOptional(releaseDate)) return false;
    // check ncsOperatorList: all elements must have length 16
    for (size_t i = 0; i < ncsOperatorList.size(); ++i) {
        if (ncsOperatorList[i].size() != 16) return false;
    }
    // check chain indices in bioAssembly-transforms and entities
    for (size_t i = 0; i < bioAssemblyList.size(); ++i) {
        const BioAssembly& ba = bioAssemblyList[i];
        for (size_t j = 0; j < ba.transformList.size(); ++j) {
            const Transform & t = ba.transformList[j];
            if (!hasValidIndices(t.chainIndexList, numChains)) return false;
        }
    }
    for (size_t i = 0; i < entityList.size(); ++i) {
        const Entity& ent = entityList[i];
        if (!hasValidIndices(ent.chainIndexList, numChains)) return false;
    }
    // check groups
    for (size_t i = 0; i < groupList.size(); ++i) {
        const GroupType& g = groupList[i];
        const size_t num_atoms = g.formalChargeList.size();
        if (g.atomNameList.size() != num_atoms) return false;
        if (g.elementList.size() != num_atoms) return false;
        if (g.bondAtomList.size() != g.bondOrderList.size() * 2) return false;
        if (!hasValidIndices(g.bondAtomList, num_atoms)) return false;
    }
    // check global bonds
    if (bondAtomList.size() != bondOrderList.size() * 2) return false;
    if (!hasValidIndices(bondAtomList, numAtoms)) return false;
    // check vector sizes
    if (xCoordList.size() != numAtoms) return false;
    if (yCoordList.size() != numAtoms) return false;
    if (zCoordList.size() != numAtoms) return false;
    if (!hasRightSizeOptional(bFactorList, numAtoms)) return false;
    if (!hasRightSizeOptional(atomIdList, numAtoms)) return false;
    if (!hasRightSizeOptional(altLocList, numAtoms)) return false;
    if (!hasRightSizeOptional(occupancyList, numAtoms)) return false;
    if (groupIdList.size() != numGroups) return false;
    if (groupTypeList.size() != numGroups) return false;
    if (!hasRightSizeOptional(secStructList, numGroups)) return false;
    if (!hasRightSizeOptional(insCodeList, numGroups)) return false;
    if (!hasRightSizeOptional(sequenceIndexList, numGroups)) return false;
    if (chainIdList.size() != numChains) return false;
    if (!hasRightSizeOptional(chainNameList, numChains)) return false;
    if (groupsPerChain.size() != numChains) return false;
    if (chainsPerModel.size() != numModels) return false;
    // check indices
    if (!hasValidIndices(groupTypeList, groupList.size())) return false;
    // collect sequence lengths from entities and use to check
    std::vector<int32_t> sequenceIndexSize(numChains);
    for (size_t i = 0; i < entityList.size(); ++i) {
        const Entity& ent = entityList[i];
        for (size_t j = 0; j < ent.chainIndexList.size(); ++j) {
            sequenceIndexSize[ent.chainIndexList[j]] = ent.sequence.length();
        }
    }
    // traverse structure for more checks
    int bond_count = bondOrderList.size();
    int chain_idx = 0; // will be count at end of loop
    int group_idx = 0; // will be count at end of loop
    int atom_idx = 0;  // will be count at end of loop
    // traverse models
    for (int model_idx = 0; model_idx < numModels; ++model_idx) {
        // traverse chains
        for (int j = 0; j < chainsPerModel[model_idx]; ++j, ++chain_idx) {
            // check chain names (fixed length)
            if (chainIdList[chain_idx].size() != 4) return false;
            if (   !isDefaultValue(chainNameList)
                && chainNameList[chain_idx].size() != 4) return false;
            // traverse groups
            for (int k = 0; k < groupsPerChain[chain_idx]; ++k, ++group_idx) {
                // check seq. idx
                if (!isDefaultValue(sequenceIndexList)) {
                    const int32_t idx = sequenceIndexList[group_idx];
                    // -1 is ok here
                    if (idx < -1 || idx >= sequenceIndexSize[chain_idx]) {
                        return false;
                    }
                }
                // count atoms
                const GroupType& group = groupList[groupTypeList[group_idx]];
                atom_idx += group.atomNameList.size();
                // count bonds
                bond_count += group.bondOrderList.size();
            }
        }
    }
    // check sizes
    if (bond_count != numBonds) return false;
    if (chain_idx != numChains) return false;
    if (group_idx != numGroups) return false;
    if (atom_idx != numAtoms) return false;

    return true;
}

// DEFAULT VALUES

template <typename T>
inline T getDefaultValue() { return std::numeric_limits<T>::max(); }

template <typename T>
inline bool isDefaultValue(const T& value) {
    return (value == getDefaultValue<T>());
}
template <typename T>
inline bool isDefaultValue(const std::vector<T>& value) {
    return value.empty();
}
template <>
inline bool isDefaultValue(const std::string& value) {
    return value.empty();
}

template <typename T>
inline void setDefaultValue(T& value) {
    value = getDefaultValue<T>();
}

} // mmtf namespace

#endif
