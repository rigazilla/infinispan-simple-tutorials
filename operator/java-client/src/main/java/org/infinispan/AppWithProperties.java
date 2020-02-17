package org.infinispan;

import java.io.IOException;
import java.io.InputStream;
import java.security.cert.X509Certificate;
import java.util.Properties;

import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.KeyManager;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

import org.infinispan.client.hotrod.RemoteCache;
import org.infinispan.client.hotrod.RemoteCacheManager;
import org.infinispan.client.hotrod.configuration.Configuration;
import org.infinispan.client.hotrod.configuration.ConfigurationBuilder;
import org.infinispan.client.hotrod.configuration.ClientIntelligence;
import org.infinispan.client.hotrod.configuration.SaslQop;

import org.infinispan.client.hotrod.impl.ConfigurationProperties;

import gnu.getopt.Getopt;

/**
 * Infinispan+operator+hotrod client quickstart
 *
 */
public class AppWithProperties {
    static String host = "127.0.0.1";
    static int port = ConfigurationProperties.DEFAULT_HOTROD_PORT;
    static String username, password;
    static String truststorePath;
    static String truststorePass;

    public static void main(String[] args) {
        Configuration configuration;

        InputStream is = AppWithProperties.class.getClassLoader().getResourceAsStream("hotrod.properties");
        Properties prop = new Properties();
        try {
            prop.load(is);
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            System.exit(1);
        }
        ConfigurationBuilder cb = new ConfigurationBuilder().withProperties(prop);

        // Accept untrusted certificates for this tutorial
        SSLContext ctx = initSSLSocketFactory();
        cb.security().ssl().sslContext(ctx);
        configuration = cb.build();

        // Connect to the server
        try (RemoteCacheManager cacheManager = new RemoteCacheManager(configuration)) {
            cacheManager.administration().createCache("quickstart-cache", "org.infinispan.DIST_SYNC");
            RemoteCache<String, String> cache = cacheManager.getCache("quickstart-cache");
            /// Store a value
            cache.put("key", "value");
            // Retrieve the value and print it out
            System.out.printf("key = %s\n", cache.get("key"));
            // Stop the cache manager and release all resources
            cacheManager.administration().removeCache("quickstart-cache");
        } catch (Exception ex) {
            // Maybe cache already exist? Go on anyway
        }
    }

    // Create a trust manager that does not validate certificate chains
    private static TrustManager[] _trustAllCerts = new TrustManager[] { new X509TrustManager() {
        public java.security.cert.X509Certificate[] getAcceptedIssuers() {
            return null;
        }

        public void checkClientTrusted(X509Certificate[] certs, String authType) {
        }

        public void checkServerTrusted(java.security.cert.X509Certificate[] certs, String authType) {
        }
    } };

    private static SSLContext initSSLSocketFactory() {

        SSLContext sc = null;
        try {
            sc = SSLContext.getInstance("SSL");
            sc.init(null, _trustAllCerts, new java.security.SecureRandom());
            HttpsURLConnection.setDefaultSSLSocketFactory(sc.getSocketFactory());
        } catch (Exception e) {
        }
        return sc;
    }
}
