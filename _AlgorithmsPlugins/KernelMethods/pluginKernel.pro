# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_KernelMethods
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)

###########################
# Source Files            #
###########################
FORMS += paramsSVM.ui paramsSVMcluster.ui paramsSVMregr.ui paramsSVMdynamic.ui
HEADERS +=	\
			$$MLDEMOS/canvas.h \
			$$MLDEMOS/datasetManager.h \
			$$MLDEMOS/mymaths.h \
			svm.h \
			SOGP.h \
			SOGP_aux.h \
			classifierSVM.h \
			classifierRVM.h \
			classifierPegasos.h \
			clustererKKM.h \
			clustererSVR.h \
			regressorSVR.h \
			regressorGPR.h \
			regressorRVM.h \
			regressorKRLS.h \
			dynamicalSVR.h \
			dynamicalGPR.h \
			interfaceSVMClassifier.h \
			interfaceSVMCluster.h \
			interfaceSVMRegress.h \
			interfaceSVMDynamic.h \
			pluginKernel.h

SOURCES += 	\
			$$MLDEMOS/canvas.cpp \
			$$MLDEMOS/datasetManager.cpp \
			$$MLDEMOS/mymaths.cpp \
			svm.cpp \
			SOGP.cpp \
			SOGP_aux.cpp \
			classifierSVM.cpp \
			classifierRVM.cpp \
			classifierPegasos.cpp \
			clustererKKM.cpp \
			clustererSVR.cpp \
			regressorSVR.cpp \
			regressorGPR.cpp \
			regressorRVM.cpp \
			regressorKRLS.cpp \
			dynamicalSVR.cpp \
			dynamicalGPR.cpp \
			interfaceSVMClassifier.cpp \
			interfaceSVMCluster.cpp \
			interfaceSVMRegress.cpp \
			interfaceSVMDynamic.cpp \
			pluginKernel.cpp

###########################
# Dependencies            #
###########################
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
SOURCES += \
	newmat11/bandmat.cpp \
	newmat11/cholesky.cpp \
	newmat11/evalue.cpp \
	newmat11/fft.cpp \
	newmat11/hholder.cpp \
	newmat11/jacobi.cpp \
	newmat11/myexcept.cpp \
	newmat11/newfft.cpp \
	newmat11/newmat1.cpp \
	newmat11/newmat2.cpp \
	newmat11/newmat3.cpp \
	newmat11/newmat4.cpp \
	newmat11/newmat5.cpp \
	newmat11/newmat6.cpp \
	newmat11/newmat7.cpp \
	newmat11/newmat8.cpp \
	newmat11/newmat9.cpp \
	newmat11/newmatex.cpp \
	newmat11/newmatnl.cpp \
	newmat11/newmatrm.cpp \
	newmat11/nm_misc.cpp \
	newmat11/solution.cpp \
	newmat11/sort.cpp \
	newmat11/submat.cpp \
	newmat11/svd.cpp
