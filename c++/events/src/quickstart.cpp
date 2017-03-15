#include <cstring>
#include <infinispan/hotrod/JBasicMarshaller.h>
#include "infinispan/hotrod/ConfigurationBuilder.h"
#include "infinispan/hotrod/RemoteCacheManager.h"
#include "infinispan/hotrod/RemoteCache.h"
#include "infinispan/hotrod/CacheClientListener.h"

#include <iostream>
#include <chrono>

using namespace infinispan::hotrod;

std::string read(std::string file) {
    std::ifstream t(file);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

template<class T> T read_valid(std::string prompt) {
    T result;
    while (true) {
        std::cout << prompt;
        if (!(std::cin >> result)) {
            std::string str;
            std::cin.clear();
            std::cin >> str;
            std::cerr << "Invalid input \"" << str << "\". Try again." << std::endl;
        } else {
            return result;
        }
    }
}

static std::string APP_MENU = "\nAvailable actions:\n"
        "0. Display available actions\n"
        "1. Add purchase\n"
        "2. Add listener\n"
        "3. Remove listener\n"
        "4. Add filtered listener\n"
        "5. Remove filtered listener\n"
        "6. Add custom listener\n"
        "7. Remove custom listener\n"
        "10. Quit\n";

void displayActions() {
    std::cout << APP_MENU << std::endl;
}

int readAction() {
    return read_valid<int>("> ");
}

void addPurchase(RemoteCache<std::string, std::string> &cache) {
    std::cout << ("Enter product: ");
    std::string product;
    std::cin.ignore();
    std::getline(std::cin, product);
    std::cout << ("Enter purchase details: ");
    std::string details;
    std::getline(std::cin, details);
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
    cache.put(product + ":" + std::to_string(ms.count()), details, 30, TimeUnit::SECONDS);
}

std::unique_ptr<CacheClientListener<std::string, std::string> > clCustomPtr, clPtr, clFilteredPtr;

void addCustomListener(Marshaller<std::string>* marshaller, RemoteCache<std::string, std::string> &cache) {
    if (clCustomPtr) {
        std::cout << "Already installed" << std::endl;
        return;
    }
    auto cl = new CacheClientListener<std::string, std::string>(cache);
    clCustomPtr.reset(cl);
    std::function<void(ClientCacheEntryCreatedEvent<std::string>)> listenerCreated =
            [](ClientCacheEntryCreatedEvent<std::string> e) {std::cout << "Custom listener: "<< e.getKey() << std::endl;};
    std::function<void(ClientCacheEntryCustomEvent)> listenerCustom = [] (ClientCacheEntryCustomEvent e)
    {
        std::string str= JBasicMarshallerHelper::unmarshall<std::string>(e.getEventData().data());
        std::cout << str << std::endl;
    };
    clCustomPtr->add_listener(listenerCreated);
    clCustomPtr->add_listener(listenerCustom);
    char fName[] = "string-is-equal-filter-factory";
    char cName[] = "to-string-converter-factory";
    clCustomPtr->filterFactoryName = std::vector<char>(fName, fName + std::strlen(fName));
    clCustomPtr->converterFactoryName = std::vector<char>(cName, cName + std::strlen(cName));
    std::vector<std::vector<char> > filterFactoryParams;
    std::vector<std::vector<char> > converterFactoryParams;

    std::string strArgs("hiking");
    std::vector<char> param;
    marshaller->marshall(strArgs, param);
    filterFactoryParams.push_back(param);
    cache.addClientListener(*clCustomPtr, filterFactoryParams, converterFactoryParams);
}

void addListener(Marshaller<std::string>* marshaller, RemoteCache<std::string, std::string> &cache) {
    if (clPtr) {
        std::cout << "Already installed" << std::endl;
        return;
    }
    auto cl = new CacheClientListener<std::string, std::string>(cache);
    clPtr.reset(cl);
    std::function<void(ClientCacheEntryCreatedEvent<std::string>)> listenerCreated =
            [](ClientCacheEntryCreatedEvent<std::string> e) {std::cout << "New entry: " << e.getKey() << std::endl;};
    std::function<void(ClientCacheEntryExpiredEvent<std::string>)> listenerExpired =
            [](ClientCacheEntryExpiredEvent<std::string> e) {std::cout << "Expired entry: " << e.getKey() << std::endl;};
    clPtr->add_listener(listenerCreated);
    clPtr->add_listener(listenerExpired);
    std::vector<std::vector<char> > filterFactoryParams;
    std::vector<std::vector<char> > converterFactoryParams;
    cache.addClientListener(*clPtr, filterFactoryParams, converterFactoryParams);
}

void addFilteredListener(Marshaller<std::string>* marshaller, RemoteCache<std::string, std::string> &cache) {
    if (clFilteredPtr) {
        std::cout << "Already installed" << std::endl;
        return;
    }
    auto cl = new CacheClientListener<std::string, std::string>(cache);
    clFilteredPtr.reset(cl);
    std::function<void(ClientCacheEntryCreatedEvent<std::string>)> listenerCreated =
            [](ClientCacheEntryCreatedEvent<std::string> e) {std::cout << "Filtered New entry: " << e.getKey() << std::endl;};
    std::function<void(ClientCacheEntryExpiredEvent<std::string>)> listenerExpired =
            [](ClientCacheEntryExpiredEvent<std::string> e) {std::cout << "Filtered Expired entry: " << e.getKey() << std::endl;};
    clFilteredPtr->add_listener(listenerCreated);
    clFilteredPtr->add_listener(listenerExpired);

    char fName[] = "string-is-equal-filter-factory";
    clFilteredPtr->filterFactoryName = std::vector<char>(fName, fName + std::strlen(fName));
    std::vector<std::vector<char> > filterFactoryParams;
    std::vector<std::vector<char> > converterFactoryParams;
    std::string strArgs("hiking");
    std::vector<char> param;
    marshaller->marshall(strArgs, param);
    filterFactoryParams.push_back(param);

    cache.addClientListener(*clFilteredPtr, filterFactoryParams, converterFactoryParams);
}

void removeListener(RemoteCache<std::string, std::string> &cache) {
    if (!clPtr) {
        std::cout << "Not installed" << std::endl;
        return;
    }
    cache.removeClientListener(*clPtr);
    clPtr.release();
}

void removeFilteredListener(RemoteCache<std::string, std::string> &cache) {
    if (!clFilteredPtr) {
        std::cout << "Not installed" << std::endl;
        return;
    }
    cache.removeClientListener(*clFilteredPtr);
    clCustomPtr.release();
}

void removeCustomListener(RemoteCache<std::string, std::string> &cache) {
    if (!clCustomPtr) {
        std::cout << "Not installed" << std::endl;
        return;
    }
    cache.removeClientListener(*clCustomPtr);
    clCustomPtr.release();
}

int main(int argc, char *argv[]) {

    ConfigurationBuilder builder;
    builder.addServer().host("127.0.0.1").port(11222);
    builder.protocolVersion(Configuration::PROTOCOL_VERSION_24);
    RemoteCacheManager cacheManager(builder.build());

    auto *testkm = new JBasicMarshaller<std::string>();
    auto *testvm = new JBasicMarshaller<std::string>();

    auto cache = cacheManager.getCache<std::string, std::string>(testkm, &Marshaller<std::string>::destroy, testvm,
            &Marshaller<std::string>::destroy, false);
    cacheManager.start();

    bool quit = false;
    displayActions();
    while (!quit) {
        int action = readAction();

        switch (action) {
        case 1:
            addPurchase(cache);
            break;
        case 2:
            addListener(testkm, cache);
            break;
        case 3:
            removeListener(cache);
            break;
        case 4:
            addFilteredListener(testkm, cache);
            break;
        case 5:
            removeFilteredListener(cache);
            break;
        case 6:
            addCustomListener(testkm, cache);
            break;
        case 7:
            removeCustomListener(cache);
            break;
        case 10:
            quit = true;
            std::cout << "Bye!" << std::endl;
            cacheManager.stop();
            break;
        default:
            std::cerr << "Invalid action: " << action << std::endl;
        case 0:
            displayActions();
        }
    }
}

