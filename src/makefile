CXX = g++
STRIP = strip

CXX_W32 = i486-mingw32-g++
STRIP_W32 = i486-mingw32-strip

CXX_WINE = wineg++ -m32

BUILD_DIR = ../build

BS2B_CC = bs2b/bs2b.c bs2b/bs2bclass.cpp

SERVER = $(BUILD_DIR)/dspr-server
SERVER_CC = dspr_server.cc server.cc server_main.cc sigpipe.cc socket.cc processor.cc module_null.cc module_bs2b.cc $(BS2B_CC)

SERVER_W32 = $(BUILD_DIR)/dspr-server.exe
SERVER_W32_CC = dspr_server.cc server.cc server_main.cc sigpipe.cc socket.cc processor.cc module_null.cc module_bs2b.cc module_winamp.cc module_vst.cc $(BS2B_CC)

SERVER_WINE = $(BUILD_DIR)/dspr-server-wine.exe

LADSPA = $(BUILD_DIR)/dspr-ladspa.so
LADSPA_CC = ladspa.cc

CMD = $(BUILD_DIR)/dspr-cmd
CMD_CC = dspr_cmd.cc

CLIENTSO = $(BUILD_DIR)/libdspr-client.so
CLIENTSO_CC = dspr_client.cc dspr_client_c.cc sigpipe.cc socket.cc

JACKCLIENT = $(BUILD_DIR)/dspr-jack
JACKCLIENT_CC = dspr_jack.cc

SERVER_LAUNCHER = $(BUILD_DIR)/dspr-server-launcher
SERVER_LAUNCHER_CC = dspr_server_launcher.cc

SHARED_CFLAGS = -O3 -g0 -Wall -D_REENTRANT -Ibs2b -pipe

CFLAGS = $(SHARED_CFLAGS) -fPIC -fvisibility=hidden -lm -Wall
LDFLAGS = -lpthread -ldl
CLIENT_LDFLAGS = -ldspr-client -L.

CFLAGS_W32 = -DWINDOWS $(SHARED_CFLAGS) -lws2_32 -pie -m32

all: $(SERVER) $(SERVER_W32) $(SERVER_WINE) $(CLIENTSO) $(LADSPA) $(CMD) $(JACKCLIENT) $(SERVER_LAUNCHER)


$(SERVER): $(SERVER_CC)
	@echo "[LD] $@"
	@$(CXX) -o $@ $(SERVER_CC) $(CFLAGS) $(LDFLAGS)
	@$(STRIP) $@

$(SERVER_W32): $(SERVER_W32_CC)
	@echo "[LD] $@"
	@$(CXX_W32) -o $@ $(SERVER_W32_CC) $(CFLAGS_W32)
	@$(STRIP_W32) $@

$(SERVER_WINE): $(SERVER_W32_CC)
	@echo "[LD] $@"
	@$(CXX_WINE) -o $@ $(SERVER_W32_CC) $(SHARED_CFLAGS)
	@$(STRIP) $(SERVER_WINE).so

$(CLIENTSO): $(CLIENTSO_CC)
	@echo "[LD] $@"
	@$(CXX) -o $@ $(CLIENTSO_CC) $(CFLAGS) $(LDFLAGS) -shared
	@$(STRIP) $@

$(LADSPA): $(LADSPA_CC)
	@echo "[LD] $@"
	@$(CXX) -o $@ $(LADSPA_CC) $(CFLAGS) $(LDFLAGS) $(CLIENT_LDFLAGS) -shared
	@$(STRIP) $@

$(CMD): $(CMD_CC)
	@echo "[LD] $@"
	@$(CXX) -o $@ $(CMD_CC) $(CFLAGS) $(LDFLAGS) $(CLIENT_LDFLAGS)
	@$(STRIP) $@

$(JACKCLIENT): $(JACKCLIENT_CC)
	@echo "[LD] $@"
	@$(CXX) -o $@ $(JACKCLIENT_CC) $(CFLAGS) $(LDFLAGS) $(CLIENT_LDFLAGS) -ljack
	@$(STRIP) $@
	@sudo chown root $(JACKCLIENT)
	@sudo chmod u+s $(JACKCLIENT)

$(SERVER_LAUNCHER): ${SERVER_LAUNCHER_CC}
	@echo "[LD] $@"
	@$(CXX) -o $@ $(SERVER_LAUNCHER_CC) $(CFLAGS) $(LDFLAGS)
	@$(STRIP) $@
	@sudo chown root $(SERVER_LAUNCHER)
	@sudo chmod u+s $(SERVER_LAUNCHER)

clean:
	@rm -f $(SERVER) $(SERVER_W32) $(SERVER_WINE) $(SERVER_WINE).so $(LADSPA) $(CLIENTSO) $(CMD) $(JACKCLIENT) $(SERVER_LAUNCHER)

