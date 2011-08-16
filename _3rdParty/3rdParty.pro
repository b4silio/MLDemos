###########################
# Configuration           #
###########################
TEMPLATE = lib
NAME = 3rdParty
MLPATH =..
CONFIG += mainApp static

include($$MLPATH/MLDemos_variables.pri)

win32{
	DESTDIR = ../_3rdParty
}
###########################
# Files                   #
###########################
HEADERS +=	\
	fgmm/em.h \
	fgmm/fgmm++.hpp \
	fgmm/fgmm.h \
	fgmm/gaussian.h \
	fgmm/regression.h \
	fgmm/smat.h

SOURCES += \
	fgmm/em.cpp \
	fgmm/gaussian.cpp \
	fgmm/gmm.cpp \
	fgmm/gmmregression.cpp \
	fgmm/smat.cpp \
	fgmm/update.cpp

HEADERS +=	\
	MathLib/Differentiator.h \
	MathLib/GradientDescent.h \
	MathLib/IKGroupSolver.h \
	MathLib/IKSubSolver.h \
	MathLib/Macros.h \
	MathLib/MathLib.h \
	MathLib/MathLibCommon.h \
	MathLib/Matrix.h \
	MathLib/Matrix3.h \
	MathLib/Matrix4.h \
	MathLib/ReferenceFrame.h \
	MathLib/Regression.h \
	MathLib/SpatialForce.h \
	MathLib/SpatialFrame.h \
	MathLib/SpatialInertia.h \
	MathLib/SpatialMatrix.h \
	MathLib/SpatialVector.h \
	MathLib/SpatialVelocity.h \
	MathLib/TMatrix.h \
	MathLib/TVector.h \
	MathLib/Vector.h \
	MathLib/Vector3.h

SOURCES += \
	MathLib/Differentiator.cpp \
	MathLib/GradientDescent.cpp \
	MathLib/IKGroupSolver.cpp \
	MathLib/IKSubSolver.cpp \
	MathLib/Macros.cpp \
	MathLib/MathLib.cpp \
	MathLib/MathLibCommon.cpp \
	MathLib/Matrix.cpp \
	MathLib/Matrix3.cpp \
	MathLib/Matrix4.cpp \
	MathLib/ReferenceFrame.cpp \
	MathLib/Regression.cpp \
	MathLib/SpatialForce.cpp \
	MathLib/SpatialFrame.cpp \
	MathLib/SpatialInertia.cpp \
	MathLib/SpatialMatrix.cpp \
	MathLib/SpatialVector.cpp \
	MathLib/SpatialVelocity.cpp \
	MathLib/TMatrix.cpp \
	MathLib/TVector.cpp \
	MathLib/Vector.cpp \
	MathLib/Vector3.cpp

HEADERS += \
	PSO/pso.h \
	PSO/memoryAllocation.h \
	PSO/optimizer.h

SOURCES += \
	PSO/pso.cpp \
	PSO/memoryAllocation.cpp \
	PSO/optimizer.cpp

HEADERS +=	\
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

HEADERS += \
	lwpr/lwpr.h \
	lwpr/lwpr.hh \
	lwpr/lwpr_aux.h \
	lwpr/lwpr_binio.h \
	lwpr/lwpr_config.h \
	lwpr/lwpr_config_def.h \
	lwpr/lwpr_math.h \
	lwpr/lwpr_mem.h \
	lwpr/lwpr_xml.h

SOURCES +=  \
	lwpr/lwpr.c \
	lwpr/lwpr_aux.c \
	lwpr/lwpr_binio.c \
	lwpr/lwpr_math.c \
	lwpr/lwpr_mem.c \
	lwpr/lwpr_xml.c

HEADERS += 	JnS/Matutil.h \
			JnS/JnS.h
SOURCES += 	JnS/Matutil.cpp \
			JnS/JnS.cpp
HEADERS += 	liblinear/linear.h \
			liblinear/tron.h \
			liblinear/blasp.h \
			liblinear/blas.h
SOURCES += 	liblinear/linear.cpp \
			liblinear/tron.cpp \
			liblinear/daxpy.c \
			liblinear/ddot.c \
			liblinear/dnrm2.c \
			liblinear/dscal.c

HEADERS += ANN/ANN.h \
	ANN/ANNperf.h \
	ANN/ANNx.h \
	ANN/bd_tree.h \
	ANN/kd_fix_rad_search.h \
	ANN/kd_pr_search.h \
	ANN/kd_search.h \
	ANN/kd_split.h \
	ANN/kd_tree.h \
	ANN/kd_util.h \
	ANN/pr_queue.h \
	ANN/pr_queue_k.h

SOURCES += ANN/ANN.cpp \
	ANN/bd_fix_rad_search.cpp \
	ANN/bd_pr_search.cpp \
	ANN/bd_search.cpp \
	ANN/bd_tree.cpp \
	ANN/brute.cpp \
	ANN/kd_dump.cpp \
	ANN/kd_fix_rad_search.cpp \
	ANN/kd_pr_search.cpp \
	ANN/kd_search.cpp \
	ANN/kd_split.cpp \
	ANN/kd_tree.cpp \
	ANN/kd_util.cpp \
	ANN/perf.cpp

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

