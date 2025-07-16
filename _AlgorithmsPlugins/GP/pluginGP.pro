# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin

# you can change the name to suit the name of your plugin
NAME = mld_GP
MLPATH = ../..
include($$MLPATH/MLDemos_variables.pri)

# ##########################
# Source Files            #
# ##########################
# add here all the files in your project
# the forms contain the widgets for changing the hyperparameters
FORMS += paramsGPClassifier.ui \
    paramsGPR.ui \
    paramsGPRdynamic.ui \

HEADERS += canvas.h \
    datasetManager.h \
    mymaths.h \
    drawUtils.h \
    pluginGP.h \
    interfaceGPClassifier.h \
    classifierGP.h \
    SOGP_aux.h \
    SOGP.h \
    regressorGPR.h \
    interfaceGPRRegress.h \
    interfaceGPRDynamic.h \
    dynamicalGPR.h \
    SECovarianceFunction.h
SOURCES += pluginGP.cpp \
    interfaceGPClassifier.cpp \
    classifierGP.cpp \
    SOGP_aux.cpp \
    SOGP.cpp \
    regressorGPR.cpp \
    interfaceGPRRegress.cpp \
    interfaceGPRDynamic.cpp \
    dynamicalGPR.cpp \
    SECovarianceFunction.cpp

# ##########################
# Dependencies            #
# ##########################
HEADERS += dlib/algs.h \
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
    dlib/xml_parser.h \
    newmat11/controlw.h \
    newmat11/include.h \
    newmat11/myexcept.h \
    newmat11/newmat.h \
    newmat11/newmatap.h \
    newmat11/newmatio.h \
    newmat11/newmatnl.h \
    newmat11/newmatrc.h \
    newmat11/newmatrm.h \
    newmat11/precisio.h \
    newmat11/solution.h

OTHER_FILES += \
    plugin.json
