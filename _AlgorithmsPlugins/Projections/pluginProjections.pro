# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_Projections
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
###########################
# Source Files            #
###########################
FORMS += \
    paramsProjections.ui \
    paramsICA.ui \
    paramsLDA.ui \
    paramsKPCA.ui \
    paramsSammon.ui \
    paramsPCA.ui \
    contourPlots.ui
HEADERS +=	\
    basicOpenCV.h \
    canvas.h \
    datasetManager.h \
    mymaths.h \
    eigen_pca.h \
    classifierLinear.h \
    classifierKPCA.h \
    interfaceProjections.h \
    pluginProjections.h \
    interfaceICAProjection.h \
    interfaceLDAProjection.h \
    interfacePCAProjection.h \
    interfaceKPCAProjection.h \
    interfaceSammonProjection.h \
    projectorPCA.h \
    projectorKPCA.h \
    projectorICA.h \
    projectorSammon.h \
    projectorLDA.h

SOURCES += 	\
    basicOpenCV.cpp \
    eigen_pca_kernel.cpp \
    classifierLinear.cpp \
    classifierKPCA.cpp \
    interfaceProjections.cpp \
    pluginProjections.cpp \
    interfaceICAProjection.cpp \
    interfaceLDAProjection.cpp \
    interfacePCAProjection.cpp \
    interfaceKPCAProjection.cpp \
    interfaceSammonProjection.cpp \
    projectorPCA.cpp \
    projectorKPCA.cpp \
    projectorICA.cpp \
    projectorSammon.cpp \
    projectorLDA.cpp

#INCLUDEPATH += /opt/local/include
#LIBS += -llapack -lblas
HEADERS +=  isomap/fibheap.h \
            isomap/isomap.h
SOURCES +=  isomap/dijkstra.cpp \
            isomap/isomap.cpp

###########################
# Dependencies            #
###########################

HEADERS +=  JnS/Matutil.h \
            JnS/JnS.h

HEADERS +=  liblinear/linear.h \
            liblinear/tron.h \
            liblinear/blasp.h \
            liblinear/blas.h

HEADERS +=	\
    dlib/algs.h \
    dlib/array.h \
    dlib/array2d.h \
    dlib/assert.h \
    dlib/base64.h \
    dlib/bayes_utils.h \
    dlib/bigint.h \
    dlib/binary_search_tree.h \
    dlib/bit_stream.h \
    dlib/bound_function_pointer.h \
    dlib/byte_orderer.h \
    dlib/cmd_line_parser.h \
    dlib/compress_stream.h \
    dlib/conditioning_class.h \
    dlib/config_reader.h \
    dlib/cpp_pretty_printer.h \
    dlib/cpp_tokenizer.h \
    dlib/crc32.h \
    dlib/data_io.h \
    dlib/dir_nav.h \
    dlib/directed_graph.h \
    dlib/enable_if.h \
    dlib/entropy_decoder.h \
    dlib/entropy_decoder_model.h \
    dlib/entropy_encoder.h \
    dlib/entropy_encoder_model.h \
    dlib/error.h \
    dlib/geometry.h \
    dlib/graph.h \
    dlib/graph_utils.h \
    dlib/gui_core.h \
    dlib/gui_widgets.h \
    dlib/hash_map.h \
    dlib/hash_set.h \
    dlib/hash_table.h \
    dlib/image_io.h \
    dlib/image_keypoint.h \
    dlib/image_transforms.h \
    dlib/is_kind.h \
    dlib/linker.h \
    dlib/logger.h \
    dlib/lz77_buffer.h \
    dlib/lzp_buffer.h \
    dlib/manifold_regularization.h \
    dlib/map.h \
    dlib/matrix.h \
    dlib/md5.h \
    dlib/member_function_pointer.h \
    dlib/memory_manager.h \
    dlib/memory_manager_global.h \
    dlib/memory_manager_stateless.h \
    dlib/misc_api.h \
    dlib/mlp.h \
    dlib/noncopyable.h \
    dlib/opencv.h \
    dlib/optimization.h \
    dlib/pipe.h \
    dlib/pixel.h \
    dlib/platform.h \
    dlib/quantum_computing.h \
    dlib/queue.h \
    dlib/rand.h \
    dlib/ref.h \
    dlib/reference_counter.h \
    dlib/revision.h \
    dlib/sequence.h \
    dlib/serialize.h \
    dlib/server.h \
    dlib/set.h \
    dlib/set_utils.h \
    dlib/sliding_buffer.h \
    dlib/smart_pointers.h \
    dlib/smart_pointers_thread_safe.h \
    dlib/sockets.h \
    dlib/sockstreambuf.h \
    dlib/sort.h \
    dlib/stack.h \
    dlib/stack_trace.h \
    dlib/static_map.h \
    dlib/static_set.h \
    dlib/statistics.h \
    dlib/std_allocator.h \
    dlib/stl_checked.h \
    dlib/string.h \
    dlib/svm.h \
    dlib/svm_threaded.h \
    dlib/sync_extension.h \
    dlib/threads.h \
    dlib/time_this.h \
    dlib/timeout.h \
    dlib/timer.h \
    dlib/tokenizer.h \
    dlib/tuple.h \
    dlib/type_safe_union.h \
    dlib/uintn.h \
    dlib/unicode.h \
    dlib/windows_magic.h \
    dlib/xml_parser.h
