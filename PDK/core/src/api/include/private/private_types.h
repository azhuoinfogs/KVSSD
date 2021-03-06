/**
 *   BSD LICENSE
 *
 *   Copyright (c) 2018 Samsung Electronics Co., Ltd.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Co., Ltd. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */



#ifndef INCLUDE_PRIVATE_PRIVATE_TYPES_H_
#define INCLUDE_PRIVATE_PRIVATE_TYPES_H_

#include <cstddef>
#include <cstdint>
#include <string>
#include <mutex>
#include <list>
#include <condition_variable>
#include "kvs_api.h"
#include "kvs_adi.h"

#ifdef __cplusplus
extern "C" {
#endif

  
class kv_device_priv {
public:
  kv_device_priv() {
    nsid = 0;
    numanode = 0;
    iskv = 0;
    vendorid = 0;
    deviceid = 0;
    isopened = false;
    isattached = false;
    isspdkdev = false;
    isemul = false;
    num_opened_qpairs = 0;
  }

  ~kv_device_priv();
  char                node[1024];
  char                spdkpath[1024];
  int                 nsid;
  char                trid[1024];
  char                pci_slot_name[1024];
  int                 numanode;
  int                 iskv;
  int                 vendorid;
  int                 deviceid;
  char                ven_dev_id[128];
  bool                isopened;
  bool 		    isattached;
  bool		    isspdkdev;
  bool		    isemul;
  bool                iskerneldev;
  int 		    num_opened_qpairs;
};

/*
 * Low level Device APIs
 *
 * A simple wrapper for KV SSD functions
 */
  /*
class KvsDeviceImpl {
    _on_iocomplete io_complete;
public:

  KvsDeviceImpl():
	  io_complete(0), num_threads(0), persistent(false)
  {}

  virtual ~KvsDeviceImpl() { }

  bool is_persistent() { return persistent; }
  int32_t make_persistent() { this->persistent = true; return KVS_SUCCESS; }

  // for emulator debugging
  virtual void dump() {}
  virtual bool init() { return false; }

  // emulator persistency
  virtual bool load(const char *datapath) { return true; }
  virtual bool set_dumppath(const char *datapath) { return true; }

  // kv operations
  virtual int32_t store_tuple(int contid, const kvs_key *key, kvs_value *value, uint8_t option) = 0;
  virtual int32_t retrieve_tuple(int contid, const kvs_key *key, kvs_value *value, uint8_t option) = 0;
  virtual int32_t retrieve_tuples(int contid, int num_tuples, kvs_key **keys, kvs_value **values, uint8_t option, int *results) { return KVS_ERR_OPTION_INVALID; }
  virtual int32_t delete_tuple(int contid, const kvs_key *key, uint8_t option) = 0;

  virtual int getfd() = 0;

protected:
  unsigned int num_threads;
  bool persistent;

};
  */

/*
 * KvsDevice represents a KV SSD
 *
 */
class KvsDriver {
public:
  kv_device_priv *dev;
  _on_iocomplete user_io_complete;
  std::list<kvs_container*> list_containers;
  std::list<kvs_container_handle> open_containers;

 public:
  KvsDriver(kv_device_priv *dev_,_on_iocomplete user_io_complete_):
	  dev(dev_), user_io_complete(user_io_complete_) {}

  virtual ~KvsDriver() {}

  bool load_data(const char *path);
  bool set_dumppath(const char *path);

  /**
   * Opens a device
   *
   * @param dev_path a path to a device. An emulator is used when a null path or '/dev/kvemul' is given.
   */
  int32_t open(const char *dev_path = 0, std::string dumppath = "auto", int dumpindex = 0);
  int32_t close();

  int32_t exit_env();

  void* operator new(std::size_t sz);
  void operator delete(void* p);
public:
  virtual int32_t init();
  virtual int32_t init(int socket) { return 0; }
  virtual int32_t init(const char* devpath, const char* configfile, int queuedepth, int is_polling) {return 0;}
  virtual int32_t init(const char* devpath, bool syncio, uint64_t sq_core, uint64_t cq_core, uint32_t mem_size_mb) {return 0;}
  virtual int32_t init(const char* devpath, bool syncio) {return 0;}
  virtual int32_t process_completions(int max) =0;
  virtual int32_t store_tuple(int contid, const kvs_key *key, const kvs_value *value, uint8_t option, void *private1=NULL, void *private2=NULL, bool sync = false) = 0;
  virtual int32_t retrieve_tuple(int contid, const kvs_key *key, kvs_value *value, uint8_t option, void *private1=NULL, void *private2=NULL, bool sync = false) = 0;
  virtual int32_t delete_tuple(int contid, const kvs_key *key, uint8_t option, void *private1=NULL, void *private2=NULL, bool sync = false) = 0;
  virtual int32_t open_iterator(int contid,  uint8_t option, uint32_t bitmask, uint32_t bit_pattern, kvs_iterator_handle *iter_hd) = 0;
  virtual int32_t close_iterator(int contid, kvs_iterator_handle hiter) = 0;
  virtual int32_t iterator_next(kvs_iterator_handle hiter, kvs_iterator_list *iter_list, void *private1=NULL, void *private2=NULL, bool sync = false) = 0;
  virtual float get_waf() {return 0.0;}
  virtual int32_t get_used_size() {return 0.0;}
  virtual int64_t get_total_size(){return 0;};
  std::string path;
};

struct _kvs_device_handle {
  kv_device_priv * dev;
  KvsDriver* driver;
};

struct _kvs_container_handle {
  kvs_device_handle dev;
  char name[256];
};

struct _kvs_iterator_handle{
  kv_iterator_handle iterh_adi;
  uint32_t iterator;
};

  
class ContainHandle {
 public:
  kvs_device_handle dev;
  char container_name[256];
  
 public:  
  ContainHandle(kvs_device_handle _dev):
    dev(_dev)
    {}

    ~ContainHandle();
  
};
  
#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INCLUDE_PRIVATE_PRIVATE_TYPES_H_ */
