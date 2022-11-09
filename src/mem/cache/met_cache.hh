/**
 * @file
 * Specifies a Met cache based on non-coherent cache
 */

#ifndef __MEM_CACHE_MET_CACHE_HH__
#define __MEM_CACHE_MET_CACHE_HH__

#include "base/compiler.hh"
#include "base/logging.hh"
#include "base/types.hh"
#include "mem/cache/noncoherent_cache.hh"
#include "mem/mem_org.hh"
#include "mem/packet.hh"


namespace gem5
{

class CacheBlk;
class MSHR;
struct MetCacheParams;

/**
 * A special cache for Merkle Tree & counter cache
 */
class MetCache : public NoncoherentCache
{
  private:
    memory::MemOrg OOPMetaData;
    memory::MemOrg HomeMetaData;

    /**
     * The SecMemCtrl-side port extends the cpu side port with access
     * functions for timing requests.
     */
    class SecMemCtrlSidePort : public CpuSidePort
    {
      private:

        // a pointer to our specific cache implementation
        MetCache *cache;

      protected:

        virtual bool recvTimingReq(PacketPtr pkt) override;

      public:

        SecMemCtrlSidePort(const std::string &_name, MetCache *_cache,
                    const std::string &_label);           
    };

    SecMemCtrlSidePort secMemCtrlSidePort;


  protected:

    bool handleEvictions(std::vector<CacheBlk*> &evict_blks,
        PacketList &writebacks) override;

    void readCtrMtreeHelper(PacketPtr pkt);
    void writeCtrMtreeHelper(PacketPtr pkt);

    void recvTimingReq(PacketPtr pkt) override;


  public:
    MetCache(const MetCacheParams &p);

    Port &getPort(const std::string &if_name, PortID idx);
};

} // namespace gem5

#endif // __MEM_CACHE_METCACHE_HH__
