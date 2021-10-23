#include "ResourceCache.h"
#include "lppch.h"

#include "AssetManager.h"

namespace Lamp {
std::unordered_map<std::filesystem::path, Ref<Asset>>
    ResourceCache::s_AssetCache;
std::mutex ResourceCache::s_CacheMutex;

bool ResourceCache::AddAsset(const std::filesystem::path &path,
                             Ref<Asset> &asset) {
  if (s_AssetCache.find(path) == s_AssetCache.end()) {
    s_AssetCache.insert(std::make_pair(path, asset));
    return true;
  }
  return false;
}

void ResourceCache::Update() {
  std::lock_guard<std::mutex> lock(s_CacheMutex);

  for (auto &asset : s_AssetCache) {
    if (asset.second.use_count() <= 1) {
      s_AssetCache.erase(asset.first);
      break;
    }
  }
}
} // namespace Lamp