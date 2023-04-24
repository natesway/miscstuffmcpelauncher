#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <netdb.h>
#include <mutex>

void (*mcpelauncher_preinithook)(const char*sym, void*val, void **orig);
void (*mcpelauncher_log)(int level, const char* tag, const char* text);

void*_ZNK11AppPlatform12isLANAllowedEv;
void*__ZNK11AppPlatform12isLANAllowedEv;
int defaultBp = 8;
int bp = defaultBp;
int patchId = 0;
bool lastWasIp = false;

int isValidIp4 (char *str) {
    int segs = 0;   /* Segment count. */
    int chcnt = 0;  /* Character count within segment. */
    int accum = 0;  /* Accumulator for segment. */
    /* Catch NULL pointer. */
    if (str == NULL)
        return 0;
    /* Process every character in string. */
    while (*str != '\0') {
        /* Segment changeover. */
        if (*str == '.') {
            /* Must have some digits in segment. */
            if (chcnt == 0)
                return 0;
            /* Limit number of segments. */
            if (++segs == 4)
                return 0;
            /* Reset segment values and restart loop. */
            chcnt = accum = 0;
            str++;
            continue;
        }
        /* Check numeric. */
        if ((*str < '0') || (*str > '9'))
            return 0;
        /* Accumulate and check segment. */
        if ((accum = accum * 10 + *str - '0') > 255)
            return 0;
        /* Advance other segment specific stuff and continue loop. */
        chcnt++;
        str++;
    }
    /* Check enough segments and enough characters in last segment. */
    if (segs != 3)
        return 0;

    if (chcnt == 0)
        return 0;
    /* Address okay. */
    return 1;
}
extern "C" void __attribute__ ((visibility ("default"))) mod_preinit() {
    auto h = dlopen("libmcpelauncher_mod.so", 0);

    mcpelauncher_preinithook = (decltype(mcpelauncher_preinithook)) dlsym(h, "mcpelauncher_preinithook");
    mcpelauncher_log = (decltype(mcpelauncher_log)) dlsym(h, "mcpelauncher_log");

    mcpelauncher_preinithook("getaddrinfo", (void*)+[](const char *node, const char *service, const addrinfo *hints, addrinfo **res) -> int {
        if (node) {
            std::string server = "";
            if (lastWasIp) {
                if (strcmp(node, "geo.hivebedrock.network") == 0) {
                    patchId++;
                    std::thread([=]() {
                        int patchIdOld = patchId;
                        bp = 1;
                        sleep(3);
                        if (patchIdOld == patchId) {
                            bp = defaultBp;
                        }
                    }).detach();
                }
                if ((isValidIp4((char*)node) == 0) && (std::string(node).find(":") == std::string::npos)) {
                    lastWasIp = false;
                }
                if (strcmp(node, "play.inpvp.net") == 0) {
                    server = "Mineville";
                } else if (strcmp(node, "mco.lbsg.net") == 0) {
                    server = "Lifeboat Network";
                } else if (strcmp(node, "mco.mineplex.com") == 0) {
                    server = "Mineplex";
                } else if (strcmp(node, "mco.cubecraft.net") == 0) {
                    server = "Cubecraft Games";
                } else if (strcmp(node, "geo.hivebedrock.network") == 0) { //} else if (strcmp(node, "localhost") == 0) {
                    server = "The Hive";
                } else if (strcmp(node, "play.galaxite.net") == 0) {
                    server = "Galaxite";
                } else if (strcmp(node, "play.pixelparadise.gg") == 0) {
                    server = "Pixel Paradise";
                }

            } else {
                if (strcmp(node, "play.inpvp.net") == 0) {
                    server = "Server / realm / friend's world";
                } else if (strcmp(node, "mco.lbsg.net") == 0) {
                    server = "Server / realm / friend's world";
                } else if (strcmp(node, "mco.mineplex.com") == 0) {
                    server = "Server / realm / friend's world";
                } else if (strcmp(node, "mco.cubecraft.net") == 0) {
                    server = "Server / realm / friend's world";
                } else if (strcmp(node, "geo.hivebedrock.network") == 0) {
                    server = "Server / realm / friend's world";
                } else if (strcmp(node, "play.galaxite.net") == 0) {
                    server = "Server / realm / friend's world";
                } else if (strcmp(node, "play.pixelparadise.gg") == 0) {
                    server = "Server / realm / friend's world";
                }
            }
            if (strcmp(node, "play.inpvp.net") == 0) {
                bp = defaultBp;
            } else if (strcmp(node, "mco.lbsg.net") == 0) {
                bp = defaultBp;
            } else if (strcmp(node, "mco.mineplex.com") == 0) {
                bp = defaultBp;
            } else if (strcmp(node, "mco.cubecraft.net") == 0) {
                bp = defaultBp;
            } else if (strcmp(node, "play.galaxite.net") == 0) {
                bp = defaultBp;
            } else if (strcmp(node, "play.pixelparadise.gg") == 0) {
                bp = defaultBp;
            }
            if ((isValidIp4((char*)node) != 0) || std::string(node).find(":") != std::string::npos) {
                lastWasIp = true;
            }
            if (!server.empty()) {
                mcpelauncher_log(0, "ServerConnection", server.c_str());
            }
        }
        return getaddrinfo(node, service, hints, res);
    }, nullptr);

    mcpelauncher_preinithook("pthread_mutex_lock", (void*)+[](pthread_mutex_t *mutex) -> int {
        int ret = pthread_mutex_lock(mutex);
        if (ret != 0) {
            mcpelauncher_log(4, "Pthread", "mutex lock failed");
        }
        return 0;
    }, nullptr);

    __ZNK11AppPlatform12isLANAllowedEv = (void*)+[](void*** t) -> bool {
        auto mc = dlopen("libminecraftpe.so", 0);
        
        auto appPlat = (void**)dlsym(mc, "_ZTV11AppPlatform");
        auto raw = &appPlat[2];
        auto othervt = *t;
        auto _ZNK11AppPlatform19supportsFilePickingEv = (void**)dlsym(mc, "_ZNK11AppPlatform19supportsFilePickingEv");
        auto _ZNK11AppPlatform17supportsScriptingEv = (void**)dlsym(mc, "_ZNK11AppPlatform17supportsScriptingEv");
        auto _ZNK11AppPlatform25getPlatformUIScalingRulesEv = (void**)dlsym(mc, "_ZNK11AppPlatform25getPlatformUIScalingRulesEv");
        auto _ZNK11AppPlatform23getMaxSimRadiusInChunksEv = (void**)dlsym(mc, "_ZNK11AppPlatform23getMaxSimRadiusInChunksEv");
        auto _ZNK11AppPlatform10getEditionEv = (void**)dlsym(mc, "_ZNK11AppPlatform10getEditionEv");
        auto _ZNK11AppPlatform27getDefaultNetworkMaxPlayersEv = (void**)dlsym(mc, "_ZNK11AppPlatform27getDefaultNetworkMaxPlayersEv");
        auto _ZNK11AppPlatform16getBuildPlatformEv = (void**)dlsym(mc, "_ZNK11AppPlatform16getBuildPlatformEv");
        auto _ZN11AppPlatform19setKeepScreenOnFlagEb = (void**)dlsym(mc, "_ZN11AppPlatform19setKeepScreenOnFlagEb");

        for(int i = 0; raw[i] && raw[i] != (void*)0xffffffffffffffe8; i++) {
            if(raw[i] == _ZNK11AppPlatform19supportsFilePickingEv) {
                othervt[i] = (void*) +[](void*t) -> bool {
                    return true;
                };
            }
            if(raw[i] == _ZNK11AppPlatform17supportsScriptingEv) {
                othervt[i] = (void*) +[](void*t) -> bool {
                    return true;
                };
            }
            if(raw[i] == _ZNK11AppPlatform25getPlatformUIScalingRulesEv) {
                othervt[i] = (void*) +[](void*t) -> int {
                    return 0;
                };
            }
            if(raw[i] == _ZNK11AppPlatform23getMaxSimRadiusInChunksEv) {
                othervt[i] = (void*) +[](void*t) -> int {
                    return 12;
                };
            }
            if(raw[i] == _ZNK11AppPlatform10getEditionEv) {
                othervt[i] = (void*) +[](void*t) -> std::string {
                    return "win10";
                };
            }
            if(raw[i] == _ZNK11AppPlatform27getDefaultNetworkMaxPlayersEv) {
                othervt[i] = (void*) +[](void*t) -> int {
                    return 8;
                };
            }
            if(raw[i] == _ZNK11AppPlatform16getBuildPlatformEv) {
                othervt[i] = (void*) +[](void*t) -> int {
                    return bp;
                };
            }
            if(raw[i] == _ZN11AppPlatform19setKeepScreenOnFlagEb) {
                othervt[i] = (void*) +[](void*t, bool on) {
                    if (on) {
                        defaultBp = 7;
                        if (bp != 1) {
                            bp = defaultBp;
                        }
                    } else {
                        defaultBp = 8;
                        if (bp != 1) {
                            bp = defaultBp;
                        }
                    }
                    mcpelauncher_log(0, "MainActivity", (on ? "setKeepScreenOnFlag: 1" : "setKeepScreenOnFlag: 0"));
                };
            }
            if(othervt[i] == __ZNK11AppPlatform12isLANAllowedEv) {
                othervt[i] = _ZNK11AppPlatform12isLANAllowedEv;
            }
        }
        
        dlclose(mc);
        return true;
    };
    mcpelauncher_preinithook("_ZNK11AppPlatform12isLANAllowedEv", __ZNK11AppPlatform12isLANAllowedEv, &_ZNK11AppPlatform12isLANAllowedEv);
    dlclose(h);
}

extern "C" __attribute__ ((visibility ("default"))) void mod_init() {
}