HEADERS +=	\
	nlopt.hpp \
	nlopt/tools.h \
	nlopt/stogo_config.h \
	nlopt/stogo.h \
	nlopt/soboldata.h \
	nlopt/slsqp.h \
	nlopt/redblack.h \
	nlopt/praxis.h \
	nlopt/nlopt-util.h \
	nlopt/nlopt-internal.h \
	nlopt/nlopt-in.hpp \
	nlopt/nlopt_optimize_usage.h \
	nlopt/nlopt.hpp \
	nlopt/nlopt.h \
	nlopt/newuoa.h \
	nlopt/neldermead.h \
	nlopt/mma.h \
	nlopt/mlsl.h \
	nlopt/luksan.h \
	nlopt/local.h \
	nlopt/linalg.h \
	nlopt/isres.h \
	nlopt/global.h \
	nlopt/f77funcs_.h \
	nlopt/f77funcs.h \
	nlopt/direct-internal.h \
	nlopt/direct.h \
	nlopt/crs.h \
	nlopt/cobyla.h \
	nlopt/cdirect.h \
	nlopt/bobyqa.h \
	nlopt/auglag.h \
	nlopt/config.h

SOURCES += 	\
	nlopt/tools.cc \
	nlopt/timer.c \
	nlopt/stop.c \
	nlopt/stogo.cc \
	nlopt/sobolseq.c \
	nlopt/slsqp.c \
	nlopt/sbplx.c \
	nlopt/rescale.c \
	nlopt/redblack.c \
	nlopt/qsort_r.c \
	nlopt/pssubs.c \
	nlopt/praxis.c \
	nlopt/pnet.c \
	nlopt/plis.c \
	nlopt/plip.c \
	nlopt/options.c \
	nlopt/optimize.c \
	nlopt/nldrmd.c \
	nlopt/newuoa.c \
	nlopt/mt19937ar.c \
	nlopt/mssubs.c \
	nlopt/mma.c \
	nlopt/mlsl.c \
	nlopt/local.cc \
	nlopt/linalg.cc \
	nlopt/isres.c \
	nlopt/hybrid.c \
	nlopt/global.cc \
	nlopt/general.c \
	nlopt/f77api.c \
	nlopt/DIRsubrout.c \
	nlopt/DIRserial.c \
	nlopt/direct_wrap.c \
	nlopt/DIRect.c \
	nlopt/deprecated.c \
	nlopt/crs.c \
	nlopt/cobyla.c \
	nlopt/cdirect.c \
	nlopt/bobyqa.c \
	nlopt/auglag.c

HEADERS += \
	LAMP_HMM/discreteObsProb.h \
	LAMP_HMM/explicitDurationTrans.h \
	LAMP_HMM/gammaProb.h \
	LAMP_HMM/gaussianObsProb.h \
	LAMP_HMM/hmm.h \
	LAMP_HMM/initStateProb.h \
	LAMP_HMM/obs.h \
	LAMP_HMM/obsProb.h \
	LAMP_HMM/obsSeq.h \
	LAMP_HMM/plainStateTrans.h \
	LAMP_HMM/stateTrans.h \
	LAMP_HMM/utils.h \
	LAMP_HMM/vectorObsProb.h

SOURCES += \
	LAMP_HMM/checkDurationDist.cpp \
	LAMP_HMM/discreteObsProb.cpp \
	LAMP_HMM/explicitDurationTrans.cpp \
	LAMP_HMM/gammaProb.cpp \
	LAMP_HMM/gaussianObsProb.cpp \
	LAMP_HMM/hmm.cpp \
	LAMP_HMM/hmmFind.cpp \
	LAMP_HMM/initStateProb.cpp \
	LAMP_HMM/obsSeq.cpp \
	LAMP_HMM/plainStateTrans.cpp \
	LAMP_HMM/readConfigFile.cpp \
	LAMP_HMM/utils.cpp \
	LAMP_HMM/vectorObsProb.cpp



#unix{
#	BOOST = /usr/local/boost_1_47_0
#}else{
#	BOOST = E:/DEV/boost_1_47_0
#}
#INCLUDEPATH += $$BOOST

#HEADERS += \
#	HMMlib/allocator_traits.hpp \
#	HMMlib/float_traits.hpp \
#	HMMlib/hmm_matrix.hpp \
#	HMMlib/hmm_table.hpp \
#	HMMlib/hmm_vector.hpp \
#	HMMlib/hmm.hpp \
#	HMMlib/operator_traits.hpp \
#	HMMlib/sse_operator_traits.hpp

#SOURCES += \
#	HMMlib/hmm_matrix_test.cpp \
#	HMMlib/hmm_test.cpp \
#	HMMlib/hmm_vector_test.cpp
