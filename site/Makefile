all: grpc

# Input paths
PYPLUGIN := $(shell which grpc_python_plugin)
PROTOPATH = ../src/main/proto

# List all input files here - no paths
PROTOFILES = lucida_service.proto

# Output path
PYMODULEPATH = ./lucidaservice

# List all output files here - no paths
PYGRPCFILES = lucida_service_pb2.py

$(addprefix $(PYMODULEPATH), $(addprefix /, $(PYGRPCFILES))): $(addprefix $(PYPROTOPATH), $(addprefix /, $(PYPROTOFILES)))
	mkdir -p $(PYMODULEPATH)
	protoc "--python_out=$(PYMODULEPATH)" "-I$(PROTOPATH)" "--plugin=protoc-gen-grpc=$(PYPLUGIN)" "--grpc_out=$(PYMODULEPATH)" $(addprefix $(PROTOPATH)/, $(PROTOFILES))
	echo "# Auto-generated from Makefile">"$(PYMODULEPATH)/__init__.py"
	for i in "$(basename $(PYGRPCFILES))";do echo "from $$i import *">>"$(PYMODULEPATH)/__init__.py";done 

grpc: $(addprefix $(PYMODULEPATH), $(addprefix /, $(PYGRPCFILES)))

clean:
	rm -rf $(PYMODULEPATH)

start_server:
	@echo "=========================================================="
	@echo "Attempting to start server in virtual console (gnu screen)"
	@echo "  man screen for help"
	@echo "  use 'ctrl-A d' to detach from the console"
	@echo "  use 'screen -r commandcenter' to reattach"
	@echo "  use 'screen -list to show all running consoles"
	@echo ""
	@echo "  pid will be saved at ./run/server.pid"
	@echo "  starting detached..."
	./start_server.sh
	
stop_server:
	@echo "=========================================================="
	@echo "Running stop_server.sh"
	@echo "  this should close the commandcenter console if it exists"
	@echo "  run 'screen -list' to show all running consoles"
	@screen -d -m -S commandcenter ../scripts/start_server.sh
	./stop_server.sh

.PHONY:	all grpc clean start_server stop_server
