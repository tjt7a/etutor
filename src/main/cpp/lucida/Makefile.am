noinst_LTLIBRARIES=liblucida.la

AUTOMAKE_OPTIONS=subdir-objects
PROTOPATH=../../proto

# List all input files here - no paths
PROTOFILES=lucida_service.proto

GENERATED_PATH=./generated

GENERATED_FILE_PATHS= \
	./generated/lucida_service.pb.h \
	./generated/lucida_service.grpc.pb.h \
	./generated/lucida_service.pb.cc \
	./generated/lucida_service.grpc.pb.cc

GENERATED_INC_PATHS= \
	$(top_srcdir)/include/lucida/generated/lucida_service.pb.h \
	$(top_srcdir)/include/lucida/generated/lucida_service.grpc.pb.h

BUILT_SOURCES=$(GENERATED_FILE_PATHS) \
				$(GENERATED_INC_PATHS)
CLEANFILES=$(GENERATED_FILE_PATHS) \
			$(GENERATED_INC_PATHS)

PROTO_FILE_PATHS= $(addprefix $(PROTOPATH), $(addprefix /, $(PROTOFILES)))
$(GENERATED_FILE_PATHS): $(PROTO_FILE_PATHS)
	mkdir -p $(GENERATED_PATH)
	protoc "--cpp_out=$(GENERATED_PATH)" "-I$(PROTOPATH)" "--plugin=protoc-gen-grpc=$(CPP_PLUGIN)" "--grpc_out=$(GENERATED_PATH)" $(PROTO_FILE_PATHS)

$(GENERATED_INC_PATHS): $(GENERATED_FILE_PATHS)
	cp $(GENERATED_PATH)/*.h $(top_srcdir)/include/lucida/generated

.PHONY: list-generated

list-generated:
	@echo "------- Generated file paths -------"
	@echo "$(GENERATED_FILE_PATHS)"
	@echo "------ Generated include paths -----"
	@echo "$(GENERATED_INC_PATHS)"
	@echo "------------------------------------"

nodist_liblucida_la_SOURCES = $(GENERATED_FILE_PATHS)
liblucida_la_SOURCES = \
	utils/path_ops.cpp \
	request_builder.cpp \
	service_names.cpp \
	service_acceptor.cpp \
	service_connector.cpp

liblucida_la_CPPFLAGS = -I$(top_srcdir)/include

# Always static linkage
liblucida_la_LDFLAGS = -static 

