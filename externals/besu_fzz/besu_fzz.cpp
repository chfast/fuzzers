#include <besu_fzz.hpp>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <jni.h>

namespace {
char* besu_classpath =
    "-Djava.class.path="
    "/home/chfast/proj/fuzzers/externals/besu_fzz:"
    "/home/chfast/proj/besu/build/install/besu/lib/Java-WebSocket-1.5.5.jar:/"
    "home/chfast/proj/besu/build/install/besu/lib/ST4-4.3.4.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/abi-4.11.1.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/animal-sniffer-annotations-1.23.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/annotations-13.0.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/annotations-4.1.1.4.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/antlr-runtime-3.5.3.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/antlr4-4.11.1.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/antlr4-runtime-4.11.1.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/arithmetic-0.9.3.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/asm-9.2.jar:/home/chfast/proj/besu/build/install/besu/"
    "lib/asm-analysis-9.2.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "asm-commons-9.2.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "asm-tree-9.2.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "asm-util-9.2.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "bcpkix-jdk18on-1.77.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "bcprov-jdk18on-1.77.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "bcutil-jdk18on-1.77.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "besu-24.7-develop-e57c811.jar:/home/chfast/proj/besu/build/install/besu/"
    "lib/besu-api-24.7-develop-e57c811.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/besu-blockcreation-24.7-develop-e57c811.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/besu-clique-24.7-develop-e57c811.jar:/"
    "home/chfast/proj/besu/build/install/besu/lib/"
    "besu-config-24.7-develop-e57c811.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/besu-consensus-common-24.7-develop-e57c811.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/besu-core-24.7-develop-e57c811.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/"
    "besu-crypto-24.7-develop-e57c811.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/besu-crypto-services-24.7-develop-e57c811.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/besu-datatypes-24.7-develop-e57c811.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/"
    "besu-eth-24.7-develop-e57c811.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/besu-ethereum-ethstats-24.7-develop-e57c811.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/"
    "besu-ethereum-rlp-24.7-develop-e57c811.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/besu-ethereum-stratum-24.7-develop-e57c811.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/"
    "besu-evm-24.7-develop-e57c811.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/besu-evmtool-24.7-develop-e57c811.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/besu-ibft-24.7-develop-e57c811.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/besu-kvstore-24.7-develop-e57c811.jar:/"
    "home/chfast/proj/besu/build/install/besu/lib/"
    "besu-merge-24.7-develop-e57c811.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/besu-metrics-core-24.7-develop-e57c811.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/"
    "besu-metrics-rocksdb-24.7-develop-e57c811.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/besu-nat-24.7-develop-e57c811.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/besu-p2p-24.7-develop-e57c811.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/"
    "besu-permissioning-24.7-develop-e57c811.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/besu-pipeline-24.7-develop-e57c811.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/besu-pki-24.7-develop-e57c811.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/"
    "besu-plugin-rocksdb-24.7-develop-e57c811.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/besu-qbft-24.7-develop-e57c811.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/besu-retesteth-24.7-develop-e57c811.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/"
    "besu-tasks-24.7-develop-e57c811.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/besu-trie-24.7-develop-e57c811.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/besu-util-24.7-develop-e57c811.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/blake2bf-0.9.3.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/bls12-381-0.9.3.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/caffeine-3.1.8.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/checker-qual-3.41.0.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/client-java-18.0.1.jar:/home/chfast/proj/besu/build/install/besu/"
    "lib/client-java-api-18.0.1.jar:/home/chfast/proj/besu/build/install/besu/"
    "lib/client-java-proto-18.0.1.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/commons-codec-1.16.0.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/commons-collections4-4.4.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/commons-compress-1.26.0.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/commons-io-2.15.1.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/commons-lang3-3.14.0.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/commons-net-3.11.0.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/core-4.11.1.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/core-support-test-24.7-develop-e57c811-test-support.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/crypto-4.11.1.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/dagger-2.50.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/discovery-22.12.0.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/dnsjava-3.5.3.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/enclave-24.7-develop-e57c811.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/encoder-1.2.3.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/ens-normalize-0.1.2.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/error_prone_annotations-2.23.0.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/failureaccess-1.0.2.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/framework-1.3.2.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/framework-internal-1.3.2.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/gnark-0.9.3.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/graphql-java-21.3.jar:/home/chfast/proj/besu/build/install/besu/"
    "lib/grpc-api-1.60.1.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "grpc-context-1.60.1.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "grpc-core-1.60.1.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "grpc-netty-1.60.1.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "grpc-util-1.60.1.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "gson-2.10.1.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "gson-fire-1.8.5.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "guava-33.0.0-jre.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "icu4j-71.1.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "jackson-annotations-2.16.1.jar:/home/chfast/proj/besu/build/install/besu/"
    "lib/jackson-core-2.16.1.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "jackson-databind-2.16.1.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "jackson-datatype-jdk8-2.16.1.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/jansi-2.4.1.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "java-dataloader-3.2.1.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "javax.annotation-api-1.3.2.jar:/home/chfast/proj/besu/build/install/besu/"
    "lib/javax.inject-1.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "javax.json-1.1.4.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "jc-kzg-4844-1.0.0.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "jffi-1.3.10-native.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "jffi-1.3.10.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "jna-5.14.0.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "jna-platform-5.14.0.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "jnr-a64asm-1.0.0.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "jnr-constants-0.10.3.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "jnr-enxio-0.32.13.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "jnr-ffi-2.2.13.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "jnr-posix-3.1.15.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "jnr-unixsocket-0.38.17.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "jnr-x86asm-1.0.2.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "jose4j-0.9.3.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "jsr305-3.0.2.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "kotlin-stdlib-1.9.22.jar:/home/chfast/proj/besu/build/install/besu/lib/"
    "kotlin-stdlib-jdk7-1.9.10.jar:/home/chfast/proj/besu/build/install/besu/"
    "lib/kotlin-stdlib-jdk8-1.9.10.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/kotlinx-coroutines-core-jvm-1.6.4.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/"
    "listenablefuture-9999.0-empty-to-avoid-conflict-with-guava.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/log4j-api-2.22.1.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/log4j-core-2.22.1.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/log4j-jul-2.22.1.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/log4j-slf4j2-impl-2.22.1.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/logging-interceptor-4.10.0.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/maven-artifact-3.9.6.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/netty-all-4.1.110.Final.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/netty-buffer-4.1.110.Final.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/netty-codec-4.1.110.Final.jar:/"
    "home/chfast/proj/besu/build/install/besu/lib/"
    "netty-codec-dns-4.1.110.Final.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/netty-codec-haproxy-4.1.110.Final.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/netty-codec-http-4.1.110.Final.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/netty-codec-http2-4.1.110.Final.jar:/"
    "home/chfast/proj/besu/build/install/besu/lib/"
    "netty-codec-memcache-4.1.110.Final.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/netty-codec-mqtt-4.1.110.Final.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/netty-codec-redis-4.1.110.Final.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/"
    "netty-codec-smtp-4.1.110.Final.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/netty-codec-socks-4.1.110.Final.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/netty-codec-stomp-4.1.110.Final.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/netty-codec-xml-4.1.110.Final.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/netty-common-4.1.110.Final.jar:/"
    "home/chfast/proj/besu/build/install/besu/lib/"
    "netty-handler-4.1.110.Final.jar:/home/chfast/proj/besu/build/install/besu/"
    "lib/netty-handler-proxy-4.1.110.Final.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/netty-handler-ssl-ocsp-4.1.110.Final.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/netty-resolver-4.1.110.Final.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/"
    "netty-resolver-dns-4.1.110.Final.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/netty-resolver-dns-classes-macos-4.1.110.Final.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/"
    "netty-resolver-dns-native-macos-4.1.110.Final-osx-aarch_64.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/"
    "netty-resolver-dns-native-macos-4.1.110.Final-osx-x86_64.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/"
    "netty-tcnative-boringssl-static-2.0.62.Final.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/netty-tcnative-classes-2.0.62.Final.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/"
    "netty-transport-4.1.110.Final.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/netty-transport-classes-epoll-4.1.110.Final.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/"
    "netty-transport-classes-kqueue-4.1.110.Final.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/"
    "netty-transport-native-epoll-4.1.110.Final-linux-aarch_64.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/"
    "netty-transport-native-epoll-4.1.110.Final-linux-riscv64.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/"
    "netty-transport-native-epoll-4.1.110.Final-linux-x86_64.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/"
    "netty-transport-native-epoll-4.1.110.Final.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/"
    "netty-transport-native-kqueue-4.1.110.Final-osx-aarch_64.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/"
    "netty-transport-native-kqueue-4.1.110.Final-osx-x86_64.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/"
    "netty-transport-native-kqueue-4.1.110.Final.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/"
    "netty-transport-native-unix-common-4.1.110.Final.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/netty-transport-rxtx-4.1.110.Final.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/"
    "netty-transport-sctp-4.1.110.Final.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/netty-transport-udt-4.1.110.Final.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/okhttp-4.12.0.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/okio-jvm-3.6.0.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/opentelemetry-api-1.33.0.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/opentelemetry-api-events-1.33.0-alpha.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/opentelemetry-context-1.33.0.jar:/"
    "home/chfast/proj/besu/build/install/besu/lib/"
    "opentelemetry-exporter-common-1.33.0.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/opentelemetry-exporter-otlp-1.33.0.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/"
    "opentelemetry-exporter-otlp-common-1.33.0.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/opentelemetry-exporter-sender-okhttp-1.33.0.jar:/"
    "home/chfast/proj/besu/build/install/besu/lib/"
    "opentelemetry-extension-incubator-1.33.0-alpha.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/"
    "opentelemetry-extension-trace-propagators-1.33.0.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/opentelemetry-sdk-1.33.0.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/opentelemetry-sdk-common-1.33.0.jar:/"
    "home/chfast/proj/besu/build/install/besu/lib/"
    "opentelemetry-sdk-extension-autoconfigure-1.33.0.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/"
    "opentelemetry-sdk-extension-autoconfigure-spi-1.33.0.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/opentelemetry-sdk-logs-1.33.0.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/"
    "opentelemetry-sdk-metrics-1.33.0.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/opentelemetry-sdk-trace-1.33.0.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/opentelemetry-semconv-1.23.1-alpha.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/org.abego.treelayout.core-1.0.3.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/org.jupnp-2.7.1.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/org.jupnp.support-2.7.1.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/oshi-core-6.4.10.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/perfmark-api-0.26.0.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/picocli-4.7.5.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/plexus-utils-3.5.1.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/plugin-api-24.7-develop-e57c811.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/protobuf-java-3.22.0.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/reactive-streams-1.0.4.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/reactor-core-3.4.24.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/referencetests-24.7-develop-e57c811.jar:/"
    "home/chfast/proj/besu/build/install/besu/lib/rlp-4.11.1.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/rocksdbjni-8.3.2.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/rxjava-2.2.21.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/secp256k1-0.9.3.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/secp256r1-0.9.3.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/simpleclient-0.16.0.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/simpleclient_common-0.16.0.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/simpleclient_guava-0.16.0.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/simpleclient_hotspot-0.16.0.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/simpleclient_httpserver-0.15.0.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/"
    "simpleclient_pushgateway-0.16.0.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/simpleclient_tracer_common-0.16.0.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/simpleclient_tracer_otel-0.16.0.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/"
    "simpleclient_tracer_otel_agent-0.16.0.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/slf4j-api-2.0.10.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/snakeyaml-2.0.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/snappy-java-1.1.10.5.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/splunk-library-javalogging-1.11.8.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/spring-security-crypto-6.2.1.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/swagger-annotations-1.6.9.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/tuples-4.11.1.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/tuweni-bytes-2.4.2.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/tuweni-concurrent-2.4.2.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/tuweni-concurrent-coroutines-2.4.2.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/tuweni-config-2.4.2.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/tuweni-crypto-2.4.2.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/tuweni-devp2p-2.4.2.jar:/home/"
    "chfast/proj/besu/build/install/besu/lib/tuweni-io-2.4.2.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/tuweni-kademlia-2.4.2.jar:/home/chfast/"
    "proj/besu/build/install/besu/lib/tuweni-net-2.4.2.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/tuweni-rlp-2.4.2.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/tuweni-toml-2.4.2.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/tuweni-units-2.4.2.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/utils-4.11.1.jar:/home/chfast/proj/besu/build/install/"
    "besu/lib/value-annotations-2.10.0.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/vertx-auth-common-4.5.8.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/vertx-auth-jwt-4.5.8.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/vertx-bridge-common-4.5.8.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/vertx-codegen-4.5.8.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/vertx-core-4.5.8.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/vertx-lang-kotlin-4.3.7.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/vertx-lang-kotlin-coroutines-4.3.7.jar:/home/chfast/proj/"
    "besu/build/install/besu/lib/vertx-unit-4.5.8.jar:/home/chfast/proj/besu/"
    "build/install/besu/lib/vertx-web-4.5.8.jar:/home/chfast/proj/besu/build/"
    "install/besu/lib/vertx-web-common-4.5.8.jar";

struct JVM {
  JNIEnv* env = nullptr;
  JavaVM* jvm = nullptr;
  jclass fzz_class = nullptr;
  jmethodID validateEOF = nullptr;

