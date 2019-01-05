#ifndef CBLOX_CORE_TSDF_SUBMAP_COLLECTION_MAP_H_
#define CBLOX_CORE_TSDF_SUBMAP_COLLECTION_MAP_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <voxblox/core/tsdf_map.h>

#include "./TsdfSubmapCollection.pb.h"

#include "cblox/core/common.h"
#include "cblox/core/tsdf_submap.h"

namespace cblox {

using namespace voxblox;

class TsdfSubmapCollection {
 public:
  typedef std::shared_ptr<TsdfSubmapCollection> Ptr;
  typedef std::shared_ptr<const TsdfSubmapCollection> ConstPtr;

  // Constructor. Constructs an empty submap collection map
  explicit TsdfSubmapCollection(const TsdfMap::Config &tsdf_map_config)
      : tsdf_map_config_(tsdf_map_config) {}

  // Constructor. Constructs a submap collection from a list of submaps
  TsdfSubmapCollection(const TsdfMap::Config &tsdf_map_config,
                       const std::vector<TsdfSubmap::Ptr> &tsdf_sub_maps)
      : tsdf_map_config_(tsdf_map_config), tsdf_sub_maps_(tsdf_sub_maps) {}

  // Gets a vector of the linked IDs
  void getIDs(std::vector<SubmapID> *submap_ids) const;
  bool isBaseFrame(const SubmapID &submap_id) const;

  // Creates a new submap on the top of the collection
  void createNewSubMap(const Transformation &T_M_S, SubmapID submap_id);
  void createNewSubMap(const Transformation &T_M_S);

  // Create a new submap which duplicates an existing source submap
  bool duplicateSubMap(const SubmapID source_submap_id,
                       const SubmapID new_submap_id);

  // Gets a const pointer to a raw submap
  TsdfSubmap::ConstPtr getSubMap(const SubmapID submap_id) const {
    // NOTE(alexmillane): This departure from convension to return a const
    // pointer (rather than a const ref) is such that we can return the nullptr
    // to indicate the non-existance of requested submap.
    const auto it = id_to_submap_.find(submap_id);
    if (it != id_to_submap_.end()) {
      return it->second;
    } else {
      return TsdfSubmap::ConstPtr();
    }
  }

  // Gets a const reference to the raw submap container
  const std::vector<TsdfSubmap::Ptr> getSubMaps() const {
    std::vector<TsdfSubmap::Ptr> submap_ptrs;
    for (const auto &blah : id_to_submap_) {
      submap_ptrs.emplace_back(blah.second);
    }
    return submap_ptrs;
  }

  // Flattens the collection map down to a normal TSDF map
  TsdfMap::Ptr getProjectedMap() const;

  // Gets the pose of the patch on the tip of the collection
  const Transformation &getActiveSubMapPose() const {
    return getActiveTsdfSubMap().getPose();
  }

  // Gets the ID of the patch on the tip of the collection
  const SubmapID getActiveSubMapID() const { return active_submap_id_; }

  // Gets a pointer to the active tsdf_map
  TsdfMap::Ptr getActiveTsdfMapPtr() {
    const auto it = id_to_submap_.find(active_submap_id_);
    CHECK(it != id_to_submap_.end());
    return (it->second)->getTsdfMapPtr();
  }
  // Gets a reference to the active tsdf_map
  const TsdfMap &getActiveTsdfMap() const {
    const auto it = id_to_submap_.find(active_submap_id_);
    CHECK(it != id_to_submap_.end());
    return (it->second)->getTsdfMap();
  }

  // Gets a reference to the active tsdf_map
  const TsdfSubmap &getActiveTsdfSubMap() const {
    const auto it = id_to_submap_.find(active_submap_id_);
    CHECK(it != id_to_submap_.end());
    return *(it->second);
  }

  // KEYFRAME RELATED FUNCTION. REMOVING
  // Associates a to the active submap
  // void associateIDToActiveSubmap(const SubmapID submap_id) {
  //  id_to_submap_[submap_id] = tsdf_sub_maps_.back();
  //}

  // Gets the tsdf submap associated with the passed ID
  bool getAssociatedTsdfSubMapID(const SubmapID submap_id,
                                 SubmapID *submap_id_ptr) const;
  bool getTsdfSubmapConstPtrById(const SubmapID submap_id,
                                 TsdfSubmap::ConstPtr &submap_const_ptr) const;

  // Interacting with the submap poses
  bool setSubMapPose(const SubmapID submap_id, const Transformation &pose);
  void setSubMapPoses(const TransformationVector &transforms);
  bool getSubMapPose(const SubmapID submap_id, Transformation &pose) const;
  void getSubMapPoses(AlignedVector<Transformation> *submap_poses) const;

  // Clears the collection, leaving an empty map
  void clear() { id_to_submap_.clear(); }

  // Returns true if the collection is empty
  bool empty() const { return id_to_submap_.empty(); }

  // The size of the collection (number of patches)
  size_t size() const { return id_to_submap_.size(); }
  size_t num_patches() const { return id_to_submap_.size(); }

  // Returns the block size of the blocks in the tsdf patches
  FloatingPoint block_size() const {
    // All maps (should) have the same block size so we just grab the first.
    return (id_to_submap_.begin()->second)->block_size();
  }

  // Save the collection to file
  bool saveToFile(const std::string &file_path) const;

  // Getting various protos for this object
  void getProto(TsdfSubmapCollectionProto *proto) const;

  // Returns the config of the tsdf sub maps
  const TsdfMap::Config &getConfig() const { return tsdf_map_config_; }

  // Fusing the submap pairs
  void fuseSubmapPair(const SubmapIdPair &submap_id_pair);

  // Gets the number of allocated blocks in the collection
  size_t getNumberAllocatedBlocks() const;

 private:
  // TODO(alexmillane): Get some concurrency guards

  // The config used for the patches
  TsdfMap::Config tsdf_map_config_;

  // The vectors of patches
  std::vector<TsdfSubmap::Ptr> tsdf_sub_maps_;

  // The active SubmapID
  SubmapID active_submap_id_;

  // A map which keeps track of which ID belongs to which submap and stores the
  // patches
  std::map<SubmapID, TsdfSubmap::Ptr> id_to_submap_;
};

}  // namespace cblox

#endif /* CBLOX_CORE_TSDF_SUBMAP_COLLECTION_MAP_H_ */
