package org.infinispan;

import java.io.IOException;
import java.io.InputStream;
import java.security.cert.X509Certificate;
import java.util.Properties;

import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

import org.infinispan.client.hotrod.RemoteCache;
import org.infinispan.client.hotrod.RemoteCacheManager;
import org.infinispan.client.hotrod.configuration.ClientIntelligence;
import org.infinispan.client.hotrod.configuration.Configuration;
import org.infinispan.client.hotrod.configuration.ConfigurationBuilder;
import org.infinispan.client.hotrod.configuration.SaslQop;
import org.infinispan.client.hotrod.impl.ConfigurationProperties;

import gnu.getopt.Getopt;

/**
 * Infinispan+operator+hotrod client quickstart
 *
 */
public class App {
    static String host = "127.0.0.1";
    static int port = ConfigurationProperties.DEFAULT_HOTROD_PORT;
    static String username, password;
    static String truststorePath;
    static String truststorePass;

    public static void main(String[] args) {
        // Accept untrusted certificates for this tutorial
        SSLContext ctx = initSSLSocketFactory();
        // getopt and configure static variables
        configure(args);
        // Create a configuration for a locally-running server

        Configuration configuration;

        { // Configuration with encryption
            InputStream is = App.class.getClassLoader().getResourceAsStream("hotrod.properties");
            Properties prop = new Properties();
            try {
                prop.load(is);
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                System.exit(1);
            }
            configuration = new ConfigurationBuilder().addServer().host(host).port(port)
                    .clientIntelligence(ClientIntelligence.BASIC).security().authentication().enable()
                    .username(username).password(password).realm("default").serverName("infinispan")
                    .saslMechanism("DIGEST-MD5").saslQop(SaslQop.AUTH).ssl().sniHostName(host).sslContext(ctx).enable()
                    .build();
        }
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

    static void configure(String args[]) {
        Getopt g = new Getopt("testprog", args, "h:p:U:P:t:k:T");
        //
        int c;
        String arg;
        while ((c = g.getopt()) != -1) {
            switch (c) {
            case 'h':
                arg = g.getOptarg();
                host = arg;
                break;
            case 'p':
                arg = g.getOptarg();
                port = Integer.parseInt(arg);
                break;
            case 'U':
                arg = g.getOptarg();
                username = arg;
                break;
            case 'P':
                arg = g.getOptarg();
                password = arg;
                break;
            case 't':
                arg = g.getOptarg();
                truststorePath = arg;
                break;
            case 'k':
                arg = g.getOptarg();
                truststorePass = arg;
                break;
            case '?':
            default:
                break;
            }
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