  JVM() {
    JavaVMOption options[] = {
        {.optionString = besu_classpath},
        // {.optionString = "-Xcheck:jni"},

        /* This disables Java's signal handler,
         * so that aborting the fuzzer (ctrl+c) won't
         * cause a crash.
         */
        {
            .optionString = (char*)"-Xrs",
        },
        {const_cast<char*>("-Xmx512m")}, // max heap size
    };
    JavaVMInitArgs vm_args{
        .version = JNI_VERSION_21,
        .nOptions = std::size(options),
        .options = options,
    };

    if (JNI_CreateJavaVM(&jvm, reinterpret_cast<void**>(&env), &vm_args) !=
        JNI_OK)
      std::abort();

    fzz_class = env->FindClass("BesuFzz");
    if (fzz_class == nullptr) {
      const auto ex = env->ExceptionOccurred();
      if (ex != nullptr) {
        env->ExceptionDescribe();
        env->ExceptionClear();
      }
      std::abort();
    }

    validateEOF = env->GetStaticMethodID(fzz_class, "validateEOF", "([BI)Z");
  }

  ~JVM() { jvm->DestroyJavaVM(); }
};

JVM jvm;
} // namespace

bool fzz_besu_validate_eof(const uint8_t* data, size_t size) noexcept {
  // TODO: Besu recognizes legacy by prefix, so don't pass non-EOF code.
  if (size < 2 || data[0] != 0xEF || data[1] != 0x00)
    return false;

  const auto env = jvm.env;
  const auto length = static_cast<jsize>(size);
  const auto byteArr = env->NewByteArray(length);
  env->SetByteArrayRegion(byteArr, 0, length,
                          reinterpret_cast<const jbyte*>(data));
  const auto res = env->CallStaticBooleanMethod(jvm.fzz_class, jvm.validateEOF,
                                                byteArr, length);
  if (env->ExceptionOccurred() != nullptr) {
    env->ExceptionDescribe();
    env->ExceptionClear();
    std::abort();
  }
  env->DeleteLocalRef(byteArr);
  return res;
}
