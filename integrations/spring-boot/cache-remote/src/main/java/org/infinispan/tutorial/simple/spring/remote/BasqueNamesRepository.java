package org.infinispan.tutorial.simple.spring.remote;

import java.lang.invoke.MethodHandles;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.logging.Logger;

import org.springframework.cache.annotation.CacheConfig;
import org.springframework.cache.annotation.CacheEvict;
import org.springframework.cache.annotation.Cacheable;
import org.springframework.stereotype.Component;

@Component
@CacheConfig(cacheNames = Data.BASQUE_NAMES_CACHE)
public class BasqueNamesRepository {

   private static final Logger logger = Logger.getLogger(MethodHandles.lookup().lookupClass().getName());
   private Map<String, BasqueName> database = new HashMap<>();

   @Cacheable()
   public BasqueName findById(String id) {
      logger.info("Call database to FIND name by id '" + id + "'");
      return database.get(id);
   }

   public void create(String id, String name) {
      logger.info("Call database to CREATE name by id '" + id + "'");
      database.put(id, new BasqueName(id, name));
   }

   @CacheEvict
   public void removeById(String id) {
      logger.info("Call database to REMOVE name by id '" + id + "'");
      database.remove(id);
   }

   public int size() {
      return database.size();
   }

}
