/*
 * main_bank.cpp
 *
 *  Created on: Feb 7, 2017
 *      Author: rigazilla
 */
#include <infinispan/hotrod/BasicTypesProtoStreamMarshaller.h>
#include <infinispan/hotrod/ProtoStreamMarshaller.h>
#include "infinispan/hotrod/ConfigurationBuilder.h"
#include "infinispan/hotrod/RemoteCacheManager.h"
#include "infinispan/hotrod/RemoteCache.h"
#include "infinispan/hotrod/Query.h"
#include "infinispan/hotrod/QueryUtils.h"

#include "bank.pb.h"

#include <iostream>


#define PROTOBUF_METADATA_CACHE_NAME "___protobuf_metadata"
#define ERRORS_KEY_SUFFIX  ".errors"

using namespace infinispan::hotrod;
using namespace ::google::protobuf;
using namespace ::org::infinispan::query::remote::client;

std::string read(std::string file)
{
    std::ifstream t(file);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}


int main(int argc, char *argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  // Connect.
  ConfigurationBuilder builder;
  builder.addServer().host("127.0.0.1").port(11222);
  builder.protocolVersion(Configuration::PROTOCOL_VERSION_24);
  RemoteCacheManager cacheManager(builder.build());

  //server side setup:
  //    installing protobuf model for cache entries
  auto *km = new BasicTypesProtoStreamMarshaller<std::string>();
  auto *vm = new BasicTypesProtoStreamMarshaller<std::string>();

  RemoteCache<std::string, std::string> metadataCache = cacheManager.getCache<std::string, std::string>(
          km, &Marshaller<std::string>::destroy, vm, &Marshaller<std::string>::destroy,PROTOBUF_METADATA_CACHE_NAME, false);

  std::string s=read("/home/rigazilla/git/jboss-jdg-quickstarts/remote-query/src/main/cpp/src/bank.proto");
  metadataCache.put("sample_bank_account/bank.proto", s);
  if (metadataCache.containsKey(ERRORS_KEY_SUFFIX))
  {
    std::cerr << "fail: error in registering .proto model" << std::endl;
    return -1;
  }








  auto *testkm = new BasicTypesProtoStreamMarshaller<int>();
  auto *testvm = new ProtoStreamMarshaller<sample_bank_account::User>();


  auto cache = cacheManager.getCache<int, sample_bank_account::User>(testkm, &Marshaller<int>::destroy, testvm, &Marshaller<sample_bank_account::User>::destroy, false);
  cacheManager.start();
//  sample_bank_account::User_Address a;
  sample_bank_account::User user1;
  user1.set_id(3);
  user1.set_name("Tom");
  user1.set_surname("Cat");
//  user1.set_gender(sample_bank_account::User_Gender_MALE);
//  sample_bank_account::User_Address * addr= user1.add_addresses();
//  addr->set_street("Via Roma");
//  addr->set_number(3);
//  addr->set_postcode("202020");
  cache.put(3, user1);
  int result;
  QueryRequest qr;
  // set_jpqlstring will be soon deprecated use set_querystring
  // qr.set_jpqlstring("from sample_bank_account.User");
  qr.set_querystring("from sample_bank_account.User");
  QueryResponse resp = cache.query(qr);

  std::vector<sample_bank_account::User> res;
  if (!unwrapResults(resp, res)) {
      std::cerr << "fail: found unexpected projection in resultset"
              << std::endl;
      result = -1;
      return result;
  }

  if (res.size() != 2) {
      std::cerr << "fail: expected 2 got " << res.size() << std::endl;
      result = -1;
      return result;
  }

  for (unsigned int i = 0; i < res.size(); i++) {
      std::cout << "User(id=" << res[i].id() << ",name=" << res[i].name()
              << ",surname=" << res[i].surname() << ")" << std::endl;
  }

  cache.clear();
}



