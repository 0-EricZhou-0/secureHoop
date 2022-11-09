/**
 * @file
 * Cache definitions.
 */

#include "mem/cache/met_cache.hh"

#include <cassert>

#include "base/logging.hh"
#include "base/trace.hh"
#include "base/types.hh"
#include "debug/Cache.hh"
#include "mem/cache/cache_blk.hh"
#include "mem/cache/mshr.hh"
#include "mem/cache/met_cache.hh"
#include "mem/cache/tags/super_blk.hh"
#include "params/MetCache.hh"

namespace gem5
{

//TODO
MetCache::MetCache(const MetCacheParams &p)
    : NoncoherentCache(p, p.system->cacheLineSize())
{
    assert(p.tags);
    assert(p.replacement_policy);
    // currently hard-coded, really dirty
    memory::initMemOrg(0, 16 * MB, &OOPMetaData, true);
    memory::initMemOrg(16 * MB, 8 * GB - 16 * MB ,&HomeMetaData, false);
}


bool
BaseCache::CpuSidePort::recvTimingReq(PacketPtr pkt)
{
    assert(pkt->isRequest());

    if (cache->system->bypassCaches()) {
        // Just forward the packet if caches are disabled.
        // @todo This should really enqueue the packet rather
        [[maybe_unused]] bool success = cache->memSidePort.sendTimingReq(pkt);
        assert(success);
        return true;
    } else if (tryTiming(pkt)) {
        cache->recvTimingReq(pkt);
        return true;
    }
    return false;
}

bool MetCache::handleEvictions(std::vector<CacheBlk*> &evict_blks,PacketList &writebacks){
    bool replacement = false;
    for (const auto& blk : evict_blks) {
        if (blk->isValid()) {
            replacement = true;

            const MSHR* mshr =
                mshrQueue.findMatch(regenerateBlkAddr(blk), blk->isSecure());
            if (mshr) {
                // Must be an outstanding upgrade or clean request on a block
                // we're about to replace
                assert((!blk->isSet(CacheBlk::WritableBit) &&
                    mshr->needsWritable()) || mshr->isCleaning());
                return false;
            }
        }
    }

    // The victim will be replaced by a new entry, so increase the replacement
    // counter if a valid block is being replaced
    if (replacement) {
        stats.replacements++;

        // Evict valid blocks associated to this victim block
        for (auto& blk : evict_blks) {
            if (blk->isValid()) {
                if (blk->isSet(CacheBlk::DirtyBit)){
                    memory::CtrMtreeEntry evicted_met_parent = memory::getMtreeEvictParent(regenerateBlkAddr(blk),&HomeMetaData);
                    // this is just a request to carry the BMT traversing
                    RequestPtr new_request_read = std::make_shared<Request>();
                    new_request_read->setPaddr(evicted_met_parent.paddr);
                    PacketPtr pkt_read = new Packet(new_request_read, MemCmd::ReadReq);
                    readCtrMtreeHelper(pkt_read);
                    RequestPtr new_request_write = std::make_shared<Request>();
                    new_request_write->setPaddr(evicted_met_parent.paddr);
                    PacketPtr pkt_write = new Packet(new_request_write, MemCmd::WriteReq);                    
                    writeCtrMtreeHelper(pkt_write);
                }
                evictBlock(blk, writebacks);
            }
        }
    }

    return true;    
}

    
void MetCache::readCtrMtreeHelper(PacketPtr pkt){
    memory::CtrMtreeEntry met_entry = memory::getCounterAddr(pkt->getAddr(),&HomeMetaData);
    while (met_entry.mtreeLevel != 0){ 
        // Access block in the tags
        Cycles tag_latency(0);
        CacheBlk *blk = tags->accessBlock(pkt, tag_latency);
        if (blk){
            // hit in cache, handle it normally
            pkt->setAddr(met_entry.paddr);
            recvTimingReq(pkt);
            break;
        }
        else {
            pkt->setAddr(met_entry.paddr);
            recvTimingReq(pkt);
        }
        met_entry = memory::getMtreeEntry(met_entry,&HomeMetaData);
    }
}

void MetCache::writeCtrMtreeHelper(PacketPtr pkt){
    memory::CtrMtreeEntry met_entry = memory::getCounterAddr(pkt->getAddr(),&HomeMetaData);
    // hit in cache, handle it normally
    pkt->setAddr(met_entry.paddr);
    recvTimingReq(pkt);    
}


bool
MetCache::SecMemCtrlSidePort::recvTimingReq(PacketPtr pkt)
{
    assert(pkt->isRequest());
    assert((pkt->isRead()||pkt->isWrite()));
    assert((cache->system->bypassCaches()==false));
//TODO
    if (tryTiming(pkt)) {
        if (pkt->isRead()){
            cache->readCtrMtreeHelper(pkt);
        }else 
        {
            cache->writeCtrMtreeHelper(pkt);   
        }
        return true;
    }
    return false;
}

Port &
MetCache::getPort(const std::string &if_name, PortID idx)
{
    if (if_name == "mem_side") {
        return memSidePort;
    } else if (if_name == "sec_mem_ctrl_side") {
        return secMemCtrlSidePort;
    }  else {
        return ClockedObject::getPort(if_name, idx);
    }
}
} // namespace gem5
